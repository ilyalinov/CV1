#include "Timer.h"

#include <iostream>

void Timer::saveTimePoint() {
	v.push_back(std::chrono::high_resolution_clock::now());
}

void Timer::printLastDuration() {
	if (v.size() > 1) {
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(v[v.size() - 1] - v[v.size() - 2]).count();
		std::cout << "time between 2 most recent time points in microseconds: " << duration << "\n";
	}
}

void Timer::printFullDuration() {
	if (v.size() > 1) {
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(v[v.size() - 1] - v[0]).count();
		std::cout << "full duration: " << duration << "\n";
	}
}

void Timer::reset() {
	v.clear();
}