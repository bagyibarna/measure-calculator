if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /w44062")
else()
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
endif()

file(GLOB_RECURSE MEASURE_CALCULATOR_TEST_SOURCE_FILES CONFIGURE_DEPENDS test-cases/*.cpp test-cases/*.hpp)
add_library(test-cases OBJECT ${MEASURE_CALCULATOR_TEST_SOURCE_FILES})
target_link_libraries(test-cases PUBLIC measure-calculator doctest)
target_compile_options(test-cases PUBLIC "-fsanitize=undefined,address")
target_link_options(test-cases PUBLIC "-fsanitize=undefined,address")
set_property(TARGET test-cases PROPERTY CXX_STANDARD 20)


add_executable(test-exe "doctest-main.cpp")
target_link_libraries(test-exe PRIVATE test-cases)
set_property(TARGET test-exe PROPERTY CXX_STANDARD 20)

add_executable(manual-test-exe "manual-test-main.cpp")
target_link_libraries(manual-test-exe PRIVATE measure-calculator)
set_property(TARGET manual-test-exe PROPERTY CXX_STANDARD 20)


add_test(test-exe test-exe)