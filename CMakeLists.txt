cmake_minimum_required(VERSION 3.20.0)
project(calc-cpp)

if (NOT DEFINED CALC_DEVELOPMENT)
	if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
		set(CALC_DEVELOPMENT ON)
	else()
		set(CALC_DEVELOPMENT OFF)
	endif()
endif()

# add header only library
if (CALC_DEVELOPMENT)
	file(GLOB_RECURSE CALC_CPP_SOURCE_FILES CONFIGURE_DEPENDS include/*.hpp)
endif()

add_library(calc-cpp INTERFACE ${CALC_CPP_SOURCE_FILES})
target_include_directories(calc-cpp INTERFACE include/)
if (CALC_DEVELOPMENT)
	set_property(TARGET calc-cpp PROPERTY CXX_STANDARD 20)
endif()

add_subdirectory(3pp)

if(CALC_DEVELOPMENT)
	include(CTest)
	enable_testing()

	add_subdirectory(test)
endif()


