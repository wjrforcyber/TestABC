#include "gtest/gtest.h"

#include "base/abc/abc.h"
#include "misc/util/abc_namespaces.h"

#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif

void Abc_Start();
void Abc_Stop();
typedef struct Abc_Frame_t_ Abc_Frame_t;
Abc_Frame_t * Abc_FrameGetGlobalFrame();

typedef struct Dec_Graph_t_ Dec_Graph_t;
int Dec_GraphUpdateNetwork( Abc_Obj_t * pRoot, Dec_Graph_t * pGraph, int fUpdateLevel, int nGain );
#define ABC_RS_DIV1_MAX    150   // the max number of divisors to consider
typedef struct Abc_ManRes_t_ Abc_ManRes_t;
int Abc_NtkResubstitute( Abc_Ntk_t * pNtk, int nCutsMax, int nNodesMax, int nMinSaved, int nLevelsOdc, int fUpdateLevel, int fVerbose, int fVeryVerbose, int Log2Probs, int Log2Divs );
Abc_ManRes_t* Abc_ManResubStart( int nLeavesMax, int nDivsMax );
Dec_Graph_t * Abc_ManResubEval( Abc_ManRes_t * p, Abc_Obj_t * pRoot, Vec_Ptr_t * vLeaves, int nSteps, int fUpdateLevel, int fVerbose );
void          Abc_ManResubStop( Abc_ManRes_t * p );
void Abc_NtkCecFraig( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int nSeconds, int fVerbose );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif



ABC_NAMESPACE_IMPL_START



/*!
 \brief Apply resubstitution to a network. This case is from https://people.eecs.berkeley.edu/~alanmi/publications/2006/iwls06_sls.pdf .
*/
TEST(AigTest, ResubAig) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi0 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi1 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi2 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi3 = Abc_NtkCreatePi(pNtk);

    Abc_Obj_t * po_n = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * po_g = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * po_m = Abc_NtkCreatePo(pNtk);

    Abc_Obj_t * and0 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi1), Abc_ObjNot(pi2));
    Abc_Obj_t * andn = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, Abc_ObjNot(and0));
    Abc_Obj_t * andm = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, pi3);
    Abc_Obj_t * andp = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, and0, Abc_ObjNot(pi3));
    Abc_Obj_t * andg = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, Abc_ObjNot(andp));

    Abc_ObjAddFanin( po_n, andn );
    Abc_ObjAddFanin(po_m, andm);
    Abc_ObjAddFanin(po_g, andg);


    Abc_NtkResubstitute( pNtk, 8, 1, 1, 0, 1, 0, 0, 0, 0 );
    EXPECT_TRUE(Abc_NtkNodeNum(pNtk) == 4);
    Abc_NtkDelete(pNtk);
}

/*!
 \brief Apply resubstitution to a single node. This case is from https://people.eecs.berkeley.edu/~alanmi/publications/2006/iwls06_sls.pdf .
*/
TEST(AigTest, ResubSingleNodeAig) {
    // Start frame only because the duplication process need a ntk check which access ABC frame.
    Abc_Start();
    Abc_ManCut_t * pManCut;
    Abc_ManRes_t * pManRes;
    Dec_Graph_t * pFForm;
    Vec_Ptr_t * vLeaves;
    int i;
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Abc_Obj_t * pi0 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi1 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi2 = Abc_NtkCreatePi(pNtk);
    Abc_Obj_t * pi3 = Abc_NtkCreatePi(pNtk);

    Abc_Obj_t * po_n = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * po_g = Abc_NtkCreatePo(pNtk);
    Abc_Obj_t * po_m = Abc_NtkCreatePo(pNtk);

    Abc_Obj_t * and0 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi1), Abc_ObjNot(pi2));
    Abc_Obj_t * andn = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, Abc_ObjNot(and0));
    Abc_Obj_t * andm = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, pi3);
    Abc_Obj_t * andp = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, and0, Abc_ObjNot(pi3));
    Abc_Obj_t * andg = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi0, Abc_ObjNot(andp));

    Abc_ObjAddFanin( po_n, andn );
    Abc_ObjAddFanin(po_m, andm);
    Abc_ObjAddFanin(po_g, andg);
    Abc_Ntk_t * pNtkOri = Abc_NtkDup(pNtk);

    // Cleanup the AIG
    Abc_AigCleanup((Abc_Aig_t *)pNtk->pManFunc);
    // Start the managers
    pManCut = Abc_NtkManCutStart( 8, 100000, 100000, 100000 );
    pManRes = Abc_ManResubStart( 8, ABC_RS_DIV1_MAX );
    Abc_NtkStartReverseLevels( pNtk, 0 );
    Abc_NtkCleanMarkAB( pNtk );
    Abc_Obj_t * pNode = andg;
    vLeaves = Abc_NodeFindCut( pManCut, pNode, 0 );
    pFForm = Abc_ManResubEval( pManRes, pNode, vLeaves, 1, 1, 0 );
    EXPECT_TRUE(pFForm != NULL);
    // The gain value is actually not used by the interface
    Dec_GraphUpdateNetwork( pNode, pFForm, 1, 0 );

    Abc_ManResubStop( pManRes );
    Abc_NtkManCutStop( pManCut );
    Abc_NtkForEachObj( pNtk, pNode, i )
        pNode->pData = NULL;
    Abc_NtkReassignIds( pNtk );
    Abc_NtkStopReverseLevels( pNtk );
    EXPECT_TRUE(Abc_NtkNodeNum(pNtk) == 4);

    Abc_NtkShortNames( pNtkOri );
    Abc_NtkShortNames( pNtk );
    Abc_NtkCecFraig( pNtkOri, pNtk, 20, 0 );

    Abc_NtkDelete( pNtkOri );
    Abc_NtkDelete(pNtk);
    Abc_Stop();
}

ABC_NAMESPACE_IMPL_END