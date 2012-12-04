#pragma once

#include <thrust/host_vector.h>
#include <glm/glm.hpp>

namespace thrender {

	//! Implementation of a naive G-Buffer
	struct gbuffer {

		typedef thrust::host_vector<glm::vec4> diffuse;
		typedef thrust::host_vector<glm::vec3> normal;
		typedef thrust::host_vector<float> depth;

		unsigned width;
		unsigned height;

		diffuse buf_diffuse;
		normal buf_normal;
		depth buf_depth;

		diffuse clear_diffuse;
		normal clear_normal;
		depth clear_depth;

		gbuffer(unsigned w, unsigned h) :
			width(w),
			height(h),
			buf_diffuse(width * height),
			buf_normal(width * height),
			buf_depth(width * height),
			clear_diffuse(buf_diffuse.size()),
			clear_normal(buf_normal.size()),
			clear_depth(buf_depth.size()){

			set_clear_values(
				glm::vec4(1.0f,1.0f,1.0f,1.0f),
				glm::vec3(0.0f,0.0f,0.0f),
				0);
		}

		void set_clear_values(const glm::vec4 & diff_value, const glm::vec3 & norm_value, float depth_value) {
			set_clear_diffuse(diff_value);
			set_clear_normal(norm_value);
			set_clear_depth(depth_value);
		}

		void set_clear_diffuse(const glm::vec4 & diff_value) {
			thrust::fill(clear_diffuse.begin(), clear_diffuse.end(), diff_value);
		}

		void set_clear_normal(const glm::vec3 & norm_value) {
			thrust::fill(clear_normal.begin(), clear_normal.end(), norm_value);
		}

		void set_clear_depth(float depth_value) {
			thrust::fill(clear_depth.begin(), clear_depth.end(), depth_value);
		}
		void clear() {
			// 114ns fill, 64ns copy
			thrust::copy(clear_diffuse.begin(), clear_diffuse.end(), buf_diffuse.begin());
			thrust::copy(clear_normal.begin(), clear_normal.end(), buf_normal.begin());
			thrust::copy(clear_depth.begin(), clear_depth.end(), buf_depth.begin());
			/*thrust::fill(buf_normal.begin(), buf_normal.end(), glm::vec3(0.0,0.0,0.0));
			thrust::fill(buf_depth.begin(), buf_depth.end(), 0);
			thrust::fill(buf_diffuse.begin(), buf_diffuse.end(), glm::vec4(0.0f,0.0f,0.0,1.0));
			*/
		}
	};

}
