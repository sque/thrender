#pragma once

#include <math.h>
#include <glm/glm.hpp>

namespace thrender{
namespace math{

	//! Line walking operator using Bresenham algorithm
	/**
	 * http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	 * @param x1 Starting point x-component
	 * @param y1 Starting point y-component
	 * @param x2 Ending point x-component
	 * @param y2 Ending point y-component
	 */
	template<class Functor>
	double line_bresenham(int x1, int y1, int x2, int y2, Functor action)
	{
		int dx, dy, i, e;
		int incx, incy, inc1, inc2;
		int x, y;
		double res = 0;

		dx = x2 - x1;
		dy = y2 - y1;

		if (dx < 0)
			dx = -dx;
		if (dy < 0)
			dy = -dy;
		incx = 1;
		if (x2 < x1)
			incx = -1;
		incy = 1;
		if (y2 < y1)
			incy = -1;
		x = x1;
		y = y1;

		if (dx > dy) {
			if (!action(x, y))
				return res;

			e = 2 * dy - dx;
			inc1 = 2 * (dy - dx);
			inc2 = 2 * dy;
			for (i = 0; i < dx; i++) {
				if (e >= 0) {
					y += incy;
					e += inc1;
				} else
					e += inc2;
				x += incx;
				if (!action(x, y))
					return res;
			}
		} else {
			if (!action(x, y))
				return res;

			e = 2 * dx - dy;
			inc1 = 2 * (dx - dy);
			inc2 = 2 * dx;
			for (i = 0; i < dy; i++) {
				if (e >= 0) {
					x += incx;
					e += inc1;
				} else
					e += inc2;
				y += incy;
				if (!action(x, y))
					return res;
			}
		}
		return 0;
	}

	/**
	 * @origin http://www.gamedev.net/topic/621445-barycentric-coordinates-c-code-check/
	 */
	template <typename InVec, typename InVec2>
	inline glm::vec4 barycoords(InVec const & a, InVec const & b, InVec const & c, InVec2 const & vec)
	{
		glm::vec4 lambda;
		float den = 1 / ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));

		lambda.x = ((b.y - c.y) * (vec.x - c.x) + (c.x - b.x) * (vec.y - c.y)) * den;
		lambda.y = ((c.y - a.y) * (vec.x - c.x) + (a.x - c.x) * (vec.y - c.y)) * den;
		lambda.z = 1 - lambda.x - lambda.y;

		return lambda;
		//return  (a * lambda.x) + (b * lambda.y) + (c * lambda.z);
	}

	//! Sort 3 vectors by y using sorting networks
	/**
	 * @origin http://stackoverflow.com/questions/2786899/fastest-sort-of-fixed-length-6-int-array
	 */
	template<class Vec>
	static inline void sort3vec_by_y(const Vec * d[3]){
		#define sort_min_y(v1, v2) (v1->y < v2->y?v1:v2)
		#define sort_max_y(v1, v2) (v1->y < v2->y?v2:v1)
		#define sort_swap(iv1, iv2) { \
			const Vec * a = sort_min_y(d[iv1], d[iv2]); \
			const Vec * b = sort_max_y(d[iv1], d[iv2]); \
				d[iv1] = a; d[iv2] = b; \
			}
			sort_swap(2, 1);
			sort_swap(2, 0);
			sort_swap(1, 0);

		#undef sort_swap
		#undef sort_max_y
		#undef sort_min_y
	}

	//! Sort 3 points by y
	static inline void sort3_v1(const glm::vec4 * pord[3], const glm::vec4 * const pin[3]){

		// Sort points by y
		pord[0] = pin[0];
		pord[1] = pin[1];
		pord[2] = pin[2];
		for (int i = 0; i < 3; i++) {
			if (pin[i]->y > pord[0]->y) {
				pord[0] = pin[i];
			} else if (pin[i]->y < pord[2]->y) {
				pord[2] = pin[i];
			}
		}

		for (int i = 0; i < 3; i++)
			if ((pord[0] != pin[i]) && (pord[2] != pin[i])) {
				pord[1] = pin[i];
				break;
			}
	}
}}
