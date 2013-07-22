#pragma once

#include "./types.hpp"

namespace thrender {

	struct viewport {

		viewport(window_size_t left, window_size_t top, window_size_t width, window_size_t height)
		:
			m_width(width),
			m_height(height),
			m_half_width(m_width / 2),
			m_half_height(m_height/2),
			m_top(top),
			m_left(left)
		{}

		inline window_size_t width() const {
			return m_width;
		}

		inline window_size_t height() const {
			return m_height;
		}

		inline window_size_t top() const {
			return m_top;
		}

		inline window_size_t left() const {
			return m_left;
		}

		inline window_size_t right() const {
			return m_left + m_width;
		}

		inline window_size_t bottom() const {
			return m_top + m_height;
		}

		inline window_size_t half_width() const {
			return m_half_width;
		}

		inline window_size_t half_height() const {
			return m_half_height;
		}

	private:
		window_size_t m_width;

		window_size_t m_height;

		window_size_t m_half_width;

		window_size_t m_half_height;

		window_size_t m_top;

		window_size_t m_left;

	};
}
