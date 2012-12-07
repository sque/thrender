#pragma once

namespace thrender {

	struct render_state {

		// Normalization of z-buffer
		float near, far;

		render_state() :
			near(1),
			far(0) {}

	};
}
