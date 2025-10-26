add_executable(util util.cc)

target_link_libraries(util
    gtest_main
    libabc
)

gtest_discover_tests(util
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)