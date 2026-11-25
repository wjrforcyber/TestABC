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
    // the visited inputs are marked; only the wrapper Abc_NodeBalanceCone
    // clears fMarkB, and it is the subject of a later step
    EXPECT_EQ( (int)a->fMarkB, 1 );
    EXPECT_EQ( (int)b->fMarkB, 1 );
    EXPECT_EQ( (int)c->fMarkB, 1 );
    EXPECT_EQ( (int)d->fMarkB, 1 );
    // clear the marks the way the wrapper does (abcBalance.c:322-323);
    // Abc_NtkDelete warns about objects that still carry a mark
    for ( int i = 0; i < Vec_PtrSize( vSuper ); i++ )
        Abc_ObjRegular( (Abc_Obj_t *)Vec_PtrEntry( vSuper, i ) )->fMarkB = 0;

    Vec_PtrFree( vSuper );
    Abc_NtkDelete( pNtk );
}

ABC_NAMESPACE_IMPL_END
