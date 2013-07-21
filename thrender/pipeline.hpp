#include "./vertex_processor.hpp"
#include "./fragment_processor.hpp"

namespace thrender {

	template<
		class VertexArray,
		class VertexShader = shaders::default_vertex_shader<VertexArray> >
	struct pipeline {

		typedef VertexArray vertex_array_type;

		//! Type of vertex shader
		typedef VertexShader vertex_shader_type;

		vertex_array & object;

		vertex_shader_type vertex_shader;

		//fragment_shader_type fragment_shader;

		//! Execute pipeline to render one frame
		void execute(const camera & _camera, vertex_array_type & _object, gbuffer & _gbuff) {
			render_state rstate;
			process_vertices<vertex_shader_type, vertex_array_type>(_object, rstate);
			process_fragments(_object.render_buffer.triangles, _gbuff);
		}

	};
}
