#include "gtest/gtest.h"

#include "base/abc/abc.h"
#include "misc/util/abc_namespaces.h"

ABC_NAMESPACE_IMPL_START

/*!
 \brief Level calculation on AIG.
*/
TEST(AigTest, LevelTagAig) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi0 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi1 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi2 = Abc_NtkCreatePi(pNtk);

    Abc_Obj_t * po = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * and0 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, pi1);
    Abc_Obj_t * and1 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi0), pi2);
    Abc_Obj_t * and2 = Abc_AigAnd((Abc_Aig_t * )pNtk->pManFunc, Abc_ObjNot(and0), Abc_ObjNot(and1));
    EXPECT_TRUE(and2->fPhase == 1);
    Abc_ObjAddFanin( po, and2 );

    int levelMax = Abc_NtkLevel(pNtk);
    int i;
    Abc_Obj_t * pCi;
    Abc_NtkForEachCi(pNtk, pCi, i)
        EXPECT_TRUE(pCi->Level == 0);
    EXPECT_TRUE(levelMax == 2);
    Abc_NtkDelete(pNtk);
}

/*!
 \brief Reverse level test on AIG.
*/
TEST(AigTest, ReverseLevelAig) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi0 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi1 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi2 = Abc_NtkCreatePi(pNtk);

    Abc_Obj_t * po = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * and0 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, pi1);
    Abc_Obj_t * and1 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi0), pi2);
    Abc_Obj_t * and2 = Abc_AigAnd((Abc_Aig_t * )pNtk->pManFunc, Abc_ObjNot(and0), Abc_ObjNot(and1));
    EXPECT_TRUE(and2->fPhase == 1);
    Abc_ObjAddFanin( po, and2 );

    //remember to clean the mark at the beginning
    Abc_NtkCleanMarkA( pNtk );
    Abc_NtkStartReverseLevels( pNtk, 0 );
    EXPECT_TRUE( pNtk->vLevelsR != NULL);
    Abc_Obj_t * pNode;
    int i, Counter = 0;
    Abc_NtkForEachNode( pNtk, pNode, i )
        if ( Abc_ObjRequiredLevel(pNode) - pNode->Level <= 1 ) // Use a loose level slack value
            pNode->fMarkA = 1, Counter++;
    EXPECT_TRUE(Counter == 3);
    Abc_NtkForEachNode( pNtk, pNode, i )
        EXPECT_TRUE(pNode->fMarkA == 1);
    Abc_NtkStopReverseLevels( pNtk );
    EXPECT_TRUE( pNtk->vLevelsR == NULL);
    Abc_NtkCleanMarkA( pNtk );
    Abc_NtkDelete(pNtk);
}

ABC_NAMESPACE_IMPL_END