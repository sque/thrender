#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thrust/host_vector.h>
#include <thrust/iterator/zip_iterator.h>
#include <iostream>
#include <boost/tuple/tuple.hpp>
#include "./triangle.hpp"

namespace thrender {


	//! A descriptive class of vertex_array datatype
	/**
	 * Vertex array is the unit to hold all vertex attributes
	 * for a rendable object.
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

		//! Get the total number of attributes
		inline static size_t total_attributes() {
			return thrust::tuple_size<vertex_type>::value;
		}
	};

#define VA_VERTEX_BEGIN() \
	thrust::tuple<
#define VA_ATTRIBUsTE(type) \
	type,
#define VA_VERTEX_END(type_name) \
	> type_name;

//! Helper macro to access a given attribute on vertex
#define VA_ATTRIBUTE(attr_list, id) \
	thrust::get<id>(attr_list)

//! The first attribute of all vertices is position
#define POSITION 0
#define NORMAL 1
#define COLOR 2
#define UV 3
};
