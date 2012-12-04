#pragma once

#include <boost/chrono.hpp>

template < class CLOCK>
struct perf_timer {

	typedef	CLOCK clock;

	typedef typename CLOCK::time_point time_point;

	typedef typename CLOCK::duration duration;

	time_point last_measure;

	perf_timer() {
		last_measure = clock::now();
	}

	//! Return duration since last reset
	duration passed(){
		return clock::now() - last_measure;
	}

	//! Return duration since last reset and reset
	duration reset() {
		time_point now = clock::now();
		duration passed = now - last_measure;
		last_measure = clock::now();
		return passed;
	}
};
