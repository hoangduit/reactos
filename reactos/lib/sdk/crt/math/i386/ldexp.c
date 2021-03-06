/* Math functions for i387.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by John C. Bowman <bowman@ipp-garching.mpg.de>, 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <precomp.h>
#include <math.h>
#include <float.h>

double ldexp (double value, int exp)
{
    register double result;
#ifndef __GNUC__
    register double __dy = (double)exp;
#endif

    /* Check for value correctness
     * and set errno if required
     */
    if (_isnan(value))
    {
        errno = EDOM;
    }

#ifdef __GNUC__
    asm ("fscale"
         : "=t" (result)
         : "0" (value), "u" ((double)exp)
         : "1");
#else /* !__GNUC__ */
    __asm
    {
        fld __dy
        fld value
        fscale
        fstp result
    }
#endif /* !__GNUC__ */
    return result;
}

