/* sltest.c -- 
 * Created: Mon Feb 19 08:57:34 1996 by faith@cs.unc.edu
 * Revised: Mon Sep 23 20:57:49 1996 by faith@cs.unc.edu
 * Copyright 1996 Rickard E. Faith (faith@cs.unc.edu)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 1, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: sltest.c,v 1.3 1996/09/24 01:06:14 faith Exp $
 * 
 */

#include "maaP.h"
#include <math.h>

#undef DUMP

static int compare( const void *datum1, const void *datum2 )
{
   long a = (long)datum1;
   long b = (long)datum2;

   if (a < b) return -1;
   if (a > b) return 1;
   return 0;
}

static int print( const void *datum )
{
   printf( "%d ", (int)datum );
   return 0;
}

static const void *key( const void *datum )
{
   return datum;
}

int main( int argc, char **argv )
{
   sl_List       sl;
   int           count;
   int           i;

   maa_init( argv[0] );
   
   if (argc == 1) {
      count = 10;
   } else if (argc != 2 ) {
      fprintf( stderr, "usage: sltest count\n" );
      return 1;
   } else {
      count = atoi( argv[1] );
   }

   printf( "Running test for count of %d\n", count );

   sl = sl_create( compare, key, NULL );
   
   for (i = 1; i < count; i++) {
      const void *datum = (void *)i;

      printf( "adding %d\n", i );
      sl_insert( sl, datum );
#ifdef DUMP
      _sl_dump( sl );
#endif
   }

   sl_iterate( sl, print );
   printf( "\n" );

   sl_delete( sl, (void *)5 );
   sl_iterate( sl, print );
   printf( "\n" );

   sl_insert( sl, (void *)0 );
   sl_iterate( sl, print );
   printf( "\n" );
   sl_insert( sl, (void *)66 );
   sl_iterate( sl, print );
   printf( "\n" );
   sl_insert( sl, (void *)100 );
   sl_iterate( sl, print );
   printf( "\n" );
   sl_insert( sl, (void *)-1 );
   sl_iterate( sl, print );
   printf( "\n" );
   sl_insert( sl, (void *)5 );
   sl_iterate( sl, print );
   printf( "\n" );
   sl_insert( sl, (void *)67 );
   sl_iterate( sl, print );
   printf( "\n" );
   sl_insert( sl, (void *)68 );
   sl_iterate( sl,print );
   printf( "\n" );
   sl_insert( sl, (void *)65 );
   sl_iterate( sl, print );
   printf( "\n" );
   
   sl_destroy( sl );

   return 0;
}

