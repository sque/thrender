#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thrust/host_vector.h>
#include <thrust/iterator/zip_iterator.h>
#include <iostream>
#include <boost/tuple/tuple.hpp>
#include "./triangle.hpp"

namespace thrender {

namespace details {
	//! Buffer needed per vertex for intermediate processing
	template<class VertexArrayType>
	struct vertex_array_intermediate_buffer {

		//! Type of vertex_array object
		typedef VertexArrayType vertex_array_type;

		//! Type of a vertex in array
		typedef typename vertex_array_type::vertex_type vertex_type;

		//! Type of discarded vertices
		typedef thrust::host_vector<bool> discarded_vertices_type;

		//! Type of triangle
		typedef triangle<vertex_type> triangle_type;

		//! Type of triangle vector
		typedef thrust::host_vector< triangle_type > triangles_vector;

		//! A vector with all processed vertices
		vertex_array_type::vertices_type processed_vertices;

		//! A bitmap with all discarded vertices
		discarded_vertices_type discarded_vertices;

		//! A vector with all mesh triangles of projected vectors
		triangles_vector triangles;

		void reset() {
			discarded_vertices.clear();
			discarded_vertices.resize(processed_vertices.size(), false);
		}

		void post_update(vertex_array_type & m, size_t total_vertices, thrust::host_vector<glm::ivec3> & itriangles) {
			processed_vertices.resize(total_vertices);
			discarded_vertices.resize(total_vertices);
			triangles.clear();

			thrust::host_vector<glm::ivec3>::iterator it_index;
			for(it_index = itriangles.begin();it_index != itriangles.end(); it_index++) {
				glm::ivec3 & indices = *it_index;
				triangles.push_back(
					triangle<vertex_type>(
						processed_vertices,
						indices.x, indices.y, indices.z,
						false)
					);
			}
		}
	};
}; //! details

	//! An array of vertices with each attributes
	/**
	 * Vertex array is the unit to hold all vertices and
	 * each attribute for a mesh object.
	 *
	 * @param VertexAttributesTuple A thrust::tuple<> that holds
	 * all attributes per vertex.
	 */
	template<class VertexAttributesTuple>
	struct vertex_array {

		//! The type of vertex
		typedef VertexAttributesTuple vertex_type;

		//! The type of vector that hold all vertices
		typedef thrust::host_vector<vertex_type> vertices_type;

		//! All vertices packed together
		vertices_type vertices;

		//! Construct and initialize the vector array
		vertex_array(size_t vectors_sz)
		:
			vertices(vectors_sz)
		{}

		//! Get the total number of vertices
		size_t total_vertices() const{
			return vertices.size();
		}

		//! Get the total number of attributes
		static size_t total_attributes() {
			return thrust::tuple_size<vertex_type>::value;
		}
	};

	template<class VertexAttributesTuple>
	struct mesh {

		//! Type of vertex attributes type
		typedef VertexAttributesTuple attributes_type;

		//! Type of vertex data
		typedef vertex_array<attributes_type> vertex_array_type;

		// Intermediate render buffer
		details::vertex_array_intermediate_buffer< vertex_array_type > render_buffer;

		//! Primitive data type (triangle)
		typedef triangle<attributes_type> triangle_type;

		//! Construct a new mesh, uninitialized
		/**
		 * All the storage will be reserved at construction time.
		 * @param vertices_sz Total vertices that this mesh has
		 * @param elements_sz Total elements that this mesh has
		 */
		mesh(size_t vertices_sz, size_t elements_sz) :
			vertex_data(vertices_sz),
			element_indices(elements_sz)
		{}

		//! Vertex data of this mesh
		vertex_array_type vertex_data;

		//! Indices of element vertices
		thrust::host_vector<glm::ivec3> element_indices;

	};

//! Helper macro to access a given attribute on vertex
#define ATTRIBUTE(attr_list, id) \
	thrust::get<id>(attr_list)

//! The first attribute of all vertices is position
#define POSITION 0
#define NORMAL 1
#define COLOR 2
#define UV 3
};
