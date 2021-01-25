cmake_minimum_required(VERSION 2.8) 

project(overlapDetection)

find_package(OpenCV REQUIRED)

set(NAME_SRC
	src/DFTFilter.cpp
	src/Laplacian.cpp
	src/Source.cpp
	src/Timer.cpp
	src/VideoPlayer.cpp
)

set(NAME_HEADERS       
	src/DFTFilter.h
	src/EdgesDetector.h
	src/Laplacian.h
	src/Timer.h
	src/VideoPlayer.h
)

add_executable(overlapDetection ${NAME_SRC} ${NAME_HEADERS})
target_link_libraries(overlapDetection ${OpenCV_LIBS})