#pragma once

#include "./raster.hpp"
#include "./gbuffer.hpp"
#include "./triangle.hpp"
#include "./math.hpp"

namespace thrender {

	// Triangle interpolation with barycoords
	struct triangle_interpolate_draw_pixel {

		gbuffer & gbuf;

		triangle_interpolate_draw_pixel(gbuffer & _gbuf)
		:
				gbuf(_gbuf)
		{}

		bool operator()(int x, int y, const triangle * ptri) {
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
					ptri->m->attributes.colors[ptri->indices[0]] * lamdas.x+
					ptri->m->attributes.colors[ptri->indices[1]] * lamdas.y+
					ptri->m->attributes.colors[ptri->indices[2]] * lamdas.z;
			glm::vec4 normal =
					ptri->m->attributes.normals[ptri->indices[0]] * lamdas.x+
					ptri->m->attributes.normals[ptri->indices[1]] * lamdas.y+
					ptri->m->attributes.normals[ptri->indices[2]] * lamdas.z;

			gbuf.depth[coords] = z;
			gbuf.diffuse[coords] = color;
			gbuf.normal[coords] = normal;
			return true;
		}
	};

	struct triangle_single_pixel {

			gbuffer & gbuf;

			triangle_single_pixel(gbuffer & _gbuf)
			:
					gbuf(_gbuf)
			{}

			bool operator()(int x, int y, const triangle * ptri) {
				size_t coords = coord2d(x,y);
				float z = ptri->pv[0]->z; // 1 Pixel no meaning for median

				if (gbuf.depth[coords] > z)	// Z-test
					return true;

				glm::vec4 color =
						ptri->m->attributes.colors[ptri->indices[0]] * (1.0f/3.0f)+
						ptri->m->attributes.colors[ptri->indices[1]] * (1.0f/3.0f)+
						ptri->m->attributes.colors[ptri->indices[2]] * (1.0f/3.0f);
				glm::vec4 normal =
						ptri->m->attributes.normals[ptri->indices[0]] * (1.0f/3.0f)+
						ptri->m->attributes.normals[ptri->indices[1]] * (1.0f/3.0f)+
						ptri->m->attributes.normals[ptri->indices[2]] * (1.0f/3.0f);

				gbuf.depth[coords] = z;
				gbuf.diffuse[coords] = color;
				gbuf.normal[coords] = normal;
				return true;
			}
		};

	struct fragment_processor_kernel {

		gbuffer & gbuf;
		triangle_interpolate_draw_pixel pix_op;
		triangle_single_pixel singlepix_op;

		fragment_processor_kernel(gbuffer & _gbuf)
		:
			gbuf(_gbuf),
			pix_op(gbuf),
			singlepix_op(gbuf)
		{
		}

		void operator()(const triangle & tr) {

			//if (m.render_buffer.discard_vertices[tr.x] || m.render_buffer.discard_vertices[tr.y]
							//|| m.render_buffer.discard_vertices[tr.z])
			if (!tr.ccw_winding_order())
				return;						// Face-culling

			// Sort points by y
			const glm::vec4 * pord[3] = {tr.pv[0], tr.pv[1], tr.pv[2]};
			math::sort3vec_by_y(pord);

			glm::vec2 size = tr.size();
			if (size.y < 1.0f && size.x < 1.0f) {
				singlepix_op(pord[0]->x, pord[1]->y, &tr);
				return;
			}

			// Find triangle contour
			polygon_horizontal_limits tri_contour;
			mark_vertical_contour mark_contour_op(tri_contour);
			thrender::utils::line_bresenham(pord[0]->x, pord[0]->y, pord[1]->x,
					pord[1]->y, mark_contour_op);
			thrender::utils::line_bresenham(pord[1]->x, pord[1]->y, pord[2]->x,
					pord[2]->y, mark_contour_op);
			thrender::utils::line_bresenham(pord[0]->x, pord[0]->y, pord[2]->x,
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
	void process_fragments(const thrust::host_vector<triangle> & triangles, gbuffer & gbuf) {
		thrust::for_each(
				triangles.begin(),
				triangles.end(),
				fragment_processor_kernel(gbuf));
	}
}
