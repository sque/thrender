#pragma once
#include "./fragment_processor.hpp"
#include "./vertex_processor.hpp"

//! Macro to query a vertex attribute, interpolated inside fragment shader
#define INTERPOLATE(attribute) \
		api.template interpolate<attribute, \
			typename thrust::tuple_element<attribute, \
				typename std::remove_reference<decltype(api.object)>::type::vertex_type>::type>()

//! Macro to access the current pixel of a buffer inside fragment shader
#define FB_PIXEL(buffer) \
	buffer[api.framebuffer_y][api.framebuffer_x]

namespace thrender {
namespace shaders {

//! Definition of an omni light
struct omni_light {

	//! Diffuse color of the light
	color_pixel_t diffuse_color;

	//! Specular color of the light
	color_pixel_t specular_color;

	//! Position of the light in world space
	glm::vec4 position_ws;
};

//! Definition of a phong material
struct phong_material {

	//! Color of the material in diffuse lighting
	color_pixel_t diffuse_color;

	//! Color of the material in the shinny spots
	color_pixel_t specular_color;

	//! Shininess of the specularity
	float shininess;

	//! Emissive lighting
	color_pixel_t emissive_color;
};

//! Vertex shader base class
/**
 * TODO: Add this base class for declaring extra hooks
 * like pre-processing, post-processing etc.
 */
struct vx_shader {

	virtual void prepare(render_context & ctx) {

	}

	virtual ~vx_shader(){}
};

//! Implementation of a Gouraud shading (per vertex)
/**
 * It will calculate per vertex
 */
struct gouraud_vx_shader {

	//! Model transformation matrix
	glm::mat4 mModel;

	//! View transformation matrix
	glm::mat4 mView;

	//! Projection transformation matrix
	glm::mat4 mProjection;

	//! Camera position in world space
	glm::vec4 vCameraPos_ws;

	//! The light
	omni_light light;

	//! The material of the object
	phong_material material;

	template<class RenderableType>
	void operator()(const typename RenderableType::vertex_type & vin, typename RenderableType::vertex_type & vout, vertex_processing_control<RenderableType> & vcontrol){
		const glm::vec4 & posIn = VA_ATTRIBUTE(vin, POSITION);
		glm::vec4 & posOut = VA_ATTRIBUTE(vout, POSITION);
		const glm::vec4 & normIn = VA_ATTRIBUTE(vin, NORMAL);

		posOut = glm::normalize(mProjection * mView * mModel * posIn);
		vcontrol.translate_to_window_space(posOut);
		vcontrol.viewport_clip(posOut);

		glm::vec4 vNormal_ws = glm::normalize(mModel * normIn);
		glm::vec4 vPos_ws = glm::normalize(mModel * posIn);
		glm::vec4 vLightDirection = glm::normalize(vPos_ws - light.position_ws);
		glm::vec4 vCameraDirection = glm::normalize(vPos_ws - vCameraPos_ws);
		glm::vec4 vReflectedLight = glm::normalize(glm::reflect(-vLightDirection, vNormal_ws));

		float fDiffuseIntensity = glm::max(0.0f, glm::dot( vNormal_ws, vLightDirection ));
		float fSpecularIntensity = glm::pow(glm::max(0.0f, glm::dot(vReflectedLight, vCameraDirection)), material.shininess);

		glm::vec4 cDiffuse = light.diffuse_color * material.diffuse_color * fDiffuseIntensity;
		glm::vec4 cSpecular = light.specular_color * material.specular_color * fSpecularIntensity;
		VA_ATTRIBUTE(vout, COLOR) = material.emissive_color + cDiffuse + cSpecular;
	}
};

//! Implementation of Gouraud shading (per vertex)
/**
 * It will use the vertex color processed by vertex
 * shader and interpolate in the primitive.
 */
struct gouraud_fg_shader {

	template<class RenderableType>
	void operator()(framebuffer_array & fb, const fragment_processing_control<RenderableType> & api) {
		FB_PIXEL(fb.color_buffer()) = INTERPOLATE(COLOR);
	}
};

//! Default vertex shader
/**
 * Default shaders process vertices and projects them
 * in 3D space based on ModelViewProjection matrix.
 */
struct default_vx_shader {

	//! Model view projection matrix
	glm::mat4 mvp_mat;

	template<class RenderableType>
	void operator()(const typename RenderableType::vertex_type & vin, typename RenderableType::vertex_type & vout, vertex_processing_control<RenderableType> & vcontrol){
		const glm::vec4 & posIn = VA_ATTRIBUTE(vin, POSITION);
		glm::vec4 & posOut = VA_ATTRIBUTE(vout, POSITION);

		posOut = mvp_mat * posIn;
		// clip coordinates

		vcontrol.translate_to_window_space(posOut);
		// translate to window space

		vcontrol.viewport_clip(posOut);
	}
};

//! Default fragment shader
/**
 * This shader uses the interpolated vertex color to fill fragment color
 */
struct default_fg_shader {

	template<class RenderableType>
	void operator()(framebuffer_array & fb, const fragment_processing_control<RenderableType> & api) {
		FB_PIXEL(fb.color_buffer()) = INTERPOLATE(COLOR);
	}
};
}
}
