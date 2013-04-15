#pragma once

#include "./render_state.hpp"
#include "./camera.hpp"
#include "./vertex_array.hpp"
#include "./shaders/default_vertex.hpp"
#include <thrust/iterator/zip_iterator.h>
#include <boost/tuple/tuple.hpp>

namespace thrender {
namespace shaders {


	//! Default vertex shader
	template<class VertexArray>
	struct default_vertex_shader {

		//! Type of object
		typedef VertexArray object_type;

		typedef typename object_type::attributes_type attributes_type;

		//! Reference to object
		object_type & object;

		//! Reference to render state
		render_state & rstate;

		//! Model view projection matrix
		const glm::mat4 & mvp;

		// To be reviewd
		unsigned half_width;
		unsigned half_height;

		default_vertex_shader(const glm::mat4 & _mvp, object_type & _object, render_state & _rstate) :
			object(_object),
			rstate(_rstate),
			mvp(_mvp)
		{
			half_width = rstate.gbuff.width / 2;
			half_height = rstate.gbuff.height / 2;
		}

		attributes_type operator()(const attributes_type & vin, size_t index) {

			const glm::vec4 &	posIn =  ATTRIBUTE(vin, POSITION);
			attributes_type vout(vin);
			glm::vec4 &	posOut = ATTRIBUTE(vout, POSITION);
			posOut = mvp * posIn;
			// clip coordinates

			posOut = posOut / posOut.w;
			// normalized device coordinates

			posOut.x = (posOut.x * half_width) + half_width;
			posOut.y = (posOut.y * half_height) + half_height;
			posOut.z = (posOut.z * (rstate.far - rstate.near)/2) + (rstate.far + rstate.near)/2;
			// window space

			// Discard vertices
			// FIXME: Add z clipping
			if (posOut.x >= rstate.gbuff.width || posOut.x < 0 || posOut.y > rstate.gbuff.height || posOut.y < 0)
				object.render_buffer.discarded_vertices[index] = true;
			return vout;
		}
	};

}

	//! Process vertices and extract projected on window space
	template<class VertexShader, class VertexArray>
	void process_vertices(VertexArray & m, render_state & rstate) {

		m.render_buffer.reset();
		glm::mat4 mvp_mat(1.0f);
		mvp_mat = rstate.cam.projection_mat * rstate.cam.view_mat /** m.model_mat*/;

		// For all vertex attributes, process them
		thrust::transform(
			m.attributes.begin(), m.attributes.end(),		// Input 1
			thrust::counting_iterator<size_t>(0),			// Input 3
			m.render_buffer.processed_vertices.begin(),		// Output
			VertexShader(mvp_mat, m, rstate));				// Operation
	}
}
