#include "gtest/gtest.h"
#include "base/abc/abc.h"
#include "aig/gia/gia.h"
#include "aig/aig/aig.h"
#include "aig/gia/giaAig.h"
#include "base/main/abcapis.h"
#include "base/main/main.h"
#include "proof/cec/cec.h"

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
Abc_Ntk_t *Abc_NtkFromAigPhase(Aig_Man_t *pMan);

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif


ABC_NAMESPACE_IMPL_START
/*!
  \brief
  Gia manager transform to Aig manager
*/
TEST(GiaSingleOperation, Gia2Aig) {
    Gia_Man_t* giaMan =  Gia_ManStart(100);

    int input1 = Gia_ManAppendCi(giaMan);
    int input2 = Gia_ManAppendCi(giaMan);
    int input3 = Gia_ManAppendCi(giaMan);

    int aOut0 = Gia_ManAppendAnd(giaMan, input1, input2);
    int aOut1 = Gia_ManAppendAnd(giaMan, aOut0, input3);
    Gia_ManAppendCo(giaMan, aOut1);

    Aig_Man_t * pAig = Gia_ManToAig( giaMan, 0 );
    EXPECT_EQ(Aig_ManCiNum( pAig ), Gia_ManPiNum(giaMan));
    EXPECT_EQ(Aig_ManCoNum(pAig), Gia_ManPoNum(giaMan));
    EXPECT_EQ(Aig_ManNodeNum(pAig), Gia_ManAndNum(giaMan));
    Gia_ManStop(giaMan);
    Aig_ManStop(pAig);
}


/*!
  \brief
  Aig manager transform back to Gia manager with cec checked
*/
TEST(GiaSingleOperation, Aig2Gia) {
    Gia_Man_t * giaMan = Gia_ManStart(100);
    Gia_Man_t * giaBack;

    int input1 = Gia_ManAppendCi(giaMan);
    int input2 = Gia_ManAppendCi(giaMan);
    int input3 = Gia_ManAppendCi(giaMan);

    int aOut0 = Gia_ManAppendAnd(giaMan, input1, input2);
    int aOut1 = Gia_ManAppendAnd(giaMan, aOut0, input3);
    Gia_ManAppendCo(giaMan, aOut1);

    Aig_Man_t * pAig = Gia_ManToAig( giaMan, 0 );
    giaBack = Gia_ManFromAig(pAig);

    EXPECT_EQ(Gia_ManPiNum(giaBack), Gia_ManPiNum(giaMan));
    EXPECT_EQ(Gia_ManPoNum(giaBack), Gia_ManPoNum(giaMan));
    EXPECT_EQ(Gia_ManAndNum(giaBack), Gia_ManAndNum(giaMan));

    Gia_Man_t * pMiter = Gia_ManMiter( giaMan, giaBack, 0, 1, 0, 0, 0 );
    Cec_ParCec_t ParsCec, * pPars = &ParsCec;
    Cec_ManCecSetDefaultParams( pPars );
    EXPECT_TRUE(Cec_ManVerify( pMiter, pPars ) == 1);
    Gia_ManStop(giaBack);
    Gia_ManStop(giaMan);
    Gia_ManStop(pMiter);
    Aig_ManStop(pAig);
}


/*!
  \brief
  ABC9 to ABC for optimization. There's no direct manager in old rw/rf/b, so should be transformed to Ntk level. An example of balance is given here.
*/
TEST(GiaSingleOperation, Gia2AigForOpt) {
    Abc_Start();
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    Gia_Man_t * giaMan = Gia_ManStart(100);
    Abc_Ntk_t * pNtk, * pNtkRes;

    int input1 = Gia_ManAppendCi(giaMan);
    int input2 = Gia_ManAppendCi(giaMan);
    int input3 = Gia_ManAppendCi(giaMan);
    int input4 = Gia_ManAppendCi(giaMan);

    int aOut0 = Gia_ManAppendAnd(giaMan, input1, input2);
    int aOut1 = Gia_ManAppendAnd(giaMan, aOut0, input3);
    int aOut2 = Gia_ManAppendAnd(giaMan, aOut1, input4);
    Gia_ManAppendCo(giaMan, aOut2);

    // prepare the Ntk used for ABC operation
    Aig_Man_t * pAig = Gia_ManToAig( giaMan, 0 );
    pNtk = Abc_NtkFromAigPhase( pAig );
    pNtk->pName = Extra_UtilStrsav(pAig->pName);
    EXPECT_TRUE(Abc_NtkIsStrash(pNtk));
    // Balance
    EXPECT_EQ(Abc_NtkLevel(pNtk), 3);
    pNtkRes = Abc_NtkBalance( pNtk, 0, 0, 1 );
    EXPECT_EQ(Abc_NtkLevel(pNtkRes), 2);
    Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
    Gia_ManStop(giaMan);
    Aig_ManStop(pAig);
    Abc_NtkDelete(pNtkRes);
    Abc_NtkDelete(pNtk);
}


ABC_NAMESPACE_IMPL_END