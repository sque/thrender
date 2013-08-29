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

#include "thrender/thrender.hpp"
#include "thrender/utils/io.hpp"
#include "thrender/utils/to_string.hpp"
#include "thrender/utils/frame_rate_keeper.hpp"
#include "thrender/utils/profiler.hpp"
#include "thrender/pipeline.hpp"
#include "thrender/exp/gui.hpp"

thrender::window * window;
thrender::texture * tex_diffuse;
thrender::texture * tex_depth;
thrender::texture * tex_normals;
thrender::camera cam(glm::vec3(0, 0, 10), 45, 4.0f / 3.0f, 5, 50);

void upload_images(thrender::gbuffer & gbuf) {

	tex_diffuse->upload(gbuf.diffuse);
	tex_normals->upload(gbuf.normal);
	tex_depth->upload(gbuf.depth);

	window->copy(0, 0, *tex_diffuse);
	window->copy(640, 0, *tex_depth);
	window->copy(640, 480, *tex_normals);
	window->update();
}

void process_events() {
    SDL_Event event;

    while ( SDL_PollEvent(&event) ) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
                /*printf("Mouse moved by %d,%d to (%d,%d)\n",
                       event.motion.xrel, event.motion.yrel,
                       event.motion.x, event.motion.y);*/
            	if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            		cam.view_mat = glm::rotate(cam.view_mat, float(event.motion.xrel), glm::vec3(0, 1, 0));
            		cam.view_mat = glm::rotate(cam.view_mat, -float(event.motion.yrel), glm::vec3(0, 0, 1));
            	}
                break;
            case SDL_MOUSEWHEEL:
                cam.view_mat = glm::translate(cam.view_mat, glm::vec3(0, 0, event.wheel.y));

                break;
            case SDL_QUIT:
                exit(0);
        }
    }
}
void render() {

	thrender::gbuffer gbuff(640, 480);
	gbuff.set_clear_diffuse(glm::vec4(0, 0, 0, 1));

	typedef thrender::renderable<thrust::tuple<
			glm::vec4,
			glm::vec4,
			glm::vec4,
			glm::vec2> > mesh_type;
	mesh_type tux = thrender::utils::load_model<mesh_type>("/home/sque/Downloads/tux__.ply");
	std::cout << thrender::utils::to_string(tux) << std::endl;

	thrender::render_context ctx(cam, gbuff);
	thrender::shaders::default_vertex_shader vx_shader;
	thrender::shaders::default_fragment_shader fg_shader;
	thrender::pipeline<mesh_type,thrender::shaders::default_vertex_shader, thrender::shaders::default_fragment_shader> pp(vx_shader, fg_shader);
	vx_shader.mvp_mat = ctx.cam.projection_mat * ctx.cam.view_mat;




	thrender::utils::frame_rate_keeper<> lock_fps(1);
	thrender::utils::profiler<boost::chrono::high_resolution_clock> prof("Render procedure");
	for (int i = 1; i < 150000; i++) {
		prof.clear();
		{	PROFILE_BLOCK(prof, "Clear buffer");
			gbuff.clear();
		}
		{	PROFILE_BLOCK(prof, "Process vertices");
			thrender::process_vertices(tux, vx_shader, ctx);
		}
		{	PROFILE_BLOCK(prof, "Process fragments");
			thrender::process_fragments(tux, fg_shader, ctx);
		}
		{
			PROFILE_BLOCK(prof, "Pipelined 500 render");
/*
			for(int k =0;k < 500;k++){
				glm::mat4 model_mat(1.0f);
				model_mat = glm::translate(model_mat, glm::vec3(0,0,0.5*k));
				model_mat = glm::rotate(model_mat, 45.0f, glm::vec3(1.0f,1.0f,.0f));
				vx_shader.mvp_mat = ctx.cam.projection_mat * ctx.cam.view_mat * model_mat;
				pp.draw(tux, ctx);
			}
			*/
		}

		{	PROFILE_BLOCK(prof, "Upload images");
			upload_images(gbuff);
		}
		vx_shader.mvp_mat = ctx.cam.projection_mat * ctx.cam.view_mat/* * m.model_mat*/;
		std::cout << prof.report() << std::endl;

		process_events();
		//lock_fps.keep_frame_rate();
	}

	while (1) ;
}

int main() {

	window = new thrender::window("Thrust Renderer",640 * 2, 480 * 2);
	tex_diffuse = new thrender::texture(window->renderer_handle(), 640, 480);
	tex_depth = new thrender::texture(window->renderer_handle(), 640, 480);
	tex_normals = new thrender::texture(window->renderer_handle(), 640, 480);

	std::cout << "Initilized SDL" << std::endl;
	render();
	return 0;
}
