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
void Abc_ManResubSimulate( Vec_Ptr_t * vDivs, int nLeaves, Vec_Ptr_t * vSims, int nLeavesMax, int nWords );
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
    Abc_ObjAssignName(pi00, "pi016", NULL);
    Abc_Obj_t * pi01 = Abc_NtkPi(pNtk, 1);
    Abc_ObjAssignName(pi01, "n984", NULL);
    Abc_Obj_t * pi02 = Abc_NtkPi(pNtk, 2);
    Abc_ObjAssignName(pi02, "n1600", NULL);
    Abc_Obj_t * pi03 = Abc_NtkPi(pNtk, 3);
    Abc_ObjAssignName(pi03, "n507", NULL);
    Abc_Obj_t * pi04 = Abc_NtkPi(pNtk, 4);
    Abc_ObjAssignName(pi04, "pi039", NULL);
    Abc_Obj_t * pi05 = Abc_NtkPi(pNtk, 5);
    Abc_ObjAssignName(pi05, "pi182", NULL);
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
    unsigned int umask[] = {0x1, 0x3, 0xf, 0xff, 0xffff};
    printf(" Node n2012(phase = %d):\n", pN2012->fPhase);
    pCut = (Cut_Cut_t *)Abc_NodeGetCutsRecursive( pCutMan, pN2012, 0, 0 );
    for ( pCut = pCut->pNext; pCut; pCut = pCut->pNext )
    {
        unsigned * pTruth = Cut_CutReadTruth(pCut);
        printf("Cut %d:(0x%016x)\n", index, *pTruth & umask[pCut->nLeaves]);
        // only one cut for n2012
        EXPECT_TRUE((*pTruth & umask[pCut->nLeaves]) == 0x4);
        for ( i = 0; i < (int)pCut->nLeaves; i++ )
        {
            Abc_Obj_t * pFanin = Abc_NtkObj( pN2012->pNtk, pCut->pLeaves[i] );
            printf("%d(%s) ", Abc_ObjId(pFanin), Abc_ObjName(pFanin));
        }
        printf("\n");
    }
    pCut = (Cut_Cut_t *)Abc_NodeGetCutsRecursive( pCutMan, pN2013, 0, 0 );
    printf(" Node n2013(phase = %d):\n", pN2013->fPhase);
    for ( pCut = pCut->pNext; pCut; pCut = pCut->pNext )
    {
        unsigned * pTruth = Cut_CutReadTruth(pCut);
        printf("Cut %d:(0x%016x)\n", index, *pTruth & umask[pCut->nLeaves]);
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
    unsigned int umask[] = {0x1, 0x3, 0xf, 0xff, 0xffff, 0xffffffff};
    pCut = (Cut_Cut_t *)Abc_NodeGetCutsRecursive( pCutMan, pRoot, 0, 0 );
    EXPECT_TRUE(pCut->pLeaves[0] == pRoot->Id && pCut->nLeaves == 1);
    printf(" Node pRoot(LSIZEMAX = 4):\n");
    for ( pCut = pCut->pNext; pCut; pCut = pCut->pNext )
    {
        unsigned * pTruth = Cut_CutReadTruth(pCut);
        printf("Cut %d:(0x%x)\n", index4, *pTruth & umask[pCut->nLeaves]);
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
        printf("Cut %d:(0x%x)\n", index5, *pTruth & umask[pCut->nLeaves]);
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
 \brief Collecting tree cuts.
*/
TEST(AigCutTest, CutTreeCollect) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Cut_Man_t * pCutMan;
    Cut_Cut_t * pCut;
    int indexTree = 0;
    int nInputs = 6;
    int nOutputs = 2;
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
    Abc_Obj_t * pRoot0 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pN2012, pi05);
    Abc_ObjAssignName(pRoot0, "nRoot0", NULL);
    Abc_ObjAddFanin(Abc_NtkPo(pNtk, 0), Abc_ObjNot(pRoot));
    Abc_ObjAddFanin(Abc_NtkPo(pNtk, 1), pRoot0);

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
    unsigned int umask[] = {0x1, 0x3, 0xf, 0xff, 0xffff, 0xffffffff};
    pCut = (Cut_Cut_t *)Abc_NodeGetCutsRecursive( pCutMan, pRoot, 0, 1 );
    //EXPECT_TRUE(pCut->pLeaves[0] == pRoot->Id && pCut->nLeaves == 1);
    printf(" Node pRoot(LSIZEMAX = 4):\n");
    for ( pCut = pCut->pNext; pCut; pCut = pCut->pNext )
    {
        unsigned * pTruth = Cut_CutReadTruth(pCut);
        printf("Cut %d:(0x%x)\n", indexTree, *pTruth & umask[pCut->nLeaves]);
        for ( i = 0; i < (int)pCut->nLeaves; i++ )
        {
            Abc_Obj_t * pFanin = Abc_NtkObj( pRoot->pNtk, pCut->pLeaves[i] );
            printf("%d(%s) ", Abc_ObjId(pFanin), Abc_ObjName(pFanin));
        }
        EXPECT_TRUE(pCut->nLeaves <= 4);
        printf("\n");
        indexTree++;
    }
    // one dag cut should be exluded (pi00, n894, n1600, n2014)
    EXPECT_TRUE(indexTree == 5);
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


/*!
  \brief A corner case test for cut truth computation
*/
TEST(AigCutTest, CutTruthSimTruth) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    // create the PIs
    int nInputs = 3;
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
    Abc_Obj_t * pi_n669 = Abc_NtkPi(pNtk, 0);
    Abc_ObjAssignName(pi_n669, "n669", NULL);
    Abc_Obj_t * pi_n670 = Abc_NtkPi(pNtk, 1);
    Abc_ObjAssignName(pi_n670, "n670", NULL);
    Abc_Obj_t * pi_n675 = Abc_NtkPi(pNtk, 2);
    Abc_ObjAssignName(pi_n675, "n675", NULL);
    
    Abc_Obj_t * pN676 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi_n670, Abc_ObjNot(pi_n675));
    Abc_ObjAssignName(pN676, "n676", NULL);
    Abc_Obj_t * pN677 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pi_n670), pi_n675);
    Abc_ObjAssignName(pN677, "n677", NULL);
    Abc_Obj_t * pN678 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, Abc_ObjNot(pN676), Abc_ObjNot(pN677));
    Abc_ObjAssignName(pN678, "n678", NULL);
    Abc_Obj_t * pN682 = Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi_n669, Abc_ObjNot(pN678));
    Abc_ObjAssignName(pN682, "n682", NULL);
    Abc_ObjAddFanin(Abc_NtkPo(pNtk, 0), pN682);
    EXPECT_TRUE(Abc_NtkPiNum(pNtk) == 3);
    EXPECT_TRUE(Abc_NtkNodeNum(pNtk) == 4);
    unsigned int uCutTruth;
    unsigned int uSimTruth;
    //Cut
    Cut_Man_t * pCutMan;
    Cut_Cut_t * pCut;
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
    // get cuts for n682
    pCut = (Cut_Cut_t *)Abc_NodeGetCutsRecursive( pCutMan, pN682, 0, 0 );
    Vec_Ptr_t * vLeavesFilter = Vec_PtrAlloc(10);
    Vec_PtrPush(vLeavesFilter, pi_n669);
    Vec_PtrPush(vLeavesFilter, pi_n670);
    Vec_PtrPush(vLeavesFilter, pi_n675);
    for ( pCut = pCut->pNext; pCut; pCut = pCut->pNext )
    {
        if(Vec_PtrFind(vLeavesFilter, Abc_NtkObj( pNtk, pCut->pLeaves[0])) >= 0 \
        && Vec_PtrFind(vLeavesFilter, Abc_NtkObj( pNtk, pCut->pLeaves[1])) >= 0 \
           && Vec_PtrFind(vLeavesFilter, Abc_NtkObj( pNtk, pCut->pLeaves[2])) >= 0)
        {
            unsigned * pTruth = Cut_CutReadTruth(pCut);
            printf("Cut:(0x%016x)\n", *pTruth & 0xff);
            uCutTruth = (*pTruth) & 0xff;
            // expected truth is 0x28 for n682
            EXPECT_TRUE((*pTruth & 0xff) == 0x28);
            for ( i = 0; i < (int)pCut->nLeaves; i++ )
            {
                Abc_Obj_t * pFanin = Abc_NtkObj( pN682->pNtk, pCut->pLeaves[i] );
                printf("%d", Abc_ObjId(pFanin));
                printf("(%s) ", Abc_ObjName(pFanin));
            }
            printf("\n");
        }
    }
    Vec_PtrFree(vLeavesFilter);
    Cut_ManStop(pCutMan);
    //Sim
    Vec_Ptr_t * vNodes = Vec_PtrAlloc(10);
    Vec_PtrPush(vNodes, pi_n669);
    Vec_PtrPush(vNodes, pi_n670);
    Vec_PtrPush(vNodes, pi_n675);
    Vec_PtrPush(vNodes, pN676);
    Vec_PtrPush(vNodes, pN677);
    Vec_PtrPush(vNodes, pN678);
    Vec_PtrPush(vNodes, pN682);
    int k;
    int nVar       = 3;
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
    Abc_ManResubSimulate( vNodes, 3, vSims, 4, nWords);
    unsigned int uNode = (*((unsigned int *)(pN682->pData))) & 0xff;
    uSimTruth = uNode;
    EXPECT_TRUE(uSimTruth == 0x28);
    EXPECT_TRUE(uCutTruth == uSimTruth);
    Vec_PtrFree(vNodes);
    Vec_PtrFree(vSims);
    ABC_FREE( pInfo );
    Abc_NtkDelete(pNtk);
}
ABC_NAMESPACE_IMPL_END