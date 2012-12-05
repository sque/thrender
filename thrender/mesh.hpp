#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thrust/host_vector.h>
#include <thrust/iterator/zip_iterator.h>
#include <iostream>

namespace thrender {

	// A naive mesh representation class
	struct mesh {

		typedef thrust::host_vector<glm::vec4> positions_container;
		typedef thrust::host_vector<glm::vec4> normals_container;
		typedef thrust::host_vector<glm::vec4> colors_container;

		typedef positions_container::iterator positions_iterator;
		typedef normals_container::iterator normals_iterator;
		typedef colors_container::iterator colors_iterator;

		typedef thrust::tuple<positions_iterator, normals_iterator, colors_iterator> attributes_tuple;

		//! Zip iterator for all attributes
		typedef thrust::zip_iterator<attributes_tuple> attributes_iterator;

		// Vertex attributes
		struct {
			positions_container positions;
			colors_container colors;
			normals_container normals;

			attributes_iterator begin(){
				return thrust::make_tuple(positions.begin(), normals.begin(), colors.begin());
			}

			attributes_iterator end(){
				return thrust::make_tuple(positions.end(), normals.end(), colors.end());
			}
		} attributes;

		// Triangle indices
		thrust::host_vector<glm::ivec3> triangles;

		glm::vec4 position;
		glm::mat4 model_mat;

		mesh() :
			position(0,0,0,1),
			model_mat(1.0f){

			model_mat = glm::rotate(glm::mat4(1.0f), -45.0f, glm::vec3(1,0,0));
		}

		void resize(size_t vectors_sz, size_t triangles_sz) {
			attributes.positions.resize(vectors_sz);
			attributes.colors.resize(vectors_sz);
			attributes.normals.resize(vectors_sz);
			triangles.resize(triangles_sz);
		}

		size_t total_vertices() const{
			return attributes.positions.size();
		}

		size_t total_triangles() const {
			return triangles.size();
		}

	};

	mesh load_model(const std::string & fname);
};
