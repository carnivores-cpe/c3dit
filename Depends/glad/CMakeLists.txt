cmake_minimum_required(VERSION 3.20)

project(glad)

include_directories(include)

set(GLAD_SOURCE_FILES glad.h glad.c)

if(BUILD_SHARED_LIB)
    add_library(glad SHARED ${GLAD_SOURCE_FILES})
else()
    add_library(glad STATIC ${GLAD_SOURCE_FILES})
endif()