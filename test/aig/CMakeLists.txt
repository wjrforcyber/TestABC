add_executable(aig aig.cc aigCut.cc aigDup.cc)

add_compile_definitions(PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(aig PRIVATE PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")

target_link_libraries(aig
    gtest_main
    libabc
)

gtest_discover_tests(aig
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)