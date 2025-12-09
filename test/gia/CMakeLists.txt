add_executable(gia gia.cc giaSingleOperation.cc giaSamplesSweeping.cc)

target_link_libraries(gia
    gtest_main
    libabc
)

gtest_discover_tests(gia
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)