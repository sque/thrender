#pragma once

#include <math.h>
#include <glm/glm.hpp>

namespace thrender {
namespace math{

	// taken from http://www.gamedev.net/topic/621445-barycentric-coordinates-c-code-check/
	template <typename InVec, typename InVec2>
	glm::vec4 barycoords(InVec const & a, InVec const & b, InVec const & c, InVec2 const & vec)
	{
		glm::vec4 lambda;
		float den = 1 / ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));

		lambda.x = ((b.y - c.y) * (vec.x - c.x) + (c.x - b.x) * (vec.y - c.y)) * den;
		lambda.y = ((c.y - a.y) * (vec.x - c.x) + (a.x - c.x) * (vec.y - c.y)) * den;
		lambda.z = 1 - lambda.x - lambda.y;

		return lambda;
		//return  (a * lambda.x) + (b * lambda.y) + (c * lambda.z);
	}

}}
