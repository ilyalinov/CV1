#pragma once
#include <chrono>
#include <vector>

class Timer {
	private:
		std::vector<std::chrono::high_resolution_clock::time_point> v;

	public:
		void saveTimePoint();

		void printLastDuration();

		void printFullDuration();

		void reset();
};

