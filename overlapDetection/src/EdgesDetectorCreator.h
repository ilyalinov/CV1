#pragma once
#include "EdgesDetector.h"
#include "DFTFilter.h"
#include "Laplacian.h"

class EdgesDetectorCreator {
public:
	virtual EdgesDetector* create() = 0;

	virtual ~EdgesDetectorCreator() {};
};

class DFTCreator : public EdgesDetectorCreator {
public:
	virtual EdgesDetector* create() {
		return new ::DFTFilter();
	};
};

class LaplacianCreator : public EdgesDetectorCreator {
public:
	virtual EdgesDetector* create() {
		return new ::Laplacian();
	};
};