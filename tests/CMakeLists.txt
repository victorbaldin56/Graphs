add_test(NAME GraphvizOutputTest
         COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py --executable $<TARGET_FILE:graphs_main>)
