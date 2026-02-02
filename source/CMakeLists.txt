cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(ToolpathConverter VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Collect all source files
file(GLOB TOOLPATH_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ZLIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/zlib/Source/*.c")

# Add the executable with all source files
add_executable(ToolpathConverter ${TOOLPATH_SOURCES} ${ZLIB_SOURCES})

# Include directories
target_include_directories(ToolpathConverter PRIVATE ../include/CppDynamic)
target_include_directories(ToolpathConverter PRIVATE ./Common)
target_include_directories(ToolpathConverter PRIVATE ./Libraries/zlib/Include)
target_include_directories(ToolpathConverter PRIVATE ./Libraries/fast_float/Include)