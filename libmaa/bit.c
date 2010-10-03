/* bit.c -- 
 * Created: Thu Sep 28 18:09:30 1995 by faith@cs.unc.edu
 * Revised: Mon Sep 23 16:23:51 1996 by faith@cs.unc.edu
 * Copyright 1995, 1996 Rickard E. Faith (faith@cs.unc.edu)
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * $Id: bit.c,v 1.5 1996/09/23 23:20:34 faith Exp $
 *
 * \section{Bit manipulation routines}
 *
 * \intro These are a set of simple routine to manipulate bits in an
 * integer.
 *
 */

#include "maaP.h"

/* \doc Set |bit| in |flags|. */

__inline__ void bit_set( unsigned long *flags, int bit )
{
   *flags |= (1 << bit);
}

/* \doc Clear |bit| in |flags|. */

__inline__ void bit_clr( unsigned long *flags, int bit )
{
   *flags &= ~(1 << bit);
}

/* \doc Test |bit| in |flags|, returning non-zero if the bit is set and
   zero if the bit is clear. */

__inline__ int bit_tst( unsigned long *flags, int bit )
{
   return (*flags & (1 << bit));
}

/* \doc Return a count of the number of bits set in |flags|. */

__inline__ int bit_cnt( unsigned long *flags )
{
   unsigned long x = *flags;

#if SIZEOF_LONG == 4
   x = (x >> 1  & 0x55555555) + (x & 0x55555555);
   x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
   x = ((x >> 4) + x) & 0x0f0f0f0f;
   x = ((x >> 8) + x);
   return (x + (x >> 16)) & 0xff;
#else
   err_internal( __FUNCTION__,
		 "Assumes 32-bit longs, not %d-bit longs\n",
		 SIZEOF_LONG * 8 );
#endif
}
