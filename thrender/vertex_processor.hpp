#pragma once

#include "./render_state.hpp"
#include "./camera.hpp"
#include "./mesh.hpp"
#include "./shaders/default_vertex.hpp"
#include <thrust/iterator/zip_iterator.h>



namespace thrender {
namespace shaders {
	//! Default vertex shader
	struct default_vertex_shader {

		const glm::mat4 & mvp;
		mesh & object;
		render_state & rstate;

		unsigned half_width;
		unsigned half_height;

		default_vertex_shader(const glm::mat4 & _mvp, mesh & _object, render_state & _rstate) :
			mvp(_mvp),
			object(_object),
			rstate(_rstate)
		{
			half_width = rstate.gbuff.width / 2;
			half_height = rstate.gbuff.height / 2;
		}

		glm::vec4 operator()(const glm::vec4 & v, size_t index) {

			glm::vec4 vn = mvp * v;
			// clip coordinates

			vn = vn / vn.w;
			// normalized device coordinates

			vn.x = (vn.x * half_width) + half_width;
			vn.y = (vn.y * half_height) + half_height;
			vn.z = (vn.z * (rstate.far - rstate.near)/2) + (rstate.far + rstate.near)/2;
			// window space

			// Discard vertices
			// FIXME: Add z clipping
			if (vn.x >= rstate.gbuff.width || vn.x < 0 || vn.y > rstate.gbuff.height || vn.y < 0)
				object.render_buffer.discarded_vertices[index] = true;
			return vn;
		}
	};

}

	//! Process vertices and extract projected on window space
	template<class VertexShader>
	void process_vertices(mesh & m, render_state & rstate) {

		m.render_buffer.reset();
		glm::mat4 mvp_mat(1.0f);
		mvp_mat = rstate.cam.projection_mat * rstate.cam.view_mat * m.model_mat;

		// For all vertex attributes, process them
		thrust::transform(
			m.attributes.positions.begin(), m.attributes.positions.end(),	// Input 1
			thrust::counting_iterator<size_t>(0),							// Input 3 // FIXME: atomic?
			m.render_buffer.projected_vertices.begin(),						// Output
			VertexShader(mvp_mat, m, rstate));								// Operation
	}
}
