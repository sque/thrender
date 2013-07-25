#pragma once

#include <glm/glm.hpp>
#include <boost/cstdint.hpp>

namespace thrender {

	//! Type of window dimension size
	typedef unsigned short window_size_t;

	//! Type of vertex id
	typedef unsigned short vertex_id_t;

	//! Type of pitch
	typedef unsigned short pitch_t;

	//! Type of depth pixel
	typedef double depth_pixel_t;

	//! Type of 3 part indices
	typedef glm::uvec3 indices3_t;

	//! Maximum supported framebuffer height
	static const size_t max_framebuffer_height = 1024;

	//! Maximum supported framebuffer width
	static const size_t max_framebuffer_width = 1024;


}
