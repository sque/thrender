#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace thrender {

	//! Base camera object
	struct camera {
		glm::mat4 projection_mat;
		glm::mat4 view_mat;
		glm::vec3 position;

		//! Construct camera
		/**
		 * @param pos The position of camera in world-space
		 * @param fov The field of view angle of this camera
		 * @param aspect_ratio The aspect_ratio of this camera
		 * @param near Near clipping plane in the z-axis
		 * @param far Far clipping plane in the z-axis
		 */
		camera(glm::vec3 pos, float fov, float aspect_ratio, float near, float far) :
			position(pos){
			projection_mat = glm::perspective(fov, aspect_ratio, near, far);
			view_mat = glm::lookAt(position, glm::vec3(0,0,0), glm::vec3(0,1,0));
		}

	};

}
