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
#include "thrender/exp/gui.hpp"

thrender::window * window;
thrender::texture * tex_diffuse;
thrender::camera cam(glm::vec3(0, 0, -10), 45, 4.0f / 3.0f, 5, 50);

void upload_images(thrender::framebuffer_array & fb) {

	tex_diffuse->upload(fb.color_buffer());

	window->copy(0, 0, *tex_diffuse);
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
            		cam.view_mat = glm::rotate(cam.view_mat, -float(event.motion.yrel), glm::vec3(1, 0, 0));
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

	thrender::framebuffer_array gbuff(640, 480);

	typedef thrender::renderable<thrust::tuple<
			glm::vec4,
			glm::vec4,
			glm::vec4,
			glm::vec2> > mesh_type;
	mesh_type tux = thrender::utils::load_model<mesh_type>("/home/sque/Downloads/tux__.ply");
	mesh_type cube = thrender::utils::load_model<mesh_type>("/home/sque/Downloads/cube.ply");
	std::cout << thrender::utils::to_string(tux) << std::endl;

	thrender::shaders::phong_material mat_plastic_blue;
	mat_plastic_blue.diffuse_color = glm::vec4(0.2f, 1.0f, 0.2f, 1.0f);
	mat_plastic_blue.emissive_color = glm::vec4(0.f, 0.f, 0.f, 0.f);
	mat_plastic_blue.specular_color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	mat_plastic_blue.shininess = 10;

	thrender::render_context ctx(cam, gbuff);
	thrender::shaders::gouraud_vx_shader vx_shader;
	vx_shader.light.position_ws = glm::vec4(10.f, 10.f, 0.f, 1.f);
	vx_shader.light.diffuse_color = glm::vec4(.8f, .8f, .8f, 1.f);
	vx_shader.light.specular_color = glm::vec4(.8f, .8f, .8f, 1.f);

	vx_shader.material = mat_plastic_blue;
	thrender::shaders::gouraud_fg_shader fg_shader;
	thrender::pipeline<mesh_type, thrender::shaders::gouraud_vx_shader, thrender::shaders::gouraud_fg_shader> pp(vx_shader, fg_shader);



	thrender::utils::frame_rate_keeper<> lock_fps(1);
	thrender::utils::profiler<boost::chrono::high_resolution_clock> prof("Render procedure");
	for (int i = 1; i < 150000; i++) {
		prof.clear();
		{	PROFILE_BLOCK(prof, "Clear buffer");
			vx_shader.mProjection = ctx.cam.projection_mat;
			vx_shader.mView = ctx.cam.view_mat;
			vx_shader.mModel = glm::mat4(1.0f);
			vx_shader.vCameraPos_ws = glm::vec4(ctx.cam.position(), 1.f);
			gbuff.clear_all();
		}
		{	PROFILE_BLOCK(prof, "Render cube");
			//pp.draw(cube, ctx);
		}
		{	PROFILE_BLOCK(prof, "Render tux");
			glm::mat4 model_mat(1.0f);
			model_mat = glm::rotate(model_mat, 90.0f, glm::vec3(1.0f,.0f,.0f));
			vx_shader.mModel = model_mat;
			pp.draw(tux, ctx);
		}

		{	PROFILE_BLOCK(prof, "Upload images");
			upload_images(gbuff);
		}
		std::cout << prof.report() << std::endl;

		process_events();
		glm::quat rot = glm::angleAxis(2.0f, glm::vec3(1.0f, .0f, .0f));
		vx_shader.light.position_ws = glm::normalize(glm::rotate(rot, vx_shader.light.position_ws));
		//lock_fps.keep_frame_rate();
	}

	while (1) ;
}

int main() {

	window = new thrender::window("Thrust Renderer",640, 480);
	tex_diffuse = new thrender::texture(window->renderer_handle(), 640, 480);

	std::cout << "Initilized SDL" << std::endl;
	render();
	return 0;
}
