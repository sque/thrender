#include "mesh.hpp"
#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> 		// Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>
#include <boost/random.hpp>

namespace thrender {
namespace utils {

	mesh load_model(const std::string & fname) {
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
		mesh outm;
		const aiMesh * m = scene->mMeshes[0];
		outm.resize(m->mNumVertices, m->mNumFaces);

		boost::random::mt19937 rng;
		boost::random::uniform_real_distribution<> one(0,1);

		for(unsigned i = 0;i < m->mNumVertices;i++){
			outm.attributes.positions[i] = glm::vec4(glm::make_vec3(&m->mVertices[i].x),1);
			outm.attributes.normals[i] =  glm::vec4(glm::make_vec3(&m->mNormals[i].x),1);
			if (m->HasVertexColors(0))
				outm.attributes.colors[i] = glm::make_vec4(&m->mColors[i]->r);
			else
				outm.attributes.colors[i] = glm::vec4(one(rng), one(rng), one(rng), 1.0f);
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
