#include "gui.hpp"
#include <stdexcept>


namespace thrender {

	const std::string generate_sdl_error(const std::string & prefix) {
		std::stringstream ss;
		ss << prefix << ": " << SDL_GetError();
		return ss.str();
	}

	void initialize_sdl() {
		if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
			throw std::runtime_error(generate_sdl_error("Unable to initialize SDL").c_str());
		}
	}

	texture::texture(SDL_Renderer * renderer, size_t w, size_t h) :
		m_width(w),
		m_height(h)
	{
		mp_texture = SDL_CreateTexture(
				renderer,
				SDL_PIXELFORMAT_ARGB8888,
				SDL_TEXTUREACCESS_STREAMING,
				w, h);
		if (!mp_texture) {
			throw std::runtime_error(generate_sdl_error("Couldn't create texture").c_str());
		}
	}

	void * texture::lock(int & pitch) {
		void * data_ptr;
		if (SDL_LockTexture(mp_texture, NULL, &data_ptr, &pitch) < 0) {
			throw std::runtime_error(generate_sdl_error("Couldn't lock texture").c_str());
		}
		return data_ptr;
	}

	void texture::unlock() {
		SDL_UnlockTexture(mp_texture);
	}

	window::window(const std::string & title, size_t width, size_t height, unsigned int flags) {
		initialize(
			title.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width,
			height,
			flags
		);
	}

	void window::clear() {
		SDL_RenderClear(mp_renderer);
	}

	void window::copy(int x, int y, texture & src) {
		SDL_Rect dstrect;
		dstrect.x = x;
		dstrect.y = y;
		dstrect.w = src.width();
		dstrect.h = src.height();
		SDL_RenderCopy(mp_renderer, src.handle(), NULL, &dstrect);
	}

	void window::update() {
		SDL_RenderPresent(mp_renderer);
	}

	void window::initialize(const std::string & title, size_t x, size_t y, size_t w, size_t h, unsigned flags) {
		initialize_sdl();

		mp_window = SDL_CreateWindow(
				title.c_str(),
				x,
				x,
				w,
				h,
				flags
		);

		mp_renderer = SDL_CreateRenderer(mp_window, -1, 0);
		if (!mp_renderer) {
			throw std::runtime_error(generate_sdl_error("Couldn't create renderer:").c_str());
		}
	}


}
