add_executable(sop sop.cc)

add_compile_definitions(PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(sop PRIVATE PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")

target_link_libraries(sop
    gtest_main
    libabc
)

gtest_discover_tests(sop
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
