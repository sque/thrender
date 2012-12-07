#pragma once
#include "mesh.hpp"
#include <sstream>

namespace thrender {
namespace utils {

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

	std::string info(const glm::vec2 & v) {
		std::stringstream ss;
		ss << v.x << "," << v.y;
		return ss.str();
	}

	std::string info(const glm::vec3 & v) {
		std::stringstream ss;
		ss << v.x << ", " << v.y << ", " << v.z;
		return ss.str();
	}

	std::string info(const glm::vec4 & v) {
		std::stringstream ss;
		ss << v.x << ", " << v.y << ", " << v.z << ", " << v.z;
		return ss.str();
	}

	std::string info(const mesh & m) {
		std::stringstream ss;
		ss << "Mesh[Vertices: " << m.total_vertices() << ", Triangles:" << m.triangles.size() << "]";
		return ss.str();
	}
}}
