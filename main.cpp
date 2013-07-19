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
#include "thrender/thrender.hpp"
#include "thrender/utils/mesh.hpp"
#include "thrender/utils/to_string.hpp"
#include "thrender/utils/frame_rate_keeper.hpp"


#define PI	3.141593
#define HPI	(PI / 2.0f)
#define PI2	(2 * PI)

typedef perf_timer<boost::chrono::high_resolution_clock> timer;
SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * tex_diffuse;
SDL_Texture * tex_depth;
SDL_Texture * tex_normals;

struct draw_pixel {
	thrender::gbuffer & gbuf;

	draw_pixel(thrender::gbuffer & _gbuf) :
			gbuf(_gbuf) {
	}

	inline bool operator()(int x, int y) {
		gbuf.diffuse[thrender::coord2d(x,y)] = glm::vec4(1, 0, 0, 1);

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
		size_t coords = thrender::coord2d(x,y);
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

	typedef thrender::vertex_array<thrust::tuple<
			glm::vec4,
			glm::vec4,
			glm::vec4> > mesh_type;
	mesh_type tux = thrender::utils::load_model<mesh_type>("/home/sque/Downloads/tux__.ply");

	//thrust::host_vector<thrender::triangle>::iterator it;
	thrender::camera cam(glm::vec3(0, 0, 10), 45, 4.0f / 3.0f, 5, 200);

	thrender::render_state rstate(cam, gbuff);
	thrender::utils::frame_rate_keeper<> lock_fps(10);
	profiler<boost::chrono::high_resolution_clock> prof("Render procedure");
	for (int i = 1; i < 150000; i++) {
		prof.reset();
		gbuff.clear();
		prof.checkpoint("Clear buffer");
		thrender::process_vertices< thrender::shaders::default_vertex_shader<mesh_type> >(tux, rstate);
		prof.checkpoint("Process vertices");
		thrender::process_fragments(tux, rstate);
		prof.checkpoint("Process fragments");
		upload_images(gbuff);
		prof.checkpoint("Upload images");

		//tux.model_mat = glm::rotate(tux.model_mat, 10.0f, glm::vec3(0, 1, 0));
		cam.view_mat = glm::rotate(cam.view_mat, 10.0f, glm::vec3(0,1,0));
		std::cout << prof.report() << std::endl;

		//lock_fps.keep_frame_rate();
	}

	std::ofstream myfile;
	myfile.open("pixels.txt");
/*	for (it = tux.render_buffer.triangles.begin(); it != tux.render_buffer.triangles.end(); it++) {
		thrender::triangle & f = *it;
		myfile << glm::to_string(f.bounding_box()) << std::endl;
	}
*/
	myfile.close();

	std::cout << "pixels written" << std::endl;
	while (1) ;
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
