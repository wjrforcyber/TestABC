#include "gtest/gtest.h"
#include <cstdlib>

#include "base/abc/abc.h"
#include "base/io/ioAbc.h"
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
 \brief Phases check after a clean AIG construction
*/
TEST(AigTest, PhasesAig) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    int nInputs = 2;
    int nOutputs = 4;
    int i;
    Abc_Obj_t * pObj;
    Abc_ObjNot( Abc_AigConst1(pNtk) );

    // create the PIs
    for ( i = 0; i < nInputs; i++ )
    {
        pObj = Abc_NtkCreatePi(pNtk);    
    }
    // create the POs
    for ( i = 0; i < nOutputs; i++ )
    {
        pObj = Abc_NtkCreatePo(pNtk);   
    }
    //create ANDs
    Abc_Obj_t * pi00 = Abc_NtkPi(pNtk, 0);
    Abc_Obj_t * pi01 = Abc_NtkPi(pNtk, 1);
    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 0), Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi00, pi01));
    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 1), Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi00), pi01));
    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 2), Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi00, Abc_ObjNot(pi01)));
    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 3), Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi00),Abc_ObjNot(pi01)));

    // phases check after construction
    EXPECT_TRUE(Abc_ObjFanin0(Abc_NtkPo(pNtk, 0))->fPhase == 0);
    EXPECT_TRUE(Abc_ObjFanin0(Abc_NtkPo(pNtk, 1))->fPhase == 0);
    EXPECT_TRUE(Abc_ObjFanin0(Abc_NtkPo(pNtk, 2))->fPhase == 0);
    EXPECT_TRUE(Abc_ObjFanin0(Abc_NtkPo(pNtk, 3))->fPhase == 1);
}


/*!
 \brief Assign Name to an Object.
*/
TEST(AigTest, NameObjAig) {
    Abc_Obj_t * pCi;
    int i;
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi0 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi1 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi2 = Abc_NtkCreatePi(pNtk);

    Abc_Obj_t * po = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * and0 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, pi1);
    Abc_Obj_t * and1 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi0), pi2);
    Abc_Obj_t * and2 = Abc_AigAnd((Abc_Aig_t * )pNtk->pManFunc, Abc_ObjNot(and0), Abc_ObjNot(and1));
    Abc_ObjAddFanin( po, and2 );

    Abc_NtkForEachCi(pNtk, pCi, i)
    {
        char index_i[10];
        assert(i < 10);
        snprintf(index_i, sizeof(index_i), "%d", i);
        Abc_ObjAssignName(pCi, "pi", index_i);
    }
    Abc_ObjAssignName(and0, "and0", NULL);
    Abc_ObjAssignName(and1, "and1", NULL);
    Abc_ObjAssignName(and2, "and2", NULL);
    EXPECT_TRUE(strcmp(Abc_ObjName(Abc_NtkCi(pNtk, 0)), "pi0") == 0);
    EXPECT_TRUE(strcmp(Abc_ObjName(Abc_NtkCi(pNtk, 1)), "pi1") == 0);
    EXPECT_TRUE(strcmp(Abc_ObjName(Abc_NtkCi(pNtk, 2)), "pi2") == 0);
    EXPECT_TRUE(strcmp(Abc_ObjName(and0), "and0") == 0);
    EXPECT_TRUE(strcmp(Abc_ObjName(and1), "and1") == 0);
    EXPECT_TRUE(strcmp(Abc_ObjName(and2), "and2") == 0);
    
    Abc_NtkDelete(pNtk);
}

/*!
 \brief Check if the node is MUX output node
*/
TEST(AigTest, IsMuxAig) {
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
    EXPECT_TRUE(Abc_NodeIsMuxType(and2));
    Abc_NtkDelete(pNtk);
}

/*!
 \brief Check if the node is MUX control node
*/
TEST(AigTest, IsMuxControlAig) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi0 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi1 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi2 = Abc_NtkCreatePi(pNtk);

    Abc_Obj_t * po = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * and0 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, pi1);
    Abc_Obj_t * and1 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi0), pi2);
    Abc_Obj_t * and2 = Abc_AigAnd((Abc_Aig_t * )pNtk->pManFunc, Abc_ObjNot(and0), Abc_ObjNot(and1));
    Abc_ObjAddFanin( po, and2 );
    EXPECT_TRUE(Abc_NodeIsMuxControlType(pi0));
    Abc_NtkDelete(pNtk);
}

/*!
 \brief Recognize the ITE.
*/
TEST(AigTest, RecITEMUXAig) {
    Abc_Obj_t * pDriver, * pNodeT, * pNodeE, * pNodeC;
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi0 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi1 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi2 = Abc_NtkCreatePi(pNtk);

    Abc_Obj_t * po = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * and0 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, pi1);
    Abc_Obj_t * and1 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi0), pi2);
    Abc_Obj_t * and2 = Abc_AigAnd((Abc_Aig_t * )pNtk->pManFunc, Abc_ObjNot(and0), Abc_ObjNot(and1));
    Abc_ObjAddFanin( po, and2 );
    EXPECT_TRUE(Abc_NodeIsMuxType(and2));
    pDriver = and2;
    pNodeC = Abc_NodeRecognizeMux( pDriver, &pNodeT, &pNodeE );
    EXPECT_EQ(pNodeC, pi0);
    EXPECT_EQ(Abc_ObjRegular(pNodeT), pi1);
    EXPECT_EQ(Abc_ObjRegular(pNodeE), pi2);
    Abc_NtkDelete(pNtk);
}

/*!
 \brief Check if the node is XOR root node
*/
TEST(AigTest, IsXORAig) {
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
    Abc_NtkDelete(pNtk);
    
    //case b
    Abc_Ntk_t * pNtk1 = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi01 = Abc_NtkCreatePi(pNtk1);
    Abc_Obj_t * pi11 = Abc_NtkCreatePi(pNtk1);

    Abc_Obj_t * po1 = Abc_NtkCreatePo(pNtk1);
    Abc_Obj_t * and01 = Abc_AigAnd((Abc_Aig_t *)pNtk1->pManFunc, Abc_ObjNot(pi01), Abc_ObjNot(pi11));
    Abc_Obj_t * and11 = Abc_AigAnd((Abc_Aig_t *)pNtk1->pManFunc, pi01, pi11);
    Abc_Obj_t * andOut1 = Abc_AigAnd((Abc_Aig_t *)pNtk1->pManFunc, Abc_ObjNot(and01),Abc_ObjNot(and11));
    Abc_ObjAddFanin(po1, andOut1);
    EXPECT_TRUE(Abc_NodeIsExorType(andOut1));
    Abc_NtkDelete(pNtk1);
    
    //case c
    Abc_Ntk_t * pNtk2 = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi02 = Abc_NtkCreatePi(pNtk2);
    Abc_Obj_t * pi12 = Abc_NtkCreatePi(pNtk2);

    Abc_Obj_t * po2 = Abc_NtkCreatePo(pNtk2);
    Abc_Obj_t * and02 = Abc_AigAnd((Abc_Aig_t *)pNtk2->pManFunc, pi02, Abc_ObjNot(pi12));
    Abc_Obj_t * and12 = Abc_AigAnd((Abc_Aig_t *)pNtk2->pManFunc, Abc_ObjNot(pi02), pi12);
    Abc_Obj_t * andOut2 = Abc_AigAnd((Abc_Aig_t *)pNtk2->pManFunc, Abc_ObjNot(and02),Abc_ObjNot(and12));
    Abc_ObjAddFanin(po2, andOut2);
    EXPECT_TRUE(Abc_NodeIsExorType(andOut2));
    Abc_NtkDelete(pNtk2);
    
    //case d
    Abc_Ntk_t * pNtk3 = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi03 = Abc_NtkCreatePi(pNtk3);
    Abc_Obj_t * pi13 = Abc_NtkCreatePi(pNtk3);

    Abc_Obj_t * po3 = Abc_NtkCreatePo(pNtk3);
    Abc_Obj_t * and03 = Abc_AigAnd((Abc_Aig_t *)pNtk3->pManFunc, Abc_ObjNot(pi03), pi13);
    Abc_Obj_t * and13 = Abc_AigAnd((Abc_Aig_t *)pNtk3->pManFunc, pi03, Abc_ObjNot(pi13));
    Abc_Obj_t * andOut3 = Abc_AigAnd((Abc_Aig_t *)pNtk3->pManFunc, Abc_ObjNot(and03),Abc_ObjNot(and13));
    Abc_ObjAddFanin(po3, andOut3);
    EXPECT_TRUE(Abc_NodeIsExorType(andOut3));
    Abc_NtkDelete(pNtk3);
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


/*!
 \brief Showcase on analyzing simulation on same structure cases with manually changed complemented attributes.
*/
TEST(AigTest, SimulationManualCompAig) {
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
    
    
    // Now let's manually change the attributes on the first leaf, it has 4 fanouts, all changed to complemented (regualr -> complemented, complemented to regular)
    Abc_Obj_t * pFanout;
    i = 0;
    Abc_ObjForEachFanout(pi00, pFanout, i)
    {
        pFanout->fCompl0 ^= 1;
    }
    pi00->fPhase ^= 1;
    Vec_Ptr_t *vSimsNew = Vec_PtrAlloc( 100 );
    unsigned int *pInfoNew = ABC_ALLOC( unsigned, nWords * (100 + 1) );
    unsigned int *pDataNew;
    for ( i = 0; i < 100; i++ )
        Vec_PtrPush( vSimsNew, pInfoNew + i * nWords );
    // set elementary truth tables
    for ( k = 0; k < 2; k++ )
    {
        pDataNew = (unsigned *)vSimsNew->pArray[k];
        for ( i = 0; i < nBits; i++ )
            if ( i & (1 << k) )
                pDataNew[i>>5] |= (1 << (i&31));
        if(k == 0)
        {
            *pDataNew = ~(*pDataNew);
        }
    }
    EXPECT_TRUE(Vec_PtrSize(vNodes) == 6);
    Abc_ManResubSimulate( vNodes, 2, vSimsNew, 2, 1);
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
    
    
    Vec_PtrFree(vNodes);
    Vec_PtrFree(vSims);
    Vec_PtrFree(vSimsNew);
    ABC_FREE(pInfo);
    Abc_NtkDelete(pNtk);
}

/*!
 \brief Analysis simulation on complex cases, an MFFC from i10.aig.
*/
TEST(AigTest, Simulation6InputsAig) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    int nInputs = 6;
    int nOutputs = 1;
    int i = 0, k = 0;
    Abc_Obj_t * pObj;
    // create the PIs
    for ( i = 0; i < nInputs; i++ )
    {
        pObj = Abc_NtkCreatePi(pNtk);    
    }
    // create the POs
    for ( i = 0; i < nOutputs; i++ )
    {
        pObj = Abc_NtkCreatePo(pNtk);   
    }
    
    Vec_Ptr_t * vNodes = Vec_PtrAlloc(10);
    Abc_Obj_t * pi00 = Abc_NtkPi(pNtk, 0);
    Vec_PtrPush(vNodes, pi00);
    Abc_Obj_t * pi01 = Abc_NtkPi(pNtk, 1);
    Vec_PtrPush(vNodes, pi01);
    Abc_Obj_t * pi02 = Abc_NtkPi(pNtk, 2);
    Vec_PtrPush(vNodes, pi02);
    Abc_Obj_t * pi03 = Abc_NtkPi(pNtk, 3);
    Vec_PtrPush(vNodes, pi03);
    Abc_Obj_t * pi04 = Abc_NtkPi(pNtk, 4);
    Vec_PtrPush(vNodes, pi04);
    Abc_Obj_t * pi05 = Abc_NtkPi(pNtk, 5);
    Vec_PtrPush(vNodes, pi05);
    Abc_Obj_t * pN2012 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi01), pi02);
    Vec_PtrPush(vNodes, pN2012);
    Abc_Obj_t * pN2013 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi00, Abc_ObjNot(pN2012));
    Vec_PtrPush(vNodes, pN2013);
    Abc_Obj_t * pN2011 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi04), pi05);
    Vec_PtrPush(vNodes, pN2011);
    Abc_Obj_t * pN2014 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi03), Abc_ObjNot(pN2011));
    Vec_PtrPush(vNodes, pN2014);
    Abc_Obj_t * pRoot = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pN2013), pN2014);
    Vec_PtrPush(vNodes, pRoot);
    Abc_ObjAddFanin(Abc_NtkPo(pNtk, 0), Abc_ObjNot(pRoot));

    int nBits      = (1 << 6);
    int nWords     = (nBits <= 32)? 1 : (nBits / 32);
    Vec_Ptr_t *vSims = Vec_PtrAlloc( 100 );
    unsigned int *pInfo = ABC_ALLOC( unsigned, nWords * (100 + 1) );
    unsigned int *pData;
    for ( i = 0; i < 100; i++ )
        Vec_PtrPush( vSims, pInfo + i * nWords );
    // set elementary truth tables
    for ( k = 0; k < 6; k++ )
    {
        pData = (unsigned *)vSims->pArray[k];
        for ( i = 0; i < nBits; i++ )
            if ( i & (1 << k) )
                pData[i>>5] |= (1 << (i&31));
    }
    Abc_ManResubSimulate( vNodes, 6, vSims, 6, 2);
    EXPECT_TRUE(*(uint64_t *)Abc_NtkPi(pNtk, 3)->pData == 0xff00ff00ff00ff00);
    EXPECT_TRUE(*(uint64_t *)Abc_NtkPi(pNtk, 4)->pData == 0xffff0000ffff0000);
    EXPECT_TRUE(*(uint64_t *)Abc_NtkPi(pNtk, 5)->pData == 0xffffffff00000000);
    EXPECT_TRUE(*(uint64_t *)pN2011->pData == 0x0000ffff00000000);
    // ~(0x00ff00ff00ff00ff & 0xffff0000ffffffff)
    EXPECT_TRUE(*(uint64_t *)pN2014->pData == 0xff00ffffff00ff00);
    EXPECT_TRUE(*(uint64_t *)pN2013->pData == 0x8a8a8a8a8a8a8a8a);
    uint64_t uRoot = ~(~(*(uint64_t *)pN2013->pData) & ~(*(uint64_t *)pN2014->pData));
    EXPECT_TRUE(*(uint64_t *)pRoot->pData == uRoot);
    Vec_PtrFree(vNodes);
    Vec_PtrFree(vSims);
    ABC_FREE(pInfo);
    Abc_NtkDelete(pNtk);
}

/*!
 \brief Directly read an aig from a file
*/
TEST(AigTest, ReadFromFileAig) {
    char * pFileName = (char *)malloc(sizeof(char) * 100);
    strcat(pFileName, PROJECT_ROOT_DIR);
    strcat(pFileName, "/i10.aig");
    Abc_Ntk_t * pNtk = Io_Read( pFileName, IO_FILE_AIGER, 1, 0 );
    // A must assert, or ABC returns a segfault without checking the validity of pNtk
    EXPECT_TRUE(pNtk != NULL);
    EXPECT_TRUE(Abc_NtkCheck(pNtk) == 1);
    Abc_NtkDelete(pNtk);
    free(pFileName);
}

/*!
 \brief Dual function test on aig
*/
TEST(AigTest, DualPropertyMig) {
    int nVar = 3;
    // Create a majority to check dual (By nature dual)
    Vec_Ptr_t * vNodes = Vec_PtrAlloc(10);
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi0 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi1 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi2 = Abc_NtkCreatePi(pNtk);

    Abc_Obj_t * po = Abc_NtkCreatePo(pNtk);
    Abc_Aig_t *pMan = (Abc_Aig_t *)pNtk->pManFunc;
    Abc_Obj_t *and_ab = Abc_AigAnd(pMan, pi0, pi1);
    Abc_Obj_t *and_bc = Abc_AigAnd(pMan, pi1, pi2);
    Abc_Obj_t *and_ac = Abc_AigAnd(pMan, pi0, pi2);
    Abc_Obj_t *or_ab_bc = Abc_AigOr(pMan, and_ab, and_bc);
    Abc_Obj_t *majority = Abc_AigOr(pMan, or_ab_bc, and_ac);
    Abc_ObjAddFanin(po, majority);
    
    Abc_Obj_t * pCi;
    Abc_Obj_t * pNode;
    int index;
    Abc_NtkForEachPi(pNtk, pCi, index)
    {
        Vec_PtrPush(vNodes, pCi);
    }
    Abc_NtkForEachNode(pNtk, pNode, index)
    {
        Vec_PtrPush(vNodes, pNode);
    }
    int i = 0, k = 0;
    int nBits      = (1 << nVar);
    int nWords     = (nBits <= 32)? 1 : (nBits / 32);
    Vec_Ptr_t *vSims = Vec_PtrAlloc( 100 );
    unsigned int *pInfo = ABC_ALLOC( unsigned, nWords * (100 + 1) );
    unsigned int *pData;
    for ( i = 0; i < 100; i++ )
        Vec_PtrPush( vSims, pInfo + i * nWords );
    // set elementary truth tables
    for ( k = 0; k < nVar; k++ )
    {
        pData = (unsigned *)vSims->pArray[k];
        for ( i = 0; i < nBits; i++ )
            if ( i & (1 << k) )
                pData[i>>5] |= (1 << (i&31));
    }
    Abc_ManResubSimulate( vNodes, nVar, vSims, 6, 1);
    unsigned int uRoot0 = *((unsigned int *)Abc_ObjFanin0(po)->pData) & 0xff;
    unsigned fPhase0 = Abc_ObjFanin0(po)->fPhase;
    Abc_NtkCleanData( pNtk );
    Vec_PtrFree(vSims);
    ABC_FREE(pInfo);

    vSims = Vec_PtrAlloc( 100 );
    pInfo = ABC_ALLOC( unsigned, nWords * (100 + 1) );
    for ( i = 0; i < 100; i++ )
        Vec_PtrPush( vSims, pInfo + i * nWords );
    // set elementary truth tables
    for ( k = 0; k < nVar; k++ )
    {
        pData = (unsigned *)vSims->pArray[k];
        for ( i = 0; i < nBits; i++ )
            if ( !(i & (1 << k)) )
                pData[i>>5] |= (1 << (i&31));
    }
    Abc_ManResubSimulate( vNodes, nVar, vSims, 6, 1);
    unsigned int uRoot1 = (*((unsigned int *)Abc_ObjFanin0(po)->pData)) & 0xFF;
    unsigned fPhase1 = Abc_ObjFanin0(po)->fPhase;
    EXPECT_TRUE((fPhase0 ^ fPhase1) == 1);
    // Invert the output to get the dual
    Vec_PtrFree(vSims);
    ABC_FREE(pInfo);
    printf("The uRoot0 is %x\n", uRoot0);
    printf("The uRoot1 is %x\n", uRoot1);
    EXPECT_EQ(uRoot0, uRoot1);
    Abc_NtkDelete(pNtk);
}

ABC_NAMESPACE_IMPL_END