cmake_minimum_required(VERSION 3.10)
project(MyOpenCVApp)

# Use C++11 or newer
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find OpenCV package
find_package(OpenCV REQUIRED)


# Add your source files
add_executable(mytest hw.cpp ImageComparisonExpert.cpp TableComparisonExpert.cpp)

# Link OpenCV libraries
target_link_libraries(mytest ${OpenCV_LIBS})
