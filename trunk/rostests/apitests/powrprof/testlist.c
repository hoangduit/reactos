/* Automatically generated file; DO NOT EDIT!! */

/* stdarg.h is needed for Winelib */
//#include <stdarg.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include "windef.h"
//#include "winbase.h"

struct test
{
    const char *name;
    void (*func)(void);
};

extern void func_power(void);

const struct test winetest_testlist[] =
{
	{ "power", func_power },
	{ 0, 0 }
};

#define WINETEST_WANT_MAIN
#include <wine/test.h>
