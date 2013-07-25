#include "./vertex_processor.hpp"
#include "./fragment_processor.hpp"

namespace thrender {

	template<
		class RenderableType,
		class VertexShader,
		class FragmentShader>
	struct pipeline {

		typedef RenderableType renderable_type;

		//! Type of vertex shader
		typedef VertexShader vertex_shader_type;

		//! Type of fragment shader
		typedef FragmentShader fragment_shader_type;

		vertex_shader_type  & vx_shader;

		fragment_shader_type & fg_shader;

		//! Execute pipeline to render one frame
		pipeline(vertex_shader_type & _vx_shader, fragment_shader_type & _fg_shader) :
			vx_shader(_vx_shader),
			fg_shader(_fg_shader)
		{

		}

		void draw(renderable_type & object, render_context & context){
			process_vertices<vertex_shader_type, renderable_type>(object, vx_shader, context);
			process_fragments<fragment_shader_type, renderable_type>(object, fg_shader, context);
		}

	};
}
