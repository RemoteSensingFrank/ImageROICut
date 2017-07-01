cmake_minimum_required(VERSION 3.7)
project(GetImageROI)

set(CMAKE_CXX_STANDARD 11)
set(GTest_LIBRARY "/usr/local/lib/libgtest.a")

set(GDAL_LIBRARY "/usr/local/lib/libgdal.so")
set(GDAL_INCLUDE "/usr/local/include/")

find_package(OpenCV REQUIRED)
find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${GDAL_INCLUDE})

set(SOURCE_FILES main.cpp)
add_executable(GetImageROI ${SOURCE_FILES})
target_link_libraries(GetImageROI ${GTEST_LIBRARIES})
target_link_libraries(GetImageROI ${GDAL_LIBRARY})
target_link_libraries(GetImageROI ${OpenCV_LIBRARIES})
TARGET_LINK_LIBRARIES(GetImageROI -lpthread -lm)