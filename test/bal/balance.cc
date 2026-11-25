#include "gtest/gtest.h"

#include "base/abc/abc.h"
#include "misc/util/abc_namespaces.h"

// abcBalance.c has no internal header and its core helpers are file-local
// (declared static at abcBalance.c:30-35). The only way to unit-test them is
// to compile the file into this test binary.
// - Namespace build: include it unwrapped. The file opens namespace
//   ABC_NAMESPACE itself; wrapping it would nest the functions one level
//   deeper, into ABC_NAMESPACE::ABC_NAMESPACE, where the tests cannot see them.
// - Default build: wrap it in extern "C", so its symbols match libabc.
#if defined(ABC_NAMESPACE)
#include "base/abci/abcBalance.c"
#elif defined(__cplusplus)
extern "C" {
#include "base/abci/abcBalance.c"
}
#else
#include "base/abci/abcBalance.c"
#endif

ABC_NAMESPACE_IMPL_START

// Abc_NodeBalanceCone_rec appends to vSuper and marks every object it
// collects (fMarkB on the regular node). The wrapper Abc_NodeBalanceCone
// starts every collection with an empty list (abcBalance.c:317) and unmarks
// when the collection is over (abcBalance.c:322-323). Tests that call the
// _rec function directly do both here, so that the network can be walked
// again and deleted (Abc_NtkDelete warns about objects that still carry a
// mark).
static void BalTest_UnmarkAndClear( Vec_Ptr_t * vSuper )
{
    for ( int i = 0; i < Vec_PtrSize( vSuper ); i++ )
        Abc_ObjRegular( (Abc_Obj_t *)Vec_PtrEntry( vSuper, i ) )->fMarkB = 0;
    Vec_PtrClear( vSuper );
}

/*!
  \brief Step 1 of balancing: collect the inputs of a chained AND gate
  (Abc_NodeBalanceCone_rec).

  An AIG can only hold 2-input gates, so the 4-input AND a*b*c*d is stored as
  a chain. Abc_NodeBalanceCone_rec walks down from the root and keeps going
  through every fanin that is itself an AND node, pushing the inputs it
  cannot pass (here: the primary inputs) into vSuper.

  The order is child0-first. Abc_AigAndCreate stores the fanin with the
  smaller object Id as child0 (abcAig.c:327), so n2 = n1 & c is stored as
  AND(c, n1) and n3 = n2 & d as AND(d, n2). The walk therefore collects
  d, c, a, b rather than a, b, c, d.
*/
TEST(BalTest, ConeRecCollectsAndChainInputs) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
    Abc_Aig_t * pMan = (Abc_Aig_t *)pNtk->pManFunc;
    Abc_Obj_t * a  = Abc_NtkCreatePi( pNtk );   // Id 1
    Abc_Obj_t * b  = Abc_NtkCreatePi( pNtk );   // Id 2
    Abc_Obj_t * c  = Abc_NtkCreatePi( pNtk );   // Id 3
    Abc_Obj_t * d  = Abc_NtkCreatePi( pNtk );   // Id 4
    Abc_Obj_t * n1 = Abc_AigAnd( pMan, a, b );   // Id 5, level 1
    Abc_Obj_t * n2 = Abc_AigAnd( pMan, n1, c );  // Id 6, level 2
    Abc_Obj_t * n3 = Abc_AigAnd( pMan, n2, d );  // Id 7, level 3

    Vec_Ptr_t * vSuper = Vec_PtrAlloc( 8 );
    // fFirst = 1: the root itself is never treated as a leaf
    int RetValue = Abc_NodeBalanceCone_rec( n3, vSuper, 1, 0, 0 );

    // no literal appears twice
    EXPECT_EQ( RetValue, 0 );
    // n1 and n2 have a single fanout each, so the walk crosses both
    EXPECT_EQ( Vec_PtrSize( vSuper ), 4 );
    // child0-first depth-first order
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 0 ), (const void *)d );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 1 ), (const void *)c );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 2 ), (const void *)a );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 3 ), (const void *)b );
    // the collected inputs are left marked (see BalTest_UnmarkAndClear)
    EXPECT_EQ( (int)a->fMarkB, 1 );
    EXPECT_EQ( (int)b->fMarkB, 1 );
    EXPECT_EQ( (int)c->fMarkB, 1 );
    EXPECT_EQ( (int)d->fMarkB, 1 );
    BalTest_UnmarkAndClear( vSuper );

    Vec_PtrFree( vSuper );
    Abc_NtkDelete( pNtk );
}

/*!
  \brief Step 2 of balancing: a complemented edge ends the walk
  (Abc_NodeBalanceCone_rec).

  The walk crosses a fanin only when the edge into it is positive, because
  x & (y & z) is the 3-input AND x & y & z while x & !(y & z) is not. So a
  complemented edge is a leaf whether it points at a primary input or at an
  AND gate, and neither -d nor -s changes that (abcBalance.c:363 tests the
  complement before it looks at the flags).

  The literal is pushed as it is, complement bit included, and the mark goes
  onto the regular node behind it.
*/
TEST(BalTest, ConeRecStopsAtComplementedEdge) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
    Abc_Aig_t * pMan = (Abc_Aig_t *)pNtk->pManFunc;
    Abc_Obj_t * a  = Abc_NtkCreatePi( pNtk );   // Id 1
    Abc_Obj_t * b  = Abc_NtkCreatePi( pNtk );   // Id 2
    Abc_Obj_t * c  = Abc_NtkCreatePi( pNtk );   // Id 3
    Abc_Obj_t * d  = Abc_NtkCreatePi( pNtk );   // Id 4
    Abc_Obj_t * m1 = Abc_AigAnd( pMan, b, c );               // Id 5, level 1
    Abc_Obj_t * n1 = Abc_AigAnd( pMan, Abc_ObjNot(a), m1 );  // Id 6, level 2:  !a & (b & c)
    Abc_Obj_t * m2 = Abc_AigAnd( pMan, b, d );               // Id 7, level 1
    Abc_Obj_t * n2 = Abc_AigAnd( pMan, a, Abc_ObjNot(m2) );  // Id 8, level 2:  a & !(b & d)
    // both inner ANDs have a single fanout, so only the edge polarity can stop the walk
    ASSERT_EQ( Abc_ObjFanoutNum(m1), 1 );
    ASSERT_EQ( Abc_ObjFanoutNum(m2), 1 );

    Vec_Ptr_t * vSuper = Vec_PtrAlloc( 8 );

    // complemented edge to a primary input: m1 is crossed, !a is kept as is
    int RetValue = Abc_NodeBalanceCone_rec( n1, vSuper, 1, 0, 0 );
    EXPECT_EQ( RetValue, 0 );
    EXPECT_EQ( Vec_PtrSize( vSuper ), 3 );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 0 ), (const void *)Abc_ObjNot(a) );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 1 ), (const void *)b );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 2 ), (const void *)c );
    // the mark sits on the regular node, behind the complement bit
    EXPECT_EQ( (int)a->fMarkB, 1 );
    EXPECT_EQ( (int)m1->fMarkB, 0 );
    BalTest_UnmarkAndClear( vSuper );

    // complemented edge to an AND gate: the gate is not crossed
    RetValue = Abc_NodeBalanceCone_rec( n2, vSuper, 1, 0, 0 );
    EXPECT_EQ( RetValue, 0 );
    EXPECT_EQ( Vec_PtrSize( vSuper ), 2 );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 0 ), (const void *)a );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 1 ), (const void *)Abc_ObjNot(m2) );
    EXPECT_EQ( (int)m2->fMarkB, 1 );
    EXPECT_EQ( (int)b->fMarkB, 0 );
    BalTest_UnmarkAndClear( vSuper );

    // the two duplication flags do not open a complemented edge
    RetValue = Abc_NodeBalanceCone_rec( n2, vSuper, 1, 1, 1 );
    EXPECT_EQ( RetValue, 0 );
    EXPECT_EQ( Vec_PtrSize( vSuper ), 2 );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 1 ), (const void *)Abc_ObjNot(m2) );
    BalTest_UnmarkAndClear( vSuper );

    Vec_PtrFree( vSuper );
    Abc_NtkDelete( pNtk );
}

/*!
  \brief Step 2 of balancing, continued: an AND gate shared with other gates
  is a leaf (Abc_NodeBalanceCone_rec).

  Crossing a gate means building its logic again inside the gate being
  balanced. To avoid that, the walk stops at an AND with more than one fanout
  (abcBalance.c:363). Options -d (fDuplicate) and -s (fSelective) remove this
  stop; for the walk they act identically.

  m = a & b feeds both n1 and n2, so for n1 the walk collects m itself rather
  than a and b. With a duplication flag set, it collects a and b.
*/
TEST(BalTest, ConeRecStopsAtSharedAnd) {
    Abc_Ntk_t * pNtk = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
    Abc_Aig_t * pMan = (Abc_Aig_t *)pNtk->pManFunc;
    Abc_Obj_t * a  = Abc_NtkCreatePi( pNtk );   // Id 1
    Abc_Obj_t * b  = Abc_NtkCreatePi( pNtk );   // Id 2
    Abc_Obj_t * c  = Abc_NtkCreatePi( pNtk );   // Id 3
    Abc_Obj_t * d  = Abc_NtkCreatePi( pNtk );   // Id 4
    Abc_Obj_t * m  = Abc_AigAnd( pMan, a, b );   // Id 5, level 1
    Abc_Obj_t * n1 = Abc_AigAnd( pMan, m, c );   // Id 6, level 2, stored as AND(c, m)
    Abc_Obj_t * n2 = Abc_AigAnd( pMan, m, d );   // Id 7, level 2
    // m is shared: its two fanouts are n1 and n2
    ASSERT_EQ( Abc_ObjFanoutNum(m), 2 );
    EXPECT_EQ( (const void *)Abc_ObjFanout(m, 0), (const void *)n1 );
    EXPECT_EQ( (const void *)Abc_ObjFanout(m, 1), (const void *)n2 );

    Vec_Ptr_t * vSuper = Vec_PtrAlloc( 8 );

    // default flags: the shared gate is a leaf
    int RetValue = Abc_NodeBalanceCone_rec( n1, vSuper, 1, 0, 0 );
    EXPECT_EQ( RetValue, 0 );
    EXPECT_EQ( Vec_PtrSize( vSuper ), 2 );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 0 ), (const void *)c );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 1 ), (const void *)m );
    EXPECT_EQ( (int)m->fMarkB, 1 );
    EXPECT_EQ( (int)a->fMarkB, 0 );
    BalTest_UnmarkAndClear( vSuper );

    // fDuplicate (option -d): the walk crosses the shared gate
    RetValue = Abc_NodeBalanceCone_rec( n1, vSuper, 1, 1, 0 );
    EXPECT_EQ( RetValue, 0 );
    EXPECT_EQ( Vec_PtrSize( vSuper ), 3 );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 0 ), (const void *)c );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 1 ), (const void *)a );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 2 ), (const void *)b );
    EXPECT_EQ( (int)m->fMarkB, 0 );
    BalTest_UnmarkAndClear( vSuper );

    // fSelective (option -s): the same walk, the same result
    RetValue = Abc_NodeBalanceCone_rec( n1, vSuper, 1, 0, 1 );
    EXPECT_EQ( RetValue, 0 );
    EXPECT_EQ( Vec_PtrSize( vSuper ), 3 );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 1 ), (const void *)a );
    EXPECT_EQ( (const void *)Vec_PtrEntry( vSuper, 2 ), (const void *)b );
    BalTest_UnmarkAndClear( vSuper );

    Vec_PtrFree( vSuper );
    Abc_NtkDelete( pNtk );
}

ABC_NAMESPACE_IMPL_END
