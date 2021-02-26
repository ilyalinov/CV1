#pragma once
#include <string>

#include "EdgesDetector.h"

class Configuration {
	public:
		Configuration(const std::string& filename);

		bool hasVideoRecording();

		bool hasMeanStandardDeviationMedianRecording();

		bool hasSaveResultsFlag();

		bool hasShowResultsFlag();

		int getCompressionFactor();

		int getFramesLimit();

		std::string getPathToVideo();

		EdgesDetector::DetectorType getEdgesDetectorType();

		void print();
	private:
		enum class StringCode {
			cVideoRecording,
			cMeanStandardDeviationRecording,
			cSaveResults,
			cShowResults,
			cCompressionFactor,
			cFramesLimit,
			cPathToVideo,
			detector,
			other
		};

		bool storeValue(std::string& key, std::string& value);

		StringCode getCode(std::string& s);

		bool videoRecordingFlag = false;
		bool meanStandardDeviationMedianRecording = false;
		bool saveResultsFlag = false;
		bool showResultsFlag = true;
		int compressionFactor = 2;
		int framesLimit = 200;
		std::string pathToVideo;
		EdgesDetector::DetectorType detectorType = EdgesDetector::DetectorType::laplacian;
};