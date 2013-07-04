#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define STANDALONE
#include "wine/test.h"

extern void func_debugging(void);
extern void func_metahost(void);
extern void func_mscoree(void);

const struct test winetest_testlist[] =
{
    { "debugging", func_debugging },
    { "metahost", func_metahost },
    { "mscoree", func_mscoree },
    { 0, 0 }
};
