add_executable(miniaig miniaig.cc)

add_compile_definitions(PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(miniaig PRIVATE PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")

target_link_libraries(miniaig
    gtest_main
    libabc
)

gtest_discover_tests(miniaig
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)