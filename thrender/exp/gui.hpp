#pragma once

#include <SDL.h>
#include <sstream>
#include "thrender/framebuffer.hpp"


namespace thrender {

	//! Generate a string containing the current SDL error and a custom string
	const std::string generate_sdl_error(const std::string & prefix);

	//! Initialize SDL subsystems
	void initialize_sdl();

namespace details{

	//! Templated operation to convert a color to SDL uint32 pixel
	template<class T>
	struct convert_to_uint32_pixel {
		static inline Uint32 convert(T & px){
			return (0xFF000000 | ((int) px << 16)
					| ((int) px << 8) | (int) px);
		}
	};

	//! Specialization for glm::vec4 color
	template<>
	struct convert_to_uint32_pixel<glm::vec4> {
		static inline Uint32 convert(glm::vec4 & color){
			return (0xFF000000 | ((int) color.r << 16)
					| ((int) color.g << 8) | (int) color.b);
		}
	};
}
	//! A Texture to upload rendered images
	struct texture {

		texture(SDL_Renderer * renderer, size_t w, size_t h);

		//! Lock access to texture and get a raw data pointer
		/**
		 * @param pitch The pitch of the returned data pointer
		 */
		void * lock(int & pitch);

		//! Unlock previous access to texture and invalidate pointer
		void unlock();

		//! Get width of texture
		inline size_t width() const {
			return m_width;
		}

		//! Get height of texture
		inline size_t height() const{
			return m_height;
		}

		//! Get the SDL handle to texture
		inline SDL_Texture * handle() const {
			return mp_texture;
		}

		//! Upload a framebuffer to texture
		template<class T>
		void upload(thrender::framebuffer_<T> & fb){
			int texture_pitch;
			Uint8 * data_ptr = reinterpret_cast<Uint8 *>(lock(texture_pitch));

			for(unsigned row = 0;row < height();++row) {
				Uint32 * dst_pixel = (Uint32*)(data_ptr + row * texture_pitch);
				for(unsigned col = 0;col < width();++col) {
					T color = fb[row][col] * 255.0f;
					*dst_pixel++ = details::convert_to_uint32_pixel<T>::convert(color);
				}
			}
			unlock();
		}

	private:
		//! SDL Handle to texture object
		SDL_Texture * mp_texture;

		//! Width of texture
		size_t m_width;

		//! Height of texture
		size_t m_height;
	};

	//! Create a gui window
	struct window {

		//! Construct and create a window
		window(const std::string & title, size_t width, size_t height, unsigned int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

		//! Clear renderer
		void clear();

		//! Copy texture to back renderer
		void copy(int x, int y, texture & src);

		//! Update window from back renderer content
		void update();

		//! Get SDL handle to renderer
		inline SDL_Renderer * renderer_handle() const {
			return mp_renderer;
		}

		//! Get SDL handle to window
		inline SDL_Window * window_handle() const {
			return mp_window;
		}

	private:

		//! Initialization of the window (called by constructors)
		void initialize(const std::string & title, size_t x, size_t y, size_t w, size_t h, unsigned flags);

		//! SDL handle to window
		SDL_Window * mp_window;

		//! SDL handle to renderer
		SDL_Renderer * mp_renderer;
	};

}
