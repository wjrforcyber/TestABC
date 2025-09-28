add_executable(aig 
    aig.cc 
    ${CMAKE_SOURCE_DIR}/src/base/abci/abcResub.c
)

target_link_libraries(aig
    gtest_main
    libabc
)

gtest_discover_tests(aig
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)