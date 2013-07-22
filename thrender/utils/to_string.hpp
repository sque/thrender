#pragma once

#include "../renderable.hpp"
#include <sstream>
#include <glm/gtx/string_cast.hpp>

namespace thrender {
namespace utils {

	template<class A>
	inline std::string to_string(const renderable<A> & m) {
		std::stringstream ss;
		ss << "Renderable[Vertices: " << m.vertices.size() << ", Triangles:" << m.element_indices.size() << "]";
		return ss.str();
	}


}}
