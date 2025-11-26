#include "gtest/gtest.h"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "misc/extra/extra.h"


#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif

int Exa7_AddClause( FILE * pFile, int * pLits, int nLits );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif


ABC_NAMESPACE_IMPL_START


TEST(UtilTest, UtilTruthHexPrint) {
    unsigned tTruth = 8;
    unsigned * pTruth = &tTruth;
    int nVars = 2;
    char * Buffer = (char *)malloc(sizeof(char) * 100);
    Extra_PrintHexadecimalString( Buffer, (unsigned *)pTruth, nVars );
    EXPECT_TRUE(strcmp(Buffer, "8") == 0);

    memset(Buffer, 0, 100);

    tTruth = 15;
    pTruth = &tTruth;
    nVars = 2;
    Extra_PrintHexadecimalString( Buffer, (unsigned *)pTruth, nVars );
    EXPECT_TRUE(strcmp(Buffer, "f") == 0);

    memset(Buffer, 0, 100);

    tTruth = 255;
    pTruth = &tTruth;
    nVars = 3;
    Extra_PrintHexadecimalString( Buffer, (unsigned *)pTruth, nVars );
    EXPECT_TRUE(strcmp(Buffer, "ff") == 0);
    free(Buffer);
}


/*!
  \brief Clause interface, give example to show what it looks like, the lits are based on AIG syntax, variables are index based(roughly divided by 2, positive or negative decided by even/odd lit).
*/
TEST(UtilTest, UtilAddClauses) {
    char * buffer = NULL;
    FILE * mem_file;
    size_t size = 0;
    int lits[] = {2, 3, 4, 5};
    mem_file = open_memstream(&buffer, &size);
    if (mem_file == NULL) {
        perror("open_memstream failed");
    }
    Exa7_AddClause( mem_file, lits, 4 );
    fflush(mem_file);
    EXPECT_TRUE(strcmp(buffer, "1 -1 2 -2 0\n") == 0);
    fclose(mem_file);
    free(buffer);
}


ABC_NAMESPACE_IMPL_END