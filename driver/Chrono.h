#pragma once

#include <chrono>
#include "core/NonCopyable.h"
#include <iostream>
#include <EASTL/string.h>

class Duration {

public:
	using u64 = unsigned long int;
	using u32 = unsigned int;

	Duration(u64 seconds, u32 subsec_nanos) : _secs(seconds), _subsec_ns(subsec_nanos) {
	}

	u64 to_nanos() const {
		return _secs * 1000000000 + _subsec_ns;
	}

	double to_micros() const {
		return _secs * 1000000 + _subsec_ns / 1000.0;
	}

	double to_millis() const {
		return _secs * 1000 + _subsec_ns / 1000000.0;
	}

	double to_secs() const {
		return _secs + _subsec_ns / 1000000000.0;
	}

	u64 seconds() const {
		return _secs;
	}

	u32 subsec_nanos() const {
		return _subsec_ns;
	}

private:
	u64 _secs;
	u32 _subsec_ns;
};


class Chrono {
	using Nano = std::chrono::nanoseconds;

public:
	Chrono() {
		start();
	}

	void start() {
		_time = std::chrono::high_resolution_clock::now();
	}

	Duration reset() {
		auto e = elapsed();
		start();
		return e;
	}

	Duration elapsed() const {
		auto nanos = Duration::u64(std::chrono::duration_cast<Nano>(std::chrono::high_resolution_clock::now() - _time).count());
		return Duration(nanos / 1000000000, nanos % 1000000000);
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _time;
};

class DebugTimer : NonCopyable {

public:
	DebugTimer(const char* msg) : _msg(msg) {
	}

	~DebugTimer() {
		std::cout << _msg.c_str() << ": " << _chrono.elapsed().to_millis() << "ms" << std::endl;
	}

private:
	eastl::string _msg;
	Chrono _chrono;
};