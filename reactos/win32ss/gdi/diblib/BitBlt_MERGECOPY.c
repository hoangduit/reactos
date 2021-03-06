
#include "DibLib.h"

#define __USES_SOURCE 1
#define __USES_PATTERN 1
#define __USES_DEST 0
#define __USES_MASK 0

#define _DibDoRop(pBltData, M, D, S, P) ROP_MERGECOPY(D,S,P)

#define __FUNCTIONNAME BitBlt_MERGECOPY
#include "DibLib_AllSrcBPP.h"

#undef __FUNCTIONNAME
#define __FUNCTIONNAME BitBlt_MERGECOPY_Solid
#define __USES_SOLID_BRUSH 1
#include "DibLib_AllSrcBPP.h"

VOID
FASTCALL
Dib_BitBlt_MERGECOPY(PBLTDATA pBltData)
{
    /* Check for solid brush */
    if (pBltData->ulSolidColor != 0xFFFFFFFF)
    {
        /* Use the solid version of PATCOPY! */
        gapfnBitBlt_MERGECOPY_Solid[pBltData->siDst.iFormat][pBltData->siSrc.iFormat](pBltData);
    }
    else
    {
        /* Use the pattern version */
        gapfnBitBlt_MERGECOPY[pBltData->siDst.iFormat][pBltData->siSrc.iFormat](pBltData);
    }
}

