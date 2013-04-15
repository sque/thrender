#pragma once

#include "../vertex_array.hpp"
#include <sstream>
#include <glm/gtx/string_cast.hpp>

namespace thrender {
namespace utils {

	template<class A>
	inline std::string to_string(const vertex_array<A> & m) {
		std::stringstream ss;
		ss << "VertexArray[Vertices: " << m.total_vertices() << ", Triangles:" << m.triangles.size() << "]";
		return ss.str();
	}


}}
