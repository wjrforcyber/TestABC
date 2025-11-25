add_executable(tt truth.cc)

add_compile_definitions(PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(tt PRIVATE PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")

target_link_libraries(tt
    gtest_main
    libabc
)

gtest_discover_tests(tt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)