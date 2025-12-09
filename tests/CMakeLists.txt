add_test(NAME GraphvizOutputTest
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py --executable $<TARGET_FILE:graphs_main>)

add_executable(unit_test unit_tests.cc)
target_link_libraries(unit_test gtest::gtest graphs)

gtest_discover_tests(unit_test)
