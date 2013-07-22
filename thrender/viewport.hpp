#pragma once

#include "./types.hpp"

namespace thrender {

	//! Rendering viewport
	/**
	 * Defines a viewport on the framebuffer where
	 * the rendering pipeline must write on. Everything
	 * outside this viewport remains untouched.
	 */
	struct viewport {

		//! Given dimensions, construct a new viewport
		/**
		 * @param left The left side of the viewport (min x)
		 * @param top The top side of the viewport (min y)
		 * @param width The width of the viewport
		 * @param height The height of the viewport
		 */
		viewport(window_size_t left, window_size_t top, window_size_t width, window_size_t height)
		:
			m_width(width),
			m_height(height),
			m_half_width(m_width / 2),
			m_half_height(m_height/2),
			m_top(top),
			m_left(left)
		{}

		//! Get the width of the viewport
		inline window_size_t width() const {
			return m_width;
		}

		//! Get the height of the viewport
		inline window_size_t height() const {
			return m_height;
		}

		//! Get the top side of the viewport (min y)
		inline window_size_t top() const {
			return m_top;
		}

		//! Get the bottom side of the viewport (max y)
		inline window_size_t bottom() const {
			return m_top + m_height;
		}

		//! Get the left side of the viewport (min x)
		inline window_size_t left() const {
			return m_left;
		}

		//! Get the right side of the viewport (max x)
		inline window_size_t right() const {
			return m_left + m_width;
		}

		//! Get the half width of the viewport
		inline window_size_t half_width() const {
			return m_half_width;
		}

		//! Get the half height of the viewport
		inline window_size_t half_height() const {
			return m_half_height;
		}

	private:

		//! The width of the viewport
		window_size_t m_width;

		//! The height of the vieport
		window_size_t m_height;

		//! Cached (m_width/2)
		window_size_t m_half_width;

		//! Cached (m_height/2)
		window_size_t m_half_height;

		//! The top side of the viewport
		window_size_t m_top;

		//! The left side of the viewport
		window_size_t m_left;

	};
}
