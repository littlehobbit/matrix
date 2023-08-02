cmake_minimum_required(VERSION 3.10)

project(${PROJECT_NAME}_tests)

find_package(GTest REQUIRED)

add_executable(
  ${PROJECT_NAME}
  test_matrix.cpp
)

target_link_libraries(
  ${PROJECT_NAME} PRIVATE
  matrix_lib
  GTest::gtest
  GTest::gtest_main
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

if (${ENABLE_COVERAGE})
  include(${CMAKE_SOURCE_DIR}/cmake/code_coverage.cmake)
  append_coverage_compiler_flags_to_target(${PROJECT_NAME})
endif()
