# TestABC
[![.github/workflows/build-posix.yml](https://github.com/wjrforcyber/TestABC/actions/workflows/build-posix.yml/badge.svg)](https://github.com/wjrforcyber/TestABC/actions/workflows/build-posix.yml)
[![.github/workflows/build-posix-cmake.yml](https://github.com/wjrforcyber/TestABC/actions/workflows/build-posix-cmake.yml/badge.svg)](https://github.com/wjrforcyber/TestABC/actions/workflows/build-posix-cmake.yml)

This is a fork version of the original Berkeley's [ABC](https://github.com/berkeley-abc/abc), which I use to write unit test in `test` folder. This probably won't be updated to the upstream since there are too many nit-picky tests, these tests are designed to be as minimum as possible to verify if the implementation align with your concept, and at the same time, help check the bug point if it exists.
<br>These tests would be a set of really helpful tutorials for correctly using each one of the interfaces. 

## 🔨 Build
The test framework is Googletest, it was added in the original ABC in CMake file, simple build with CMake:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
> [!TIP]
> I suggest you enable `-DCMAKE_BUILD_TYPE=Debug` to get symbol table, so that you could launch lldb, gdb, etc. to debug if anything goes wrong. This is a simple example, personally I recommend you build it with Ninja, by default full load on threads. You could provide customized namespace if you want. Check workflow file for detail. Or just

```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build
```

## 🧪 Test list
This section will include all the tests and test suite.

- `AigTest, ConstructAig` : Construct a 2 input and 1 output AIG(A single AND node).
- `AigTest, StructureAnalysisAig` : Analysis complemented attributes on simple AIG.
- `AigTest, PhasesAig` : Phases are actually initialized in AIG when you call function `Abc_Obj_t * Abc_AigAnd( Abc_Aig_t * pMan, Abc_Obj_t * p0, Abc_Obj_t * p1 )`. It is determined by 4 factors, the **complemented attributes** and **regular node phases** of both $p_0$ and $p_1$.
- `AigTest, SimulationAig` : Simulation on every condition of a 2 input AND gate. It shows a way isolated from `resub` to perform an exact simulation.
- `AigTest, Simulation6InputsAig` : Give a detailed analysis on a 6-input window from a case i10.aig. Show the exact simulation result in some internal nodes. Check this example to get a deep comprehension about the phases and node encoding process. The window is structured as below:
<p align="center">
    <img src="./resources/images/SimulationCase.png" width="250"/>
</p>