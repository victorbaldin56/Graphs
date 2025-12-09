cmake_minimum_required(VERSION 3.14)
project(graphs)

find_package(Boost REQUIRED)
find_package(GTest REQUIRED)

include(CTest)
include(GoogleTest)

add_library(graphs INTERFACE)
target_include_directories(graphs INTERFACE include)
target_link_libraries(graphs INTERFACE boost::boost)

add_executable(graphs_main src/main.cc)
target_link_libraries(graphs_main PRIVATE graphs)

option(ENABLE_TESTING "Enable testing" ON)
if (ENABLE_TESTING)
  enable_testing()

  add_subdirectory(tests)
endif()
