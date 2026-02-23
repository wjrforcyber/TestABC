#include "gtest/gtest.h"

#include "base/abc/abc.h"
#include "misc/util/abc_namespaces.h"

ABC_NAMESPACE_IMPL_START

/*!
  \brief Manually construct a choice network. Take XOR case (a) and case (b) as an example, they won't create choices since they are hashed to be the same.
*/
TEST(AigTest, ChoiceXORNoChoice) {
    //case a
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi0 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi1 = Abc_NtkCreatePi(pNtk);

    Abc_Obj_t * po = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * and0 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, pi1);
    Abc_Obj_t * and1 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi0), Abc_ObjNot(pi1));
    Abc_Obj_t * andOut = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(and0),Abc_ObjNot(and1));
    Abc_ObjAddFanin(po, andOut);
    EXPECT_TRUE(Abc_NodeIsExorType(andOut));

    //case b
    Abc_Obj_t * and01 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi0), Abc_ObjNot(pi1));
    Abc_Obj_t * and11 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, pi1);
    Abc_Obj_t * andOut1 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(and01),Abc_ObjNot(and11));

    EXPECT_TRUE(Abc_ObjFanoutNum(andOut1) == 1);
    EXPECT_TRUE(Abc_NodeIsExorType(andOut1));
    EXPECT_TRUE(andOut == andOut1);
    Abc_NtkDelete(pNtk);
}


/*!
  \brief Manually construct a choice network. Take XOR case (a) and case (c) as an example.
*/
TEST(AigTest, ChoiceXORChoice) {
    //case a
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi0 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi1 = Abc_NtkCreatePi(pNtk);

    Abc_Obj_t * po = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * and0 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, pi1);
    Abc_Obj_t * and1 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi0), Abc_ObjNot(pi1));
    Abc_Obj_t * andOut = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(and0),Abc_ObjNot(and1));
    Abc_ObjAddFanin(po, andOut);
    EXPECT_TRUE(Abc_NodeIsExorType(andOut));

    //case c (NXOR)
    Abc_Obj_t * and01 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, Abc_ObjNot(pi1));
    Abc_Obj_t * and11 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi0), pi1);
    Abc_Obj_t * andOut1 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(and01),Abc_ObjNot(and11));
    // flip the original phase to form an XOR
    andOut1->fPhase ^= andOut1->fPhase;
    andOut->pData = andOut1;
    EXPECT_TRUE(Abc_NodeIsExorType(andOut1));
    EXPECT_TRUE(Abc_NtkGetChoiceNum(pNtk) == 1);
    Abc_NtkDelete(pNtk);
}

ABC_NAMESPACE_IMPL_END