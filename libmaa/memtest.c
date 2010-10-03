/* memtest.c -- 
 * Created: Sat Jul  1 22:42:09 1995 by r.faith@ieee.org
 * Revised: Fri Aug 25 01:16:43 1995 by faith@cs.unc.edu
 * Copyright 1995 Rickard E. Faith (r.faith@ieee.org)
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
 * $Id: memtest.c,v 1.2 1995/08/25 05:20:51 faith Exp $
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef __sparc__
extern int printf( const char *, ... );
#endif

int main( void )
{
   char *buf;
   int  i;

#ifdef KH_GNU_MALLOC
   mcheck(0);
#endif

   buf = malloc( 6 );

   for (i = 0; i < 7; i++) {
      printf( "i = %d\n", i );
      buf[i] = i;
   }

   for (i = -1; i < 7; i++) printf( "buf[%d] = %d\n", i, buf[i] );

   free( buf );

   return 0;
}
