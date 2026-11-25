add_executable(bal balance.cc)

target_link_libraries(bal
    gtest_main
    libabc
)

gtest_discover_tests(bal
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    # the first launch of a new executable can take several seconds on macOS
    DISCOVERY_TIMEOUT 60
)
