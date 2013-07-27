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

		//! Construct control on fragment processing
		fragment_processing_control(const renderable_type & _object, render_context & _context, const triangle_type & _triangle)
		:
			object(_object),
			context(_context),
			primitive(_triangle)
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
			barycoords = math::barycoords(*primitive.positions[0], *primitive.positions[1], *primitive.positions[2], glm::vec2(x,y));
		}
	private:

		// non-copyable
		fragment_processing_control(const fragment_processing_control&);
		fragment_processing_control& operator=(const fragment_processing_control&);
	};

namespace shaders {


	struct default_fragment_shader {

//		typedef RenderableType renderable_type;

		//typedef typename renderable_type::vertex_type vertex_type;

		template<class RenderableType>
		void operator()(gbuffer & gbuf, const fragment_processing_control<RenderableType> & fg_control, size_t coords) {

			gbuf.diffuse.serial_at(coords) = fg_control.template interpolate<COLOR, glm::vec4>();
			gbuf.normal.serial_at(coords) = fg_control.template interpolate<NORMAL, glm::vec4>();
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
					size_t coords = coord2d(x,y);
					fgcontrol.set_coords(x,y);

					float z = fgcontrol.template interpolate<0, glm::vec4>().z;

					if (context.fb.depth.serial_at(coords) > z)	// Z-test
						continue;
						context.fb.depth.serial_at(coords) = z;
					shader(context.fb, fgcontrol, coords);
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
