add_executable(gia gia.cc giaSingleOperation.cc giaSamplesSweeping.cc giaCut.cc)

target_compile_definitions(gia PRIVATE ASSETS_DIR="${CMAKE_BINARY_DIR}/test/")

target_link_libraries(gia
    gtest_main
    libabc
)

gtest_discover_tests(gia
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    # the first launch of a new executable can take several seconds on macOS
    DISCOVERY_TIMEOUT 60
)