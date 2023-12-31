######################################################################## 
 # Build Soapy SDR support module for HackRF
 ######################################################################## 

cmake_minimum_required(VERSION 3.10)
project(SoapyHackRFDuplex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(SoapySDR "0.4.0" NO_MODULE)
 if (NOT SoapySDR_FOUND) 
    message(FATAL_ERROR "Soapy SDR development files not found...") 
 endif () 

find_package(LIBHACKRF)
if (NOT LIBHACKRF_FOUND) 
     message(FATAL_ERROR "HackRF development files not found...") 
endif () 
message(STATUS "LIBHACKRF_INCLUDE_DIRS - ${LIBHACKRF_INCLUDE_DIRS}")
message(STATUS "LIBHACKRF_LIBRARIES - ${LIBHACKRF_LIBRARIES}")

#version check for recent hackrf with device list API
message(STATUS "Checking for hackrf_device_list API...")
message(STATUS "  Reading ${LIBHACKRF_INCLUDE_DIRS}/hackrf.h...")
file(READ ${LIBHACKRF_INCLUDE_DIRS}/hackrf.h hackrf_h)
string(FIND "${hackrf_h}" "hackrf_device_list" has_hackrf_device_list)
if ("${has_hackrf_device_list}" STREQUAL "-1")
    message(FATAL_ERROR "  libhackrf too old, missing hackrf_device_list API")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${LIBHACKRF_INCLUDE_DIRS})

#enable c++17 features
if(CMAKE_COMPILER_IS_GNUCXX)

    #c++17 is a required language feature for this project
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++17" HAS_STD_CXX11)
    if(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    else(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()

    #disable warnings for unused parameters
    add_definitions(-Wno-unused-parameter)

endif(CMAKE_COMPILER_IS_GNUCXX)

if (APPLE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wc++17-extensions")
endif(APPLE)

# SOAPY_SDR_MODULE_UTIL(
#     TARGET HackRFDuplexSupport
#     SOURCES
# 	src/TimedBufferQueue.cpp
#     LIBRARIES ${LIBHACKRF_LIBRARIES}
# )

add_executable(TimedBufferQueueExample 
    ${CMAKE_SOURCE_DIR}/src/TimedBufferQueue.cpp
    ${CMAKE_SOURCE_DIR}/example/TimedBufferQueue/main.cpp
)
target_include_directories(TimedBufferQueueExample PUBLIC ${CMAKE_SOURCE_DIR}/src/)


add_definitions(
    -w
)
