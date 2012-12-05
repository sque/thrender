#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace thrender {

	//! Camera representation object
	struct camera {
		glm::mat4 proj_mat;
		glm::mat4 view_mat;
		glm::vec3 position;

		camera(glm::vec3 _pos, float fov, float aspect, float near, float far) :
			position(_pos){
			proj_mat = glm::perspective(fov, aspect, near, far);
			view_mat = glm::lookAt(position, glm::vec3(0,0,0), glm::vec3(0,1,0));
		}

	};

}
