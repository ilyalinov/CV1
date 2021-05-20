#pragma once
#include "Smoothing.h"
#include "ExpFilter.h"
#include "Mean.h"

class SmoothingCreator {
public:
	virtual Smoothing* create() = 0;

	virtual ~SmoothingCreator() {};
};

class MeanCreator : public SmoothingCreator {
public:
	virtual Smoothing* create() {
		return new ::Mean();
	};
};

class ExpFilterCreator : public SmoothingCreator {
public:
	virtual Smoothing* create() {
		return new ::ExpFilter();
	};
};