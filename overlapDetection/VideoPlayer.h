#pragma once
#include "EdgesDetector.h"

class VideoPlayer {
	private:
		std::string pathToVideo;
		EdgesDetector* detector;

	public:
		VideoPlayer(std::string& inputName, EdgesDetector* e) : pathToVideo(inputName), detector(e) {};

		void playVideo();
};