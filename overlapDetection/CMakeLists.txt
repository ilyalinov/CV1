cmake_minimum_required(VERSION 2.8) 

project(overlapDetection)

find_package(OpenCV REQUIRED)

set(NAME_SRC
	src/Configuration.cpp
	src/DFTFilter.cpp
	src/Laplacian.cpp
	src/Source.cpp
	src/Timer.cpp
	src/VideoHandler.cpp
)

set(NAME_HEADERS   
	src/Configuration.h    
	src/DFTFilter.h
	src/EdgesDetector.h
	src/Laplacian.h
	src/Timer.h
	src/VideoPlayer.h
)

add_executable(overlapDetection ${NAME_SRC} ${NAME_HEADERS})
target_link_libraries(overlapDetection ${OpenCV_LIBS})