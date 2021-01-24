cmake_minimum_required(VERSION 3.8)

project(opencv_example_project)

# find opencv
find_package(OpenCV REQUIRED)

# add sources
add_executable(opencv_example Source.cpp)

target_link_libraries(opencv_example PRIVATE ${OpenCV_LIBS})