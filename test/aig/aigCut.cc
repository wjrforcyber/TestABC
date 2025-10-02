#include "gtest/gtest.h"
#include "base/abc/abc.h"
#include "misc/util/abc_global.h"
#include "misc/util/abc_namespaces.h"
#include "misc/vec/vecPtr.h"
#include "opt/cut/cutInt.h"

ABC_NAMESPACE_IMPL_START

/*!
 \brief Verify the words num used for truth simulation
*/
TEST(AigCutTest, CutTruthVarNumAig) {
    int n1 = Cut_TruthWords( 2 );
    int n2 = Cut_TruthWords( 4 );
    int n3 = Cut_TruthWords(5);
    int n4 = Cut_TruthWords(6);
    int n5 = Cut_TruthWords(7);
    EXPECT_TRUE(n1 == n2 && n2 == n3);
    EXPECT_TRUE(n1 == 1); //    32/32
    EXPECT_TRUE(n4 == 2); //    64/32
    EXPECT_TRUE(n5 == 4); //    128/32
}


ABC_NAMESPACE_IMPL_END