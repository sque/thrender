#pragma once

#include "./gbuffer.hpp"
#include "./camera.hpp"
#include "./types.hpp"
#include "./viewport.hpp"

namespace thrender {

	//! Depth range toolkit
	/**
	 * It is used by vertex processor for
	 * translating NDC coordinates to window space
	 * coordinates.
	 */
	struct depth_range_tk {

		//! Construct by defining near and far of window space depth.
		/**
		 * @param near The depth buffer value for near plane
		 * @param far The depth buffer value for the far plane
		 */
		depth_range_tk(depth_pixel_t near, depth_pixel_t far)
		:
			m_near(near),
			m_far(far),
			m_half_distance((m_far - m_near)/2),
			m_half_sum((m_far + m_near)/2)
		{}

		//! Get depth buffer value for near plane
		inline depth_pixel_t near() const{
			return m_near;
		}

		//! Get depth buffer value for far plane
		inline depth_pixel_t far() const {
			return m_far;
		}

		//! Translate an NDC z value to window space
		inline depth_pixel_t translate_to_window_space(depth_pixel_t z) const {
			return z * m_half_distance + m_half_sum;

		}
	private:

		//! Depth buffer value for near plane
		depth_pixel_t m_near;

		//! Depth buffer value for far plane
		depth_pixel_t m_far;

		//! Cached (far-near)/2
		depth_pixel_t m_half_distance;

		//! Cached (near+far)/2
		depth_pixel_t m_half_sum;

	};

	//! Rendering context
	/**
	 * It holds all the needed objects and information
	 * for a batch to be rendered.
	 */
	struct render_context {


		framebuffer_array & fb;
		camera & cam;
		viewport vp;
		depth_range_tk depth_range;

		render_context(camera & _camera, framebuffer_array & _fb) :
			fb(_fb),
			cam(_camera),
			vp(0, 0, fb.width(), fb.height()),
			depth_range(1, 0)
		{}


		inline camera & get_camera() {
			return cam;
		}
	};
}
