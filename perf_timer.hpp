#pragma once

#include <boost/chrono.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <sstream>

template < class CLOCK>
struct perf_timer {

	typedef	CLOCK clock;

	typedef typename CLOCK::time_point time_point;

	typedef typename CLOCK::duration duration;

	time_point last_measurement;

	perf_timer() {
		last_measurement = clock::now();
	}

	//! Return duration since last reset
	duration passed(){
		return clock::now() - last_measurement;
	}

	//! Return duration since last reset and reset
	duration reset() {
		time_point now = clock::now();
		duration passed = now - last_measurement;
		last_measurement = clock::now();
		return passed;
	}
};

template<class CLOCK>
struct profiler{

	typedef perf_timer<CLOCK> timer_type;

	typedef boost::tuple<std::string, typename timer_type::duration> checkpoint_tuple;

	typedef std::vector<checkpoint_tuple > checkpoint_vector;
	checkpoint_vector checkpoints;

	timer_type timer;

	std::string name;

	profiler(const std::string & _name)
	:
		name(_name)
	{
	}

	void checkpoint(const std::string & name) {
		checkpoints.push_back(checkpoint_tuple(name, timer.passed()));
	}

	void reset() {
		checkpoints.clear();
		timer.reset();
	}

	std::string report() {
		std::stringstream ss;
		ss << ">>" << name << std::endl;
		typename timer_type::duration dt_last;
		for(typename checkpoint_vector::iterator it = checkpoints.begin();it != checkpoints.end();it++) {
			ss << "\t";
			ss.width(20);
			ss << std::left << boost::get<0>(*it) << ": ";
			ss.width(10);
			ss << std::right << (boost::get<1>(*it) - dt_last) << std::endl;
			dt_last = boost::get<1>(*it);
		}
		ss << "<< total: " << dt_last << std::endl;
		return ss.str();
	}
};

