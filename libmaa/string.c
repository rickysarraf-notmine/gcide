/* string.c -- String pool for Khepera
 * Created: Wed Dec 21 21:32:34 1994 by faith@cs.unc.edu
 * Revised: Tue May 20 14:38:20 1997 by faith@acm.org
 * Copyright 1994, 1995, 1996, 1997 Rickard E. Faith (faith@acm.org)
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
 * $Id: string.c,v 1.10 1997/05/20 21:30:27 faith Exp $
 *
 * \section{String Pool Routines}
 *
 * \intro These routines provide support for string pool objects.  In
 * general, only the |str_find| and |str_findn| functions will be used.
 * This function takes a pointer to a null-terminated string and returns a
 * pointer to another null-terminated string which contains the same
 * information.  The pointer returned will be identical for all identical
 * strings.  Memory for string storage is automatically reclaimed at
 * program termination on systems that support |atexit| or |on_exit|.
 *
 */

#include "maaP.h"

static str_Pool global;

typedef struct poolInfo {
   mem_String    string;
   hsh_HashTable hash;
} *poolInfo;

/* \doc |str_pool_create| initialized a string pool object. */

str_Pool str_pool_create( void )
{
   poolInfo pool = xmalloc( sizeof( struct poolInfo ) );

   pool->string = mem_create_strings();
   pool->hash   = hsh_create( NULL, NULL );

   return pool;
}

/* \doc |str_pool_destroy| destroys the string pool object, |pool|, and all
   memory associated with it. */

void str_pool_destroy( str_Pool pool )
{
   poolInfo p = (poolInfo)pool;
   
   if (!p || !p->string || !p->hash)
	 err_fatal( __FUNCTION__, "String pool improperly initialized\n" );

   mem_destroy_strings( p->string );
   hsh_destroy( p->hash );
   xfree( p );			/* terminal */
}

/* \doc |str_pool_exists| returns non-zero if the string, |s|, is already
   in the string pool, |pool|. */

int str_pool_exists( str_Pool pool, const char *s )
{
   const char *datum;
   poolInfo   p = (poolInfo)pool;
   
   if ((datum = hsh_retrieve( p->hash, s ))) return 1;
   return 0;
}

/* \doc |str_pool_find| looks up the string, |s|, in the memory associated
   with the string pool object, |pool|.  If the string is found, a pointer
   to the previously stored string is returned.  Otherwise, the string is
   copied into string pool memory, and a pointer to the newly allocated
   memory is returned. */

const char *str_pool_find( str_Pool pool, const char *s )
{
   const char *datum;
   poolInfo   p = (poolInfo)pool;
   
   if ((datum = hsh_retrieve( p->hash, s ))) return datum;
   datum = mem_strcpy( p->string, s );
   hsh_insert( p->hash, datum, datum );

   return datum;
}

/* \doc |str_pool_copy| returns a copy of the string, |s|, using memory
   from the string pool object, |pool|.  This can be used for data that is
   known to be unique.  No checks are made for uniqueness, however; and a
   pointer to the string is not placed in the hash table. */

const char *str_pool_copy( str_Pool pool, const char *s )
{
   poolInfo   p = (poolInfo)pool;
   
   return mem_strcpy( p->string, s );
}

/* \doc |str_pool_copyn| returns a copy of the string, |s|, using memory
   from the string pool object, |pool|.  The string will be |length| bytes
   long, and will be "NULL" terminated.  This can be used for data that is
   known to be unique.  No checks are made for uniqueness, however; and a
   pointer to the string is not placed in the hash table. */

const char *str_pool_copyn( str_Pool pool, const char *s, int length )
{
   poolInfo   p = (poolInfo)pool;
   
   return mem_strncpy( p->string, s, length );
}

/* \doc |str_pool_grow| will grow a string in the specified |pool| until
   |str_pool_finish| is called.  There must not be any other calls to
   modify the specified string |pool| between the first call to
   |str_pool_grow| and the call to |str_pool_finish|. */

void str_pool_grow( str_Pool pool, const char *s, int length )
{
   poolInfo p = (poolInfo)pool;

   mem_grow( p->string, s, length );
}

/* \doc |str_pool_finish| will finish the growth of a string performed by
   multiple calls to |str_pool_grow|.  The string will be null terminated
   and will be entered into the specified string |pool|.  Calls to
   |str_pool_grow| follows by a call to |str_pool_finish| is equivalent to
   a single call to |str_pool_find|. */

const char *str_pool_finish( str_Pool pool )
{
   poolInfo   p      = (poolInfo)pool;
   const char *datum;

   mem_grow( p->string, "\0", 1 ); /* guarantee null termination */
   datum = mem_finish( p->string );
   hsh_insert( p->hash, datum, datum );

   return datum;
}

/* \doc |str_pool_get_stats| returns statistics about the specified string
   |pool|.  The |str_Stats| structure is shown in \grind{str_Stats}. */

str_Stats str_pool_get_stats( str_Pool pool )
{
   poolInfo        p = (poolInfo)pool;
   str_Stats       s = xmalloc( sizeof( struct str_Stats ) );

   if (p) {
      mem_StringStats m = mem_get_string_stats( p->string );
      hsh_Stats       h = hsh_get_stats( p->hash );
      
      s->count      = m->count;
      s->bytes      = m->bytes;
      s->retrievals = h->retrievals;
      s->hits       = h->hits;
      s->misses     = h->misses;
      
      xfree( h );		/* rare */
      xfree( m );		/* rare */
   } else {
      s->count      = 0;
      s->bytes      = 0;
      s->retrievals = 0;
      s->hits       = 0;
      s->misses     = 0;
   }

   return s;
}

/* \doc |str_pool_print_stats| prints the statistics for the specified
   string |pool| on the specified |stream|.  If |stream| is "NULL", then
   "stdout" will be used. */

void str_pool_print_stats( str_Pool pool, FILE *stream )
{
   FILE      *str = stream ? stream : stdout;
   str_Stats s    = str_pool_get_stats( pool );

   fprintf( str, "Statistics for %sstring pool at %p:\n",
	    pool == global ? "global " : "", pool );
   fprintf( str, "   %d strings using %d bytes\n", s->count, s->bytes );
   fprintf( str, "   %d retrievals (%d from top, %d failed)\n",
	    s->retrievals, s->hits, s->misses );
   xfree( s );			/* rare */
}

static void _str_check_global( void )
{
   if (!global) global = str_pool_create();
}

/* \doc |str_exists| acts like |str_pool_exists|, except the global string
   pool is used. */

int str_exists( const char *s )
{
   return str_pool_exists( global, s );
}

/* \doc |str_find| acts like |str_pool_find|, except the global string pool
   is used.  If the global string pool has not been initialized, it will be
   initialized automatically.  Further, on systems that support |atexit| or
   |on_exit|, |str_destroy| will be called automatically at program
   termination. */

const char *str_find( const char *s )
{
   _str_check_global();
   return str_pool_find( global, s );
}

/* \doc |str_findn| acts like |str_find|, except that the length of the
   string is specified, and the string does not have to be "NULL"
   terminated. */

const char *str_findn( const char *s, int length )
{
   char *tmp = alloca( sizeof( char ) * (length + 1) );
   
   _str_check_global();
   strncpy( tmp, s, length );
   tmp[ length ] = '\0';
   return str_pool_find( global, tmp );
}

/* \doc |str_copy| acts like |str_pool_copy|, except the global string pool
   is used.  If the global string pool has not been initialized, it will be
   initialized automatically.  Further, on systems that support |atexit| or
   |on_exit|, |str_destroy| will be called automatically at program
   termination. */

const char *str_copy( const char *s )
{
   _str_check_global();
   return str_pool_copy( global, s );
}

/* \doc |str_copyn| acts like |str_copy|, except that the length of the
   string is specified, and the string does not have to be "NULL"
   terminated. */

const char *str_copyn( const char *s, int length )
{
   return str_pool_copyn( global, s, length );
}

/* \doc |str_grow| will grow a string until |str_finish| is called.  There
   must not be any other calls to modify the global string pool between the
   first call to |str_grow| and the call to |str_finish|. */

void str_grow( const char *s, int length )
{
   _str_check_global();
   str_pool_grow( global, s, length );
}

/* \doc |str_finish| will finish the growth of a string performed by
   multiple calls to |str_grow|.  The string will be null terminated and
   will be entered into the global string pool tables.  Calls to |str_grow|
   follows by a call to |str_finish| is equivalent to a single call to
   |str_findn|. */

const char *str_finish( void )
{
   _str_check_global();
   return str_pool_finish( global );
}

/* \doc |str_unique| returns a unique string with the given prefix.  This
   is not the most pretty way to generate unique strings, and should be
   improved.  The string is placed in the string pool and does not need to
   be freed. */

const char *str_unique( const char *prefix )
{
   static int i       = 1;
   char       *buf    = alloca( strlen( prefix ) + 100 );

   do {
      sprintf( buf, "%s%d", prefix, i++ );
   } while (str_exists( buf ));
   return str_find( buf );
}

/* \doc |str_destroy| frees all of the memory associated with the global
   string pool.  Since this function is called automatically at program
   termination on systems that support |atexit| or |on_exit|, there should
   be no need to call this function explicitly.

   If this function is called explicitly, the next call to |str_find| will
   re-initialize the global string pool. */

void str_destroy( void )
{
   if (global) str_pool_destroy( global );
   global = NULL;
}

/* \doc |str_get_stats| returns statistics about the global string pool.
   The |str_Stats| structure is shown in \grindref{fig:strStats}. */

str_Stats str_get_stats( void )
{
   return str_pool_get_stats( global );
}

/* \doc |str_print_stats| prints the statistics for the global string pool
   on the specified |stream|.  If |stream| is "NULL", then "stdout" will be
   used. */

void str_print_stats( FILE *stream )
{
   str_pool_print_stats( global, stream );
}

