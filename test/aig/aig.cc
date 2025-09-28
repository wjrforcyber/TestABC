#include "gtest/gtest.h"

#include "base/abc/abc.h"
#include "misc/util/abc_namespaces.h"
#include "misc/vec/vecPtr.h"

ABC_NAMESPACE_IMPL_START

/*!
 \brief Construct a 2 input and 1 output AIG(A single AND node)
*/
TEST(AigTest, ConstructAig) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Vec_Ptr_t * vNodes = Vec_PtrAlloc(100);
    int nInputs = 2;
    int nLatches = 0;
    int nAnds = 1;
    int nOutputs = 1;
    int i;
    Abc_Obj_t * pObj, * pNodeAttach;
    vNodes = Vec_PtrAlloc( 1 + nInputs + nLatches + nAnds );
    Vec_PtrPush( vNodes, Abc_ObjNot( Abc_AigConst1(pNtk) ) );

    // create the PIs
    for ( i = 0; i < nInputs; i++ )
    {
        pObj = Abc_NtkCreatePi(pNtk);    
        Vec_PtrPush( vNodes, pObj );
    }
    // create the POs
    for ( i = 0; i < nOutputs; i++ )
    {
        pObj = Abc_NtkCreatePo(pNtk);   
    }
    //create ANDs
    Abc_Obj_t * pi00 = (Abc_Obj_t *)Vec_PtrEntry(vNodes, 1);
    Abc_Obj_t * pi01 = (Abc_Obj_t *)Vec_PtrEntry(vNodes, 2);
    Vec_PtrPush( vNodes, Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi00, pi01) );

    pNodeAttach = Abc_ObjNotCond( (Abc_Obj_t *)Vec_PtrEntry(vNodes, 3), 1 );
    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 0), pNodeAttach );

    EXPECT_TRUE(Abc_NtkPiNum(pNtk) == nInputs);
    EXPECT_TRUE(Abc_NtkPoNum(pNtk) == nOutputs);
    // 1 const, 2 inputs, 1 and, 1 output = 5
    EXPECT_TRUE(Abc_NtkObjNum(pNtk) == nInputs + 1 + 1 + nOutputs);
    EXPECT_TRUE(Abc_NtkNodeNum(pNtk) == 1);
    Vec_PtrFree(vNodes);
    Abc_NtkDelete(pNtk);
}

/*!
 \brief Analysis a 2 input and 1 output AIG(A single AND node)
*/
TEST(AigTest, StructureAnalysisAig) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Vec_Ptr_t * vNodes = Vec_PtrAlloc(100);
    int nInputs = 2;
    int nLatches = 0;
    int nAnds = 1;
    int nOutputs = 1;
    int i;
    Abc_Obj_t * pObj, * pNodeAttach;
    vNodes = Vec_PtrAlloc( 1 + nInputs + nLatches + nAnds );
    Vec_PtrPush( vNodes, Abc_ObjNot( Abc_AigConst1(pNtk) ) );

    // create the PIs
    for ( i = 0; i < nInputs; i++ )
    {
        pObj = Abc_NtkCreatePi(pNtk);    
        Vec_PtrPush( vNodes, pObj );
    }
    // create the POs
    for ( i = 0; i < nOutputs; i++ )
    {
        pObj = Abc_NtkCreatePo(pNtk);   
    }
    //create ANDs
    Abc_Obj_t * pi00 = (Abc_Obj_t *)Vec_PtrEntry(vNodes, 1);
    Abc_Obj_t * pi01 = (Abc_Obj_t *)Vec_PtrEntry(vNodes, 2);
    Vec_PtrPush( vNodes, Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi00, pi01) );

    //complemented attributes test
    pNodeAttach = Abc_ObjNotCond( (Abc_Obj_t *)Vec_PtrEntry(vNodes, 3), 1 );
    EXPECT_TRUE(Abc_ObjIsComplement(pNodeAttach) == 1);
    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 0), pNodeAttach );
    // the fanin phases located on the node ITSELF instead of the fanin nodes
    EXPECT_TRUE(Abc_ObjFaninC0(Abc_NtkPo(pNtk, 0)) == 1);
    // this is a false usage, you should consider a node has multiple fanouts, there exist both phases
    EXPECT_FALSE(Abc_ObjIsComplement( Abc_ObjFanin0(Abc_NtkPo(pNtk, 0))));
    EXPECT_TRUE(Abc_ObjFanin0(Abc_NtkPo(pNtk, 0)) == (Abc_Obj_t *)Vec_PtrEntry(vNodes, 3));
    Vec_PtrFree(vNodes);
    Abc_NtkDelete(pNtk);
}

ABC_NAMESPACE_IMPL_END