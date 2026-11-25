add_executable(dch dchSat.cc)

target_link_libraries(dch
    gtest_main
    libabc
)

gtest_discover_tests(dch
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    # the first launch of a new executable can take several seconds on macOS
    DISCOVERY_TIMEOUT 60
)
