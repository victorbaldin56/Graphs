cmake_minimum_required(VERSION 3.14)
project(graphs)

include(CTest)

option(ENABLE_TESTING "Enable testing" ON)
if (ENABLE_TESTING)
  enable_testing()

  add_subdirectory(tests)
endif()
