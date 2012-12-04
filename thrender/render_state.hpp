#pragma once

namespace thrender {

	struct render_state {
		thrust::host_vector<bool> discard_vertex;

		float near = 1, far = 0;

		void reset(size_t sz_vertices) {
			discard_vertex.clear();
			discard_vertex.resize(sz_vertices, false);
		}
	};
}
