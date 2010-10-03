/* mkrnd.c -- 
 * Created: Wed Sep 25 09:55:23 1996 by faith@cs.unc.edu
 * Revised: Wed Sep 25 09:59:02 1996 by faith@cs.unc.edu
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
 * $Id: mkrnd.c,v 1.1 1996/09/25 14:00:56 faith Exp $
 * 
 */

#include "maaP.h"

int main( int argc, char **argv )
{
   int count;
   int i;
   
   if (argc != 2) {
      fprintf( stderr, "Usage: mkrnd <count>\n" );
      exit( 1 );
   }

   count = atoi( argv[1] );
   for (i = 0; i < count; i++) {
      printf( "%f, ", (double)random()/(double)INT_MAX);
   }
   printf( "\n" );
   return 0;
}
