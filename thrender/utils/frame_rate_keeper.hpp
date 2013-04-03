#pragma once

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <iostream>

namespace thrender {
namespace utils {

	namespace C = boost::chrono;

	//! Helper structure to lock frame rate
	/**
	 * Create an object with the desired frame rate
	 * and call keep_frame_rate() at the end of the rendering loop.
	 */
	template <class CLOCK = C::steady_clock>
	struct frame_rate_keeper {

		//! Type of the clock that is used
		typedef CLOCK clock;

		//! Type of time point returned by this clock
		typedef typename CLOCK::time_point time_point;

		//! Type of duration returned by this clock
		typedef typename CLOCK::duration duration;

		//! Create a frame rate keeper object
		/**
		 * @param target_frame_rate The desired frame rate that keep_frame_rate() function
		 * will try to approximate.
		 */
		frame_rate_keeper(float target_frame_rate):
			m_target_frame_rate(target_frame_rate),
			m_frame_period(long(1000.0f/target_frame_rate))
		{
			m_previous_frame_time = clock::now();
		}

		//! The responsible function to keep frame rate
		/**
		 * Should be called at the end of each render loop. It will
		 * calculate the loop duration and will block the needed time
		 * to adjust the frame rate to the desired one.
		 *
		 * @note If the rendering takes more time than the desired period, it
		 * is like it was never called.
		 */
		void keep_frame_rate() {
			C::milliseconds time_passed =
					C::duration_cast<C::milliseconds>(clock::now() - m_previous_frame_time);
			C::milliseconds time_to_wait = m_frame_period - time_passed;
			if(time_to_wait.count() > 0) {
				boost::this_thread::sleep(boost::posix_time::milliseconds(time_to_wait.count()));
			}
			m_previous_frame_time = clock::now();
		}
	private:

		//! Target final fps
		float m_target_frame_rate;

		//! Target period of a frame (1/target_fps)
		C::milliseconds m_frame_period;

		//! Time of previous frame
		time_point m_previous_frame_time;
	};
}
}
