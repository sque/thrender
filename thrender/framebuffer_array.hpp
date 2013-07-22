#pragma once

#include "framebuffer.hpp"
#include <boost/shared_ptr.hpp>
#include <glm/glm.hpp>

namespace thrender {

	struct framebuffer_array {

		typedef std::vector<boost::shared_ptr<framebuffer> > extra_buffers_type;

		typedef boost::shared_ptr<framebuffer_<depth_pixel_t*> > depth_buffer_type;

		typedef boost::shared_ptr<framebuffer_<glm::vec4> > color_buffer_type;

		depth_buffer_type depth_buffer;

		color_buffer_type color_buffer;

		extra_buffers_type extra_buffers;

		framebuffer_array(size_t width, size_t height)
		:
			depth_buffer(depth_buffer_type(new framebuffer_<depth_pixel_t*>(width, height))),
			color_buffer(color_buffer_type(new framebuffer_<color_buffer_type::element_type::pixel_type>(width, height)))
		{}

		void clear() {
			for(extra_buffers_type::iterator
					it = extra_buffers.begin();it!= extra_buffers.end();it++) {
				//it->clear();
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
	};

}
