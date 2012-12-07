#pragma once

namespace thrender {

//! Structure to hold (convex) polygon horizontal limits
	struct polygon_horizontal_limits {

		int leftmost[480];

		int rightmost[480];

		polygon_horizontal_limits() {
			for (int i = 0; i < 480; i++) {
				leftmost[i] = 10000;
				rightmost[i] = 0;
			}
		}
	};

	//! Functor to find polygon vertical contour
	struct mark_vertical_contour {

		polygon_horizontal_limits & limits;

		mark_vertical_contour(polygon_horizontal_limits & _limits) :
				limits(_limits) {
		}

		bool operator()(int x, int y) {
			if (x < limits.leftmost[y])
				limits.leftmost[y] = x;
			if (x > limits.rightmost[y])
				limits.rightmost[y] = x;
			return true;
		}
	};
}
