project(TELICAM_DRIVER)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Telicam SDK
set(TELICAM_INCLUDE_DIR "/opt/TeliCamSDK/include")
set(TELICAM_LIB_DIR "/opt/TeliCamSDK/lib")

include_directories(${TELICAM_INCLUDE_DIR})
link_directories(${TELICAM_LIB_DIR})

include_directories(include)

# Source files
set(SOURCE_FILES src/telicam.cpp src/main.cpp)

# Executable
add_executable(telicam_driver ${SOURCE_FILES})
target_link_libraries(telicam_driver ${OpenCV_LIBS} TeliCamApi TeliCamApi_64 TeliCamUtl TeliCamUtl_64)
