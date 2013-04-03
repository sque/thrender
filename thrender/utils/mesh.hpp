#pragma once

#include "../mesh.hpp"

namespace thrender {
namespace utils {

	//! Helper function to load a mesh from file
	/**
	 * @param fname The file name that holds model data
	 * @note This function uses assimp to parse files. You can consult your
	 * installed assimp version to see what file formats are supported.
	 */
	mesh load_model(const std::string & fname);

}
}
