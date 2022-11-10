include(FetchContent)

project(TELICAM_DRIVER)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)

##################################################
# Options
##################################################
option(BUILD_VIEWER "Build viewer" ON)

##################################################
# Dependencies
##################################################

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Telicam SDK
set(TELICAM_INCLUDE_DIR "/opt/TeliCamSDK/include")
set(TELICAM_LIB_DIR "/opt/TeliCamSDK/lib")

# JSON (for viewer)
if(BUILD_VIEWER)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
    FetchContent_MakeAvailable(json)
endif()

include_directories(${TELICAM_INCLUDE_DIR})
link_directories(${TELICAM_LIB_DIR})

include_directories(include)

################################################
# Targets
################################################

# Executable
if(BUILD_VIEWER)
    add_executable(telicam_viewer src/telicam.cpp src/main.cpp)
    target_link_libraries(telicam_viewer ${OpenCV_LIBS} TeliCamApi_64 TeliCamUtl_64 nlohmann_json::nlohmann_json)
endif()

# Library
add_library(telicam SHARED src/telicam.cpp)
target_link_libraries(telicam ${OpenCV_LIBS} TeliCamApi_64 TeliCamUtl_64)
