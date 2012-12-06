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

		typedef thrust::host_vector<glm::vec4> diffuce_vector;
		typedef thrust::host_vector<glm::vec4> normal_vector;
		typedef thrust::host_vector<float> depth_vector;

		unsigned width;
		unsigned height;

		diffuce_vector diffuse;
		normal_vector normal;
		depth_vector depth;

		diffuce_vector clear_diffuse;
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

		void set_clear_values(const diffuce_vector::value_type & diff_value, const normal_vector::value_type & norm_value, const depth_vector::value_type & depth_value) {
			set_clear_diffuse(diff_value);
			set_clear_normal(norm_value);
			set_clear_depth(depth_value);
		}

		void set_clear_diffuse(const diffuce_vector::value_type & diff_value) {
			thrust::fill(clear_diffuse.begin(), clear_diffuse.end(), diff_value);
		}

		void set_clear_normal(const normal_vector::value_type & norm_value) {
			thrust::fill(clear_normal.begin(), clear_normal.end(), norm_value);
		}

		void set_clear_depth(const depth_vector::value_type & depth_value) {
			thrust::fill(clear_depth.begin(), clear_depth.end(), depth_value);
		}
		void clear() {
			// 114ns fill, 64ns copy
			thrust::copy(clear_diffuse.begin(), clear_diffuse.end(), diffuse.begin());
			thrust::copy(clear_normal.begin(), clear_normal.end(), normal.begin());
			thrust::copy(clear_depth.begin(), clear_depth.end(), depth.begin());
			/*thrust::fill(buf_normal.begin(), buf_normal.end(), glm::vec3(0.0,0.0,0.0));
			thrust::fill(buf_depth.begin(), buf_depth.end(), 0);
			thrust::fill(buf_diffuse.begin(), buf_diffuse.end(), glm::vec4(0.0f,0.0f,0.0,1.0));
			*/
		}
	};

}
