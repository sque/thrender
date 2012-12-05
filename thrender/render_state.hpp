#pragma once

namespace thrender {

	struct render_state {

		//! Vector with all discarded vertices
		thrust::host_vector<bool> discard_vertex;

		// Normalization of z-buffer
		float near, far;

		render_state() :
			near(1),
			far(0) {}

		void reset(size_t sz_vertices) {
			discard_vertex.clear();
			discard_vertex.resize(sz_vertices, false);
		}
	};
}
