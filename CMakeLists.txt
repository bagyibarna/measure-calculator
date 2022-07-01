cmake_minimum_required(VERSION 3.20.0)
project(measure-calculator)

if (NOT DEFINED MEASURE_CALCULATOR_DEV)
	if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
		set(MEASURE_CALCULATOR_DEV ON)
	else()
		set(MEASURE_CALCULATOR_DEV OFF)
	endif()
endif()

# add header only library
if (MEASURE_CALCULATOR_DEV)
	file(GLOB_RECURSE MEASURE_CALCULATOR_DEV_HPP_SOURCE_FILES CONFIGURE_DEPENDS include/*.hpp)
endif()

add_library(measure-calculator INTERFACE ${MEASURE_CALCULATOR_DEV_HPP_SOURCE_FILES})
target_include_directories(measure-calculator INTERFACE include/)

add_subdirectory(3pp)

if(MEASURE_CALCULATOR_DEV)
	include(CTest)
	enable_testing()

	add_subdirectory(test)
endif()


