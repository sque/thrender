#pragma once

#include "./gbuffer.hpp"
#include "./camera.hpp"
#include "./types.hpp"
#include "./viewport.hpp"

namespace thrender {


	//! Context of current rendering
	struct render_context {


		gbuffer & gbuff;
		camera & cam;
		viewport vp;

		render_context(camera & _camera, gbuffer & _gbuffer) :
			gbuff(_gbuffer),
			cam(_camera),
			vp(0, 0, gbuff.width, gbuff.height),
			depth_buffer_near(1),
			depth_buffer_far(0)
		{}


		inline camera & get_camera() {
			return cam;
		}

		inline gbuffer get_framebuffer() {
			return gbuff;
		}

		//! Depth buffer value representing near plane
		depth_pixel_t depth_buffer_near;

		//! Depth buffer value representing far plane
		depth_pixel_t depth_buffer_far;
	};
}
