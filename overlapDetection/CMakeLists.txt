cmake_minimum_required(VERSION 2.8) 

project(overlapDetection)

find_package(OpenCV REQUIRED)

set(NAME_SRC
	src/Clustering.cpp
	src/Configuration.cpp
	src/DFTFilter.cpp
	src/ExpFilter.cpp
	src/Laplacian.cpp
	src/Mean.cpp
	src/OpticalFlow.cpp
	src/Source.cpp
	src/Timer.cpp
	src/VideoHandler.cpp
)

set(NAME_HEADERS
	src/Clustering.h   
	src/Configuration.h    
	src/DFTFilter.h
	src/EdgesDetector.h
	src/EdgesDetectorCreator.h
	src/ExpFilter.h
	src/Laplacian.h
	src/Mean.h
	src/OpticalFlow.h
	src/Smoothing.h
	src/SmoothingCreator.h
	src/Timer.h
	src/VideoHandler.h
)

add_executable(overlapDetection ${NAME_SRC} ${NAME_HEADERS})
target_link_libraries(overlapDetection ${OpenCV_LIBS})