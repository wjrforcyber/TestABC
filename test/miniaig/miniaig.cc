#include "gtest/gtest.h"
#include "aig/gia/gia.h"
#include "base/abc/abc.h"
#include "base/io/ioAbc.h"
#include "misc/util/abc_global.h"
#include "aig/miniaig/miniaig.h"

ABC_NAMESPACE_IMPL_START

/*!
 \brief Test the efficiency of minaig compared to aig and gia structure
*/
TEST(GiaTest, GiaCollectFanoutInfo) {
    // Mini aig
    abctime clk;
    abctime clkmini, clkaig, clkgia;
    clk = Abc_Clock();
    char * pFileName = (char *)malloc(sizeof(char) * 100);
    strcat(pFileName, PROJECT_ROOT_DIR);
    strcat(pFileName, "/i10.aig");
    for(int i = 0; i < 1000; i++)
    {
        Mini_Aig_t * p = Mini_AigerRead( pFileName, 0 );
        EXPECT_TRUE(p != NULL);
        Mini_AigStop( p );
    }
    clkmini = Abc_Clock() - clk;
    clk = Abc_Clock();
    for(int i = 0; i < 1000; i++)
    {
        Abc_Ntk_t * pNtk = Io_Read( pFileName, IO_FILE_AIGER, 1, 0 );
        EXPECT_TRUE(pNtk != NULL);
        Abc_NtkDelete(pNtk);
    }
    clkaig = Abc_Clock() - clk;
    clk = Abc_Clock();
    for(int i = 0; i < 1000; i++)
    {
        Gia_Man_t * pMan =  Gia_AigerRead( pFileName, 0, 0, 0 );
        EXPECT_TRUE(pMan != NULL);
        Gia_ManStop(pMan);
    }
    clkgia = Abc_Clock() - clk;
    EXPECT_TRUE(clkmini < clkaig);
    EXPECT_TRUE(clkmini < clkgia);
    free(pFileName);
}

ABC_NAMESPACE_IMPL_END