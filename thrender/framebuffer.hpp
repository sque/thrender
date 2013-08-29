#pragma once

#include "./types.hpp"
#include <thrust/fill.h>

namespace thrender {

	/**
	 * @brief Abstract framebuffer inplementation
	 *
	 * A framebuffer container of any type and size
	 */
	struct framebuffer {
		framebuffer(size_t width, size_t height, size_t pixel_size)
		:
			m_width(width),
			m_height(height),
			m_pitch(width + ((width * pixel_size) % 4)),
			m_pixel_size(pixel_size),
			m_data_size(m_height * m_pitch * m_pixel_size)
		{
			m_data = new unsigned char[m_data_size];
		}

		//! Inheritable
		virtual ~framebuffer() {
			delete m_data;
		}

		//! Element access operator
		/**
		 * It will access the indice based on the size of pixel type
		 */
		template<class PixelType>
		inline PixelType * operator[](size_t row) {
			return reinterpret_cast<PixelType *>(m_data) + (row * m_pitch);
		}

		//! Element access operator (const)
		template<class PixelType>
		inline const PixelType * operator[](size_t row) const {
			return reinterpret_cast<const PixelType *>(m_data) + (row * m_pitch);
		}

		//! Element access at the nth pixel
		/**
		 * Fixme: It does not take in account the pitching
		 */
		template<class PixelType>
		inline PixelType & serial_at(size_t index) {
			return *(reinterpret_cast<PixelType *>(m_data) + index);
		}

		//! Element access at the nth pixel
		template<class PixelType>
		inline const PixelType & serial_at(size_t index) const{
			return *(reinterpret_cast<const PixelType *>(m_data) + index);
		}

		//! Pointer to raw data
		inline unsigned char * raw_data() {
			return m_data;
		}

		//! Pointer to raw data (const)
		inline const unsigned char * raw_data() const{
			return m_data;
		}

		//! Get the pitching of this buffer
		inline pitch_t pitch() const {
			return m_pitch;
		}

		//! Get the width of the framebuffer
		inline size_t width() const {
			return m_width;
		}

		//! Get the height of the framebuffer
		inline size_t height() const {
			return m_height;
		}

		//! Get the allocated data size
		inline size_t data_size() const {
			return m_data_size;
		}

		//! Get the shape of the framebuffer
		inline glm::uvec2 shape() const {
			return glm::uvec2(width(), height());
		}

	protected:

		//! Width of the framebuffer
		size_t m_width;

		//! Height of the framebuffer
		size_t m_height;

		//! Array pitching
		pitch_t m_pitch;

		//! Total pixels of the framebuffer
		size_t m_pixel_size;

		//! The actual allocated size
		size_t m_data_size;

		//! Pointer to data
		unsigned char * m_data;

		//non copyable
		framebuffer(const framebuffer &) = delete;
		framebuffer& operator=(const framebuffer &) = delete;
	};


	template<class PixelType>
	struct framebuffer_ :
		public framebuffer{

		//! Type of a pixel
		typedef PixelType pixel_type;

		typedef pixel_type value_type;

		typedef pixel_type * iterator;

		typedef const pixel_type * const_iterator;

		framebuffer_(size_t width, size_t height)
			:framebuffer(width, height, sizeof(pixel_type)){
		}

		inline iterator begin() {
			return reinterpret_cast<pixel_type*>(m_data);
		}

		inline const_iterator cbegin() const{
			return reinterpret_cast<const pixel_type*>(m_data);
		}

		inline iterator end() {
			return reinterpret_cast<pixel_type*>(m_data) + (m_pitch * m_height);
		}

		inline const_iterator cend() const{
			return reinterpret_cast<const pixel_type*>(m_data) + (m_pitch * m_height);
		}

		inline pixel_type * operator[](size_t row) {
			return framebuffer::operator[]<pixel_type>(row);
		}

		inline pixel_type & serial_at(size_t index) {
			return framebuffer::serial_at<pixel_type>(index);
		}

		inline const pixel_type & serial_at(size_t index) const {
			return framebuffer::serial_at<pixel_type>(index);
		}

		inline void clear(pixel_type & value) {
			thrust::fill(begin(), end(), value);
		}

	};

}
