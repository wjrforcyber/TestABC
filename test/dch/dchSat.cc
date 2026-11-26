#include "gtest/gtest.h"

#include "proof/dch/dchInt.h"
#include "misc/util/abc_namespaces.h"

// simulation callbacks defined in dchSim.c but not declared in dchInt.h
ABC_NAMESPACE_HEADER_START
extern unsigned Dch_NodeHash( void * p, Aig_Obj_t * pObj );
extern int      Dch_NodeIsConst( void * p, Aig_Obj_t * pObj );
extern int      Dch_NodesAreEqual( void * p, Aig_Obj_t * pObj0, Aig_Obj_t * pObj1 );
extern int      Dch_NodeIsConstCex( void * p, Aig_Obj_t * pObj );
extern int      Dch_NodesAreEqualCex( void * p, Aig_Obj_t * pObj0, Aig_Obj_t * pObj1 );
ABC_NAMESPACE_HEADER_END

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

/*!
  \brief Step 2 of the SAT usage in choice verification: the assumption miter
  on a provably equivalent pair.

  With fPolarFlip = 0 and fPhase = 0, Dch_NodesAreEquiv solves assumptions
  (A=1,B=0) then (A=0,B=1); both UNSAT proves the equivalence and each solve
  records it as a learned clause. The pair is functionally equal but
  structurally distinct (strash keeps a*(a*b)), so the solver sees two
  independent Tseitin views over the same leaves. The solver is not created
  up front: the first Dch_NodesAreEquiv call does the internal recycle.
*/
TEST(DchTest, NodesAreEquivProvesEquivalentPair) {
    // tiny AIG:  A = a & b,  B = a & (a & b)
    Aig_Man_t * pAig = Aig_ManStart( 8 );
    Aig_Obj_t * a    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * b    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * pA   = Aig_And( pAig, a, b );
    Aig_Obj_t * pB   = Aig_And( pAig, a, pA );

    Dch_Pars_t Pars, * pPars = &Pars;
    Dch_ManSetDefaultParams( pPars );
    pPars->fPolarFlip = 0;

    Dch_Man_t * p = Dch_ManCreate( pAig, pPars );
    p->pAigFraig = Aig_ManStart( 4 );

    // the first SAT call in a manager creates the solver internally
    ASSERT_EQ( p->pSat, nullptr );
    int RetValue = Dch_NodesAreEquiv( p, pA, pB );
    ASSERT_NE( p->pSat, nullptr );

    // both assumption solves are UNSAT
    EXPECT_EQ( RetValue, 1 );
    EXPECT_EQ( p->nSatCalls, 1 );
    EXPECT_EQ( p->nSatCallsUnsat, 2 );
    EXPECT_EQ( p->nSatCallsSat, 0 );

    // pOld (pA) is encoded first, then pB; pA is folded into pB's supergate
    EXPECT_EQ( p->nSatVars, 6 );  // const1 + { pA, a, b, pB }
    EXPECT_EQ( Dch_ObjSatNum(p, pA), 2 );
    EXPECT_EQ( Dch_ObjSatNum(p, a),  3 );
    EXPECT_EQ( Dch_ObjSatNum(p, b),  4 );
    EXPECT_EQ( Dch_ObjSatNum(p, pB), 5 );

    // 6 encoding clauses + 2 learned clauses (conflicts under assumptions
    // take the analyze_final path, so the solver adds nothing internally;
    // canceluntil(0) before the explicit addclause keeps vars unassigned)
    EXPECT_EQ( sat_solver_nclauses(p->pSat), 8 );

    // no memoization at this level: the second call re-solves and re-adds
    // the same learned clauses as duplicates (bsat does not dedup against
    // the existing DB). The first solve now conflicts in the assumption
    // push itself, through the learned clause (!pA + pB)
    RetValue = Dch_NodesAreEquiv( p, pA, pB );
    EXPECT_EQ( RetValue, 1 );
    EXPECT_EQ( p->nSatCalls, 2 );
    EXPECT_EQ( p->nSatCallsUnsat, 4 );
    EXPECT_EQ( sat_solver_nclauses(p->pSat), 10 );

    // Dch_ManStop frees pAigFraig but not pAigTotal
    Dch_ManStop( p );
    Aig_ManStop( pAig );
}

/*!
  \brief Step 2 of the SAT usage in choice verification: the assumption miter
  on a pair with a counterexample.

  The first solve (A=1, C=0) is satisfiable (a=1, b=1), so the equivalence
  is disproved after one solve, nothing is learned on the SAT path, and the
  solver keeps the counterexample model -- the input pattern that
  Dch_ManResimulateCex consumes in the next step.
*/
TEST(DchTest, NodesAreEquivFindsCounterexample) {
    // tiny AIG:  A = a & b,  C = a & !b
    Aig_Man_t * pAig = Aig_ManStart( 8 );
    Aig_Obj_t * a    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * b    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * pA   = Aig_And( pAig, a, b );
    Aig_Obj_t * pC   = Aig_And( pAig, a, Aig_Not(b) );

    Dch_Pars_t Pars, * pPars = &Pars;
    Dch_ManSetDefaultParams( pPars );
    pPars->fPolarFlip = 0;

    Dch_Man_t * p = Dch_ManCreate( pAig, pPars );
    p->pAigFraig = Aig_ManStart( 4 );

    int RetValue = Dch_NodesAreEquiv( p, pA, pC );

    // the first assumption solve is SAT: not equivalent
    EXPECT_EQ( RetValue, 0 );
    EXPECT_EQ( p->nSatCalls, 1 );
    EXPECT_EQ( p->nSatCallsSat, 1 );
    EXPECT_EQ( p->nSatCallsUnsat, 0 );

    // nothing learned on the SAT path
    EXPECT_EQ( p->nSatVars, 6 );  // const1 + { pA, a, b, pC }
    EXPECT_EQ( Dch_ObjSatNum(p, pA), 2 );
    EXPECT_EQ( Dch_ObjSatNum(p, pC), 5 );
    EXPECT_EQ( sat_solver_nclauses(p->pSat), 6 );

    // the counterexample is still in the model: a=1, b=1 gives A=1, C=0
    EXPECT_EQ( sat_solver_var_value(p->pSat, Dch_ObjSatNum(p, a)), 1 );
    EXPECT_EQ( sat_solver_var_value(p->pSat, Dch_ObjSatNum(p, b)), 1 );

    // Dch_ManStop frees pAigFraig but not pAigTotal
    Dch_ManStop( p );
    Aig_ManStop( pAig );
}

/*!
  \brief Step 3 of the SAT usage in choice verification: counterexample
  resimulation splits the false candidate class.

  Production reaches Dch_ManResimulateCex when Dch_NodesAreEquiv returns 0:
  random simulation believed a pair equivalent, SAT disproved it, and the
  counterexample input pattern is re-simulated so the class manager can
  split the class. The candidate class here is forged through hand-written
  simulation words (the one shortcut -- production obtains it from
  Dch_PerformRandomSimulation); everything else is the real machinery,
  including the callback swap to the single-pattern comparators that
  production makes after candidate building (dchSim.c).
*/
TEST(DchTest, ResimulateCexSplitsFalseCandidate) {
    // AIG:  a, b, c CIs;  nAb = a & b;  nAbn = a & !b;  filler chain c·f
    // padded to 16 objects so the class hash table gets 5 buckets
    Aig_Obj_t * pObj;
    Aig_Man_t * pAig = Aig_ManStart( 16 );
    Aig_Obj_t * a    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * b    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * c    = Aig_ObjCreateCi( pAig );
    Aig_Obj_t * nAb  = Aig_And( pAig, a, b );
    Aig_Obj_t * nAbn = Aig_And( pAig, a, Aig_Not(b) );
    Aig_Obj_t * pFill = Aig_And( pAig, c, a );  // a new node; Aig_And(c, c) would fold to c
    int i;
    for ( i = 0; i < 9; i++ )
        pFill = Aig_And( pAig, c, pFill );  // Ids 6..15, never touching nAb/nAbn

    // forge the random-simulation result: nAb and nAbn carry the same word,
    // so Dch_ClassesPrepare buckets them into one candidate class; the
    // fillers carry the all-zero word, so they become const-1 candidates
    // and are skipped by the hashing entirely
    Vec_Ptr_t * vSims = Vec_PtrAllocSimInfo( Aig_ManObjNumMax(pAig), 1 );
    unsigned * pSim;
    pSim = (unsigned *)Vec_PtrEntry( vSims, 0 );        pSim[0] = (unsigned)-1; // const1
    pSim = (unsigned *)Vec_PtrEntry( vSims, a->Id );    pSim[0] = 1;
    pSim = (unsigned *)Vec_PtrEntry( vSims, b->Id );    pSim[0] = 2;
    pSim = (unsigned *)Vec_PtrEntry( vSims, c->Id );    pSim[0] = 3;
    pSim = (unsigned *)Vec_PtrEntry( vSims, nAb->Id );  pSim[0] = 5; // nonzero:
    pSim = (unsigned *)Vec_PtrEntry( vSims, nAbn->Id ); pSim[0] = 5; // word 0 would make them const-1 candidates
    for ( i = 6; i < 16; i++ ) {
        pSim = (unsigned *)Vec_PtrEntry( vSims, i );
        pSim[0] = 0;
    }

    Dch_Cla_t * ppClasses = Dch_ClassesStart( pAig );
    Dch_ClassesSetData( ppClasses, vSims, Dch_NodeHash, Dch_NodeIsConst, Dch_NodesAreEqual );
    Dch_ClassesPrepare( ppClasses, 0, 0 );
    // production swaps to the single-pattern comparators before sweeping
    Dch_ClassesSetData( ppClasses, NULL, NULL, Dch_NodeIsConstCex, Dch_NodesAreEqualCex );
    Vec_PtrFree( vSims );

    // the forged candidate class: nAb heads, nAbn is its single member
    EXPECT_EQ( Aig_ObjRepr( pAig, nAbn ), nAb );
    EXPECT_EQ( Aig_ObjRepr( pAig, nAb ), nullptr );

    Dch_Pars_t Pars, * pPars = &Pars;
    Dch_ManSetDefaultParams( pPars );
    pPars->fPolarFlip = 0;

    Dch_Man_t * p = Dch_ManCreate( pAig, pPars );
    p->ppClasses = ppClasses;

    // mirror the AIG into the fraig AIG the way Dch_ManSweep does
    p->pAigFraig = Aig_ManStart( Aig_ManObjNumMax( pAig ) );
    Aig_ManConst1( pAig )->pData = Aig_ManConst1( p->pAigFraig );
    Aig_ManForEachCi( pAig, pObj, i )
        pObj->pData = Aig_ObjCreateCi( p->pAigFraig );
    Aig_ManForEachNode( pAig, pObj, i )
        pObj->pData = Aig_And( p->pAigFraig,
            Aig_NotCond( (Aig_Obj_t *)Aig_ObjFanin0(pObj)->pData, Aig_ObjFaninC0(pObj) ),
            Aig_NotCond( (Aig_Obj_t *)Aig_ObjFanin1(pObj)->pData, Aig_ObjFaninC1(pObj) ) );

    // SAT disproves the forged equivalence; the CEX stays in the model
    int RetValue = Dch_NodesAreEquiv( p, Dch_ObjFraig(nAb), Dch_ObjFraig(nAbn) );
    EXPECT_EQ( RetValue, 0 );
    EXPECT_EQ( p->nSatCalls, 1 );
    EXPECT_EQ( p->nSatCallsSat, 1 );
    EXPECT_EQ( p->nSatCallsUnsat, 0 );

    // resimulate: applies the CEX input pattern to the solved cones and
    // lets the class manager refine the class of nAb
    Dch_ManResimulateCex( p, nAbn, nAb );

    // the false equivalence is gone: both nodes are singletons now
    EXPECT_EQ( Aig_ObjRepr( pAig, nAbn ), nullptr );
    EXPECT_EQ( Aig_ObjRepr( pAig, nAb ), nullptr );
    // the counterexample pattern propagated through the solved cones:
    // a=1, b=1 gives nAb=1 but nAbn=0, which is what forced the split
    EXPECT_EQ( a->fMarkB, 1 );
    EXPECT_EQ( b->fMarkB, 1 );
    EXPECT_EQ( nAb->fMarkB, 1 );
    EXPECT_EQ( nAbn->fMarkB, 0 );
    // cone stats: the two AND nodes of the solved cones carry SAT vars
    // (CIs return before the counter in Dch_ManResimulateSolved_rec)
    EXPECT_EQ( p->nConeThis, 2 );
    EXPECT_EQ( p->nConeMax, 2 );

    // resimulation leaves fMarkB on the solved cones; Dch_ManSweep cleans
    // it at the end, and Aig_ManStop asserts clean marks
    Aig_ManCleanMarkB( pAig );

    // Dch_ManStop frees ppClasses (Dch_ClassesStop) and pAigFraig
    Dch_ManStop( p );
    Aig_ManStop( pAig );
}

ABC_NAMESPACE_IMPL_END
