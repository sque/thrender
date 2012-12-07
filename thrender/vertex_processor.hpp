#pragma once

#include "render_state.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "utils.hpp"
#include <thrust/iterator/zip_iterator.h>

namespace thrender {

	struct vertex_shader {

		const glm::mat4 & mvp;
		mesh & object;
		render_state & rstate;

		vertex_shader(const glm::mat4 & _mvp, mesh & _object, render_state & _rstate) :
			mvp(_mvp),
			object(_object),
			rstate(_rstate){}

		glm::vec4 operator()(const glm::vec4 & v, size_t index) {

			glm::vec4 vn = mvp * v;
			// clip coordinates

			vn = vn / vn.w;
			// normalized device coordinates

			vn.x = (vn.x * 320) + 320;
			vn.y = (vn.y * 240) + 240;
			vn.z = (vn.z * (rstate.far - rstate.near)/2) + (rstate.far + rstate.near)/2;
			// window space

			// Discard vertices
			// FIXME: Add z clipping
			if (vn.x >= 640 || vn.x < 0 || vn.y > 480 || vn.y < 0)
				object.render_buffer.discard_vertices[index] = true;
			return vn;
		}
	};

	// Process vertices and extract projected on window space
	size_t process_vertices(mesh & m, const thrender::camera & cam, render_state & rstate) {

		m.render_buffer.reset();
		glm::mat4 mvp_mat(1.0f);
		mvp_mat = cam.proj_mat * cam.view_mat * m.model_mat;

		// For all vertex attributes, process them
		thrust::transform(
				m.attributes.positions.begin(), m.attributes.positions.end(),	// Input 1
				thrust::counting_iterator<size_t>(0),							// Input 3 // FIXME: atomic?
				m.render_buffer.proj_vertices.begin(),							// Output
				vertex_shader(mvp_mat, m, rstate));								// Operation

		return m.render_buffer.proj_vertices.size();
	}
}
