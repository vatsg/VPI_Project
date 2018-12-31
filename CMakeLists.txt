cmake_minimum_required(VERSION 3.7)
project(c___code)
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Sharpening.cpp Sharpening.h Contouring.cpp Contouring.h Reconstruction.cpp Reconstruction.h Contour.cpp Contour.h Tree.cpp Tree.h)
add_executable(c___code ${SOURCE_FILES})

include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(c___code ${OpenCV_LIBS})