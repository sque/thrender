#pragma once

#include "./gbuffer.hpp"
#include "./camera.hpp"
namespace thrender {

	struct render_state {

		// Normalization of z-buffer
		float near, far;

		gbuffer & gbuff;
		camera & cam;

		render_state(camera & _camera, gbuffer & _gbuffer) :
			near(1),
			far(0),
			gbuff(_gbuffer),
			cam(_camera){}

	};
}
