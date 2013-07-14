/**************************************************************************
 * 
 * Copyright 2009 VMware, Inc.
 * Copyright 2008 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

//#include <windows.h>
#include <stdarg.h>
#include <windef.h>
#include <wingdi.h>

#define WGL_WGLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/wglext.h>
#include "util/u_debug.h"

/* A dummy implementation of this extension.
 *
 * Required as some applications retrieve and call these functions
 * regardless of the fact that we don't advertise the extension and
 * further more the results of wglGetProcAddress are NULL.
 */
WINGDIAPI BOOL APIENTRY
wglSwapIntervalEXT(int interval)
{
   (void) interval;
   debug_printf("%s: %d\n", __FUNCTION__, interval);
   return TRUE;
}

WINGDIAPI int APIENTRY
wglGetSwapIntervalEXT(void)
{
   return 0;
}

