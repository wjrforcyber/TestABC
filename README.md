# TestABC

This is a fork version of the original Berkeley's [ABC](https://github.com/berkeley-abc/abc), which I use to write unit test in `test` folder. This probably won't be updated to the upstream since there are too many nit-picky tests, these tests are designed to be as minimum as possible to verify if the implementation align with your concept, and at the same time, help check the bug point if it exists.
<br>These tests would be a set of really helpful tutorials for correctly using each one of the interfaces. 

## 🔨 Build
The test framework is Googletest, it was added in the original ABC in CMake file, build with CMake:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
I suggest you enable `-DCMAKE_BUILD_TYPE=Debug` to get symbol table, so that you could launch lldb, gdb, etc. to debug if anything goes wrong.

## 🧪 Test list
This section will include all the tests and test suite.

- `AigTest, ConstructAig` : Construct a 2 input and 1 output AIG(A single AND node).
- `AigTest, StructureAnalysisAig` : Analysis complemented attributes on simple AIG.
- `AigTest, SimulationAig` : Simulation on every condition of a 2 input AND gate. It shows a way isolated from `resub` to perform an exact simulation.