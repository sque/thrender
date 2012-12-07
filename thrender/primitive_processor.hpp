#pragma once

#include "./math.hpp"
#include "./triangle.hpp"
#include "./render_state.hpp"

namespace thrender {

	struct primitives_proc_kernel {

		const mesh & m;
		const thrust::host_vector<glm::vec4> & ws_vertices;
		render_state & rstate;

		primitives_proc_kernel(const thrust::host_vector<glm::vec4> & v,
				const thrender::mesh & _m, thrender::render_state & _rstate) :
				m(_m), ws_vertices(v), rstate(_rstate) {
		}

		triangle operator()(const glm::ivec3 & tr) {
			return triangle(
					m,
					ws_vertices,
					tr.x, tr.y, tr.z,
					rstate.discard_vertex[tr.x] || rstate.discard_vertex[tr.y]
							|| rstate.discard_vertex[tr.z]);
		}
	};

	// Process projected vertices and extract primitives.
	thrust::host_vector<triangle> process_primitives(const mesh & m,
			const thrust::host_vector<glm::vec4> & proj_vertices,
			thrender::render_state & rstate) {
		thrust::host_vector<triangle> primitives(m.total_triangles());

		thrust::transform(
				m.triangles.begin(), m.triangles.end(),		// Input
				primitives.begin(),							// Output
				primitives_proc_kernel(proj_vertices, m, rstate));
		return primitives;
	}
}
