# TestABC

This is a fork version of the original Berkeley's [ABC](https://github.com/berkeley-abc/abc), which I use to write unit test in `test` folder. This probably won't be updated to the upstream since there are too many nit-picky tests, these tests are designed to be as minimum as possible to verify if the implementation align with your concept, and at the same time, help check the bug point if it exists.

## Build
The test framework is googletest, it was added in the original ABC in CMake file, build with CMake:
```
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
I suggest you enable `-DCMAKE_BUILD_TYPE=Debug` to get symbol table, so that you could launch lldb, gdb, etc. to debug if anything goes wrong.