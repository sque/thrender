#pragma once

#include "./types.hpp"
#include "./render_context.hpp"
#include "./camera.hpp"
#include "./renderable.hpp"
#include <thrust/iterator/zip_iterator.h>

namespace thrender {

	//! Vertex processing control mechanism
	/**
	 * It is used to control the vertex processing
	 * pipeline. It provides all the supported functions
	 * per batch and per vertex.
	 */
	template<class RenderableType>
	struct vertex_processing_control {

		//! Type of renderable object
		typedef RenderableType renderable_type;

		//! The id of this vertex
		vertex_id_t vertex_id;

		//! Reference to the owner object
		const renderable_type & object;

		//! Construct control on vertex processing
		vertex_processing_control(const renderable_type & _object)
		:
			vertex_id(0),
			object(_object)
		{}

		//! Drops the current vertex as discarded
		void discard() const{
			const_cast<renderable_type &>(object).intermediate_buffer.discarded_vertices[vertex_id] = true;
		}
	};


namespace shaders {

	//! Default vertex shader
	template<class RenderableType>
	struct default_vertex_shader {

		//! Type of renderable object
		typedef RenderableType renderable_type;

		//! Type of vertex
		typedef typename renderable_type::vertex_type vertex_type;

		//! Reference to render state
		render_context & rcontext;

		//! Model view projection matrix
		const glm::mat4 & mvp;

		// To be reviewed
		unsigned half_width;
		unsigned half_height;

		default_vertex_shader(const glm::mat4 & _mvp, render_context & _rcontext) :
			rcontext(_rcontext),
			mvp(_mvp)
		{
			half_width = rcontext.gbuff.width / 2;
			half_height = rcontext.gbuff.height / 2;
		}

		void operator()(const vertex_type & vin, vertex_type & vout, vertex_processing_control<RenderableType> & vcontrol){
			const glm::vec4 & posIn = VA_ATTRIBUTE(vin, POSITION);
			glm::vec4 & posOut = VA_ATTRIBUTE(vout, POSITION);

			posOut = mvp * posIn;
			// clip coordinates

			posOut = posOut / posOut.w;
			// normalized device coordinates

			posOut.x = (posOut.x * half_width) + half_width;
			posOut.y = (posOut.y * half_height) + half_height;
			posOut.z = (posOut.z * (rcontext.far - rcontext.near)/2) + (rcontext.far + rcontext.near)/2;
			// window space

			// Discard vertices
			// FIXME: Add z clipping
			if (posOut.x >= rcontext.gbuff.width || posOut.x < 0 || posOut.y > rcontext.gbuff.height || posOut.y < 0)
				vcontrol.discard();
			return;
		}
	};
}


	//! Kernel for processing vertices
	/**
	 * Unwraps vertex parameters to the vertex shaders API
	 */
	template <class VertexShader, class RenderableType>
	struct vertex_processor_kernel {

		//! Type of vertex shader
		typedef VertexShader shader_type;

		//! Type of renderable object
		typedef RenderableType renderable_type;

		//! Reference to shader
		shader_type & shader;

		//! Reference to renderable object
		const renderable_type & object;

		//! Vertex processing control object
		vertex_processing_control<RenderableType> vcontrol;

		//! Initialize by referencing the wrapped shader
		vertex_processor_kernel(shader_type & _shader, const renderable_type & _object)
		:
			shader(_shader),
			object(_object),
			vcontrol(object)
		{}

		template<class T>
		void operator()(T & v) {
			vcontrol.vertex_id = thrust::get<2>(v);
			shader(thrust::get<0>(v), thrust::get<1>(v), vcontrol);
		}

	};

	//! Process vertices and extract projected on window space
	template<class VertexShader, class RenderableType>
	void process_vertices(RenderableType & object, render_context & rstate) {

		// Prepare object
		object.prepare_for_rendering();
		glm::mat4 mvp_mat(1.0f);
		mvp_mat = rstate.cam.projection_mat * rstate.cam.view_mat/* * m.model_mat*/;

		// Custom shader
		VertexShader shader(mvp_mat, rstate) ;

		// Process vertices
		size_t total_vertices = object.vertices.size();
		thrust::counting_iterator<vertex_id_t> count_begin(0);
		thrust::for_each(
			thrust::make_zip_iterator(thrust::make_tuple(object.vertices.cbegin(), object.intermediate_buffer.processed_vertices.begin(), count_begin)),
			thrust::make_zip_iterator(thrust::make_tuple(object.vertices.cend(), object.intermediate_buffer.processed_vertices.end(), count_begin + total_vertices)),
			vertex_processor_kernel<VertexShader, RenderableType>(shader, object));		// Operation
	}
}
