#include "gtest/gtest.h"
#include <cstdlib>
#include <cstring>
#include "misc/extra/extra.h"

ABC_NAMESPACE_IMPL_START


TEST(UtilTest, UtilTruthHexPrint) {
    unsigned tTruth = 8;
    unsigned * pTruth = &tTruth;
    int nVars = 2;
    char * Buffer = (char *)malloc(sizeof(char) * 100);
    Extra_PrintHexadecimalString( Buffer, (unsigned *)pTruth, nVars );
    EXPECT_TRUE(strcmp(Buffer, "8") == 0);

    memset(Buffer, 0, 100);

    tTruth = 15;
    pTruth = &tTruth;
    nVars = 2;
    Extra_PrintHexadecimalString( Buffer, (unsigned *)pTruth, nVars );
    EXPECT_TRUE(strcmp(Buffer, "f") == 0);

    memset(Buffer, 0, 100);

    tTruth = 255;
    pTruth = &tTruth;
    nVars = 3;
    Extra_PrintHexadecimalString( Buffer, (unsigned *)pTruth, nVars );
    EXPECT_TRUE(strcmp(Buffer, "ff") == 0);
    free(Buffer);
}


ABC_NAMESPACE_IMPL_END