/* hashtest.c -- Test program for Khepera hash table routines
 * Created: Sun Nov  6 18:55:23 1994 by faith@cs.unc.edu
 * Revised: Wed Sep 25 10:16:27 1996 by faith@cs.unc.edu
 * Copyright 1994, 1995 Rickard E. Faith (faith@cs.unc.edu)
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
 * $Id: hashtest.c,v 1.5 1996/09/25 14:20:51 faith Exp $
 */

#include "maaP.h"

extern void init_rand( void );
extern int get_rand( int ll, int ul );

static int iterator( const void *key, const void *datum )
{
   printf( "%s: %s\n", (const char *)key, (const char *)datum );
   return 0;
}

static int freer( const void *key, const void *datum )
{
   xfree( (void *)datum );
   xfree( (void *)key );
   return 0;
}

static int free_data( const void *key, const void *datum )
{
   xfree( (void *)datum );
   return 0;
}

int main( int argc, char **argv )
{
   hsh_HashTable t;
   int           i;
   int           j;
   int           count;

   if (argc == 1) {
      count = 100;
   } else if (argc != 2 ) {
      fprintf( stderr, "usage: hashtest count\n" );
      return 1;
   } else {
      count = atoi( argv[1] );
   }

   printf( "Running test for count of %d\n", count );

				/* Test sequential keys */
   t = hsh_create( NULL, NULL );
   
   for (i = 0; i < count; i++) {
      char *key   = xmalloc( 20 );
      char *datum = xmalloc( 20 );

      sprintf( key, "key%d", i );
      sprintf( datum, "datum%d", i );
      hsh_insert( t, key, datum );
   }

   for (i = count; i >= 0; i--) {
      char        key[100];
      char        datum[100];
      const char *pt;

      sprintf( key, "key%d", i );
      sprintf( datum, "datum%d", i );
      pt = hsh_retrieve( t, key );

      if (!pt || strcmp( pt, datum )) {
	 printf( "Expected \"%s\", got \"%s\"\n", datum, pt ? pt : "(null)" );
      }
   }

   if (count <= 200) hsh_iterate( t, iterator );
   
   hsh_print_stats( t, stdout );

   hsh_iterate( t, freer );
   hsh_destroy( t );



				/* Test random keys */
   t = hsh_create( NULL, NULL );

   init_rand();
   for (i = 0; i < count; i++) {
      int  len = get_rand( 2, 32 );
      char *key   = xmalloc( len + 1 );
      char *datum = xmalloc( 20 );

      for (j = 0; j < len; j++) key[j] = get_rand( 32, 128 );
      key[ len ] = '\0';
      sprintf( datum, "datum%d", i );
      hsh_insert( t, key, datum );
   }
   
   init_rand();
   for (i = 0; i < count; i++) {
      int         len = get_rand( 2, 32 );
      char       *key = xmalloc( len + 1 );
      char        datum[100];
      const char *pt;

      for (j = 0; j < len; j++) key[j] = get_rand( 32, 128 );
      key[ len ] = '\0';
      sprintf( datum, "datum%d", i );
      pt = hsh_retrieve( t, key );

      if (!pt || strcmp( pt, datum ))
	    printf( "Expected \"%s\", got \"%s\" for key \"%s\"\n",
		    datum, pt, key );

      xfree( key );
   }
   
   hsh_print_stats( t, stdout );

   hsh_iterate( t, freer );
   hsh_destroy( t );



				/* Test (random) integer keys */
   t = hsh_create( hsh_pointer_hash, hsh_pointer_compare );

   init_rand();
   for (i = 0; i < count; i++) {
      int  key    = get_rand( 1, 16777216 );
      char *datum = xmalloc( 20 );

      sprintf( datum, "datum%d", i );
      hsh_insert( t, (void *)key, datum );
   }

   init_rand();
   for (i = 0; i < count; i++) {
      int         key = get_rand( 1, 16777216 );
      char        datum[100];
      const char *pt;

      sprintf( datum, "datum%d", i );
      pt = hsh_retrieve( t, (void *)key );

      if (!pt || strcmp( pt, datum ))
	    printf( "Expected \"%s\", got \"%s\" for key %d\n",
		    datum, pt, key );
   }
   
   hsh_print_stats( t, stdout );

   hsh_iterate( t, free_data );
   hsh_destroy( t );

   return 0;
}
