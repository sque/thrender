#pragma once

#include "./types.hpp"
#include "./render_context.hpp"
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

		//! Type of vertex
		typedef typename renderable_type::vertex_type vertex_type;

		//! The id of this vertex
		vertex_id_t vertex_id;

		//! Reference to the owner object
		const renderable_type & object;

		//! Reference to current render context
		render_context & context;

		//! Construct control on vertex processing
		vertex_processing_control(const renderable_type & _object, render_context & _context, vertex_id_t _vertex_id)
		:
			vertex_id(_vertex_id),
			object(_object),
			context(_context)
		{}

		//! Drops the current vertex as discarded
		/**
		 * If a vertex is discarded, all the related elements
		 * are discarded too.
		 */
		void discard() const{
			const_cast<renderable_type &>(object).intermediate_buffer.discarded_vertices[vertex_id] = true;
		}

		//! Translate clip coordinates to window space
		template<class V>
		void translate_to_window_space(V & pos) {

			// normalized device coordinates
			pos = pos / pos.w;

			// window space
			pos.x = context.vp.left() + (pos.x * context.vp.half_width()) + context.vp.half_width();
			pos.y = context.vp.top() + (pos.y * context.vp.half_height()) + context.vp.half_height();
			pos.z = context.depth_range.translate_to_window_space(pos.z);
		}

		//! Viewport clipping
		/**
		 * It will check if vertex is in the limits of viewport
		 * and rendering frustum and will discard the vertex
		 * if not.
		 */
		template<class V>
		void viewport_clip(V & pos) {

			if (pos.x >= context.vp.width()	|| pos.x < 0
					|| pos.y > context.vp.height() || pos.y < 0
					|| pos.z > context.depth_range.near() || pos.z < context.depth_range.far()
					// FixME: Why z must be opposite of near and far?
				)
			{
				discard();
			}
		}
	};


namespace shaders {

	//! Default vertex shader
	/**
	 * Default shaders process vertices and projects them
	 * in 3D space based on ModelViewProjection matrix.
	 */
	struct default_vertex_shader {

		//! Model view projection matrix
		glm::mat4 mvp_mat;

		template<class RenderableType>
		void operator()(const typename RenderableType::vertex_type & vin, typename RenderableType::vertex_type & vout, vertex_processing_control<RenderableType> & vcontrol){
			const glm::vec4 & posIn = VA_ATTRIBUTE(vin, POSITION);
			glm::vec4 & posOut = VA_ATTRIBUTE(vout, POSITION);

			posOut = mvp_mat * posIn;
			// clip coordinates

			vcontrol.translate_to_window_space(posOut);
			// translate to window space

			vcontrol.viewport_clip(posOut);
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

		//! Reference to context
		render_context & context;

		//! Initialize by referencing the wrapped shader
		vertex_processor_kernel(shader_type & _shader, const renderable_type & _object, render_context & _context)
		:
			shader(_shader),
			object(_object),
			context(_context)
		{}

		template<class T>
		void operator()(T & v) {
			vertex_processing_control<renderable_type> vcontrol(object, context, thrust::get<2>(v));
			thrust::get<1>(v) = thrust::get<0>(v);
			shader(thrust::get<0>(v), thrust::get<1>(v), vcontrol);
		}

	};

	//! Process vertices and extract projected on window space
	template<class VertexShader, class RenderableType>
	void process_vertices(RenderableType & object, VertexShader & shader, render_context & context) {

		// Prepare object
		object.prepare_for_rendering();

		// Process vertices
		size_t total_vertices = object.vertices.size();
		thrust::counting_iterator<vertex_id_t> count_begin(0);
		thrust::for_each(
			thrust::make_zip_iterator(thrust::make_tuple(object.vertices.cbegin(), object.intermediate_buffer.processed_vertices.begin(), count_begin)),
			thrust::make_zip_iterator(thrust::make_tuple(object.vertices.cend(), object.intermediate_buffer.processed_vertices.end(), count_begin + total_vertices)),
			vertex_processor_kernel<VertexShader, RenderableType>(shader, object, context));		// Operation
	}
}
