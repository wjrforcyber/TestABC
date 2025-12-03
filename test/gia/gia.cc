#include "gtest/gtest.h"

#include "aig/gia/gia.h"

ABC_NAMESPACE_IMPL_START

TEST(GiaTest, CanAllocateGiaManager) {
  Gia_Man_t* aig_manager =  Gia_ManStart(100);

  EXPECT_TRUE(aig_manager != nullptr);
  Gia_ManStop(aig_manager);
}

TEST(GiaTest, CanAddACi) {
  Gia_Man_t* aig_manager =  Gia_ManStart(100);
  Gia_ManAppendCi(aig_manager);

  EXPECT_EQ(Gia_ManCiNum(aig_manager), 1);
  Gia_ManStop(aig_manager);
}

TEST(GiaTest, CanAddACo) {
  Gia_Man_t* aig_manager =  Gia_ManStart(100);
  int input1 = Gia_ManAppendCi(aig_manager);
  Gia_ManAppendCo(aig_manager, input1);

  EXPECT_EQ(Gia_ManCiNum(aig_manager), 1);
  EXPECT_EQ(Gia_ManCoNum(aig_manager), 1);
  Gia_ManStop(aig_manager);
}

TEST(GiaTest, CanAddAnAndGate) {
  Gia_Man_t* aig_manager =  Gia_ManStart(100);

  int input1 = Gia_ManAppendCi(aig_manager);
  int input2 = Gia_ManAppendCi(aig_manager);

  int and_output = Gia_ManAppendAnd(aig_manager, input1, input2);
  Gia_ManAppendCo(aig_manager, and_output);

  Vec_Wrd_t* stimulus = Vec_WrdAlloc(2);
  Vec_WrdPush(stimulus, /*A*/1);
  Vec_WrdPush(stimulus, /*B*/1);
  Vec_Wrd_t* output = Gia_ManSimPatSimOut(aig_manager, stimulus, /*fouts*/1);

  EXPECT_EQ(Gia_ManCiNum(aig_manager), 2);
  EXPECT_EQ(Gia_ManCoNum(aig_manager), 1);
  // A = 1, B = 1 -> A & B == 1
  EXPECT_EQ(Vec_WrdGetEntry(output, 0), 1);
  Vec_WrdFree(output);
  Gia_ManStop(aig_manager);
}


/*!
  \brief Standard process on collecting fanout information.
*/
TEST(GiaTest, GiaCollectFanoutInfo) {
    Gia_Obj_t * pObj;
    int i;
    Gia_Man_t * giaMan = Gia_ManStart(100);
    int input1 = Gia_ManAppendCi(giaMan);
    int input2 = Gia_ManAppendCi(giaMan);
    int input3 = Gia_ManAppendCi(giaMan);

    int aOut0 = Gia_ManAppendAnd(giaMan, input1, input2);
    int aOut1 = Gia_ManAppendAnd(giaMan, aOut0, input3);
    Gia_ManAppendCo(giaMan, aOut1);

    EXPECT_TRUE(giaMan->vFanoutNums == NULL);
    Gia_ManStaticFanoutStart(giaMan);
    EXPECT_TRUE(giaMan->vFanoutNums != NULL);
    Gia_ManForEachAnd(giaMan, pObj, i)
    {
        EXPECT_TRUE(Gia_ObjFanoutNum(giaMan, pObj) >= 1);
    }
    Gia_ManStaticFanoutStop(giaMan);
    EXPECT_TRUE(giaMan->vFanoutNums == NULL);
    Gia_ManStop(giaMan);
}

ABC_NAMESPACE_IMPL_END
