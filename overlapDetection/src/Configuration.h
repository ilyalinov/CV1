#pragma once
#include <string>

#include "EdgesDetector.h"
#include "Smoothing.h"

class Configuration {
	public:
		Configuration(const std::string& filename);

		bool hasVideoRecording();

		bool hasMeanStandardDeviationMedianRecording();

		bool hasSaveResultsFlag();

		bool hasShowResultsFlag();

		int getSizeFactor();

		int getFramesLimit();

		std::string getPathToVideo();

		EdgesDetector::DetectorType getEdgesDetectorType();

		Smoothing::SmoothingType getSmoothingType();

		void print();
	private:
		enum class StringCode {
			cVideoRecording,
			cMeanStandardDeviationRecording,
			cSaveResults,
			cShowResults,
			cSizeFactor,
			cFramesLimit,
			cPathToVideo,
			detector,
			smoothing,
			other
		};

		bool storeValue(std::string& key, std::string& value);

		StringCode getCode(std::string& s);

		bool videoRecordingFlag = false;
		bool meanStandardDeviationMedianRecording = false;
		bool saveResultsFlag = false;
		bool showResultsFlag = true;
		int sizeFactor = 2;
		int framesLimit = 200;
		std::string pathToVideo;
		EdgesDetector::DetectorType detectorType = EdgesDetector::DetectorType::laplacian;
		Smoothing::SmoothingType smoothingType = Smoothing::SmoothingType::mean;
};