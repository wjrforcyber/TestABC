#include "gtest/gtest.h"

#include "base/abc/abc.h"
#include "misc/util/abc_global.h"
#include "misc/util/abc_namespaces.h"
#include "misc/vec/vecPtr.h"

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

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif


ABC_NAMESPACE_IMPL_START
/*!
 \brief Duplicate an AIG network
 Note that you might wonder why abc frame is needed here. The reason is the network checking process inside the duplication method or at the end checks for the flag in the frame by design. So even it looks like useless(it is as a matter of fact), you should initilize it.
*/

TEST(AigTest, DuplicateAig) {
    Abc_Frame_t * pAbc;
    // start the ABC framework
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();

    Abc_Ntk_t * pNew;
    Abc_Ntk_t * pNtk = Abc_NtkAlloc(ABC_NTK_STRASH, ABC_FUNC_AIG, 1);
    Vec_Ptr_t * vNodes = Vec_PtrAlloc(100);
    int nInputs = 2;
    int nLatches = 0;
    int nAnds = 1;
    int nOutputs = 1;
    int i;
    Abc_Obj_t * pObj, * pNodeAttach;
    vNodes = Vec_PtrAlloc( 1 + nInputs + nLatches + nAnds );
    Vec_PtrPush( vNodes, Abc_ObjNot( Abc_AigConst1(pNtk) ) );

    // create the PIs
    for ( i = 0; i < nInputs; i++ )
    {
        pObj = Abc_NtkCreatePi(pNtk);    
        Vec_PtrPush( vNodes, pObj );
    }
    // create the POs
    for ( i = 0; i < nOutputs; i++ )
    {
        pObj = Abc_NtkCreatePo(pNtk);   
    }
    //create ANDs
    Abc_Obj_t * pi00 = (Abc_Obj_t *)Vec_PtrEntry(vNodes, 1);
    Abc_Obj_t * pi01 = (Abc_Obj_t *)Vec_PtrEntry(vNodes, 2);
    Vec_PtrPush( vNodes, Abc_AigAnd((Abc_Aig_t *)pNtk->pManFunc, pi00, pi01) );

    pNodeAttach = Abc_ObjNotCond( (Abc_Obj_t *)Vec_PtrEntry(vNodes, 3), 1 );
    Abc_ObjAddFanin( Abc_NtkPo(pNtk, 0), pNodeAttach );

    pNew = Abc_NtkDup(pNtk);

    Vec_PtrFree(vNodes);
    Abc_NtkDelete(pNtk);
    EXPECT_TRUE( pNew->ntkType == ABC_NTK_STRASH );
    EXPECT_TRUE(pNew->ntkFunc == ABC_FUNC_AIG);
    EXPECT_TRUE( Abc_NtkPiNum(pNew) == nInputs );
    EXPECT_TRUE( Abc_NtkPoNum(pNew) == nOutputs );
    EXPECT_TRUE( Abc_NtkNodeNum(pNew) == nAnds );
    EXPECT_TRUE(Abc_NtkCheck(pNew) == 1);
    Abc_NtkDelete(pNew);
    Abc_Stop();
}

ABC_NAMESPACE_IMPL_END