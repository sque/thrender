#pragma once

#include <thrust/host_vector.h>
#include <glm/glm.hpp>

namespace thrender {

	typedef size_t coord2d_t;

	//! Convert 2D coordinates to linear 1D (assuming pitch == max(Y)))
	template<class T>
	inline coord2d_t coord2d(T x, T y) {
		return (((y) * 640) + (x));
	}

	//! Implementation of a naive G-Buffer
	struct gbuffer {

		typedef thrust::host_vector<glm::vec4> diffuse_vector;
		typedef thrust::host_vector<glm::vec4> normal_vector;
		typedef thrust::host_vector<float> depth_vector;

		unsigned width;
		unsigned height;

		diffuse_vector diffuse;
		normal_vector normal;
		depth_vector depth;

		diffuse_vector clear_diffuse;
		normal_vector clear_normal;
		depth_vector clear_depth;

		gbuffer(unsigned w, unsigned h) :
			width(w),
			height(h),
			diffuse(width * height),
			normal(width * height),
			depth(width * height),
			clear_diffuse(diffuse.size()),
			clear_normal(normal.size()),
			clear_depth(depth.size()){

			set_clear_values(
				glm::vec4(1.0f,1.0f,1.0f,1.0f),
				glm::vec4(0.0f,0.0f,0.0f, 1.0f),
				0);
		}

		void set_clear_values(const diffuse_vector::value_type & diff_value, const normal_vector::value_type & norm_value, const depth_vector::value_type & depth_value) {
			set_clear_diffuse(diff_value);
			set_clear_normal(norm_value);
			set_clear_depth(depth_value);
		}

		void set_clear_diffuse(const diffuse_vector::value_type & diff_value) {
			thrust::fill(clear_diffuse.begin(), clear_diffuse.end(), diff_value);
		}

		void set_clear_normal(const normal_vector::value_type & norm_value) {
			thrust::fill(clear_normal.begin(), clear_normal.end(), norm_value);
		}

		void set_clear_depth(const depth_vector::value_type & depth_value) {
			thrust::fill(clear_depth.begin(), clear_depth.end(), depth_value);
		}
		void clear() {
			// ~5 ms (Debug & Release) !
			/*memcpy(&diffuse[0], &clear_diffuse[0], clear_diffuse.size()*sizeof(diffuse_vector::value_type));
			memcpy(&normal[0], &clear_normal[0], clear_normal.size()*sizeof(normal_vector::value_type));
			memcpy(&depth[0], &clear_depth[0], clear_depth.size()*sizeof(depth_vector::value_type));
			*/
			// 64ms copy (Debug) ~7ms (Release)
			/*thrust::copy(clear_diffuse.begin(), clear_diffuse.end(), diffuse.begin());
			thrust::copy(clear_normal.begin(), clear_normal.end(), normal.begin());
			thrust::copy(clear_depth.begin(), clear_depth.end(), depth.begin());
			*/
			// 114ms fill (Debug) ~5ms (Release)
			thrust::fill(normal.begin(), normal.end(), clear_normal[0]);
			thrust::fill(diffuse.begin(), diffuse.end(), clear_diffuse[0]);
			thrust::fill(depth.begin(), depth.end(), clear_depth[0]);
		}
	};

}
