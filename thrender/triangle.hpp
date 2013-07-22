#pragma once

#include <glm/glm.hpp>
#include "./math.hpp"
#include "./vertex_array.hpp"

namespace thrender {

	//! Triangle primitive
	template <class VertexType>
	struct triangle {

		//! Type of vertex
		typedef VertexType vertex_type;

		//! Vector indices
		indices3_t indices;

		//! Pointer to position attributes
		const glm::vec4 * positions[3];

		//! Pointer to processed vertices list
		const thrust::host_vector<vertex_type> (*processed_vertices);

		//! Triangle flags
		struct {
			bool discarded;
		} flags;

		//! Construct a new triangle
		triangle(const thrust::host_vector<vertex_type> & _processed_vertices, indices3_t _indices, bool _discard) :
				indices(_indices),
				processed_vertices(&_processed_vertices){
			positions[0] = &thrust::get<0>((*processed_vertices)[indices[0]]);
			positions[1] = &thrust::get<0>((*processed_vertices)[indices[1]]);
			positions[2] = &thrust::get<0>((*processed_vertices)[indices[2]]);
		}

		triangle() {}

		//! Check if triangle has Counter-Clock-Wise winding order
		bool is_ccw_winding_order() const{
			float adiff = atan2f(positions[1]->y - positions[0]->y, positions[1]->x - positions[0]->x)
					- atan2f(positions[2]->y - positions[0]->y, positions[2]->x - positions[0]->x);
			if (adiff < 0)
				adiff = (M_PI * 2) + adiff;
			if (adiff < M_PI) {
				return false;
			}
			return true;
		}

		//! Calculate the size of the smallest bounding box that fits this triangle
		glm::vec2 bounding_box() const {
			float x_max = std::max(std::max(positions[0]->x, positions[1]->x), positions[2]->x);
			float x_min = std::min(std::min(positions[0]->x, positions[1]->x), positions[2]->x);

			float y_max = std::max(std::max(positions[0]->y, positions[1]->y), positions[2]->y);
			float y_min = std::min(std::min(positions[0]->y, positions[1]->y), positions[2]->y);
			return glm::vec2(x_max-x_min, y_max-y_min);
		}
	};

}
