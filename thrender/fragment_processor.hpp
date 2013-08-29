#pragma once

#include "./raster.hpp"
#include "./gbuffer.hpp"
#include "./vertex_array.hpp"
#include "./math.hpp"
#include "./utils/profiler.hpp"

namespace thrender {

	template<class RenderableType>
	struct fragment_processing_control {

		//! Type of renderable object
		typedef RenderableType renderable_type;

		//! Type of vertex
		typedef typename renderable_type::vertex_type vertex_type;

		//! Type of triangle
		typedef typename renderable_type::triangle_type triangle_type;

		//! Reference to the owner object
		const renderable_type & object;

		//! Reference to current render context
		render_context & context;

		//! Reference to current primitive
		const triangle_type & primitive;

		//! Barycoords of the processed pixel
		glm::vec3 barycoords;

		//! Framebuffer X coordinate of current pixel
		window_size_t framebuffer_x;

		//! Framebuffer Y coordinate of current pixel
		window_size_t framebuffer_y;

		//! Construct control on fragment processing
		fragment_processing_control(const renderable_type & _object, render_context & _context, const triangle_type & _triangle)
		:
			object(_object),
			context(_context),
			primitive(_triangle),
			framebuffer_x(0),
			framebuffer_y(0)
		{}

		//! Drops the current fragment as discarded
		/**
		 * If a pixel is discarded
		 */
		void discard() const{
			//const_cast<renderable_type &>(object).intermediate_buffer.discarded_vertices[vertex_id] = true;
		}

		template<size_t AttrID, class T>
		T interpolate() const{
			return VA_ATTRIBUTE(primitive.vertice(0), AttrID) * barycoords.x
				+ VA_ATTRIBUTE(primitive.vertice(1), AttrID) * barycoords.y
				+ VA_ATTRIBUTE(primitive.vertice(2), AttrID) * barycoords.z;
		}

		void set_coords(float x, float y) {
			framebuffer_x = x;
			framebuffer_y = y;
			barycoords = math::barycoords(*primitive.positions[0], *primitive.positions[1], *primitive.positions[2], glm::vec2(x,y));
		}

		// non-copyable
		fragment_processing_control(const fragment_processing_control&) = delete;
		fragment_processing_control& operator=(const fragment_processing_control&) = delete;
	};

//! Macro to query a vertex attribute, interpolated
#define INTERPOLATE(attribute) \
		api.template interpolate<attribute, \
			typename thrust::tuple_element<attribute, \
				typename std::remove_reference<decltype(api.object)>::type::vertex_type>::type>()


namespace shaders {

	struct default_fragment_shader {

		template<class RenderableType>
		void operator()(gbuffer::pixel_type & pixel, const fragment_processing_control<RenderableType> & api) {

			FB_ATTRIBUTE(FB_DIFFUSE, pixel) = INTERPOLATE(COLOR);
			FB_ATTRIBUTE(FB_NORMAL, pixel) = INTERPOLATE(NORMAL);
		}
	};
}
	template<class FragmentShader, class RenderableType>
	struct fragment_processor_kernel {

		//! Type of fragment shader
		typedef FragmentShader fragment_shader;

		//! Type of the rendererable object
		typedef RenderableType renderable_type;

		//! Type of the primitive
		typedef typename renderable_type::triangle_type triangle_type;

		//! Reference to the object that this fragment belongs to.
		const renderable_type & object;

		//! Reference to current rendering context
		render_context & context;

		//! Reference to fragment shader
		fragment_shader & shader;

		//! Construct the kernel for a specific object and context
		fragment_processor_kernel(const renderable_type & _object, fragment_shader & _shader, render_context & _context)
		:
			object(_object),
			context(_context),
			shader(_shader)
		{
		}

		void operator()(const triangle_type & tr)  {

			// If any vertex is discarded, the whole triangle is.
			if (object.intermediate_buffer.discarded_vertices[tr.indices[0]]
				|| object.intermediate_buffer.discarded_vertices[tr.indices[1]]
				|| object.intermediate_buffer.discarded_vertices[tr.indices[2]])
			{
				return;
			}

			// Face-culling
			if (!tr.is_ccw_winding_order())
				return;

			// Sort points by y
			const glm::vec4 * pord[3] = {tr.positions[0], tr.positions[1], tr.positions[2]};
			math::sort3vec_by_y(pord);

			/*glm::vec2 size = tr.bounding_box();
			if (size.y < 1.0f && size.x < 1.0f) {
				singlepix_op(pord[0]->x, pord[1]->y, &tr);
				return;
			}*/

			// Find triangle contour
			details::polygon_vertical_limits tri_contour;
			tri_contour.clear(context.vp.height());
			details::mark_vertical_contour mark_contour_op(tri_contour);
			thrender::math::line_bresenham(pord[0]->x, pord[0]->y, pord[1]->x,
					pord[1]->y, mark_contour_op);
			thrender::math::line_bresenham(pord[1]->x, pord[1]->y, pord[2]->x,
					pord[2]->y, mark_contour_op);
			thrender::math::line_bresenham(pord[0]->x, pord[0]->y, pord[2]->x,
					pord[2]->y, mark_contour_op);


			// Scan conversion fill
			fragment_processing_control<RenderableType> fgcontrol(object, context, tr);
			for (window_size_t y = pord[2]->y; y <= pord[0]->y; y++) {
				for (window_size_t x = tri_contour.leftmost[y]; x < tri_contour.rightmost[y]; x++) {
					fgcontrol.set_coords(x,y);
					gbuffer::pixel_type px =context.fb.get_pixel(x,y);
					float z = fgcontrol.template interpolate<0, glm::vec4>().z;

					if (FB_ATTRIBUTE(FB_DEPTH, px) > z)	// Z-test
						continue;
					FB_ATTRIBUTE(FB_DEPTH, px) = z;
					shader(px, fgcontrol);
				}

			}

		}
	};

	// Rasterization of fragments/primitives
	template<class FragmentShader, class RenderableType>
	void process_fragments(const RenderableType & object, FragmentShader & shader, render_context & context) {
		thrust::for_each(
				object.intermediate_buffer.elements.begin(),
				object.intermediate_buffer.elements.end(),
				fragment_processor_kernel<FragmentShader, RenderableType >(object, shader, context));
	}
}
