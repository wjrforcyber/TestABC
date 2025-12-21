#include "gtest/gtest.h"
#include "base/abc/abc.h"
#include "base/io/ioAbc.h"
#include "misc/util/abc_global.h"
#include "misc/util/abc_namespaces.h"
#include "misc/vec/vecPtr.h"
#include "opt/cut/cut.h"
#include "opt/cut/cutInt.h"

#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif

Cut_Man_t * Abc_NtkCuts( Abc_Ntk_t * pNtk, Cut_Params_t * pParams );
void Abc_Start();
void Abc_Stop();
typedef struct Abc_Frame_t_ Abc_Frame_t;
Abc_Frame_t * Abc_FrameGetGlobalFrame();

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif



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

/*!
 \brief Collecting cuts on a single node on a Ntk, a regular process.
*/
TEST(AigCutTest, CutCollect) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Cut_Man_t * pCutMan;
    Cut_Cut_t * pCut;
    int index = 0;

    int nInputs = 6;
    int nOutputs = 1;
    int i = 0;
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
    Abc_Obj_t * pi00 = Abc_NtkPi(pNtk, 0);
    Abc_ObjAssignName(pi00, "pi00", NULL);
    Abc_Obj_t * pi01 = Abc_NtkPi(pNtk, 1);
    Abc_ObjAssignName(pi01, "pi01", NULL);
    Abc_Obj_t * pi02 = Abc_NtkPi(pNtk, 2);
    Abc_ObjAssignName(pi02, "pi02", NULL);
    Abc_Obj_t * pi03 = Abc_NtkPi(pNtk, 3);
    Abc_ObjAssignName(pi03, "pi03", NULL);
    Abc_Obj_t * pi04 = Abc_NtkPi(pNtk, 4);
    Abc_ObjAssignName(pi04, "pi04", NULL);
    Abc_Obj_t * pi05 = Abc_NtkPi(pNtk, 5);
    Abc_ObjAssignName(pi05, "pi05", NULL);
    Abc_Obj_t * pN2012 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi01), pi02);
    Abc_ObjAssignName(pN2012, "n2012", NULL);
    Abc_Obj_t * pN2013 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi00, Abc_ObjNot(pN2012));
    Abc_ObjAssignName(pN2013, "n2013", NULL);
    Abc_Obj_t * pN2011 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi04), pi05);
    Abc_ObjAssignName(pN2011, "n2011", NULL);
    Abc_Obj_t * pN2014 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi03), Abc_ObjNot(pN2011));
    Abc_ObjAssignName(pN2014, "n2014", NULL);
    Abc_Obj_t * pRoot = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pN2013), pN2014);
    Abc_ObjAssignName(pRoot, "nRoot", NULL);
    Abc_ObjAddFanin(Abc_NtkPo(pNtk, 0), Abc_ObjNot(pRoot));

    Cut_Params_t Params, * pParams = &Params;
    memset( pParams, 0, sizeof(Cut_Params_t) );
    pParams->nVarsMax  = 4;     // the max cut size ("k" of the k-feasible cuts)
    pParams->nKeepMax  = 250;   // the max number of cuts kept at a node
    pParams->fTruth    = 1;     // compute truth tables
    pParams->fFilter   = 1;     // filter dominated cuts
    pParams->fSeq      = 0;     // compute sequential cuts
    pParams->fDrop     = 0;     // drop cuts on the fly
    pParams->fVerbose  = 0;     // the verbosiness flag
    pParams->nIdsMax   = Abc_NtkObjNumMax( pNtk );

    pCutMan = Cut_ManStart( pParams );
    // set cuts for PIs
    Abc_NtkForEachCi( pNtk, pObj, i )
        if ( Abc_ObjFanoutNum(pObj) > 0 )
            Cut_NodeSetTriv( pCutMan, pObj->Id );
        
    pCut = (Cut_Cut_t *)Abc_NodeGetCutsRecursive( pCutMan, pN2013, 0, 0 );
    printf(" Node n2013:\n");
    for ( pCut = pCut->pNext; pCut; pCut = pCut->pNext )
    {
        unsigned * pTruth = Cut_CutReadTruth(pCut);
        printf("Cut %d:(0x%x)\n", index, *pTruth);
        for ( i = 0; i < (int)pCut->nLeaves; i++ )
        {
            Abc_Obj_t * pFanin = Abc_NtkObj( pN2013->pNtk, pCut->pLeaves[i] );
            printf("%d(%s) ", Abc_ObjId(pFanin), Abc_ObjName(pFanin));
        }
        printf("\n");
        index++;
    }
    EXPECT_TRUE(index == 2);
    Cut_ManStop(pCutMan);
    Abc_NtkDelete(pNtk);
}


/*!
 \brief Collecting cuts on a single node on a Ntk, a regular process.
*/
TEST(AigCutTest, CutLeavesSizeCollect) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Cut_Man_t * pCutMan;
    Cut_Cut_t * pCut;
    int index4 = 0;
    int index5 = 0;

    int nInputs = 6;
    int nOutputs = 1;
    int i = 0;
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
    Abc_Obj_t * pi00 = Abc_NtkPi(pNtk, 0);
    Abc_ObjAssignName(pi00, "pi00", NULL);
    Abc_Obj_t * pi01 = Abc_NtkPi(pNtk, 1);
    Abc_ObjAssignName(pi01, "pi01", NULL);
    Abc_Obj_t * pi02 = Abc_NtkPi(pNtk, 2);
    Abc_ObjAssignName(pi02, "pi02", NULL);
    Abc_Obj_t * pi03 = Abc_NtkPi(pNtk, 3);
    Abc_ObjAssignName(pi03, "pi03", NULL);
    Abc_Obj_t * pi04 = Abc_NtkPi(pNtk, 4);
    Abc_ObjAssignName(pi04, "pi04", NULL);
    Abc_Obj_t * pi05 = Abc_NtkPi(pNtk, 5);
    Abc_ObjAssignName(pi05, "pi05", NULL);
    Abc_Obj_t * pN2012 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi01), pi02);
    Abc_ObjAssignName(pN2012, "n2012", NULL);
    Abc_Obj_t * pN2013 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi00, Abc_ObjNot(pN2012));
    Abc_ObjAssignName(pN2013, "n2013", NULL);
    Abc_Obj_t * pN2011 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi04), pi05);
    Abc_ObjAssignName(pN2011, "n2011", NULL);
    Abc_Obj_t * pN2014 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi03), Abc_ObjNot(pN2011));
    Abc_ObjAssignName(pN2014, "n2014", NULL);
    Abc_Obj_t * pRoot = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pN2013), pN2014);
    Abc_ObjAssignName(pRoot, "nRoot", NULL);
    Abc_ObjAddFanin(Abc_NtkPo(pNtk, 0), Abc_ObjNot(pRoot));

    Cut_Params_t Params, * pParams = &Params;
    memset( pParams, 0, sizeof(Cut_Params_t) );
    pParams->nVarsMax  = 4;     // the max cut size ("k" of the k-feasible cuts)
    pParams->nKeepMax  = 250;   // the max number of cuts kept at a node
    pParams->fTruth    = 1;     // compute truth tables
    pParams->fFilter   = 1;     // filter dominated cuts
    pParams->fSeq      = 0;     // compute sequential cuts
    pParams->fDrop     = 0;     // drop cuts on the fly
    pParams->fVerbose  = 0;     // the verbosiness flag
    pParams->nIdsMax   = Abc_NtkObjNumMax( pNtk );

    pCutMan = Cut_ManStart( pParams );
    // set cuts for PIs
    Abc_NtkForEachCi( pNtk, pObj, i )
        if ( Abc_ObjFanoutNum(pObj) > 0 )
            Cut_NodeSetTriv( pCutMan, pObj->Id );
        
    pCut = (Cut_Cut_t *)Abc_NodeGetCutsRecursive( pCutMan, pRoot, 0, 0 );
    EXPECT_TRUE(pCut->pLeaves[0] == pRoot->Id && pCut->nLeaves == 1);
    printf(" Node pRoot(LSIZEMAX = 4):\n");
    for ( pCut = pCut->pNext; pCut; pCut = pCut->pNext )
    {
        unsigned * pTruth = Cut_CutReadTruth(pCut);
        printf("Cut %d:(0x%x)\n", index4, *pTruth);
        for ( i = 0; i < (int)pCut->nLeaves; i++ )
        {
            Abc_Obj_t * pFanin = Abc_NtkObj( pRoot->pNtk, pCut->pLeaves[i] );
            printf("%d(%s) ", Abc_ObjId(pFanin), Abc_ObjName(pFanin));
        }
        EXPECT_TRUE(pCut->nLeaves <= 4);
        printf("\n");
        index4++;
    }
    Cut_ManStop(pCutMan);

    printf(" Node pRoot(LSIZEMAX = 5):\n");
    pParams->nVarsMax = 5;
    pCutMan = Cut_ManStart( pParams );
    Abc_NtkForEachCi( pNtk, pObj, i )
        if ( Abc_ObjFanoutNum(pObj) > 0 )
            Cut_NodeSetTriv( pCutMan, pObj->Id );
    pCut = (Cut_Cut_t *)Abc_NodeGetCutsRecursive( pCutMan, pRoot, 0, 0 );
    EXPECT_TRUE(pCut->pLeaves[0] == pRoot->Id && pCut->nLeaves == 1);
    for ( pCut = pCut->pNext; pCut; pCut = pCut->pNext )
    {
        unsigned * pTruth = Cut_CutReadTruth(pCut);
        printf("Cut %d:(0x%x)\n", index5, *pTruth);
        for ( i = 0; i < (int)pCut->nLeaves; i++ )
        {
            Abc_Obj_t * pFanin = Abc_NtkObj( pRoot->pNtk, pCut->pLeaves[i] );
            printf("%d(%s) ", Abc_ObjId(pFanin), Abc_ObjName(pFanin));
        }
        EXPECT_TRUE(pCut->nLeaves <= 5);
        printf("\n");
        index5++;
    }
    EXPECT_TRUE(index5 > index4);
    Cut_ManStop(pCutMan);
    Abc_NtkDelete(pNtk);
}

/*!
 \brief Collecting cuts on a single node on a Ntk, a regular process.
*/
TEST(AigCutTest, CutLeavesSizeCollectReal) {
    Abc_Start();
    Cut_Man_t * pCutMan;
    Cut_Cut_t * pCut;
    char * pFileName = (char *)malloc(sizeof(char) * 100);
    strcat(pFileName, PROJECT_ROOT_DIR);
    strcat(pFileName, "/i10.aig");
    Abc_Ntk_t * pNtk = Io_Read( pFileName, IO_FILE_AIGER, 1, 0 );
    
    Abc_Obj_t * pNode, * pObj;
    int i = 0;
    
    Cut_Params_t Params, * pParams = &Params;
    memset( pParams, 0, sizeof(Cut_Params_t) );
    pParams->nVarsMax  = 4;     // the max cut size ("k" of the k-feasible cuts)
    pParams->nKeepMax  = 250;   // the max number of cuts kept at a node
    pParams->fTruth    = 1;     // compute truth tables
    pParams->fFilter   = 1;     // filter dominated cuts
    pParams->fSeq      = 0;     // compute sequential cuts
    pParams->fDrop     = 0;     // drop cuts on the fly
    pParams->fVerbose  = 0;     // the verbosiness flag
    pParams->nIdsMax   = Abc_NtkObjNumMax( pNtk );

    pCutMan = Cut_ManStart( pParams );
    // set cuts for PIs
    Abc_NtkForEachCi( pNtk, pObj, i )
        if ( Abc_ObjFanoutNum(pObj) > 0 )
            Cut_NodeSetTriv( pCutMan, pObj->Id );
    Abc_NtkForEachNode( pNtk, pNode, i ) {
        pCut = (Cut_Cut_t *)Abc_NodeGetCutsRecursive( pCutMan, pNode, 0, 0 );
        EXPECT_TRUE(pCut != NULL);
    }
    Cut_ManStop(pCutMan);
    Abc_NtkDelete(pNtk);
    Abc_Stop();
    free(pFileName);
}


ABC_NAMESPACE_IMPL_END