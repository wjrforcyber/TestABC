#include "gtest/gtest.h"

#include "aig/aig/aig.h"
#include "aig/gia/gia.h"
#include "misc/util/abc_global.h"
#include "proof/dch/dch.h"

#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif

Gia_Man_t * Gia_ManAigSynch2Choices( Gia_Man_t * pGia1, Gia_Man_t * pGia2, Gia_Man_t * pGia3, Dch_Pars_t * pPars );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif


ABC_NAMESPACE_IMPL_START

/*!
  \brief Construct choice network using 3 snapshots from `Gia` network.
*/
TEST(GiaSamples, FormChoicesWith3Samples) {
    Dch_Pars_t * pParsDch = (Dch_Pars_t *)malloc(sizeof(Dch_Pars_t));
    Dch_ManSetDefaultParams( pParsDch );
    pParsDch->nBTLimit = 100;
    Gia_Man_t * pGia0, * pGia1, * pGia2;
    pGia0 = Gia_AigerRead( "../data/i10Samples/i10_SpId_0.aig", 0, 0, 0 );
    pGia1 = Gia_AigerRead( "../data/i10Samples/i10_SpId_1.aig", 0, 0, 0 );
    pGia2 = Gia_AigerRead( "../data/i10Samples/i10_SpId_2.aig", 0, 0, 0 );
    EXPECT_TRUE(pGia0 != nullptr);
    EXPECT_TRUE(pGia1 != nullptr);
    EXPECT_TRUE(pGia2 != nullptr);
    Gia_Man_t * pFinal  = Gia_ManAigSynch2Choices( pGia0, pGia1, pGia2, pParsDch);
    Gia_ManStop( pGia0 );
    Gia_ManStop( pGia1 );
    Gia_ManStop( pGia2 );
    ABC_FREE(pParsDch);
    EXPECT_TRUE(pFinal != nullptr);
    EXPECT_TRUE(Gia_ManHasChoices(pFinal) == 1);
}

ABC_NAMESPACE_IMPL_END