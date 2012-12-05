#pragma once

#include <thrust/host_vector.h>
#include <glm/glm.hpp>
#include "./mesh.hpp"
#include "./math.hpp"

namespace thrender {

	// Triangle primitive
	struct triangle {

		size_t indices[3];

		glm::vec4 v[3];
		const thrust::host_vector<glm::vec4> (*proj_vertices);
		const thrender::mesh * m;

		struct {
			bool discard;
		} flags;

		triangle(const mesh & _m,
				const thrust::host_vector<glm::vec4> & _proj_vertices,
				size_t iv0,
				size_t iv1,
				size_t iv2,
				bool _discard) :
				proj_vertices(&_proj_vertices), m(&_m) {
			indices[0] = iv0;
			indices[1] = iv1;
			indices[2] = iv2;
			v[0] = (*proj_vertices)[iv0];
			v[1] = (*proj_vertices)[iv1];
			v[2] = (*proj_vertices)[iv2];
			flags.discard = _discard || !ccw_winding_order();
		}

		triangle() {}

		//! Check if triangle has Counter-Clock-Wise winding order
		bool ccw_winding_order() {
			float adiff = atan2f(v[1].y - v[0].y, v[1].x - v[0].x)
					- atan2f(v[2].y - v[0].y, v[2].x - v[0].x);
			if (adiff < 0)
				adiff = (M_PI * 2) + adiff;
			if (adiff < M_PI) {
				return false;
			}
			return true;
		}
	};

}
