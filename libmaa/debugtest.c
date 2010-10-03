/* debugtest.c -- 
 * Created: Sun Dec 25 18:57:38 1994 by faith@cs.unc.edu
 * Revised: Mon Sep 23 16:23:50 1996 by faith@cs.unc.edu
 * Copyright 1994, 1996 Rickard E. Faith (faith@cs.unc.edu)
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
 * $Id: debugtest.c,v 1.5 1996/09/23 23:20:36 faith Exp $
 */

#include "maaP.h"
#include <errno.h>

#define DBG_VERBOSE 0x00000001
#define DBG_TEST    0x00000002

int main( int argc, char **argv )
{
   int c;

   maa_init( argv[0] );

   dbg_register( DBG_VERBOSE, "verbose" );
   dbg_register( DBG_TEST, "test" );

   while ((c = getopt( argc, argv, "d:" )) != -1) {
      switch (c) {
      case 'd':
	 dbg_set( optarg );
	 break;
      default:
	 fprintf( stderr, "Usage: debugtest [-dverbose] [-dtest]\n" );
	 break;
      }
   }

   if (dbg_test( DBG_VERBOSE )) printf( "Verbose set\n" );
   else                         printf( "Verbose not set\n" );
   if (dbg_test( DBG_TEST )) printf( "Test set\n" );
   else                      printf( "Test not set\n" );

   return 0;
}
