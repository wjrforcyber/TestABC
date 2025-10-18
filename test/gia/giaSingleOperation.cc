#include "gtest/gtest.h"
#include "aig/gia/gia.h"
#include "aig/aig/aig.h"
#include "aig/gia/giaAig.h"
#include "proof/cec/cec.h"

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
    Gia_Man_t * giaMan =  Gia_ManStart(100);
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
ABC_NAMESPACE_IMPL_END