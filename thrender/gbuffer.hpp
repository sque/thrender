#pragma once

#include <thrust/host_vector.h>
#include "./framebuffer_array.hpp"

namespace thrender {

	typedef size_t coord2d_t;

	//! Convert 2D coordinates to linear 1D (assuming pitch == max(Y)))
	template<class T>
	inline coord2d_t coord2d(T x, T y) {
		return (((y) * 640) + (x));
	}

	//! Implementation of a naive G-Buffer
	struct gbuffer {

		typedef framebuffer_<glm::vec4> diffuse_vector;
		typedef framebuffer_<glm::vec4> normal_vector;
		typedef framebuffer_<double> depth_vector;

		unsigned width;
		unsigned height;

		diffuse_vector diffuse;
		normal_vector normal;
		depth_vector depth;

		diffuse_vector::value_type clear_diffuse;
		normal_vector::value_type clear_normal;
		depth_vector::value_type clear_depth;

		gbuffer(unsigned w, unsigned h) :
			width(w),
			height(h),
			diffuse(width, height),
			normal(width, height),
			depth(width, height){

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
			clear_diffuse = diff_value;
		}

		void set_clear_normal(const normal_vector::value_type & norm_value) {
			clear_normal = norm_value;
		}

		void set_clear_depth(const depth_vector::value_type & depth_value) {
			clear_depth = depth_value;
		}
		void clear() {
			// 114ms fill (Debug) ~5ms (Release)
			thrust::fill(normal.begin(), normal.end(), clear_normal);
			thrust::fill(diffuse.begin(), diffuse.end(), clear_diffuse);
			thrust::fill(depth.begin(), depth.end(), clear_depth);
		}
	};

}
