#pragma once

#include "mesh.hpp"
#include <sstream>
#include <glm/gtx/string_cast.hpp>

namespace thrender {
namespace utils {



	inline std::string to_string(const mesh & m) {
		std::stringstream ss;
		ss << "Mesh[Vertices: " << m.total_vertices() << ", Triangles:" << m.triangles.size() << "]";
		return ss.str();
	}


}}
