#pragma once

#include "./gbuffer.hpp"
#include "./camera.hpp"
#include "./types.hpp"

namespace thrender {

	//! Context of current rendering
	struct render_context {

		// Normalization of z-buffer
		float near, far;

		gbuffer & gbuff;
		camera & cam;

		render_context(camera & _camera, gbuffer & _gbuffer) :
			near(1),
			far(0),
			gbuff(_gbuffer),
			cam(_camera){}

		inline window_size_t window_width() const {
			return gbuff.width;
		}

		inline window_size_t window_height() const {
			return gbuff.height;
		}

		inline camera & get_camera() {
			return cam;
		}

		inline gbuffer get_framebuffer() {
			return gbuff;
		}
	};
}
