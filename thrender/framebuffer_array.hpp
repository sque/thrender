#pragma once

#include "framebuffer.hpp"
#include <boost/shared_ptr.hpp>
#include <glm/glm.hpp>
#include <thrust/host_vector.h>

namespace thrender {


	struct framebuffer_array {

		typedef thrust::host_vector<boost::shared_ptr<framebuffer_<glm::vec4>> > extra_buffers_type;

		typedef boost::shared_ptr<framebuffer_<depth_pixel_t> > depth_buffer_type;

		typedef boost::shared_ptr<framebuffer_<glm::vec4> > color_buffer_type;

		depth_buffer_type depth_buffer;

		color_buffer_type color_buffer;

		extra_buffers_type extra_buffers;

		framebuffer_array(size_t width, size_t height)
		:
			depth_buffer(depth_buffer_type(new framebuffer_<depth_pixel_t>(width, height))),
			color_buffer(color_buffer_type(new framebuffer_<color_buffer_type::element_type::pixel_type>(width, height))),
			m_width(width),
			m_height(height)
		{}

		/**
		 * @return the index of the new buffer
		 */
		size_t add_extra_buffer() {
			extra_buffers.push_back(extra_buffers_type::value_type(new framebuffer_<extra_buffers_type::value_type::element_type::pixel_type>(width(), height())));
			return extra_buffers.size() - 1;
		}

		void clear() {
			depth_buffer->clear(0);
			color_buffer->clear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			for(extra_buffers_type::iterator
				it = extra_buffers.begin();it!= extra_buffers.end();it++) {
				// FixME: it->clear();
			}
		}

		bool is_complete() const{
			if (!depth_buffer || !color_buffer)
				return false;

			if (depth_buffer->shape() != color_buffer->shape())
				return false;

			for(extra_buffers_type::const_iterator it
					= extra_buffers.begin(); it != extra_buffers.end();it++) {
				if (depth_buffer->shape() != (*it)->shape())
					return false;
			}
			return true;
		}

		window_size_t width() const{
			return m_width;
		}

		window_size_t height() const{
			return m_height;
		}

	private:

		window_size_t m_width;

		window_size_t m_height;

	};

#define FB_ATTRIBUTE(attr, pix) \
	thrust::get<attr>(pix)
#define FB_DEPTH 0
#define FB_DIFFUSE 1
#define FB_NORMAL 2

}
