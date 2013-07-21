#pragma once

#include "./raster.hpp"
#include "./gbuffer.hpp"
#include "./vertex_array.hpp"
#include "./math.hpp"

namespace thrender {

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
			glm::vec4 lamdas = math::barycoords(*ptri->pv[0], *ptri->pv[1], *ptri->pv[2], glm::vec2(x,y));
			z = lamdas.x * ptri->pv[0]->z + lamdas.y * ptri->pv[1]->z + lamdas.z * ptri->pv[2]->z;
			if (gbuf.depth[coords] > z)	// Z-test
				return true;

			glm::vec4 color =
					VA_ATTRIBUTE(m.vertices[ptri->indices[0]], COLOR) * lamdas.x+
					VA_ATTRIBUTE(m.vertices[ptri->indices[1]], COLOR) * lamdas.y+
					VA_ATTRIBUTE(m.vertices[ptri->indices[2]], COLOR) * lamdas.z;
			glm::vec4 normal =
					VA_ATTRIBUTE(m.vertices[ptri->indices[0]], NORMAL) * lamdas.x+
					VA_ATTRIBUTE(m.vertices[ptri->indices[1]], NORMAL) * lamdas.y+
					VA_ATTRIBUTE(m.vertices[ptri->indices[2]], NORMAL) * lamdas.z;

			gbuf.depth[coords] = z;
			gbuf.diffuse[coords] = color;
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
				float z = ptri->pv[0]->z; // 1 Pixel no meaning for median

				if (gbuf.depth[coords] > z)	// Z-test
					return true;

				glm::vec4 color =
						VA_ATTRIBUTE(m.vertices[ptri->indices[0]], COLOR) * (1.0f/3.0f)+
						VA_ATTRIBUTE(m.vertices[ptri->indices[1]], COLOR) * (1.0f/3.0f)+
						VA_ATTRIBUTE(m.vertices[ptri->indices[2]], COLOR) * (1.0f/3.0f);
				glm::vec4 normal =
						VA_ATTRIBUTE(m.vertices[ptri->indices[0]], NORMAL) * (1.0f/3.0f)+
						VA_ATTRIBUTE(m.vertices[ptri->indices[1]], NORMAL) * (1.0f/3.0f)+
						VA_ATTRIBUTE(m.vertices[ptri->indices[2]], NORMAL) * (1.0f/3.0f);

				gbuf.depth[coords] = z;
				gbuf.diffuse[coords] = color;
				gbuf.normal[coords] = normal;
				return true;
			}
		};

	template<class RenderableType>
	struct fragment_processor_kernel {

		typedef RenderableType renderable_type;

		typedef typename renderable_type::triangle_type triangle_type;

		gbuffer & gbuf;
		triangle_interpolate_draw_pixel<renderable_type> pix_op;
		triangle_single_pixel<renderable_type> singlepix_op;
		const renderable_type & object;

		fragment_processor_kernel(const renderable_type & _object, gbuffer & _gbuf)
		:
			gbuf(_gbuf),
			pix_op(_object, gbuf),
			singlepix_op(_object, gbuf),
			object(_object)
		{
		}

		void operator()(const triangle_type & tr)  {

			//if (m.render_buffer.discard_vertices[tr.x] || m.render_buffer.discard_vertices[tr.y]
							//|| m.render_buffer.discard_vertices[tr.z])
			if (!tr.is_ccw_winding_order())
				return;						// Face-culling

			// Sort points by y
			const glm::vec4 * pord[3] = {tr.pv[0], tr.pv[1], tr.pv[2]};
			math::sort3vec_by_y(pord);

			glm::vec2 size = tr.bounding_box();
			if (size.y < 1.0f && size.x < 1.0f) {
				singlepix_op(pord[0]->x, pord[1]->y, &tr);
				return;
			}

			// Find triangle contour
			polygon_horizontal_limits tri_contour;
			mark_vertical_contour mark_contour_op(tri_contour);
			thrender::math::line_bresenham(pord[0]->x, pord[0]->y, pord[1]->x,
					pord[1]->y, mark_contour_op);
			thrender::math::line_bresenham(pord[1]->x, pord[1]->y, pord[2]->x,
					pord[2]->y, mark_contour_op);
			thrender::math::line_bresenham(pord[0]->x, pord[0]->y, pord[2]->x,
					pord[2]->y, mark_contour_op);

			// Scan conversion fill
			for (int y = pord[2]->y; y <= pord[0]->y; y++) {
				for (int x = tri_contour.leftmost[y]; x < tri_contour.rightmost[y];
						x++)
					pix_op(x, y, &tr);
			}
		}
	};

	// Rasterization of fragments/primitives
	template<class RenderableType>
	void process_fragments(const RenderableType & object, render_state & rstate) {
		thrust::for_each(
				object.intermediate_buffer.elements.begin(),
				object.intermediate_buffer.elements.end(),
				fragment_processor_kernel<RenderableType >(object, rstate.gbuff));
	}
}
