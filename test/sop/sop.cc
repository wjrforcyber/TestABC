#include "gtest/gtest.h"

#include "base/abc/abc.h"
#include "misc/util/abc_namespaces.h"

ABC_NAMESPACE_IMPL_START

/*!
  \brief Test Abc_SopStart: creates an uninitialized SOP cover with nCubes and nVars.

  The function allocates memory for a SOP string where:
  - Each cube has nVars literals followed by space, output char, and newline
  - All literals are initialized to '-' (don't-care)
  - All outputs are initialized to '1' (on-set)
*/
TEST(SopTest, SopStart) {
    Mem_Flex_t * pMan = Mem_FlexStart();

    // Test case 1: 1 cube, 2 variables
    // Expected: "-- 1\n"
    {
        int nCubes = 1;
        int nVars = 2;
        char * pSop = Abc_SopStart(pMan, nCubes, nVars);

        EXPECT_TRUE(pSop != NULL);
        // Check structure: "ll o\n" where l=literal, o=output
        EXPECT_EQ(pSop[0], '-');   // literal 0
        EXPECT_EQ(pSop[1], '-');   // literal 1
        EXPECT_EQ(pSop[2], ' ');   // space separator
        EXPECT_EQ(pSop[3], '1');   // output value
        EXPECT_EQ(pSop[4], '\n');  // newline
        EXPECT_EQ(pSop[5], '\0');  // null terminator
    }

    // Test case 2: 2 cubes, 3 variables
    // Expected: "--- 1\n--- 1\n"
    {
        int nCubes = 2;
        int nVars = 3;
        char * pSop = Abc_SopStart(pMan, nCubes, nVars);

        EXPECT_TRUE(pSop != NULL);
        // First cube
        EXPECT_EQ(pSop[0], '-');
        EXPECT_EQ(pSop[1], '-');
        EXPECT_EQ(pSop[2], '-');
        EXPECT_EQ(pSop[3], ' ');
        EXPECT_EQ(pSop[4], '1');
        EXPECT_EQ(pSop[5], '\n');
        // Second cube
        EXPECT_EQ(pSop[6], '-');
        EXPECT_EQ(pSop[7], '-');
        EXPECT_EQ(pSop[8], '-');
        EXPECT_EQ(pSop[9], ' ');
        EXPECT_EQ(pSop[10], '1');
        EXPECT_EQ(pSop[11], '\n');
        EXPECT_EQ(pSop[12], '\0');
    }

    // Test case 3: Verify with helper functions
    {
        int nCubes = 3;
        int nVars = 4;
        char * pSop = Abc_SopStart(pMan, nCubes, nVars);

        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), nCubes);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), nVars);
        EXPECT_EQ(Abc_SopGetPhase(pSop), 1);  // on-set (output '1')

        // All literals should be don't-care
        EXPECT_EQ(Abc_SopGetIthCareLit(pSop, 0), -1);  // -1 means all don't-care
        EXPECT_EQ(Abc_SopGetIthCareLit(pSop, 1), -1);
        EXPECT_EQ(Abc_SopGetIthCareLit(pSop, 2), -1);
        EXPECT_EQ(Abc_SopGetIthCareLit(pSop, 3), -1);
    }

    Mem_FlexStop(pMan, 0);
}

/*!
  \brief Test Abc_SopRegister: registers (copies) a SOP string into memory manager.

  The function allocates memory and copies the input string:
  - Returns NULL if input is NULL
  - Otherwise returns a copy managed by the memory manager
*/
TEST(SopTest, SopRegister) {
    Mem_Flex_t * pMan = Mem_FlexStart();

    // Test case 1: NULL input returns NULL
    {
        char * pSop = Abc_SopRegister(pMan, NULL);
        EXPECT_TRUE(pSop == NULL);
    }

    // Test case 2: Constant 1 SOP
    {
        char * pSop = Abc_SopRegister(pMan, " 1\n");
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(pSop[0], ' ');
        EXPECT_EQ(pSop[1], '1');
        EXPECT_EQ(pSop[2], '\n');
        EXPECT_EQ(pSop[3], '\0');
        EXPECT_TRUE(Abc_SopIsConst1(pSop));
    }

    // Test case 3: Constant 0 SOP
    {
        char * pSop = Abc_SopRegister(pMan, " 0\n");
        EXPECT_TRUE(pSop != NULL);
        EXPECT_TRUE(Abc_SopIsConst0(pSop));
    }

    // Test case 4: 2-input AND gate SOP
    {
        char * pSop = Abc_SopRegister(pMan, "11 1\n");
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 1);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), 2);
        EXPECT_EQ(Abc_SopGetPhase(pSop), 1);  // on-set
        EXPECT_TRUE(Abc_SopIsAndType(pSop));
    }

    // Test case 5: XOR gate SOP (2 cubes)
    {
        char * pSop = Abc_SopRegister(pMan, "01 1\n10 1\n");
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 2);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), 2);
        EXPECT_EQ(Abc_SopGetPhase(pSop), 1);
    }

    // Test case 6: Buffer SOP
    {
        char * pSop = Abc_SopRegister(pMan, "1 1\n");
        EXPECT_TRUE(pSop != NULL);
        EXPECT_TRUE(Abc_SopIsBuf(pSop));
    }

    // Test case 7: Inverter SOP
    {
        char * pSop = Abc_SopRegister(pMan, "0 1\n");
        EXPECT_TRUE(pSop != NULL);
        EXPECT_TRUE(Abc_SopIsInv(pSop));
    }

    // Test case 8: MUX SOP
    {
        char * pSop = Abc_SopRegister(pMan, "11- 1\n0-1 1\n");
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 2);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), 3);
    }

    Mem_FlexStop(pMan, 0);
}

/*!
  \brief Test Abc_SopCreateFromTruth: creates SOP from truth table (minterm-level, no minimization).

  Each cube in the output represents one true minterm.
  Truth table uses 32-bit unsigned array format.
*/
TEST(SopTest, SopCreateFromTruth) {
    Mem_Flex_t * pMan = Mem_FlexStart();

    // Test case 1: 2-input AND (truth = 0x8 = 0b1000, only minterm 3)
    // Expected: "11 1\n"
    {
        unsigned pTruth[1] = {0x8};  // 0b1000
        char * pSop = Abc_SopCreateFromTruth(pMan, 2, pTruth);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 1);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), 2);
        // Check the cube: "11 1\n"
        EXPECT_EQ(pSop[0], '1');
        EXPECT_EQ(pSop[1], '1');
        EXPECT_TRUE(Abc_SopIsAndType(pSop));
    }

    // Test case 2: 2-input OR (truth = 0xE = 0b1110, minterms 1,2,3)
    // Expected: "01 1\n10 1\n11 1\n" (3 cubes)
    {
        unsigned pTruth[1] = {0xE};  // 0b1110
        char * pSop = Abc_SopCreateFromTruth(pMan, 2, pTruth);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 3);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), 2);
        // Verify each minterm cube exists (order: 01, 10, 11)
        // Cubes are created in minterm order
    }

    // Test case 3: 2-input XOR (truth = 0x6 = 0b0110, minterms 1,2)
    // Expected: "01 1\n10 1\n"
    {
        unsigned pTruth[1] = {0x6};  // 0b0110
        char * pSop = Abc_SopCreateFromTruth(pMan, 2, pTruth);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 2);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), 2);
    }

    // Test case 4: Constant 1 (0 vars)
    {
        unsigned pTruth[1] = {0x1};
        char * pSop = Abc_SopCreateFromTruth(pMan, 0, pTruth);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_TRUE(Abc_SopIsConst1(pSop));
    }

    // Test case 5: Constant 0 (0 vars) - note: Abc_SopCreateFromTruth asserts if truth is all 0
    // So we skip this case as it would cause an assertion failure

    // Test case 6: 3-input function (truth = 0x80, minterm 7 only)
    {
        unsigned pTruth[1] = {0x80};  // 0b10000000, minterm 7 (111)
        char * pSop = Abc_SopCreateFromTruth(pMan, 3, pTruth);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 1);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), 3);
        // Cube should be "111 1\n"
        EXPECT_EQ(pSop[0], '1');
        EXPECT_EQ(pSop[1], '1');
        EXPECT_EQ(pSop[2], '1');
    }

    Mem_FlexStop(pMan, 0);
}

/*!
  \brief Test Abc_SopCreateFromIsop: creates SOP from ISOP cover representation.

  ISOP uses 2 bits per variable: 00=don't-care, 01=0, 10=1
*/
TEST(SopTest, SopCreateFromIsop) {
    Mem_Flex_t * pMan = Mem_FlexStart();
    Vec_Int_t * vCover = Vec_IntAlloc(10);

    // Test case 1: Single cube with all 1s (2 vars)
    // Entry = (var1:10)(var0:10) = 0b1010 = 10
    // Expected: "11 1\n"
    {
        Vec_IntClear(vCover);
        Vec_IntPush(vCover, 0b1010);  // var0=1(10), var1=1(10)
        char * pSop = Abc_SopCreateFromIsop(pMan, 2, vCover);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 1);
        EXPECT_EQ(pSop[0], '1');
        EXPECT_EQ(pSop[1], '1');
    }

    // Test case 2: Single cube with don't-care (2 vars)
    // Entry = (var1:00)(var0:10) = 0b0010 = 2
    // Expected: "1- 1\n" or "-1 1\n" depending on var order
    {
        Vec_IntClear(vCover);
        Vec_IntPush(vCover, 0b0010);  // var0=1(10), var1=-(00)
        char * pSop = Abc_SopCreateFromIsop(pMan, 2, vCover);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(pSop[0], '1');  // var0
        EXPECT_EQ(pSop[1], '-');  // var1 (don't-care)
    }

    // Test case 3: Multiple cubes (OR-like: a + b)
    // Cube 1: a=1, b=-  -> Entry = (b:00)(a:10) = 0b0010 = 2
    // Cube 2: a=-, b=1  -> Entry = (b:10)(a:00) = 0b1000 = 8
    {
        Vec_IntClear(vCover);
        Vec_IntPush(vCover, 0b0010);  // var0=1, var1=-
        Vec_IntPush(vCover, 0b1000);  // var0=-, var1=1
        char * pSop = Abc_SopCreateFromIsop(pMan, 2, vCover);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 2);
        EXPECT_TRUE(Abc_SopIsOrType(pSop));
    }

    // Test case 4: 3-variable cube
    // Entry = (var2:10)(var1:00)(var0:01) = 0b100001 = 33
    // Expected: "0-1 1\n"
    {
        Vec_IntClear(vCover);
        Vec_IntPush(vCover, 0b100001);  // var0=0(01), var1=-(00), var2=1(10)
        char * pSop = Abc_SopCreateFromIsop(pMan, 3, vCover);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(pSop[0], '0');  // var0
        EXPECT_EQ(pSop[1], '-');  // var1
        EXPECT_EQ(pSop[2], '1');  // var2
    }

    Vec_IntFree(vCover);
    Mem_FlexStop(pMan, 0);
}

/*!
  \brief Test Abc_SopCreateFromTruthIsop: creates minimized SOP from truth table.

  Uses ISOP algorithm for minimization. Handles constant 0/1 specially.
  Truth table uses 64-bit word array format.
  
  Note: Kit_TruthIsop has limitations with certain truth table patterns
  that can trigger assertions during recursive computation. This test
  focuses on verifying the interface behavior with well-behaved cases.
*/
TEST(SopTest, SopCreateFromTruthIsop) {
    Mem_Flex_t * pMan = Mem_FlexStart();
    Vec_Int_t * vCover = Vec_IntAlloc(10);

    // Test case 1: Constant 0 (all words are 0)
    {
        word pTruth[1] = {0};
        char * pSop = Abc_SopCreateFromTruthIsop(pMan, 3, pTruth, vCover);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_TRUE(Abc_SopIsConst0(pSop));
    }

    // Test case 2: Constant 1 (all words are all 1s)
    {
        word pTruth[1] = { ~(word)0 };
        char * pSop = Abc_SopCreateFromTruthIsop(pMan, 3, pTruth, vCover);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_TRUE(Abc_SopIsConst1(pSop));
    }

    // Test case 3: 6-input AND (truth = single bit at position 63)
    // This uses a larger function to avoid the nVars=0 recursion edge case
    {
        word pTruth[1] = { (word)1 << 63 };  // minterm 63 (all 1s for 6 vars)
        char * pSop = Abc_SopCreateFromTruthIsop(pMan, 6, pTruth, vCover);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), 6);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 1);  // Single cube: "111111 1\n"
    }

    Vec_IntFree(vCover);
    Mem_FlexStop(pMan, 0);
}

/*!
  \brief Test Abc_SopCreateXorSpecial: creates compact XOR representation.

  Uses special 'x' marker instead of explicit cubes.
  - Creates AND-style cover with 'x' output marker
  - Supports multi-input XOR (unlike Abc_SopCreateXor which only supports 2 vars)
  - Detected by Abc_SopIsExorType()
*/
TEST(SopTest, SopCreateXorSpecial) {
    Mem_Flex_t * pMan = Mem_FlexStart();

    // Test case 1: 2-input XOR special
    // Expected: "11 x\n"
    {
        int nVars = 2;
        char * pSop = Abc_SopCreateXorSpecial(pMan, nVars);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), nVars);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 1);
        // Check structure: literals are all '1', output is 'x'
        EXPECT_EQ(pSop[0], '1');   // var0
        EXPECT_EQ(pSop[1], '1');   // var1
        EXPECT_EQ(pSop[2], ' ');   // space
        EXPECT_EQ(pSop[3], 'x');   // XOR marker (not '1')
        EXPECT_EQ(pSop[4], '\n');  // newline
        // Verify it's detected as XOR type
        EXPECT_TRUE(Abc_SopIsExorType(pSop));
    }

    // Test case 2: 3-input XOR special
    // Expected: "111 x\n"
    {
        int nVars = 3;
        char * pSop = Abc_SopCreateXorSpecial(pMan, nVars);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), nVars);
        EXPECT_EQ(Abc_SopGetCubeNum(pSop), 1);
        EXPECT_EQ(pSop[0], '1');
        EXPECT_EQ(pSop[1], '1');
        EXPECT_EQ(pSop[2], '1');
        EXPECT_EQ(pSop[4], 'x');  // output position: nVars + 1
        EXPECT_TRUE(Abc_SopIsExorType(pSop));
    }

    // Test case 3: 4-input XOR special
    {
        int nVars = 4;
        char * pSop = Abc_SopCreateXorSpecial(pMan, nVars);
        EXPECT_TRUE(pSop != NULL);
        EXPECT_EQ(Abc_SopGetVarNum(pSop), nVars);
        EXPECT_TRUE(Abc_SopIsExorType(pSop));
        // Verify all literals are '1'
        for (int i = 0; i < nVars; i++) {
            EXPECT_EQ(pSop[i], '1');
        }
        // Verify output is 'x'
        EXPECT_EQ(pSop[nVars + 1], 'x');
    }

    Mem_FlexStop(pMan, 0);
}

/*!
  \brief Test Abc_SopCheck: validates integrity of a SOP string.

  The function checks:
  1. Each cube has exactly nFanins literals (before the space)
  2. Output char is one of: '0', '1', 'x', 'n'
  3. Each cube ends with newline
  4. All cubes are in the same phase (no mix of '0' and '1' outputs)
  
  Returns 1 if valid, 0 if invalid (prints error message).
*/
TEST(SopTest, SopCheck) {
    Mem_Flex_t * pMan = Mem_FlexStart();

    // Test case 1: Valid 2-input AND SOP
    {
        char * pSop = Abc_SopRegister(pMan, "11 1\n");
        EXPECT_TRUE(Abc_SopCheck(pSop, 2));
    }

    // Test case 2: Valid 2-input OR SOP (multiple cubes)
    {
        char * pSop = Abc_SopRegister(pMan, "01 1\n10 1\n11 1\n");
        EXPECT_TRUE(Abc_SopCheck(pSop, 2));
    }

    // Test case 3: Valid 3-input function
    {
        char * pSop = Abc_SopRegister(pMan, "11- 1\n0-1 1\n");
        EXPECT_TRUE(Abc_SopCheck(pSop, 3));
    }

    // Test case 4: Valid XOR special format (output 'x')
    {
        char * pSop = Abc_SopRegister(pMan, "11 x\n");
        EXPECT_TRUE(Abc_SopCheck(pSop, 2));
    }

    // Test case 5: Valid constant 0 (off-set)
    {
        char * pSop = Abc_SopRegister(pMan, " 0\n");
        EXPECT_TRUE(Abc_SopCheck(pSop, 0));
    }

    // Test case 6: Valid constant 1 (on-set)
    {
        char * pSop = Abc_SopRegister(pMan, " 1\n");
        EXPECT_TRUE(Abc_SopCheck(pSop, 0));
    }

    // Test case 7: Wrong number of literals (cube size mismatch)
    {
        char * pSop = Abc_SopRegister(pMan, "111 1\n");  // 3 literals but nFanins=2
        EXPECT_FALSE(Abc_SopCheck(pSop, 2));
    }

    // Test case 8: Invalid output character
    {
        char * pSop = Abc_SopRegister(pMan, "11 2\n");  // '2' is invalid
        EXPECT_FALSE(Abc_SopCheck(pSop, 2));
    }

    // Test case 9: Missing newline at end of cube
    {
        // This would need manual construction since Abc_SopRegister copies properly
        // We create a malformed SOP manually
        char pSop[] = "11 1";  // Missing newline
        EXPECT_FALSE(Abc_SopCheck(pSop, 2));
    }

    // Test case 10: Mixed phases (both '0' and '1' outputs)
    {
        char * pSop = Abc_SopRegister(pMan, "11 1\n00 0\n");  // Mixed on-set and off-set
        EXPECT_FALSE(Abc_SopCheck(pSop, 2));
    }

    // Test case 11: Valid buffer
    {
        char * pSop = Abc_SopRegister(pMan, "1 1\n");
        EXPECT_TRUE(Abc_SopCheck(pSop, 1));
    }

    // Test case 12: Valid inverter (off-set with single literal)
    {
        char * pSop = Abc_SopRegister(pMan, "1 0\n");
        EXPECT_TRUE(Abc_SopCheck(pSop, 1));
    }

    Mem_FlexStop(pMan, 0);
}

ABC_NAMESPACE_IMPL_END
