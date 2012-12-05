/*
 * main.cpp
 *
 *  Created on: Nov 30, 2012
 *      Author: sque
 */
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/integer.hpp>
#include <glm/gtc/swizzle.hpp>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/iterator/constant_iterator.h>
#include <iostream>
#include <fstream>
#include "perf_timer.hpp"
#include <SDL/SDL.h>
#include <math.h>
#include "thrender/camera.hpp"
#include "thrender/vertex_processor.hpp"
#include "thrender/mesh.hpp"
#include "thrender/gbuffer.hpp"
#include "thrender/utils.hpp"
#include "thrender/math.hpp"
#include "thrender/triangle.hpp"

#define PI	3.141593
#define HPI	(PI / 2.0f)
#define PI2	(2 * PI)

typedef perf_timer<boost::chrono::thread_clock> timer;
SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * tex_diffuse;
SDL_Texture * tex_depth;
SDL_Texture * tex_normals;

#define coord2d(x__,y__) (((y__) * 640) + (x__))

struct face {
	glm::vec3 v[3];
	struct {
		bool discard;
	} flags;

	face(const glm::vec3 & v0, const glm::vec3 & v1, const glm::vec3 & v2,
			bool _disc) {
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		flags.discard = _disc || !ccw_winding_order();
	}
	face() {
	}

	bool ccw_winding_order() {
		float adiff = atan2f(v[1].y - v[0].y, v[1].x - v[0].x)
				- atan2f(v[2].y - v[0].y, v[2].x - v[0].x);
		if (adiff < 0)
			adiff = PI2 + adiff;
		if (adiff < PI) {
			return false;
		}
		return true;
	}
};


struct primitives_proc_kernel {

	const thrender::mesh & m;
	const thrust::host_vector<glm::vec4> & ws_vertices;
	thrender::render_state & rstate;

	primitives_proc_kernel(const thrust::host_vector<glm::vec4> & v,
			const thrender::mesh & _m, thrender::render_state & _rstate) :
			m(_m), ws_vertices(v), rstate(_rstate) {
	}

	thrender::triangle operator()(const glm::ivec3 & tr) {
		return thrender::triangle(m, ws_vertices, tr.x, tr.y, tr.z,
				rstate.discard_vertex[tr.x] || rstate.discard_vertex[tr.y]
						|| rstate.discard_vertex[tr.z]);
	}
};

// Extract primitives
thrust::host_vector<thrender::triangle> process_primitives(const thrender::mesh & m,
		const thrust::host_vector<glm::vec4> & proj_vertices,
		thrender::render_state & rstate) {
	thrust::host_vector<thrender::triangle> primitives(m.total_triangles());

	thrust::transform(m.triangles.begin(), m.triangles.end(),
			primitives.begin(),
			primitives_proc_kernel(proj_vertices, m, rstate));
	return primitives;
}

struct draw_pixel {
	thrender::gbuffer & gbuf;

	draw_pixel(thrender::gbuffer & _gbuf) :
			gbuf(_gbuf) {
	}

	inline bool operator()(int x, int y) {
		gbuf.diffuse[coord2d(x,y)] = glm::vec4(1, 0, 0, 1);

		return true;
	}
};

struct interpolate_draw_pixel {
	thrender::gbuffer & gbuf;

	const glm::vec4 & from, &to;
	float sqlength;
	float zdist;

	interpolate_draw_pixel(thrender::gbuffer & _gbuf, const glm::vec4 & _from,
			const glm::vec4 & _to) :
			gbuf(_gbuf), from(_from), to(_to), zdist(from.z - to.z) {
		//sqlength = glm::length(to-from);
		sqlength = pow(to.x - from.x, 2) + pow(to.y - from.y, 2);
	}

	inline bool operator()(int x, int y) {
		size_t coords = coord2d(x,y);
		float factor, z;
		if (!sqlength) {
			z = from.z;
		} else {
			float sqdist = pow(x - from.x, 2) + pow(y - from.y, 2);
			factor = sqdist / sqlength;
			z = (zdist) * factor + from.z;
		}
		if (gbuf.depth[coords] > z)	// Z-test
			return true;

		gbuf.depth[coords] = z;
		gbuf.diffuse[coords] = glm::vec4(1, 0, 0, 1);
		//gbuf.buf_normal[coords] =
		return true;
	}
};

struct edge {
	float slope;
	bool vertical;

	const glm::vec3 & p1;
	const glm::vec3 & p2;

	edge(const glm::vec3 & _p1, const glm::vec3 & _p2) :
			vertical(false), p1(_p1), p2(_p2) {

		if (p2.x - p1.x)
			slope = (p2.y - p1.y) / (p2.x - p1.x);
		else {

			vertical = false;
		}
	}

	bool check_point_leftof(int x, int y) {
		if ((vertical) || (y == p1.y))
			return x <= p1.x;
		if (y == p2.y)
			return x <= p2.x;
		return false;
	}
};

template<class PixelOperation>
struct walk_scan_line {

	const glm::vec3 ** points;

	PixelOperation & pixel_op;

	edge e01, e02, e21;

	walk_scan_line(PixelOperation _op, const glm::vec3 * _points[]) :
			points(_points), pixel_op(_op), e01(*points[0], *points[1]), e02(
					*points[0], *points[2]), e21(*points[2], *points[1]) {
	}

	bool operator()(int x, int y) {
		if (y <= points[2]->y) {
			for (int xw = x; e02.check_point_leftof(xw, y); xw++)
				pixel_op(xw, y);
		}
		return true;
	}
};

struct triangle_interpolate_draw_pixel {

	thrender::gbuffer & gbuf;
	const thrender::triangle & tri;

	triangle_interpolate_draw_pixel(thrender::gbuffer & _gbuf,
			const thrender::triangle & _tri) :
			gbuf(_gbuf), tri(_tri) {
		//sqlength = glm::length(to-from);
		//sqlength = pow(to.x-from.x,2) + pow(to.y-from.y,2);
	}

	inline bool operator()(int x, int y) {
		size_t coords = coord2d(x,y);
		float z;

		glm::vec4 lamdas = thrender::math::barycoords(tri.v[0], tri.v[1], tri.v[2], glm::vec2(x,y));

		z = lamdas.x * tri.v[0].z + lamdas.y * tri.v[1].z + lamdas.z * tri.v[2].z;
		glm::vec4 color =
				tri.m->attributes.colors[tri.indices[0]] * lamdas.x+
				tri.m->attributes.colors[tri.indices[1]] * lamdas.y+
				tri.m->attributes.colors[tri.indices[2]] * lamdas.z;
		glm::vec4 normal =
				tri.m->attributes.normals[tri.indices[0]] * lamdas.x+
				tri.m->attributes.normals[tri.indices[1]] * lamdas.y+
				tri.m->attributes.normals[tri.indices[2]] * lamdas.z;

		if (gbuf.depth[coords] > z)	// Z-test
			return true;

		gbuf.depth[coords] = z;
		gbuf.diffuse[coords] = color;
		gbuf.normal[coords] = tri.m->attributes.normals[tri.indices[0]];
		return true;
	}
};

//! Structure to hold (convex) polygon horizontal limits
struct poly_hor_limits {

	int leftmost[480];

	int rightmost[480];

	poly_hor_limits() {
		for (int i = 0; i < 480; i++) {
			leftmost[i] = 10000;
			rightmost[i] = 0;
		}
	}
};

//! Kernel to find poly vertical contour
struct mark_vert_contour {

	poly_hor_limits & limits;

	mark_vert_contour(poly_hor_limits & _limits) :
			limits(_limits) {
	}

	bool operator()(int x, int y) {
		if (x < limits.leftmost[y])
			limits.leftmost[y] = x;
		if (x > limits.rightmost[y])
			limits.rightmost[y] = x;
		return true;
	}
};

struct raster_kernel {

	thrender::gbuffer & gbuf;
	draw_pixel draw_pixel_op;

	raster_kernel(thrender::gbuffer & _gbuf) :
			gbuf(_gbuf), draw_pixel_op(gbuf) {
	}

	void operator()(const thrender::triangle & f) {
		if (f.flags.discard)
			return;

		// Sort points by y
		const glm::vec4 * pord[3] = { f.v, f.v + 1, f.v + 2 };
		for (int i = 0; i < 3; i++) {
			if (f.v[i].y > pord[0]->y) {
				pord[0] = f.v + i;
			} else if (f.v[i].y < pord[2]->y) {
				pord[2] = f.v + i;
			}
		}

		for (int i = 0; i < 3; i++)
			if ((pord[0] != f.v + i) && (pord[2] != f.v + i)) {
				pord[1] = f.v + i;
				break;
			}

		// Find triangle contour
		poly_hor_limits tri_contour;
		mark_vert_contour mark_contour_op(tri_contour);
		thrender::utils::line_bresenham(pord[0]->x, pord[0]->y, pord[1]->x,
				pord[1]->y, mark_contour_op);
		thrender::utils::line_bresenham(pord[1]->x, pord[1]->y, pord[2]->x,
				pord[2]->y, mark_contour_op);
		thrender::utils::line_bresenham(pord[0]->x, pord[0]->y, pord[2]->x,
				pord[2]->y, mark_contour_op);

		// Scan conversion fill
		triangle_interpolate_draw_pixel pix_op(gbuf, f);
		for (int y = pord[2]->y; y <= pord[0]->y; y++) {
			for (int x = tri_contour.leftmost[y]; x < tri_contour.rightmost[y];
					x++)
				pix_op(x, y);
		}
	}
};

// Rasterization
void raster_proc(const thrust::host_vector<thrender::triangle> & triangles, thrender::gbuffer & gbuf) {
	thrust::for_each(
			triangles.begin(),
			triangles.end(),
			raster_kernel(gbuf));
}

void upload_images(thrender::gbuffer & gbuf) {

	long src_index = 0;
	Uint32 *dst_dif, *dst_depth, *dst_normals;
	int row, col;
	void *pixels_dif;
	void *pixels_depth;
	void *pixels_normals;
	int pitch_dif, pitch_depth, pitch_normals;
	SDL_Rect dstrect;

	if (SDL_LockTexture(tex_diffuse, NULL, &pixels_dif, &pitch_dif) < 0) {
		std::cerr << "Couldn't lock texture:" << SDL_GetError() << std::endl;
		return;
	}
	if (SDL_LockTexture(tex_depth, NULL, &pixels_depth, &pitch_depth) < 0) {
		std::cerr << "Couldn't lock texture:" << SDL_GetError() << std::endl;
		return;
	}
	if (SDL_LockTexture(tex_normals, NULL, &pixels_normals, &pitch_normals) < 0) {
		std::cerr << "Couldn't lock texture:" << SDL_GetError() << std::endl;
		return;
	}

	for (row = 0; row < 480; ++row) {
		dst_dif = (Uint32*) ((Uint8*) pixels_dif + row * pitch_dif);
		dst_depth = (Uint32*) ((Uint8*) pixels_depth + row * pitch_depth);
		dst_normals = (Uint32*) ((Uint8*) pixels_normals + row * pitch_normals);

		for (col = 0; col < 640; ++col) {
			glm::vec4 color = gbuf.diffuse[src_index++] * 255.0f;
			glm::vec4 normal = gbuf.normal[src_index] * 255.0f;
			float d = gbuf.depth[src_index] * 255.0f;
			//*dst++ = (0xFF000000|(color.r << 16)|(color.g << 8) | color.b);
			*dst_dif++ = (0xFF000000 | ((int) color.r << 16)
					| ((int) color.g << 8) | (int) color.b);
			*dst_depth++ = (((int) d << 16) | ((int) d << 8) | (int) d);
			*dst_normals++ = (0xFF000000 | ((int) normal.r << 16)
								| ((int) normal.g << 8) | (int) normal.b);

		}
	}

	SDL_UnlockTexture(tex_normals);
	SDL_UnlockTexture(tex_depth);
	SDL_UnlockTexture(tex_diffuse);
	SDL_RenderClear(renderer);
	dstrect.h = 480;
	dstrect.w = 640;
	dstrect.x = 0;
	dstrect.y = 0;
	SDL_RenderCopy(renderer, tex_diffuse, NULL, &dstrect);
	dstrect.x = 640;
	SDL_RenderCopy(renderer, tex_depth, NULL, &dstrect);
	dstrect.y = 480;
	dstrect.x = 640;
	SDL_RenderCopy(renderer, tex_normals, NULL, &dstrect);
	SDL_RenderPresent(renderer);

}

void render() {

	timer tm;
	thrender::gbuffer gbuff(640, 480);
	gbuff.set_clear_diffuse(glm::vec4(0, 0, 0, 1));
	thrust::host_vector<thrender::triangle> triangles;
	thrender::render_state rstate;
	thrender::mesh tux = thrender::load_model("/home/sque/Downloads/tux__.ply");

	thrender::camera cam(glm::vec3(0, 0, 10), 45, 4.0f / 3.0f, 5, 200);

	for (int i = 1; i < 15000; i++) {
		tm.reset();
		gbuff.clear();
		triangles = process_primitives(tux,
				thrender::process_vertices(tux, cam, rstate), rstate);
		raster_proc(triangles, gbuff);
		timer::duration dt = tm.passed();
		std::cout << "Frame took " << dt << std::endl;
		upload_images(gbuff);

		tux.model_mat = glm::rotate(tux.model_mat, 10.0f, glm::vec3(0, 1, 0));
		cam.view_mat = glm::rotate(cam.view_mat, 10.0f, glm::vec3(0,1,0));
	}
	thrust::host_vector<thrender::triangle>::iterator it;

	std::ofstream myfile;
	myfile.open("pixels.txt");
	for (it = triangles.begin(); it != triangles.end(); it++) {
		thrender::triangle & f = *it;
		for (int i = 0; i < 3; i++) {
			myfile << f.v[i].x << "," << f.v[i].y << "," << f.v[i].z
					<< std::endl;
		}
	}
	myfile.close();
	while (1) {
	}
	std::cout << "pixels written" << std::endl;
}

int main() {
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
		std::cerr << "Unable to init SDL:" << SDL_GetError() << std::endl;
		return 1;
	}

	window = SDL_CreateWindow("Thrust Renderer", SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, 640 * 2, 480 * 2,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		std::cerr << "Couldn't set create renderer:" << SDL_GetError()
				<< std::endl;
	}

	tex_diffuse = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, 640, 480);
	if (!tex_diffuse) {
		std::cerr << "Couldn't set create texture:" << SDL_GetError()
				<< std::endl;
		return 5;
	}

	tex_depth = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
			SDL_TEXTUREACCESS_STREAMING, 640, 480);
	if (!tex_depth) {
		std::cerr << "Couldn't set create texture:" << SDL_GetError()
				<< std::endl;
		return 5;
	}

	tex_normals = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
				SDL_TEXTUREACCESS_STREAMING, 640, 480);
	if (!tex_normals) {
		std::cerr << "Couldn't set create texture:" << SDL_GetError()
				<< std::endl;
		return 5;
	}

	std::cout << "Scene loaded" << std::endl;
	render();
	std::cout << "Scene rendered" << std::endl;
	return 0;
}
