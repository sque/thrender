#pragma once

#include "./math.hpp"
#include "./triangle.hpp"
#include "./render_state.hpp"

namespace thrender {

	struct primitives_proc_kernel {

		const mesh & m;
		const thrust::host_vector<glm::vec4> & ws_vertices;
		render_context & rstate;

		primitives_proc_kernel(const thrust::host_vector<glm::vec4> & v,
				const mesh & _m, thrender::render_context & _rstate) :
				m(_m), ws_vertices(v), rstate(_rstate) {
		}

		triangle operator()(const glm::ivec3 & tr) {
			return triangle();
					/*m,
					ws_vertices,
					tr.x, tr.y, tr.z,
					m.render_buffer.discard_vertices[tr.x] || m.render_buffer.discard_vertices[tr.y]
							|| m.render_buffer.discard_vertices[tr.z]);*/
		}
	};

	// Process projected vertices and extract primitives.
	thrust::host_vector<triangle> process_primitives(const mesh & m, thrender::render_context & rstate) {
		thrust::host_vector<triangle> primitives(m.total_triangles());

		thrust::transform(
				m.triangles.begin(), m.triangles.end(),		// Input
				primitives.begin(),							// Output
				primitives_proc_kernel(m.render_buffer.projected_vertices, m, rstate));
		return primitives;
	}
}
