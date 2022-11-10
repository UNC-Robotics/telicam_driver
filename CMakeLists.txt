project(TELICAM_DRIVER)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)

##################################################
# Dependencies
##################################################

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Telicam SDK
set(TELICAM_INCLUDE_DIR "/opt/TeliCamSDK/include")
set(TELICAM_LIB_DIR "/opt/TeliCamSDK/lib")

include_directories(${TELICAM_INCLUDE_DIR})
link_directories(${TELICAM_LIB_DIR})

include_directories(include)

################################################
# Targets
################################################

# Executable
add_executable(telicam_viewer src/telicam.cpp src/main.cpp)
target_link_libraries(telicam_viewer ${OpenCV_LIBS} TeliCamApi_64 TeliCamUtl_64)

# Library
add_library(telicam SHARED src/telicam.cpp)
target_link_libraries(telicam ${OpenCV_LIBS} TeliCamApi_64 TeliCamUtl_64)
