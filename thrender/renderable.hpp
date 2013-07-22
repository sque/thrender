#pragma once

#include "./vertex_array.hpp"
#include "./triangle.hpp"

namespace thrender{

namespace details {

	//! Buffer needed per vertex for intermediate processing
	template<class VertexArrayType, class PrimitiveType>
	struct rendable_intermediate_buffer {

		//! Type of vertex_array object
		typedef VertexArrayType vertex_array_type;

		//! Type of discarded vertices
		typedef thrust::host_vector<bool> discarded_vertices_type;

		//! Type of primitive
		typedef PrimitiveType primitive_type;

		//! Type of elements container
		typedef thrust::host_vector< primitive_type > elements_type;

		//! A vector with all processed vertices
		typename vertex_array_type::vertices_type processed_vertices;

		//! A bitmap with all discarded vertices
		discarded_vertices_type discarded_vertices;

		//! A vector with all mesh elements of projected vectors
		elements_type elements;

		//! Clear and prepare intermediate buffer for rendering.
		void clear(size_t vertices_sz, size_t elements_sz) {
			processed_vertices.resize(vertices_sz);
			discarded_vertices.resize(vertices_sz, false);
			thrust::fill(discarded_vertices.begin(), discarded_vertices.end(), false);
		}

		void rebuild(size_t vertices_sz , thrust::host_vector<indices3_t> & itriangles) {
			processed_vertices.resize(vertices_sz);
			discarded_vertices.resize(vertices_sz);
			elements.clear();

			thrust::host_vector<indices3_t>::iterator it_index;
			for(it_index = itriangles.begin();it_index != itriangles.end(); it_index++) {
				indices3_t & indices = *it_index;
				elements.push_back( primitive_type(processed_vertices, indices, false) );
			}
		}
	};
}; //! details

	template<class VertexAttributesTuple>
	struct renderable {

		//! The type of vertex
		typedef VertexAttributesTuple vertex_type;

		//! Vertex array type
		typedef vertex_array<vertex_type> vertex_array_type;

		//! Primitive data type (triangle)
		typedef triangle<vertex_type> triangle_type;

		//! All vertices packed together
		typename vertex_array_type::vertices_type vertices;

		// Intermediate render buffer
		details::rendable_intermediate_buffer< vertex_array_type, triangle_type> intermediate_buffer;

		//! Indices of vertices per element
		thrust::host_vector<indices3_t> element_indices;

		//! Construct a new renderable object, uninitialized
		/**
		 * All the storage will be reserved at construction time.
		 * @param vertices_sz Total vertices that this renderable object has.
		 * @param elements_sz Total elements that this renderable object has.
		 */
		renderable(size_t vertices_sz, size_t elements_sz)
		:
			vertices(vertices_sz),
			element_indices(elements_sz),
			m_is_dirty(true)
		{}

		//! Prepare object for rendering
		/**
		 * @brief This function is called by rendering
		 * pipeline every time before object gets rendered
		 */
		void prepare_for_rendering() {
			if(m_is_dirty) {
				intermediate_buffer.rebuild(vertices.size(), element_indices);
				m_is_dirty = false;
			}
			intermediate_buffer.clear(vertices.size(), element_indices.size());
		}

		//! Mark object's data as changed
		void data_updated() {
			m_is_dirty = true;
		}

	private:

		//! Flag if object data has been changed
		bool m_is_dirty;

	};
}
