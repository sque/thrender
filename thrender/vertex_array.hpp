#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thrust/host_vector.h>
#include <thrust/iterator/zip_iterator.h>
#include <iostream>
#include <boost/tuple/tuple.hpp>
#include "./triangle.hpp"

namespace thrender {

	//! Buffer needed per vertex for intermediate processing
	template<class VertexArrayType>
	struct vertex_array_intermediate_buffer {

		//! Type of vertex_array object
		typedef VertexArrayType vertex_array_type;

		//! Type of vertex_array attributes
		typedef typename vertex_array_type::attributes_type attributes_type;

		//! Type of processed vertices vector
		typedef thrust::host_vector<attributes_type> processed_vertices_type;

		//! Type of discarded vertices
		typedef thrust::host_vector<bool> discarded_vertices_type;

		typedef triangle<attributes_type> triangle_type;

		//! Type of
		typedef thrust::host_vector< triangle_type > triangles_vector;

		//! A vector with all processed vertices
		processed_vertices_type processed_vertices;

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
					triangle<attributes_type>(
						processed_vertices,
						indices.x, indices.y, indices.z,
						false)
					);
			}
		}
	};

	//! An array of vertices with each attributes
	/**
	 * Vertex array is the unit to hold all vertices and
	 * each attribute for a mesh object.
	 *
	 * @param VertexAttributesTuple A thrust::tuple<> that holds
	 * all attributes per vertex.
	 *
	 * @note Triangle should be separated, and permit rendering
	 * of other primitives too.
	 */
	template<class VertexAttributesTuple>
	struct vertex_array {

		//! The type of vertex attribute
		typedef VertexAttributesTuple attributes_type;

		//! The type of vector that hold all vertices
		typedef thrust::host_vector<attributes_type> attributes_vector_type;

		//! All attributes packed together
		attributes_vector_type attributes;

		// Intermediate render buffer
		vertex_array_intermediate_buffer< vertex_array<attributes_type> > render_buffer;

		// Triangle indices
		thrust::host_vector<glm::ivec3> triangles;

		//! Primitive data type (triangle)
		typedef triangle<attributes_type> primitive_type;

		//! Resize the array vector
		void resize(size_t vectors_sz, size_t triangles_sz) {
			attributes.resize(vectors_sz);
			triangles.resize(triangles_sz);
		}

		//! Get the total number of vertices
		size_t total_vertices() const{
			return attributes.size();
		}

		//! Get the total number of triangles
		size_t total_triangles() const {
			return triangles.size();
		}

		//! Get the total number of attributes
		static size_t total_attributes() {
			return thrust::tuple_size<attributes_type>::value;
		}

		//! Action that must be called if mesh data are updated
		void post_update() {
			render_buffer.post_update(*this, total_vertices(), triangles);
		}
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
