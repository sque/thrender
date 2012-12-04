/*
 * main.cpp
 *
 *  Created on: Nov 30, 2012
 *      Author: sque
 */
#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags
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
#include "utils.hpp"
#include <SDL/SDL.h>
#include <math.h>

#define PI	3.141593
#define HPI	(PI / 2.0f)
#define PI2	(2 * PI)

typedef perf_timer<boost::chrono::thread_clock> timer;
SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * tex_diffuse;
SDL_Texture * tex_depth;


struct gbuffer {
	typedef thrust::host_vector<glm::vec4> diffuse;
	typedef thrust::host_vector<glm::vec3> normal;
	typedef thrust::host_vector<float> depth;

	unsigned width;
	unsigned height;

	diffuse buf_diffuse;
	normal buf_normal;
	depth buf_depth;

	diffuse clear_diffuse;
	normal clear_normal;
	depth clear_depth;

	gbuffer(unsigned w, unsigned h) :
		width(w),
		height(h),
		buf_diffuse(width * height),
		buf_normal(width * height),
		buf_depth(width * height),
		clear_diffuse(buf_diffuse.size()),
		clear_normal(buf_normal.size()),
		clear_depth(buf_depth.size()){

		set_clear_values(
			glm::vec4(1.0f,1.0f,1.0f,1.0f),
			glm::vec3(0.0f,0.0f,0.0f),
			0);
	}

	void set_clear_values(const glm::vec4 & diff_value, const glm::vec3 & norm_value, float depth_value) {
		set_clear_diffuse(diff_value);
		thrust::fill(clear_normal.begin(), clear_normal.end(), norm_value);
		thrust::fill(clear_depth.begin(), clear_depth.end(), depth_value);
	}

	void set_clear_diffuse(const glm::vec4 & diff_value) {
		thrust::fill(clear_diffuse.begin(), clear_diffuse.end(), diff_value);
	}
	void clear() {
		thrust::copy(clear_diffuse.begin(), clear_diffuse.end(), buf_diffuse.begin());
		thrust::copy(clear_normal.begin(), clear_normal.end(), buf_normal.begin());
		thrust::copy(clear_depth.begin(), clear_depth.end(), buf_depth.begin());
	}
};

#define coord2d(x__,y__) (((y__) * 640) + (x__))



struct camera {
	glm::mat4 proj_mat;
	glm::mat4 view_mat;
	glm::vec3 position;

	camera(glm::vec3 _pos, float fov, float aspect, float near, float far) :
		position(_pos){
		proj_mat = glm::perspective(fov, aspect, near, far);
		view_mat = glm::lookAt(position, glm::vec3(0,0,0), glm::vec3(0,1,0));
		//view_mat = glm::translate(view_mat, -position);
	}
} cam(glm::vec3(0,0, 10), 45, 4.0f/3.0f, 5, 200);

struct mesh {
	thrust::host_vector<glm::vec4> vertices;
	thrust::host_vector<glm::vec4> colors;
	thrust::host_vector<glm::vec4> normals;
	thrust::host_vector<glm::ivec3> triangles;

	size_t total_vertices;
	glm::vec4 position;
	glm::mat4 model_mat;


	mesh() :
		total_vertices(0),
		position(0,0,0,1),
		model_mat(1.0f){

		model_mat = glm::rotate(glm::mat4(1.0f), -45.0f, glm::vec3(1,0,0));
	}

	void resize(size_t vectors_sz, size_t triangles_sz) {
		total_vertices = vectors_sz;
		vertices.resize(vectors_sz);
		colors.resize(vectors_sz);
		normals.resize(vectors_sz);
		triangles.resize(triangles_sz);

	}

} tux;

// Render booking
struct booking {
	thrust::host_vector<bool> discard_vertex;

	float near = 1, far = 0;

	void reset(size_t sz_vertices) {
		discard_vertex.clear();
		discard_vertex.resize(sz_vertices, false);
	}
};

struct vertex_proc_kernel {

	const glm::mat4 & mvp;
	booking & book;

	vertex_proc_kernel(const glm::mat4 & _mvp, booking & b) :
		mvp(_mvp),
		book(b){}

	glm::vec4 operator()(const glm::vec4 & v, size_t index) {
		glm::vec4 vn = mvp * v;
		// clip coordinates
		vn = vn / vn.w;
		// normalized device coordinates
		vn.x = (vn.x * 320) + 320;
		vn.y = (vn.y * 240) + 240;
		vn.z = (vn.z * (book.far - book.near)/2) + (book.far + book.near)/2;

		// window space
		if (vn.x >= 640 || vn.x < 0 || vn.y > 480 || vn.y < 0)
			book.discard_vertex[index] = true;
		return vn;
	}
};
// Extract projected vertices
thrust::host_vector<glm::vec4> vertex_proc(const mesh & m, const camera & c, booking & book) {
	thrust::host_vector<glm::vec4> proj_vert(m.total_vertices);	// Projected vertices
	book.reset(m.total_vertices);
	glm::mat4 mvp_mat(1.0f);
	mvp_mat = c.proj_mat * c.view_mat * m.model_mat;

	thrust::transform(
			m.vertices.begin(), m.vertices.end(),			// Input 1
			thrust::counting_iterator<size_t>(0),			// Input 3
			proj_vert.begin(),								// Output
			vertex_proc_kernel(mvp_mat, book));				// Operation

	return proj_vert;
}

struct face {
	glm::vec3 v[3];
	struct {
		bool discard;
	} flags;

	typedef glm::vec3 * iterator;

	typedef const glm::vec3 * const_iterator;

	face(const glm::vec3 & v0, const glm::vec3 & v1, const glm::vec3 & v2, bool _disc) {
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		flags.discard = _disc || !ccw_winding_order();
	}
	face(){}

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

struct primitives_kernel {

	const thrust::host_vector<glm::vec4> & ws_vertices;
	booking & book;

	primitives_kernel(const thrust::host_vector<glm::vec4> & v, booking & b) :
		ws_vertices(v),
		book(b) {}
	face operator()(const glm::ivec3 & tr, const mesh *m) {
		return face(
			glm::vec3(ws_vertices[tr.x]),
			glm::vec3(ws_vertices[tr.y]),
			glm::vec3(ws_vertices[tr.z]),
			book.discard_vertex[tr.x] || book.discard_vertex[tr.y] || book.discard_vertex[tr.z]
		);
	}
};

// Extract primitives
thrust::host_vector<face> primitives_proc(const mesh & m, const thrust::host_vector<glm::vec4> & ws_vertices, booking & book){
	thrust::host_vector<face> prim_coords(m.triangles.size());
	std::cout << "Triangles: " << m.triangles.size() << std::endl;
	thrust::transform(
			m.triangles.begin(), m.triangles.end(),
			thrust::make_constant_iterator(&m),
			prim_coords.begin(),
			primitives_kernel(ws_vertices, book));
	return prim_coords;
}



struct draw_pixel {
	gbuffer & gbuf;

	draw_pixel(gbuffer & _gbuf) : gbuf(_gbuf){}

	inline bool operator()(int x, int y) {
		gbuf.buf_diffuse[coord2d(x,y)] = glm::vec4(1,0,0,1);

		return true;
	}
};

struct interpolate_draw_pixel {
	gbuffer & gbuf;

	const glm::vec3 & from, & to;
	float sqlength;
	float zdist;

	interpolate_draw_pixel(gbuffer & _gbuf, const glm::vec3 & _from, const glm::vec3 & _to) :
		gbuf(_gbuf),
		from(_from),
		to(_to),
		zdist(from.z - to.z)
	{
		//sqlength = glm::length(to-from);
		sqlength = pow(to.x-from.x,2) + pow(to.y-from.y,2);
	}

	inline bool operator()(int x, int y) {
		size_t coords = coord2d(x,y);
		float factor,z;
		if (!sqlength) {
			z= from.z;
		} else {
			float sqdist = pow(x-from.x,2) + pow(y-from.y,2);
			factor = sqdist/sqlength;
			z = (zdist)*factor + from.z;
		}
		if (gbuf.buf_depth[coords] > z)	// Z-test
			return true;

		gbuf.buf_depth[coords] = z;
		gbuf.buf_diffuse[coords] = glm::vec4(1,0,0,1);
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
		vertical(false),
		p1(_p1),
		p2(_p2){


		if (p2.x - p1.x)
			slope = (p2.y - p1.y)/(p2.x - p1.x);
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

template <class PixelOperation>
struct walk_scan_line {

	const glm::vec3 ** points;

	PixelOperation & pixel_op;

	edge e01, e02, e21;

	walk_scan_line(PixelOperation _op, const glm::vec3 * _points[]) :
		points(_points),
		pixel_op(_op),
		e01(*points[0], *points[1]),
		e02(*points[0], *points[2]),
		e21(*points[2], *points[1]){
	}

	bool operator()(int x, int y) {
		if (y <= points[2]->y) {
			for(int xw = x; e02.check_point_leftof(xw,y) ; xw++)
				pixel_op(xw,y);
		}
		return true;
	}
};

struct contour {
	int left_limits[480];
	int right_limits[480];

	contour() {
		for(int i = 0;i < 480;i++) {
			left_limits[i] = 10000;
			right_limits[i] = 0;
		}
	}
};
struct mark_contour {

	contour & cont;

	mark_contour(contour & _cont):
		cont(_cont){
	}

	bool operator()(int x, int y) {
		if (x < cont.left_limits[y])
			cont.left_limits[y] = x;
		if (x > cont.right_limits[y])
			cont.right_limits[y] = x;
		return true;
	}
};

struct raster_kernel {

	gbuffer & gbuf;
	draw_pixel draw_pixel_op;


	raster_kernel(gbuffer & _gbuf) :
		gbuf(_gbuf),
		draw_pixel_op(gbuf)	{
	}

	void operator()(const face & f) {
		if (f.flags.discard)
			return;

		// Sort points by y
		const glm::vec3 * pord[3] = { f.v, f.v+1, f.v+2 };
		for(int i = 0;i < 3;i++) {
			if (f.v[i].y > pord[0]->y) {
				pord[0] = f.v+i;
			} else if (f.v[i].y < pord[2]->y) {
				pord[2] = f.v+i;
			}
		}

		for(int i = 0;i< 3;i++)
			if ((pord[0] != f.v+i) && (pord[2] != f.v+i)) {
				pord[1] = f.v+i;
				break;
			}

/*		if (pord[2]->y < pord[1]->x)
			std::swap(pord[1], pord[2]);*/

		contour tri_contour;


		line_bresenham(pord[0]->x, pord[0]->y, pord[1]->x, pord[1]->y, 0,
				mark_contour(tri_contour));
		line_bresenham(pord[1]->x, pord[1]->y, pord[2]->x, pord[2]->y, 0,
				mark_contour(tri_contour));
		line_bresenham(pord[0]->x, pord[0]->y, pord[2]->x, pord[2]->y, 0,
				mark_contour(tri_contour));
		interpolate_draw_pixel pix_op(gbuf, *pord[1],*pord[2]);
		for(int y = pord[2]->y; y <= pord[0]->y; y++) {
			for(int x  = tri_contour.left_limits[y]; x< tri_contour.right_limits[y];x++)
				pix_op(x,y);
		}
/*		line_bresenham(pord[0]->x, pord[0]->y, pord[2]->x, pord[2]->y, 0,
			interpolate_draw_pixel(gbuf, *pord[0],*pord[2]));
		line_bresenham(pord[1]->x, pord[1]->y, pord[2]->x, pord[2]->y, 0,
					interpolate_draw_pixel(gbuf, *pord[1],*pord[2]));

		//line_bresenham(pord[0]->x, pord[0]->y, pord[1]->x, pord[1]->y, 0,
				//walk_scan_line<(interpolate_draw_pixel(gbuf, *pord[0],*pord[1]),pord));
		line_bresenham(pord[0]->x, pord[0]->y, pord[2]->x, pord[2]->y, 0,
			interpolate_draw_pixel(gbuf, *pord[0],*pord[2]));
		line_bresenham(pord[1]->x, pord[1]->y, pord[2]->x, pord[2]->y, 0,
					interpolate_draw_pixel(gbuf, *pord[1],*pord[2]));
*/

/*		line_bresenham(f.v[0].x, f.v[0].y, f.v[1].x, f.v[1].y, 0,
				interpolate_draw_pixel(gbuf, f.v[0], f.v[1]));
		line_bresenham(f.v[0].x, f.v[0].y, f.v[2].x, f.v[2].y, 0,
				interpolate_draw_pixel(gbuf, f.v[0], f.v[2]));
		line_bresenham(f.v[2].x, f.v[2].y, f.v[1].x, f.v[1].y, 0,
				interpolate_draw_pixel(gbuf, f.v[2], f.v[1]))*/;
	}
};

// Rasterization
void raster_proc(const thrust::host_vector<face> & faces, gbuffer & gbuf) {
	thrust::for_each(
			faces.begin(),
			faces.end(),
			raster_kernel(gbuf)
			);
}

void upload_image(gbuffer & gbuf){

	long src_index = 0;
	Uint32 *dst_dif, * dst_depth;
	int row, col;
    void *pixels_dif;
    void *pixels_depth;
    int pitch_dif, pitch_depth;
    SDL_Rect dstrect;

    if (SDL_LockTexture(tex_diffuse, NULL, &pixels_dif, &pitch_dif) < 0) {
    	std::cerr << "Couldn't lock texture:" << SDL_GetError() << std::endl;
    	return;
    }
    if (SDL_LockTexture(tex_depth, NULL, &pixels_depth, &pitch_depth) < 0) {
        	std::cerr << "Couldn't lock texture:" << SDL_GetError() << std::endl;
        	return;
        }

    for (row = 0; row < 480; ++row) {
    	dst_dif = (Uint32*)((Uint8*)pixels_dif + row * pitch_dif);
    	dst_depth = (Uint32*)((Uint8*)pixels_depth + row * pitch_depth);

    	for (col = 0; col < 640; ++col) {
    		glm::vec4 color = gbuf.buf_diffuse[src_index++] * 255.0f;
    		float d = gbuf.buf_depth[src_index] * 255.0f;
    		//*dst++ = (0xFF000000|(color.r << 16)|(color.g << 8) | color.b);
    		*dst_dif++ = (0xFF000000|((int)color.r << 16)|((int)color.g << 8) | (int)color.b);
    		*dst_depth++ = (((int)d << 16)|((int)d << 8) | (int)d);

    	}
    }

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
    SDL_RenderPresent(renderer);

}

void render() {

	timer tm;
	gbuffer gbuff(640, 480);
	gbuff.set_clear_diffuse(glm::vec4(0,0,0,1));
	thrust::host_vector<face> faces;
	booking book;

	for(int i = 1; i < 15000;i++) {
		tm.reset();
		gbuff.clear();
		faces = primitives_proc(tux, vertex_proc(tux, cam, book), book);
		raster_proc(faces, gbuff);
		timer::duration dt = tm.passed();
		std::cout << "Frame took " << dt << std::endl;
		upload_image(gbuff);

		tux.model_mat = glm::rotate(tux.model_mat, 10.0f, glm::vec3(0,1,0));
		//cam.view_mat = glm::rotate(cam.view_mat, 10.0f, glm::vec3(0,1,0));
	}
	thrust::host_vector<face>::iterator it;

	std::ofstream myfile;
	myfile.open ("pixels.txt");
	for(it = faces.begin();it != faces.end();it++) {
		face & f = *it;
		for(int i = 0; i < 3;i++){
			myfile << f.v[i].x << "," << f.v[i].y << "," << f.v[i].z << std::endl;
		}
	}
	myfile.close();
	while(1){}
	std::cout << "pixels written" << std::endl;
}

bool loadScene() {
	 // Create an instance of the Importer class
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile( "/home/sque/Downloads/tux__.ply",
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);
	// If the import failed, report it
	if( !scene)	{
		std::cerr << importer.GetErrorString() << std::endl;
		return false;
	}

	if ( scene->mNumMeshes != 1) {
		std::cerr << "More than one model" << std::endl;
		return false;
	}

	// Load TUX
	const aiMesh * m = scene->mMeshes[0];
	tux.resize(m->mNumVertices, m->mNumFaces);

	for(unsigned i = 0;i < m->mNumVertices;i++){
		tux.vertices[i] = glm::vec4(glm::make_vec3(&m->mVertices[i].x),1);
		tux.normals[i] =  glm::vec4(glm::make_vec3(&m->mNormals[i].x),1);
		//tux.colors[i] =  glm::make_vec4(&m->mColors[i]->r);
	}

	for(unsigned i = 0;i < m->mNumFaces;i++){
			tux.triangles[i] = glm::ivec3(
					m->mFaces[i].mIndices[0],
					m->mFaces[i].mIndices[1],
					m->mFaces[i].mIndices[2]
					);
		}
	return true;
}


int main(){
	if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
		std::cerr << "Unable to init SDL:" << SDL_GetError() << std::endl;
		return 1;
	}

	window = SDL_CreateWindow("Thrust Renderer",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			640*2, 480,
			SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE
	);

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
    	std::cerr << "Couldn't set create renderer:" << SDL_GetError() << std::endl;
    }

    tex_diffuse = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 640,480);
    if (!tex_diffuse) {
    	std::cerr <<  "Couldn't set create texture:" << SDL_GetError() << std::endl;
    	return 5;
    }

    tex_depth= SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 640,480);
    if (!tex_depth) {
    	std::cerr <<  "Couldn't set create texture:" << SDL_GetError() << std::endl;
    	return 5;
    }

	if (!loadScene()){
		std::cerr << "Error loading scene" << std::endl;
	}
	std::cout << "Scene loaded" << std::endl;
	render();
	std::cout << "Scene rendered" << std::endl;
	return 0;
}
