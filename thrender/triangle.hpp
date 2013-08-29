#pragma once

#include <glm/glm.hpp>
#include "./math.hpp"
#include "./vertex_array.hpp"
#include "./raster.hpp"

namespace thrender {

	//! Triangle primitive
	template <class VertexType>
	struct triangle {

		//! Type of vertex
		typedef VertexType vertex_type;

		//! Type of vertices container
		typedef typename vertex_array<vertex_type>::vertices_type vertices_type;

		//! Vector indices in array
		indices3_t indices;

		//! Pointer to position attributes
		const glm::vec4 * positions[3];

		//! Triangle flags
		struct {
			bool discarded;	//! If this triangle is completely discarded
		} flags;

		//! Construct a new triangle
		triangle(const vertices_type & _processed_vertices, indices3_t _indices, bool _discard)
		:
			indices(_indices),
			mp_processed_vertices(&_processed_vertices)
		{
			positions[0] = &VA_ATTRIBUTE(vertice(0), POSITION);
			positions[1] = &VA_ATTRIBUTE(vertice(1), POSITION);
			positions[2] = &VA_ATTRIBUTE(vertice(2), POSITION);
		}

		//! Default constructor
		triangle() {
			std::cout << "empty constructor" << std::endl;
		}

		triangle(const triangle & rv){
			//std::cout << "Copied" << std::endl;
			*this = rv;
		}

		/*triangle & operator=(const triangle &){
			std::cout << "assigned" << std::endl;
			return *this;
		}*/

		//! Get reference to owner object vertices
		inline const vertices_type & owner_vertices() const {
			return *mp_processed_vertices;
		}

		//! Get access to one of the 3 vertices of this triangle
		/**
		 * @param index The index of the 3 vertices (zero based)
		 * @return Reference to the specified vertex
		 */
		inline const vertex_type & vertice(size_t index) const {
			return (*mp_processed_vertices)[indices[index]];
		}

		//! Check if triangle has Counter-Clock-Wise winding order
		/**
		 * It will check this based on the current position of the triangle in 2d space.
		 */
		bool is_ccw_winding_order() const{
			// FixMe: optimize this? (http://www.gamedev.net/topic/475852-efficient-method-to-determine-winding-order-of-triangle/)
			float adiff = atan2f(positions[1]->y - positions[0]->y, positions[1]->x - positions[0]->x)
					- atan2f(positions[2]->y - positions[0]->y, positions[2]->x - positions[0]->x);
			if (adiff < 0)
				adiff = (M_PI * 2) + adiff;
			return (adiff >= M_PI);
		}

		//! Calculate the size of the smallest bounding box that fits this triangle
		/**
		 * Returns a vec4 value where 0,1 elements are the coordinates of the top left
		 * corner of the box and 2,3 elements are the width and height of the box.
		 */
		glm::vec4 bounding_box() const {
			float x_max = std::max(std::max(positions[0]->x, positions[1]->x), positions[2]->x);
			float x_min = std::min(std::min(positions[0]->x, positions[1]->x), positions[2]->x);

			float y_max = std::max(std::max(positions[0]->y, positions[1]->y), positions[2]->y);
			float y_min = std::min(std::min(positions[0]->y, positions[1]->y), positions[2]->y);
			return glm::vec4(x_min, y_min, x_max-x_min, y_max-y_min);
		}


	private:

		//! Pointer to processed vertices list
		const vertices_type (*mp_processed_vertices);
	};

}
