#pragma once

#include <thrust/host_vector.h>
#include <glm/glm.hpp>
#include "./math.hpp"

namespace thrender {

	template<class A>
	struct vertex_array;

	//! Triangle primitive
	template <class VertexType>
	struct triangle {

		//! Type of vertex
		typedef VertexType vertex_type;

		//! Vector indices
		size_t indices[3];

		//! Pointer to position attributes
		const glm::vec4 * pv[3];

		//! Pointer to processed vertices list
		const thrust::host_vector<vertex_type> (*processed_vertices);

		//! Triangle flags
		struct {
			bool discarded;
		} flags;

		//! Construct a new triangle
		triangle(const thrust::host_vector<vertex_type> & _processed_vertices,
				size_t iv0,
				size_t iv1,
				size_t iv2,
				bool _discard) :
				processed_vertices(&_processed_vertices){
			indices[0] = iv0;
			indices[1] = iv1;
			indices[2] = iv2;
			pv[0] = &thrust::get<0>((*processed_vertices)[iv0]);
			pv[1] = &thrust::get<0>((*processed_vertices)[iv1]);
			pv[2] = &thrust::get<0>((*processed_vertices)[iv2]);
			flags.discarded = _discard || !is_ccw_winding_order();
		}

		triangle() {}

		//! Check if triangle has Counter-Clock-Wise winding order
		bool is_ccw_winding_order() const{
			float adiff = atan2f(pv[1]->y - pv[0]->y, pv[1]->x - pv[0]->x)
					- atan2f(pv[2]->y - pv[0]->y, pv[2]->x - pv[0]->x);
			if (adiff < 0)
				adiff = (M_PI * 2) + adiff;
			if (adiff < M_PI) {
				return false;
			}
			return true;
		}

		//! Calculate the size of the smallest bounding box that fits this triangle
		glm::vec2 bounding_box() const {
			float x_max = std::max(std::max(pv[0]->x, pv[1]->x), pv[2]->x);
			float x_min = std::min(std::min(pv[0]->x, pv[1]->x), pv[2]->x);

			float y_max = std::max(std::max(pv[0]->y, pv[1]->y), pv[2]->y);
			float y_min = std::min(std::min(pv[0]->y, pv[1]->y), pv[2]->y);
			return glm::vec2(x_max-x_min, y_max-y_min);
		}
	};

}
