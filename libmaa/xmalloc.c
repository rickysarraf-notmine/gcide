/* xmalloc.c -- Error-checking malloc
 * Created: Sun Nov  6 18:14:10 1994 by faith@cs.unc.edu
 * Revised: Mon Sep 23 16:23:45 1996 by faith@cs.unc.edu
 * Copyright 1994, 1995, 1996 Rickard E. Faith (faith@cs.unc.edu)
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
 * $Id: xmalloc.c,v 1.6 1996/09/23 23:20:47 faith Exp $
 */

#include "maaP.h"

#if STDC_HEADERS
#include <stdlib.h>
#else
void *malloc();
void *calloc();
void *realloc();
void free();
#endif

#ifndef DMALLOC_FUNC_CHECK

__inline__ void *xmalloc( unsigned int size )
{
   void *pt = malloc( size );

   if (!pt) err_fatal( __FUNCTION__,
		       "Out of memory while allocating %lu bytes\n",
		       (unsigned long)size );
   return pt;
}

__inline__ void *xcalloc( unsigned int num, unsigned int size )
{
   void *pt = calloc( num, size );

   if (!pt) err_fatal( __FUNCTION__,
		       "Out of memory while allocating %lu X %lu byte block\n",
		       (unsigned long)num, (unsigned long)size );
   return pt;
}

__inline__ void *xrealloc( void *pt, unsigned int size )
{
   void *new = realloc( pt, size );

   if (!new) err_fatal( __FUNCTION__,
			"Out of memory while reallocating block at %p to"
			" %lu bytes\n",
			pt,
			(unsigned long)size );
   return new;
}

__inline__ void xfree( void *pt )
{
   if (pt) free( pt );
   else err_fatal( __FUNCTION__, "Attempt to free null pointer\n" );
}

__inline__ char *xstrdup( const char *s )
{
   char *pt = strdup( s );

   if (!pt) err_fatal( __FUNCTION__,
		       "Out of memory while duplicating string\n" );

   return pt;
}

#endif
