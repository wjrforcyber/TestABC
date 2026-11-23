#include "gtest/gtest.h"

#include "proof/dch/dchInt.h"
#include "misc/util/abc_namespaces.h"

ABC_NAMESPACE_IMPL_START

/*!
  \brief Step 1 of the SAT usage in choice verification: prepare the SAT call.

  Before the first sat_solver_solve, the choicing manager maps every touched
  AIG node to a fresh Tseitin variable through pSatVars[Node->Id]
  (Dch_ObjSatNum). The mapping starts empty (slot 0 = unassigned), variable 1
  is reserved for the constant-1 node of the fraig AIG, and nodes interior to
  a supergate (single-fanout AND cone) are folded: they never get a variable
  of their own.
*/
TEST(DchTest, PrepareSatCallCollectsVars) {
    // tiny AIG:  nAb = a & b (single-fanout),  nAbc = nAb & c
    Aig_Man_t * pAig = Aig_ManStart( 8 );
    Aig_Obj_t * a    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * b    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * c    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * nAb  = Aig_And( pAig, a, b );
    Aig_Obj_t * nAbc = Aig_And( pAig, nAb, c );

    Dch_Pars_t Pars, * pPars = &Pars;
    Dch_ManSetDefaultParams( pPars );
    pPars->fPolarFlip = 0;  // plain polarity: a SAT var stands for the node value

    Dch_Man_t * p = Dch_ManCreate( pAig, pPars );
    // the real flow creates the fraig AIG in Dch_ManSweep; only its const1
    // node is needed by the solver setup
    p->pAigFraig = Aig_ManStart( 4 );

    // nothing is mapped before the first SAT call
    EXPECT_EQ( p->pSat, nullptr );
    EXPECT_EQ( p->nSatVars, 1 );
    EXPECT_EQ( Dch_ObjSatNum(p, Aig_ManConst1(p->pAigFraig)), 0 );
    EXPECT_EQ( Dch_ObjSatNum(p, a), 0 );
    EXPECT_EQ( Dch_ObjSatNum(p, nAbc), 0 );

    // create the solver the same way the first Dch_NodesAreEquiv call does
    Dch_ManSatSolverRecycle( p );
    ASSERT_NE( p->pSat, nullptr );
    EXPECT_EQ( p->nSatVars, 2 );  // var 0 is unused, var 1 is const1
    EXPECT_EQ( Dch_ObjSatNum(p, Aig_ManConst1(p->pAigFraig)), 1 );
    // the const1 unit clause is enqueued for propagation, not stored
    EXPECT_NE( p->pSat->qtail, p->pSat->qhead );
    EXPECT_EQ( sat_solver_nclauses(p->pSat), 0 );
    // propagate it, mirroring dchSat.c
    EXPECT_NE( sat_solver_simplify(p->pSat), 0 );
    EXPECT_EQ( p->pSat->qtail, p->pSat->qhead );

    // collect variables for nAbc's cone
    Dch_CnfNodeAddToSolver( p, nAbc );
    EXPECT_EQ( p->nSatVars, 6 );  // const1 + { nAbc, a, b, c }
    EXPECT_EQ( Dch_ObjSatNum(p, nAbc), 2 );
    // leaf order [c, a, b]: Aig_ObjCreateGhost canonicalizes fanins so the
    // smaller-Id fanin is child0 (c is child0 of nAbc), and
    // Dch_CollectSuper_rec visits child0 first
    EXPECT_EQ( Dch_ObjSatNum(p, c), 3 );
    EXPECT_EQ( Dch_ObjSatNum(p, a), 4 );
    EXPECT_EQ( Dch_ObjSatNum(p, b), 5 );
    // nAb is interior to the supergate: folded, no variable of its own
    EXPECT_EQ( Dch_ObjSatNum(p, nAb), 0 );
    EXPECT_EQ( Vec_PtrSize(p->vUsedNodes), 4 );
    // encodes nAbc = a & b & c (nAb folded into the supergate):
    //   ( a + !nAbc), ( b + !nAbc), ( c + !nAbc)   any leaf 0 => nAbc 0
    //   (!a + !b + !c + nAbc)                      all leaves 1 => nAbc 1
    EXPECT_EQ( sat_solver_nclauses(p->pSat), 4 );

    // collecting for nAb adds exactly one new variable; leaves keep theirs
    Dch_CnfNodeAddToSolver( p, nAb );
    EXPECT_EQ( p->nSatVars, 7 );
    EXPECT_EQ( Dch_ObjSatNum(p, nAb), 6 );
    EXPECT_EQ( Dch_ObjSatNum(p, a), 4 );
    EXPECT_EQ( Vec_PtrSize(p->vUsedNodes), 5 );
    // encodes nAb = a & b:
    //   ( a + !nAb), ( b + !nAb), (!a + !b + nAb)
    EXPECT_EQ( sat_solver_nclauses(p->pSat), 7 );

    // repeated collection is a no-op
    Dch_CnfNodeAddToSolver( p, nAbc );
    EXPECT_EQ( p->nSatVars, 7 );
    EXPECT_EQ( sat_solver_nclauses(p->pSat), 7 );

    // Dch_ManStop frees pAigFraig but not pAigTotal
    Dch_ManStop( p );
    Aig_ManStop( pAig );
}

/*!
  \brief Step 1b of the SAT usage in choice verification: variable collection
  across a complemented edge.

  Dch_CollectSuper_rec stops descending at a complemented edge, so the fold
  of a single-fanout AND cone ends there: the node behind the edge becomes a
  frontier node with its own variable and clauses. The complement is absorbed
  into the leaf literal polarity (Dch_AddClausesSuper), keeping the two-
  direction n+1 encoding per supergate.
*/
TEST(DchTest, ComplementedEdgeBlocksSupergateFolding) {
    // tiny AIG:  nAb = a & b,  pC = !nAb & c
    Aig_Man_t * pAig = Aig_ManStart( 8 );
    Aig_Obj_t * a    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * b    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * c    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * nAb  = Aig_And( pAig, a, b );
    Aig_Obj_t * pC   = Aig_And( pAig, Aig_Not(nAb), c );

    Dch_Pars_t Pars, * pPars = &Pars;
    Dch_ManSetDefaultParams( pPars );
    pPars->fPolarFlip = 0;

    Dch_Man_t * p = Dch_ManCreate( pAig, pPars );
    p->pAigFraig = Aig_ManStart( 4 );
    Dch_ManSatSolverRecycle( p );
    sat_solver_simplify( p->pSat );

    Dch_CnfNodeAddToSolver( p, pC );

    // the complemented edge stops the fold: pC's supergate is {c, !nAb},
    // and nAb gets its own variable (unlike the pure-AND cone of the
    // PrepareSatCallCollectsVars test)
    EXPECT_EQ( p->nSatVars, 7 );  // const1 + { pC, c, nAb, a, b }
    EXPECT_EQ( Dch_ObjSatNum(p, pC),  2 );
    EXPECT_EQ( Dch_ObjSatNum(p, c),   3 );
    EXPECT_EQ( Dch_ObjSatNum(p, nAb), 4 );
    EXPECT_EQ( Dch_ObjSatNum(p, a),   5 );
    EXPECT_EQ( Dch_ObjSatNum(p, b),   6 );
    EXPECT_EQ( Vec_PtrSize(p->vUsedNodes), 5 );

    // pC's super (leaf !nAb): (c + !pC), (!nAb + !pC), (!c + nAb + pC)
    // nAb's super:            (a + !nAb), (b + !nAb), (!a + !b + nAb)
    EXPECT_EQ( sat_solver_nclauses(p->pSat), 6 );

    // probe the complemented-literal clauses through the encoding
    // (learned clauses may be added by these solves; counts asserted above)
    // nAb = 1 forces pC = 0 through (!nAb + !pC)
    int pLits1[2] = { toLit(Dch_ObjSatNum(p, nAb)), toLit(Dch_ObjSatNum(p, pC)) };
    EXPECT_EQ( sat_solver_solve(p->pSat, pLits1, pLits1 + 2, 0, 0, 0, 0), l_False );
    // nAb = 0 & c = 1 forces pC = 1 through (!c + nAb + pC)
    int pLits2[3] = { lit_neg(toLit(Dch_ObjSatNum(p, nAb))), toLit(Dch_ObjSatNum(p, c)),
                      lit_neg(toLit(Dch_ObjSatNum(p, pC))) };
    EXPECT_EQ( sat_solver_solve(p->pSat, pLits2, pLits2 + 3, 0, 0, 0, 0), l_False );
    // the encoding is not over-constrained: nAb = 0 & c = 1 & pC = 1 is satisfiable
    int pLits3[3] = { lit_neg(toLit(Dch_ObjSatNum(p, nAb))), toLit(Dch_ObjSatNum(p, c)),
                      toLit(Dch_ObjSatNum(p, pC)) };
    EXPECT_EQ( sat_solver_solve(p->pSat, pLits3, pLits3 + 3, 0, 0, 0, 0), l_True );

    // Dch_ManStop frees pAigFraig but not pAigTotal
    Dch_ManStop( p );
    Aig_ManStop( pAig );
}

ABC_NAMESPACE_IMPL_END
