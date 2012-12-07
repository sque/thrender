#pragma once

#include <thrust/host_vector.h>
#include <glm/glm.hpp>
#include "./math.hpp"

namespace thrender {

	struct mesh;

	// Triangle primitive
	struct triangle {

		size_t indices[3];

		const glm::vec4 * pv[3];
		const thrust::host_vector<glm::vec4> (*proj_vertices);
		const mesh * m;

		struct {
			bool discard;
		} flags;

		triangle(mesh & _m,
				const thrust::host_vector<glm::vec4> & _proj_vertices,
				size_t iv0,
				size_t iv1,
				size_t iv2,
				bool _discard) :
				proj_vertices(&_proj_vertices), m(&_m) {
			indices[0] = iv0;
			indices[1] = iv1;
			indices[2] = iv2;
			pv[0] = &(*proj_vertices)[iv0];
			pv[1] = &(*proj_vertices)[iv1];
			pv[2] = &(*proj_vertices)[iv2];
			flags.discard = _discard || !ccw_winding_order();
		}

		triangle() {}

		//! Check if triangle has Counter-Clock-Wise winding order
		bool ccw_winding_order() const{
			float adiff = atan2f(pv[1]->y - pv[0]->y, pv[1]->x - pv[0]->x)
					- atan2f(pv[2]->y - pv[0]->y, pv[2]->x - pv[0]->x);
			if (adiff < 0)
				adiff = (M_PI * 2) + adiff;
			if (adiff < M_PI) {
				return false;
			}
			return true;
		}

		/*inline triangle & operator=(const triangle & tr) {
			memcpy(this, &tr, sizeof(triangle));
			return *this;
		}*/
	};

}
