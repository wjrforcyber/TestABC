# Copy test fixtures into the build tree; ASSETS_DIR points here.
file(COPY ${CMAKE_SOURCE_DIR}/test/data DESTINATION ${CMAKE_BINARY_DIR}/test)
file(COPY ${CMAKE_SOURCE_DIR}/NangateOpenCellLibrary_typical.lib DESTINATION ${CMAKE_BINARY_DIR}/test)

add_subdirectory(truth)
add_subdirectory(gia)
add_subdirectory(aig)
add_subdirectory(dch)
add_subdirectory(miniaig)
add_subdirectory(util)
add_subdirectory(sop)
