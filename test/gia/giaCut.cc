#include "gtest/gtest.h"
#include "aig/gia/gia.h"

// giaCut.c exposes no public header -- forward-declare what we use.
// Gia_Sto_t is opaque (defined only inside giaCut.c).
#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif
typedef struct Gia_Sto_t_ Gia_Sto_t;
Gia_Sto_t * Gia_StoAlloc( Gia_Man_t * pGia, int nCutSize, int nCutNum, int fCutMin, int fTruthMin, int fVerbose );
void        Gia_StoFree( Gia_Sto_t * p );
void        Gia_StoRefObj( Gia_Sto_t * p, int iObj );
void        Gia_StoComputeCutsConst0( Gia_Sto_t * p, int iObj );
void        Gia_StoComputeCutsCi( Gia_Sto_t * p, int iObj );
void        Gia_StoComputeCutsNode( Gia_Sto_t * p, int iObj );
void        Gia_StoComputeCuts( Gia_Man_t * pGia );
int         Gia_ManCountSelfCuts( Gia_Man_t * p, Gia_Sto_t * pSto );
int         Gia_StoSelectOneCut( Vec_Wec_t * vCuts, int iObj, Vec_Int_t * vCut, int nCutSizeMin );
Vec_Wec_t * Gia_ManExtractCuts( Gia_Man_t * pGia, int nCutSize0, int nCuts0, int fVerbose0 );
#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif


ABC_NAMESPACE_IMPL_START

// GIA: (a & b) -> PO
static Gia_Man_t * GiaCutTest_BuildSimpleGia()
{
    Gia_Man_t * p = Gia_ManStart( 100 );
    int a = Gia_ManAppendCi( p );
    int b = Gia_ManAppendCi( p );
    int and0 = Gia_ManAppendAnd( p, a, b );
    Gia_ManAppendCo( p, and0 );
    return p;
}

// GIA: ((a & b) & c) -> PO
static Gia_Man_t * GiaCutTest_BuildChainGia()
{
    Gia_Man_t * p = Gia_ManStart( 100 );
    int a  = Gia_ManAppendCi( p );
    int b  = Gia_ManAppendCi( p );
    int c  = Gia_ManAppendCi( p );
    int n1 = Gia_ManAppendAnd( p, a, b );
    int n2 = Gia_ManAppendAnd( p, n1, c );
    Gia_ManAppendCo( p, n2 );
    return p;
}

// Mirror the inner loop of Gia_ManMatchCutsInt.
static void GiaCutTest_ComputeAllCuts( Gia_Man_t * pGia, Gia_Sto_t * p )
{
    Gia_Obj_t * pObj; int i, iObj;
    Gia_ManForEachObj( pGia, pObj, iObj )
        Gia_StoRefObj( p, iObj );
    Gia_StoComputeCutsConst0( p, 0 );
    Gia_ManForEachCiId( pGia, iObj, i )
        Gia_StoComputeCutsCi( p, iObj );
    Gia_ManForEachAnd( pGia, pObj, iObj )
        Gia_StoComputeCutsNode( p, iObj );
}

TEST(GiaCutTest, CanAllocAndFreeSto)
{
    Gia_Man_t * pGia = GiaCutTest_BuildSimpleGia();
    ASSERT_TRUE( pGia != nullptr );
    Gia_Sto_t * p = Gia_StoAlloc( pGia, 8, 6, 0, 0, 0 );
    EXPECT_TRUE( p != nullptr );
    Gia_StoFree( p );
    Gia_ManStop( pGia );
}

TEST(GiaCutTest, CanAllocAndFreeStoWithCutMin)
{
    Gia_Man_t * pGia = GiaCutTest_BuildSimpleGia();
    ASSERT_TRUE( pGia != nullptr );
    Gia_Sto_t * p = Gia_StoAlloc( pGia, 6, 8, 1, 0, 0 );
    EXPECT_TRUE( p != nullptr );
    Gia_StoFree( p );
    Gia_ManStop( pGia );
}

// Gia_Sto_t is opaque: validate Const0/Ci/Node indirectly -- every AND owns a
// size-2 self-cut {fan0,fan1}, so Gia_ManCountSelfCuts == Gia_ManAndNum.
TEST(GiaCutTest, ComputeCutsIncremental_SimpleGia)
{
    Gia_Man_t * pGia = GiaCutTest_BuildSimpleGia();
    ASSERT_TRUE( pGia != nullptr );
    Gia_Sto_t * p = Gia_StoAlloc( pGia, 8, 8, 0, 0, 0 );
    ASSERT_TRUE( p != nullptr );
    GiaCutTest_ComputeAllCuts( pGia, p );
    EXPECT_EQ( Gia_ManCountSelfCuts(pGia, p), Gia_ManAndNum(pGia) );
    Gia_StoFree( p );
    Gia_ManStop( pGia );
}

TEST(GiaCutTest, ComputeCutsIncremental_ChainGia)
{
    Gia_Man_t * pGia = GiaCutTest_BuildChainGia();
    ASSERT_TRUE( pGia != nullptr );
    Gia_Sto_t * p = Gia_StoAlloc( pGia, 8, 8, 0, 0, 0 );
    ASSERT_TRUE( p != nullptr );
    GiaCutTest_ComputeAllCuts( pGia, p );
    EXPECT_EQ( Gia_ManCountSelfCuts(pGia, p), Gia_ManAndNum(pGia) );
    Gia_StoFree( p );
    Gia_ManStop( pGia );
}

// Gia_StoComputeCuts is void and frees its own Sto: assert it runs clean and
// leaves the input GIA unchanged.
TEST(GiaCutTest, StoComputeCutsRunsOnSimpleGia)
{
    Gia_Man_t * pGia = GiaCutTest_BuildSimpleGia();
    ASSERT_TRUE( pGia != nullptr );
    int nObjs = Gia_ManObjNum( pGia );
    int nAnds = Gia_ManAndNum( pGia );
    Gia_StoComputeCuts( pGia );
    EXPECT_EQ( Gia_ManObjNum( pGia ), nObjs );
    EXPECT_EQ( Gia_ManAndNum( pGia ), nAnds );
    Gia_ManStop( pGia );
}

// Synthetic cut-store in the Sdb_ForEachCut packed layout [nCuts, (nLeaves, leaves..., iFunc)...].
static Vec_Wec_t * GiaCutTest_BuildCutStore()
{
    Vec_Wec_t * vCuts = Vec_WecStart( 3 );
    Vec_Int_t * v0 = Vec_WecEntry( vCuts, 0 );
    Vec_IntPush( v0, 2 );
    Vec_IntPush( v0, 1 ); Vec_IntPush( v0, 1 ); Vec_IntPush( v0, 7 );     // {1}
    Vec_IntPush( v0, 3 ); Vec_IntPush( v0, 2 ); Vec_IntPush( v0, 3 );
    Vec_IntPush( v0, 4 ); Vec_IntPush( v0, 8 );                           // {2,3,4}
    Vec_Int_t * v1 = Vec_WecEntry( vCuts, 1 );
    Vec_IntPush( v1, 1 );
    Vec_IntPush( v1, 1 ); Vec_IntPush( v1, 5 ); Vec_IntPush( v1, 9 );     // {5}
    return vCuts;   // node 2 left empty
}

TEST(GiaCutTest, SelectOneCut_SkipsBelowThresholdAndPicksFirstQualifying)
{
    Vec_Wec_t * vCuts = GiaCutTest_BuildCutStore();
    Vec_Int_t * vCut  = Vec_IntAlloc( 10 );

    EXPECT_EQ( Gia_StoSelectOneCut( vCuts, 0, vCut, 2 ), 1 );   // skips {1}, picks {2,3,4}
    ASSERT_EQ( Vec_IntSize(vCut), 4 );
    EXPECT_EQ( Vec_IntEntry(vCut, 0), 3 );
    EXPECT_EQ( Vec_IntEntry(vCut, 1), 2 );
    EXPECT_EQ( Vec_IntEntry(vCut, 2), 3 );
    EXPECT_EQ( Vec_IntEntry(vCut, 3), 4 );

    EXPECT_EQ( Gia_StoSelectOneCut( vCuts, 0, vCut, 1 ), 1 );   // min 1 -> first cut {1}
    ASSERT_EQ( Vec_IntSize(vCut), 2 );
    EXPECT_EQ( Vec_IntEntry(vCut, 0), 1 );
    EXPECT_EQ( Vec_IntEntry(vCut, 1), 1 );

    Vec_IntFree( vCut );
    Vec_WecFree( vCuts );
}

TEST(GiaCutTest, SelectOneCut_ReturnsZeroWhenNoneQualifyOrEmpty)
{
    Vec_Wec_t * vCuts = GiaCutTest_BuildCutStore();
    Vec_Int_t * vCut  = Vec_IntAlloc( 10 );

    EXPECT_EQ( Gia_StoSelectOneCut( vCuts, 1, vCut, 2 ), 0 );   // {5} below min 2
    EXPECT_EQ( Vec_IntSize(vCut), 0 );
    EXPECT_EQ( Gia_StoSelectOneCut( vCuts, 2, vCut, 1 ), 0 );   // empty node
    EXPECT_EQ( Vec_IntSize(vCut), 0 );

    Vec_IntFree( vCut );
    Vec_WecFree( vCuts );
}

// Only the AND has a size>=2 cut (self-cut {1,2}); selection is deterministic.
TEST(GiaCutTest, ExtractCuts_SimpleGia)
{
    Gia_Man_t * pGia = GiaCutTest_BuildSimpleGia();
    ASSERT_TRUE( pGia != nullptr );
    int nCuts0 = 5, nCutSize0 = 3;
    Vec_Wec_t * vSel = Gia_ManExtractCuts( pGia, nCutSize0, nCuts0, 0 );
    ASSERT_TRUE( vSel != nullptr );
    EXPECT_EQ( Vec_WecSize(vSel), nCuts0 );
    for ( int i = 0; i < Vec_WecSize(vSel); i++ )
    {
        Vec_Int_t * vCut = Vec_WecEntry( vSel, i );
        ASSERT_GT( Vec_IntSize(vCut), 0 );
        int nLeaves = Vec_IntEntry( vCut, 0 );
        EXPECT_GE( nLeaves, nCutSize0 - 1 );
        EXPECT_EQ( Vec_IntSize(vCut), nLeaves + 1 );
        EXPECT_EQ( nLeaves, 2 );
        EXPECT_EQ( Vec_IntEntry(vCut, 1), 1 );
        EXPECT_EQ( Vec_IntEntry(vCut, 2), 2 );
    }
    Vec_WecFree( vSel );
    Gia_ManStop( pGia );
}

ABC_NAMESPACE_IMPL_END
