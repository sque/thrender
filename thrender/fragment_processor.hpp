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

	// Triangle interpolation with barycoords
	template<class RenderableType>
	struct triangle_interpolate_draw_pixel {

		typedef RenderableType renderable_type;
		typedef typename renderable_type::triangle_type triangle_type;

		gbuffer & gbuf;
		const renderable_type & m;

		triangle_interpolate_draw_pixel(const renderable_type & object, gbuffer & _gbuf)
		:
				gbuf(_gbuf),
				m(object)
		{}

		bool operator()(int x, int y, const triangle_type * ptri) {
			size_t coords = coord2d(x,y);
			float z;

			/** FixMe: Coordinates are passed as integers (floor rounding).
			 * In small triangles this is out of borders
			 */
			glm::vec3 lamdas = math::barycoords(*ptri->positions[0], *ptri->positions[1], *ptri->positions[2], glm::vec2(x,y));
			z = lamdas.x * ptri->positions[0]->z + lamdas.y * ptri->positions[1]->z + lamdas.z * ptri->positions[2]->z;
			if (gbuf.depth[coords] > z)	// Z-test
				return true;

			//std::cout << lamdas.x << ", " << lamdas.y << ", " << lamdas.z << std::endl;
			//std::cout << lamdas.x << ", " << lamdas.y << ", " << lamdas.z << std::endl;
			glm::vec4 color =
					VA_ATTRIBUTE(ptri->vertice(0), COLOR) * lamdas.x+
					VA_ATTRIBUTE(ptri->vertice(1), COLOR) * lamdas.y+
					VA_ATTRIBUTE(ptri->vertice(2), COLOR) * lamdas.z;
			glm::vec4 normal =
					VA_ATTRIBUTE(ptri->vertice(0), NORMAL) * lamdas.x+
					VA_ATTRIBUTE(ptri->vertice(1), NORMAL) * lamdas.y+
					VA_ATTRIBUTE(ptri->vertice(2), NORMAL) * lamdas.z;

			gbuf.depth[coords] = z;
			gbuf.diffuse.serial_at(coords) = VA_ATTRIBUTE(ptri->vertice(0), COLOR);
			gbuf.normal[coords] = normal;
			return true;
		}
	};

	template <class RenderableType>
	struct triangle_single_pixel {

			typedef RenderableType renderable_type;
			typedef typename renderable_type::triangle_type triangle_type;

			gbuffer & gbuf;
			const renderable_type & m;

			triangle_single_pixel(const renderable_type & object, gbuffer & _gbuf)
			:
					gbuf(_gbuf),
					m(object)
			{}

			bool operator()(int x, int y, const triangle_type * ptri) {
				size_t coords = coord2d(x,y);
				float z = ptri->positions[0]->z; // 1 Pixel no meaning for median

				if (gbuf.depth[coords] > z)	// Z-test
					return true;

				glm::vec4 color =
						VA_ATTRIBUTE(ptri->vertice(0), COLOR) * (1.0f/3.0f)+
						VA_ATTRIBUTE(ptri->vertice(1), COLOR) * (1.0f/3.0f)+
						VA_ATTRIBUTE(ptri->vertice(2), COLOR) * (1.0f/3.0f);
				glm::vec4 normal =
						VA_ATTRIBUTE(ptri->vertice(0), NORMAL) * (1.0f/3.0f)+
						VA_ATTRIBUTE(ptri->vertice(1), NORMAL) * (1.0f/3.0f)+
						VA_ATTRIBUTE(ptri->vertice(2), NORMAL) * (1.0f/3.0f);

				gbuf.depth[coords] = z;
				gbuf.diffuse.serial_at(coords) = color;
				gbuf.normal[coords] = normal;
				return true;
			}
		};

namespace shaders {

	template<class RenderableType>
	struct default_fragment_shader {

		typedef RenderableType renderable_type;

		typedef typename renderable_type::vertex_type vertex_type;

		void operator()(gbuffer & gbuf, const fragment_processing_control<renderable_type> & fg_control, size_t coords) {

			gbuf.diffuse.serial_at(coords) = fg_control.template interpolate<COLOR, glm::vec4>();
			gbuf.normal[coords] = fg_control.template interpolate<NORMAL, glm::vec4>();
		}
	};
}
	template<class RenderableType>
	struct fragment_processor_kernel {

		//! Type of the rendererable object
		typedef RenderableType renderable_type;

		//! Type of the primitive
		typedef typename renderable_type::triangle_type triangle_type;

		triangle_interpolate_draw_pixel<renderable_type> pix_op;
		triangle_single_pixel<renderable_type> singlepix_op;

		//! Reference to the object that this fragment belongs to.
		const renderable_type & object;

		//! Reference to current rendering context
		render_context & context;

		fragment_processor_kernel(const renderable_type & _object, render_context & _context)
		:
			pix_op(_object, _context.fb),
			singlepix_op(_object, context.fb),
			object(_object),
			context(_context)
		{
		}

		void operator()(const triangle_type & tr)  {

			// If any vertice is discarded, the whole triangle is.
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
			thrender::math::line_bresenham(
					pord[0]->x, pord[0]->y, pord[2]->x,
					pord[2]->y, mark_contour_op);

			// Scan conversion fill
			fragment_processing_control<RenderableType> fgcontrol(object, context, tr);
			shaders::default_fragment_shader<RenderableType> fgshader;
			for (window_size_t y = pord[2]->y; y <= pord[0]->y; y++) {
				for (window_size_t x = tri_contour.leftmost[y]; x < tri_contour.rightmost[y]; x++) {
					size_t coords = coord2d(x,y);
					fgcontrol.set_coords(x,y);

					float z = fgcontrol.template interpolate<0, glm::vec4>().z;

					if (context.fb.depth[coords] > z)	// Z-test
						continue;
						context.fb.depth[coords] = z;
					fgshader(context.fb, fgcontrol, coords);
				}

			}

		}
	};

	// Rasterization of fragments/primitives
	template<class RenderableType>
	void process_fragments(const RenderableType & object, render_context & context) {
		thrust::for_each(
				object.intermediate_buffer.elements.begin(),
				object.intermediate_buffer.elements.end(),
				fragment_processor_kernel<RenderableType >(object, context));
	}
}
