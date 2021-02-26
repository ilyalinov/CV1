#include "Configuration.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

Configuration::Configuration(const string& filename) {
	std::ifstream fs(filename);
	bool isError = false;
	if (!fs.is_open()) {
		cout << "error opening configuration file" << "\n";
		return;
	}

	string line;
	while (getline(fs, line)) {
		istringstream ls(line);
		string key;
		if (getline(ls, key, '=')) {
			string value;
			if (getline(ls, value)) {
				//std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
				isError = storeValue(key, value);
				if (isError) {
					cout << "Incorrect configuration file format, using default settings." << "\n";
					break;
				}
			}
		}
	}

	if (isError) {
		videoRecordingFlag = false;
		meanStandardDeviationMedianRecording = false;
		saveResultsFlag = false;
		showResultsFlag = true;
		compressionFactor = 2;
		int framesLimit = 200;
		detectorType = EdgesDetector::DetectorType::laplacian;
	}
}

bool Configuration::hasVideoRecording() {
	return videoRecordingFlag;
}

bool Configuration::hasMeanStandardDeviationMedianRecording() {
	return meanStandardDeviationMedianRecording;
}

bool Configuration::hasSaveResultsFlag() {
	return saveResultsFlag;
}

bool Configuration::hasShowResultsFlag() {
	return showResultsFlag;
}

int Configuration::getCompressionFactor() {
	return compressionFactor;
}

int Configuration::getFramesLimit()
{
	return framesLimit;
}

std::string Configuration::getPathToVideo() {
	return pathToVideo;
}

EdgesDetector::DetectorType Configuration::getEdgesDetectorType() {
	return detectorType;
}

void Configuration::print() {
	cout << videoRecordingFlag << "\n";
	cout << meanStandardDeviationMedianRecording << "\n";
	cout << saveResultsFlag << "\n";
	cout << showResultsFlag << "\n"; 
	cout << compressionFactor << "\n";
	cout << framesLimit << "\n";
	cout << pathToVideo << "\n";
	cout << static_cast<std::underlying_type<EdgesDetector::DetectorType>::type>(detectorType) << "\n";
}

bool Configuration::storeValue(string& key, string& value) {
	bool isError = false;
	switch (getCode(key)) {
		case StringCode::cVideoRecording:
			if (value == "true") {
				videoRecordingFlag = true;
			}
			else if (value == "false") {
				videoRecordingFlag = false;
			}
			else {
				isError = true;
			}
			break;
		case StringCode::cMeanStandardDeviationRecording:
			if (value == "true") {
				meanStandardDeviationMedianRecording = true;
			}
			else if (value == "false") {
				meanStandardDeviationMedianRecording = false;
			}
			else {
				isError = true;
			}
			break;
		case StringCode::cSaveResults:
			if (value == "true") {
				saveResultsFlag = true;
			}
			else if (value == "false") {
				saveResultsFlag = false;
			}
			else {
				isError = true;
			}
			break;
		case StringCode::cShowResults:
			if (value == "true") {
				showResultsFlag = true;
			}
			else if (value == "false") {
				showResultsFlag = false;
			}
			else {
				isError = true;
			}
			break;
		case StringCode::cCompressionFactor:
			try {
				int n = stoi(value);
				if (n > 10 || n < 1) {
					isError = true;
				}
				else {
					compressionFactor = n;
				}
			}
			catch (const std::exception& e) {
				cout << e.what() << "\n";
				isError = true;
			}
			break;
		case StringCode::cFramesLimit:
			try {
				int n = stoi(value);
				if (n > 1000 || n <= 1) {
					isError = true;
				}
				else {
					framesLimit = n;
				}
			}
			catch (const std::exception& e) {
				cout << e.what() << "\n";
				isError = true;
			}
			break;
		case StringCode::cPathToVideo:
			pathToVideo = value;
			break;
		case StringCode::detector:
			if (value == "laplacian") {
				detectorType = EdgesDetector::DetectorType::laplacian;
			}
			else if (value == "dft") {
				detectorType = EdgesDetector::DetectorType::dft;
			}
			else {
				isError = true;
			}
			break;
		case StringCode::other:
			isError = true;
			break;
	}

	return isError;
}

Configuration::StringCode Configuration::getCode(string& s) {
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	if (s == "video.recording") return StringCode::cVideoRecording;
	if (s == "mean.standard.deviation.median.recording") return StringCode::cMeanStandardDeviationRecording;
	if (s == "save.results") return StringCode::cSaveResults;
	if (s == "show.results") return StringCode::cShowResults;
	if (s == "compression.factor") return StringCode::cCompressionFactor;
	if (s == "frames.limit") return StringCode::cFramesLimit;
	if (s == "path.to.video") return StringCode::cPathToVideo;
	if (s == "detector") return StringCode::detector;
	return StringCode::other;
}