#pragma once

#include <limits>
#include <boost/array.hpp>
#include "./types.hpp"
#include "./math.hpp"

namespace thrender {
namespace details {

	//! Structure to hold (convex) polygon vertical limits
	struct polygon_vertical_limits {

		//! Type of array
		typedef boost::array<window_size_t, thrender::max_framebuffer_height> array_type;

		//! Leftmost limits
		array_type leftmost;

		//! Rightmost limits
		array_type rightmost;

		//! Clear limits by setting default values on them
		void clear() {
			thrust::fill(leftmost.begin(), leftmost.end(), std::numeric_limits<window_size_t>::max());
			thrust::fill(rightmost.begin(), rightmost.end(), std::numeric_limits<window_size_t>::min());
		}
	};

	//! Structure to hold (convex) polygon vertical limits
	/*struct polygon_vertical_limits {

		thrust::host_vector<window_size_t> leftmost;

		thrust::host_vector<window_size_t> rightmost;


		polygon_vertical_limits():
			leftmost(480, std::numeric_limits<window_size_t>::max()),
			rightmost(480, std::numeric_limits<window_size_t>::min())
		{}


		void reset(window_size_t height) {

			thrust::fill(leftmost.begin(), leftmost.end(), std::numeric_limits<window_size_t>::max());
			thrust::fill(rightmost.begin(), rightmost.end(), std::numeric_limits<window_size_t>::min());
		}
	};*/


	//! Functor to find polygon vertical contour
	struct mark_vertical_contour {

		polygon_vertical_limits & limits;

		mark_vertical_contour(polygon_vertical_limits & _limits) :
			limits(_limits)
		{}

		bool operator()(window_size_t x, window_size_t y) {
			if (x < limits.leftmost[y])
				limits.leftmost[y] = x;
			if (x > limits.rightmost[y])
				limits.rightmost[y] = x;
			return true;
		}
	};
/*
	template<class PrimitiveType>
	details::polygon_vertical_limits get_primitive_contour(){
		details::polygon_vertical_limits tri_contour;

		tri_contour.clear();
		mark_vertical_contour mark_contour_op(tri_contour);
		thrender::math::line_bresenham(pord[0]->x, pord[0]->y, pord[1]->x,
			pord[1]->y, mark_contour_op);
		thrender::math::line_bresenham(pord[1]->x, pord[1]->y, pord[2]->x,
			pord[2]->y, mark_contour_op);
		thrender::math::line_bresenham(pord[0]->x, pord[0]->y, pord[2]->x,
			pord[2]->y, mark_contour_op);
	}
*/
}
}
