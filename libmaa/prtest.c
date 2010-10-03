/* prtest.c -- 
 * Created: Fri Jan 12 14:18:32 1996 by r.faith@ieee.org
 * Revised: Wed Oct  2 20:06:41 1996 by faith@cs.unc.edu
 * Copyright 1996 Rickard E. Faith (r.faith@ieee.org)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
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
 * $Id: prtest.c,v 1.3 1996/10/03 01:36:07 faith Exp $
 * 
 */

#include "maaP.h"

int main( int argc, char **argv )
{
   int  prev = 0;
   int  next; 
   char buf[BUFSIZ];
   int  c;
   int  i;

   maa_init( argv[0] );

   while ((c = getopt( argc, argv, "D" )) != EOF)
      switch (c) {
      case 'D': dbg_set( ".pr" ); break;
      }

   if (argc-optind == 0) {
      pr_open( "echo foo", PR_USE_STDIN | PR_CREATE_STDOUT,
	       &prev, &next, NULL );
   } else {
      for (i = optind; i < argc; i++) {
	 pr_open( argv[i], PR_USE_STDIN | PR_CREATE_STDOUT,
		  &prev, &next, NULL );
	 prev = next;
      }
   }
   
   while ((read( next, buf, BUFSIZ )))
      printf( "Got: \"%s\"\n", buf );
   printf( "status = 0x%02x\n", pr_close( next ) );
   
#if 0
   printf( "%s\n", maa_version() );

   pr_open( "echo foobar", PR_USE_STDIN | PR_CREATE_STDOUT,
	    NULL, &instr, NULL );
   
   pr_open( "cat", PR_USE_STDIN | PR_CREATE_STDOUT,
	    &instr, &outstr, NULL );
   fgets( buf, sizeof( buf ), outstr );
   printf( "Got \"%s\"\n", buf );

   printf( "status = 0x%02x\n", pr_close( outstr ) );

   pr_open( "cat", PR_CREATE_STDIN | PR_CREATE_STDOUT,
	    &instr, &outstr, NULL );
   fprintf( instr, "this is a test\n" );
   fflush( instr );
   fgets( buf, sizeof( buf ), outstr );
   printf( "Got \"%s\"\n", buf );
   printf( "status = 0x%02x\n", pr_close( instr ) );
   printf( "status = 0x%02x\n", pr_close( outstr ) );
#endif
   
   return 0;
}
