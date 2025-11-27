#include "base/abc/abc.h"
#include "misc/util/abc_global.h"
#include "misc/util/utilTruth.h"
#include "gtest/gtest.h"
#include <cstdlib>
ABC_NAMESPACE_IMPL_START

/*!
  \brief Show the direct interface on reading truth table into abc.
  1. Check for case.
  2. Check for ignoring leading 0x.
  3. Check for MSB(order).
  4. Check for returned number of variables.
*/
TEST(TTTest, ReadTT) {
    word * pTruth = (word *)malloc(sizeof(word) * 10);
    char * pString = "ff";
    Abc_TtReadHex( pTruth, pString );
    word * pTruth1 = (word *)malloc(sizeof(word) * 10);
    char * pString1 = "FF";
    Abc_TtReadHex( pTruth1, pString1 );
    EXPECT_EQ(*pTruth, *pTruth1);
    word * pTruth2 = (word *)malloc(sizeof(word) * 10);
    char * pString2 = "0xFF";
    Abc_TtReadHex( pTruth2, pString2 );
    EXPECT_EQ(*pTruth, *pTruth2);

    word * pTruth3 = (word *)malloc(sizeof(word) * 10);
    char * pString3 = "0xACACACACB2B2B2B2";
    int varN = Abc_TtReadHex( pTruth3, pString3 );
    EXPECT_EQ(pTruth3[0], 0xACACACACB2B2B2B2);
    EXPECT_EQ(varN, 6);

    word * pTruth4 = (word *)malloc(sizeof(word) * 10);
    char * pString4 = "0xACACACACB2B2B2B2C3C3C3C3C3C3C3C3";
    int varN1 = Abc_TtReadHex( pTruth4, pString4 );
    EXPECT_EQ(pTruth4[0], 0xC3C3C3C3C3C3C3C3);
    EXPECT_EQ(pTruth4[1], 0xACACACACB2B2B2B2);
    EXPECT_EQ(varN1, 7);

    free(pTruth);
    free(pTruth1);
    free(pTruth2);
    free(pTruth3);
    free(pTruth4);
}

/*!
  \brief Get a bit from an array of word. (bit level)
*/
TEST(TTest, GetBitTT) {
    word w[] = {0x1,0x2,0x3};
    // k could start with 0
    int extBit0 = Abc_TtGetBit(w, 0);
    EXPECT_EQ(extBit0, 1);
    // the 1st bit of 0x3
    int extBit128 = Abc_TtGetBit( w, 128 );
    EXPECT_EQ(extBit128, 1);
    // the 2nd bit of 0x3
    int extBit129 = Abc_TtGetBit( w, 129 );
    EXPECT_EQ(extBit129, 1);
    int extBit1 = Abc_TtGetBit( w, 130 );
    EXPECT_EQ(extBit1, 0);
}

ABC_NAMESPACE_IMPL_END