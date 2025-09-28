#include "gtest/gtest.h"

#include "base/abc/abc.h"
#include "misc/util/abc_global.h"
#include "misc/util/abc_namespaces.h"
#include "misc/vec/vecPtr.h"

#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif

void Abc_ManResubSimulate( Vec_Ptr_t * vDivs, int nLeaves, Vec_Ptr_t * vSims, int nLeavesMax, int nWords );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif


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

/*!
 \brief Analysis simulation on different cases
*/
TEST(AigTest, SimulationAig) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Vec_Ptr_t * vNodes = Vec_PtrAlloc(100);
    int nInputs = 2;
    int nLatches = 0;
    int nAnds = 3;
    int nOutputs = 4;
    int i, k;
    Abc_Obj_t * pObj;
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
    // AND idnex 3
    Vec_PtrPush( vNodes, Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi00, pi01) );
    // AND index 4
    Vec_PtrPush( vNodes, Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi00), pi01) );
    // AND index 5
    Vec_PtrPush( vNodes, Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi00, Abc_ObjNot(pi01)) );
    // AND index 6
    Vec_PtrPush( vNodes, Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi00),Abc_ObjNot(pi01)) );

    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 0), (Abc_Obj_t *)Vec_PtrEntry(vNodes, 3) );
    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 1), (Abc_Obj_t *)Vec_PtrEntry(vNodes, 4));
    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 2), (Abc_Obj_t *)Vec_PtrEntry(vNodes, 5));
    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 3), (Abc_Obj_t *)Vec_PtrEntry(vNodes, 6));

    int nBits      = (1 << 2);
    int nWords     = (nBits <= 32)? 1 : (nBits / 32);
    Vec_Ptr_t *vSims = Vec_PtrAlloc( 100 );
    unsigned int *pInfo = ABC_ALLOC( unsigned, nWords * (100 + 1) );
    unsigned int *pData;
    for ( i = 0; i < 100; i++ )
        Vec_PtrPush( vSims, pInfo + i * nWords );
    // set elementary truth tables
    for ( k = 0; k < 2; k++ )
    {
        pData = (unsigned *)vSims->pArray[k];
        for ( i = 0; i < nBits; i++ )
            if ( i & (1 << k) )
                pData[i>>5] |= (1 << (i&31));
    }
    // the first leaf, PI index 0
    EXPECT_TRUE(*(unsigned *)Vec_PtrEntry(vSims, 0) == 0xa);
    // the second leaf, PI index 1
    EXPECT_TRUE(*(unsigned *)Vec_PtrEntry(vSims, 1) == 0xc);
    // take out the const at the index 0, now shift left for 1
    Vec_PtrDrop(vNodes, 0);
    EXPECT_TRUE(Vec_PtrSize(vNodes) == 6);
    Abc_ManResubSimulate( vNodes, 2, vSims, 2, 1);
    EXPECT_TRUE(Abc_ObjFaninC0((Abc_Obj_t *)Vec_PtrEntry(vNodes, 5)) == 1);
    EXPECT_TRUE(Abc_ObjFaninC1((Abc_Obj_t *)Vec_PtrEntry(vNodes, 5)) == 1);
    // the first leaf, PI index 0
    EXPECT_TRUE(*(unsigned *)(Abc_ObjData((Abc_Obj_t *)Vec_PtrEntry(vNodes, 0))) == 0xa);
    // the second leaf, PI index 1
    EXPECT_TRUE(*(unsigned *)(Abc_ObjData((Abc_Obj_t *)Vec_PtrEntry(vNodes, 1))) == 0xc);
    // the check on the output result on PO (original index 3)
    EXPECT_TRUE(*(unsigned *)Vec_PtrEntry(vSims, 2) == 0x8);
    EXPECT_TRUE(*(unsigned *)(Abc_ObjData((Abc_Obj_t *)Vec_PtrEntry(vNodes, 2))) == 0x8);
    // the check on the output result on PO (original index 4)
    EXPECT_TRUE(*(unsigned *)Vec_PtrEntry(vSims, 3) == 0x4);
    EXPECT_TRUE(*(unsigned *)(Abc_ObjData((Abc_Obj_t *)Vec_PtrEntry(vNodes, 3))) == 0x4);
    // the check on the output result on PO (original index 5)
    EXPECT_TRUE(*(unsigned *)Vec_PtrEntry(vSims, 4) == 0x2);
    EXPECT_TRUE(*(unsigned *)(Abc_ObjData((Abc_Obj_t *)Vec_PtrEntry(vNodes, 4))) == 0x2);
    // the check on the output result on PO (original index 6)
    // Note you should be clear on this, instead of 0x1, it is 0xe with a phase change at the node due to the simulation design, which is used to deal with the ffff at the beginning
    EXPECT_TRUE(*(unsigned *)Vec_PtrEntry(vSims, 5) == 0xe);
    EXPECT_TRUE(Abc_ObjFanin0(Abc_NtkPo(pNtk, 3))->fPhase == 1);
    Vec_PtrFree(vNodes);
    Vec_PtrFree(vSims);
    ABC_FREE(pInfo);
    Abc_NtkDelete(pNtk);
}


ABC_NAMESPACE_IMPL_END