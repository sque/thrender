#pragma once

#include "framebuffer.hpp"
#include <memory>
#include <glm/glm.hpp>
#include <thrust/host_vector.h>

namespace thrender {

	//! An array of framebuffers
	/**
	 * The framebuffer_array holds all the needed framebuffers
	 * by the rendering context plus some global api to unified
	 * check and control of them.
	 */
	struct framebuffer_array {

		//! The framebuffer type of an extra_buffuer
		typedef framebuffer_<color_pixel_t> extra_buffer_type;

		//! The framebuffer type of the depth buffer
		typedef framebuffer_<depth_pixel_t> depth_buffer_type;

		//! The framebuffer type of the color buffer
		typedef framebuffer_<color_pixel_t> color_buffer_type;

		//! The type of shared pointer used for color buffer
		typedef std::shared_ptr<color_buffer_type> color_buffer_pointer_type;

		//! The type of shared pointer used for depth buffer
		typedef std::shared_ptr<depth_buffer_type> depth_buffer_pointer_type;

		//! The type of shared pointer used for extra buffer
		typedef std::shared_ptr<extra_buffer_type> extra_buffer_pointer_type;

		//! The type of container to hold all extra buffers
		typedef thrust::host_vector<extra_buffer_pointer_type> extra_buffers_container_type;

		//! Initialize a new framebuffer
		/**
		 * It will allocate the needed buffers (color, depth).
		 * There is no way to change framebuffer size after initialization.
		 */
		framebuffer_array(size_t width, size_t height)
		:
			m_width(width),
			m_height(height),
			m_depth_buffer(depth_buffer_pointer_type(new depth_buffer_type(width, height))),
			m_color_buffer(color_buffer_pointer_type(new color_buffer_type(width, height)))
		{
			m_depth_buffer->set_clear_value(default_depth_clear_value);
			m_color_buffer->set_clear_value(default_color_clear_value);
		}

		//! Get access to depth buffer
		inline depth_buffer_type & depth_buffer() {
			return *m_depth_buffer;
		}

		//! Get access to color_buffer
		inline color_buffer_type & color_buffer() {
			return * m_color_buffer;
		}

		//! Get access to nth extra_buffer
		inline extra_buffer_type & extra_buffer(size_t index){
			return * extra_buffers[index];
		}

		/**
		 * @return the index of the new buffer
		 */
		size_t add_extra_buffer() {
			extra_buffers.push_back(extra_buffer_pointer_type(new extra_buffer_type(width(), height())));
			return extra_buffers.size() - 1;
		}

		//! Clear all framebuffers in this array
		void clear_all() {
			m_depth_buffer->clear();
			m_color_buffer->clear();
			for(extra_buffers_container_type::iterator
				it = extra_buffers.begin();it!= extra_buffers.end();it++) {
					(*it)->clear();
			}
		}

		bool is_complete() const{
			if (!m_depth_buffer || !m_color_buffer)
				return false;

			if (m_depth_buffer->shape() != m_color_buffer->shape())
				return false;

			for(extra_buffers_container_type::const_iterator it
					= extra_buffers.begin(); it != extra_buffers.end();it++) {
				if (m_depth_buffer->shape() != (*it)->shape())
					return false;
			}
			return true;
		}

		//! Get the width of this framebuffer array
		window_size_t width() const{
			return m_width;
		}

		//! Get the height of this framebuffer array
		window_size_t height() const{
			return m_height;
		}

	private:

		//! The width of the framebuffer
		window_size_t m_width;

		//! The height of the framebuffer
		window_size_t m_height;

		//! Pointer to depth buffer
		depth_buffer_pointer_type m_depth_buffer;

		//! Pointer to color buffer
		color_buffer_pointer_type m_color_buffer;

		//! Vector of all extra buffers
		extra_buffers_container_type extra_buffers;

	};
}
