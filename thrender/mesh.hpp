#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thrust/host_vector.h>
#include <thrust/iterator/zip_iterator.h>
#include <iostream>
#include "./triangle.hpp"

namespace thrender {

	struct mesh;

	//! Buffer needed per mesh for intermediate processing
	struct mesh_intermediate_buffer
	{

		typedef thrust::host_vector<glm::vec4> projvertices_vector;

		typedef thrust::host_vector<bool> discardvertices_vector;

		typedef thrust::host_vector<triangle> triangles_vector;


		//! A vector with all projected vertices
		projvertices_vector projected_vertices;

		//! A bitmap with all discarded vertices
		discardvertices_vector discarded_vertices;

		//! A vector with all mesh triangles of projected vectors
		triangles_vector triangles;

		mesh_intermediate_buffer() {}

		void reset() {
			discarded_vertices.clear();
			discarded_vertices.resize(projected_vertices.size(), false);
		}

		void post_update(mesh & m, size_t total_vertices, thrust::host_vector<glm::ivec3> & itriangles) {
			projected_vertices.resize(total_vertices);
			discarded_vertices.resize(total_vertices);
			triangles.clear();

			thrust::host_vector<glm::ivec3>::iterator it_index;
			for(it_index = itriangles.begin();it_index != itriangles.end(); it_index++) {
				glm::ivec3 & indices = *it_index;
				triangles.push_back(
					triangle(
						m,
						projected_vertices,
						indices.x, indices.y, indices.z,
						false)
					);
			}
		}
	};

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

		// Intermediate render buffer
		mesh_intermediate_buffer render_buffer;

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

		//! Get the total number of vertices
		size_t total_vertices() const{
			return attributes.positions.size();
		}

		//! Get the total number of triangles
		size_t total_triangles() const {
			return triangles.size();
		}

		//! Action that must be called if object is updated
		void post_update() {
			render_buffer.post_update(*this, total_vertices(), triangles);
		}

	};
};
