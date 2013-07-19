#pragma once

#include <boost/chrono.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/typeof/typeof.hpp>
#include <vector>
#include <sstream>

namespace thrender {
namespace utils {

	//! A simple timer to measure time since the reset point.
	/**
	 * This a clock wrapper to get relative duration since
	 * it was last reseted.
	 * @param CLOCK boost clock type that will be used for measuring time.
	 */
	template < class CLOCK>
	struct plain_timer {

		//! Type of the clock used to measure time
		typedef	CLOCK clock_type;

		//! Type of the time point
		typedef typename CLOCK::time_point time_point;

		//! Time of the duration
		typedef typename CLOCK::duration duration;

		//! Construct the timer and initialize it to present
		plain_timer() {
			m_last_reset = clock_type::now();
		}

		//! Return duration since last reset
		duration passed(){
			return clock_type::now() - m_last_reset;
		}

		//! Reset the timer
		/**
		 * @return The time passed since last reset
		 */
		duration reset() {
			time_point now = clock_type::now();
			duration passed = now - m_last_reset;
			m_last_reset = clock_type::now();
			return passed;
		}

	private:

		//! Time of last_measurement
		time_point m_last_reset;
	};

	//! Profiler to record the performance of a system
	/**
	 * It provides the mechanism to record the time
	 * between two checkpoints.
	 */
	template<class CLOCK>
	struct profiler{

		//! Type of timer
		typedef plain_timer<CLOCK> timer_type;

		//! Type of duration
		typedef typename timer_type::duration duration;

		//! Construct a new profiler
		/**
		 * @param _name A symbolic name to title this profiler
		 */
		profiler(const std::string & _name)
		:
			last_measurement(0),
			name(_name)
		{}

		//! Call it to record the time passed since last record
		/**
		 * @param _name The name of this checkpoint
		 */
		void record_checkpoint(const std::string & _name) {
			duration passed = timer.passed();
			checkpoints.push_back(checkpoint_tuple(_name, passed - last_measurement));
			last_measurement = passed;
		}

		//! Drop any recorded time till now.
		/**
		 * Any time measured till now will be not included in the next recorded chechpoint.
		 */
		void drop_measured_time() {
			last_measurement = timer.passed();
		}

		//! Clear all records on this profiler
		void clear() {
			checkpoints.clear();
			timer.reset();
			last_measurement = duration(0);
		}

		//! Generate report from this profiler
		/**
		 * It will generate a human readable text containing
		 * all the recorded checkpoints.
		 */
		std::string report() {

			// Find size of longest name
			size_t longest_name_size = 0;
			for(typename checkpoint_vector::iterator it = checkpoints.begin();it != checkpoints.end();it++) {
				if (boost::get<0>(*it).size() > longest_name_size)
					longest_name_size = boost::get<0>(*it).size();
			}

			std::stringstream ss;
			ss << ">> " << name << std::endl;

			duration dt_total_tracked(0);
			for(typename checkpoint_vector::iterator it = checkpoints.begin();it != checkpoints.end();it++) {
				duration & dt_checkpoint = boost::get<1>(*it);

				ss << "\t"
						<< std::setw(longest_name_size + 2) << std::left << boost::get<0>(*it) << ": "
						<< std::setw(10) << std::right << dt_checkpoint
						<< std::endl;
				dt_total_tracked += dt_checkpoint;
			}
			ss << "\t"
					<< std::setw(longest_name_size + 2) << std::left << "(not recorded)" << ": "
					<< std::setw(10) << std::right << (last_measurement - dt_total_tracked)
					<< std::endl;
			ss << "<< total: " << last_measurement << std::endl;
			return ss.str();
		}

	private:

		//! Type of checkpoint tuple
		typedef boost::tuple<std::string, typename timer_type::duration> checkpoint_tuple;

		//! Type of checkpoints vector
		typedef std::vector<checkpoint_tuple > checkpoint_vector;

		//! All recorded checkpoints
		checkpoint_vector checkpoints;

		//! The timer object
		timer_type timer;

		//! Relative time of last measurement
		duration last_measurement;

		//! Name of this profiler
		std::string name;

	};

	//! Automatic block profiler
	/**
	 * It will record the time from construction to destruction
	 * of this object.
	 *
	 * @see PROFILE_BLOCK
	 */
	template<class PROFILER>
	struct block {

		//! Construct a block profiler
		/**
		 * @param _profiler The profiler to use for recording
		 * @param _name Name of this block timer
		 */
		block(PROFILER & _profiler, const std::string & _name)
		:
			m_profiler(_profiler),
			m_name(_name)
		{
			m_profiler.drop_measured_time();
		}

		//! Destruct and write time on profiler
		~block(){
			m_profiler.record_checkpoint(m_name);
		}

	private:
		//! Reference to belonging profiler
		PROFILER & m_profiler;

		//! Pointer to name
		std::string m_name;
	};

	//! Helper function to easily create the block object
#	define PROFILE_BLOCK(prof, name) \
		thrender::utils::block<BOOST_TYPEOF(prof) > checkpoint_automatic_block(prof, name);

}
}
