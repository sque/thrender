cmake_minimum_required(VERSION 2.8)

project(thrender)

# Find SDL Package
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL sdl2)
include_directories(${SDL_INCLUDE_DIRS})

# Enable C++11
if(CMAKE_COMPILER_IS_GNUCXX)
	list( APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif()

add_subdirectory(thrender)
add_subdirectory(examples)
