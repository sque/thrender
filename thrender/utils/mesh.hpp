#pragma once

#include "../vertex_array.hpp"
#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> 		// Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>
#include <boost/random.hpp>

namespace thrender {
namespace utils {

	//! Helper function to load a mesh from file
	/**
	 * @param fname The file name that holds model data
	 * @note This function uses assimp to parse files. You can consult your
	 * installed assimp version to see what file formats are supported.
	 *
	 * Order of attributes is:
	 *  - POSITION (Homogenous vec4)
	 *  - NORMAL (Homogenous vec4)
	 *  - COLOR (RGBA vec4)
	 *  - UV CORDS (vec2)
	 */
	template<class MeshType>
	MeshType load_model(const std::string & fname) {
		// Create an instance of the Importer class
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile( fname.c_str(),
				aiProcess_CalcTangentSpace |
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices |
				aiProcess_SortByPType);
		// If the import failed, report it
		if( !scene)	{
			throw std::runtime_error(importer.GetErrorString());
		}

		if ( scene->mNumMeshes != 1) {
			throw std::runtime_error(importer.GetErrorString());
		}

		// Load mesh from file

		const aiMesh * m = scene->mMeshes[0];
		MeshType outm(m->mNumVertices, m->mNumFaces);
		MeshType::vertex_array_type
		boost::random::mt19937 rng;
		boost::random::uniform_real_distribution<> one(0,1);

		for(unsigned i = 0;i < m->mNumVertices;i++){
			VA_ATTRIBUTE(outm.attributes[i], POSITION) = glm::vec4(glm::make_vec3(&m->mVertices[i].x),1);
			VA_ATTRIBUTE(outm.attributes[i], NORMAL) =  glm::vec4(glm::make_vec3(&m->mNormals[i].x),1);
			if (m->HasVertexColors(0))
				VA_ATTRIBUTE(outm.attributes[i], COLOR) = glm::make_vec4(&m->mColors[i]->r);
			else
				VA_ATTRIBUTE(outm.attributes[i], COLOR) = glm::vec4(one(rng), one(rng), one(rng), 1.0f);
		}

		for(unsigned i = 0;i < m->mNumFaces;i++){
			outm.triangles[i] = glm::ivec3(
				m->mFaces[i].mIndices[0],
				m->mFaces[i].mIndices[1],
				m->mFaces[i].mIndices[2]
				);
			}

		outm.post_update();
		return outm;
	}
}}
