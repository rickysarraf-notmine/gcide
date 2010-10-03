/* bittest.c -- Test the bit functions, especially the counter
 * Created: Mon Oct  2 10:10:57 1995 by faith@cs.unc.edu
 * Revised: Sun Nov 19 13:30:30 1995 by faith@cs.unc.edu
 * Copyright 1995 Rickard E. Faith (faith@cs.unc.edu)
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
 * $Id: bittest.c,v 1.2 1995/11/19 19:28:33 faith Exp $
 * 
 */

#include "maaP.h"

int main( int argc, char **argv )
{
   unsigned long t = 0;

   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );

   bit_set( &t, 0 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 1 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 2 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 3 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 4 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 5 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 6 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 7 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 31 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 30 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 29 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 28 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_set( &t, 27 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );

   printf( "\n" );
   bit_clr( &t, 0 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_clr( &t, 31 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_clr( &t, 1 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_clr( &t, 4 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   bit_clr( &t, 7 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );
   
   printf( "\n" );
   bit_clr( &t, 16 );
   printf( "0x%08lx has %d bits set\n", t, bit_cnt( &t ) );

   return 0;
}
