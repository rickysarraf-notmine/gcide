/* timer.c -- Timer support
 * Created: Sat Oct  7 13:05:31 1995 by faith@cs.unc.edu
 * Revised: Mon Jul  7 15:57:06 1997 by faith@acm.org
 * Copyright 1995, 1996, 1997 Rickard E. Faith (faith@acm.org)
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
 * 
 * $Id: timer.c,v 1.16 1997/07/07 20:08:17 faith Exp $
 *
 * \section{Timer Support}
 *
 * \intro These routines provide access to a microsecond-resolution time
 * that can be used for profiling.
 *
 */

#include "maaP.h"

static hsh_HashTable _tim_Hash;

typedef struct tim_Entry {
   unsigned long  real;		/* wall time in usec */
   unsigned long  self_user;	/* user time in usec */
   unsigned long  self_system;	/* system time in usec */
   unsigned long  children_user; /* user time in usec */
   unsigned long  children_system; /* system time in usec */
   struct timeval real_mark;
   struct rusage  self_mark;
   struct rusage  children_mark;
} *tim_Entry;

static void _tim_check( void )
{
   if (!_tim_Hash) _tim_Hash = hsh_create( NULL, NULL );
}

/* \doc Start the named timer. */

void tim_start( const char *name )
{
   tim_Entry entry;

   _tim_check();
   if (!(entry = (tim_Entry)hsh_retrieve( _tim_Hash, name ))) {
      entry = xmalloc( sizeof( struct tim_Entry  ) );
      entry->real = 0;
      entry->self_user = entry->self_system = 0;
      entry->children_user = entry->children_system = 0;
      hsh_insert( _tim_Hash, name, entry );
   }

#if HAVE_GETTIMEOFDAY
   gettimeofday( &entry->real_mark, NULL );
#else
   time( &entry->real_mark.tv_sec );
   entry->real_mark.tv_usec = 0;
#endif
   
   getrusage( RUSAGE_SELF, &entry->self_mark );
   getrusage( RUSAGE_CHILDREN, &entry->children_mark );
}

/* \doc Stop the named timer. */

void tim_stop( const char *name )
{
   tim_Entry entry;
   struct timeval  real;
   struct rusage   rusage;

#define DIFFTIME(now,then)\
   (((now).tv_sec - (then).tv_sec)*1000 \
    + ((now).tv_usec - (then).tv_usec)/1000)

   _tim_check();
#if HAVE_GETTIMEOFDAY
   gettimeofday( &real, NULL );
#else
   time( &real.tv_sec );
   real.tv_usec = 0;
#endif
   
   if (!(entry = (tim_Entry)hsh_retrieve( _tim_Hash, name ) ))
      err_internal ( __FUNCTION__, "No timer: %s\n", name );
   
   entry->real   = DIFFTIME( real, entry->real_mark );
   getrusage( RUSAGE_SELF, &rusage );
   entry->self_user   = DIFFTIME( rusage.ru_utime, entry->self_mark.ru_utime );
   entry->self_system = DIFFTIME( rusage.ru_stime, entry->self_mark.ru_stime );
   
   getrusage( RUSAGE_CHILDREN, &rusage );
   entry->children_user
      = DIFFTIME( rusage.ru_utime, entry->children_mark.ru_utime );
   entry->children_system
      = DIFFTIME( rusage.ru_stime, entry->children_mark.ru_stime );
}

/* \doc Reset the named timer to zero elapsed time.  Use |tim_start| to reset
   the start time.  */

void tim_reset( const char *name )
{
   tim_Entry entry;
   
   _tim_check();
   if (!(entry = (tim_Entry)hsh_retrieve( _tim_Hash, name ) ))
      err_internal ( __FUNCTION__, "No timer: %s\n", name );

   entry->real = 0;
   entry->self_user = entry->self_system = 0;
   entry->children_user = entry->children_system = 0;
}

/* \doc Get the wall time in seconds from the named timer.  The return
   value is a |double| and has microsecond resolution if the current system
   provides that accuracy (most don't). */

double tim_get_real( const char *name )
{
   tim_Entry entry;
   
   _tim_check();
   if (!(entry = (tim_Entry)hsh_retrieve( _tim_Hash, name ) ))
      err_internal ( __FUNCTION__, "No timer: %s\n", name );

   return entry->real / 1000.0;
}

/* \doc Get the number of seconds of user CPU time. */

double tim_get_user( const char *name )
{
   tim_Entry entry;
   
   _tim_check();
   if (!(entry = (tim_Entry)hsh_retrieve( _tim_Hash, name ) ))
      err_internal ( __FUNCTION__, "No timer: %s\n", name );

#if 0
   printf( "self: maxrss %ld ixrss %ld idrss %ld isrss %ld minflt %ld"
	   " majflt %ld nswap %ld inblock %ld outblock %ld msgsnd %ld"
	   " msgrcv %ld nsignals %ld nvcwm %ld nivcsm %ld\n",
	   entry->self_mark.ru_maxrss,
	   entry->self_mark.ru_ixrss,
	   entry->self_mark.ru_idrss,
	   entry->self_mark.ru_isrss,
	   entry->self_mark.ru_minflt,
	   entry->self_mark.ru_majflt,
	   entry->self_mark.ru_nswap,
	   entry->self_mark.ru_inblock,
	   entry->self_mark.ru_oublock,
	   entry->self_mark.ru_msgsnd,
	   entry->self_mark.ru_msgrcv,
	   entry->self_mark.ru_nsignals,
	   entry->self_mark.ru_nvcsw,
	   entry->self_mark.ru_nivcsw );
   printf( "chld: maxrss %ld ixrss %ld idrss %ld isrss %ld minflt %ld"
	   " majflt %ld nswap %ld inblock %ld outblock %ld msgsnd %ld"
	   " msgrcv %ld nsignals %ld nvcwm %ld nivcsm %ld\n",
	   entry->children_mark.ru_maxrss,
	   entry->children_mark.ru_ixrss,
	   entry->children_mark.ru_idrss,
	   entry->children_mark.ru_isrss,
	   entry->children_mark.ru_minflt,
	   entry->children_mark.ru_majflt,
	   entry->children_mark.ru_nswap,
	   entry->children_mark.ru_inblock,
	   entry->children_mark.ru_oublock,
	   entry->children_mark.ru_msgsnd,
	   entry->children_mark.ru_msgrcv,
	   entry->children_mark.ru_nsignals,
	   entry->children_mark.ru_nvcsw,
	   entry->children_mark.ru_nivcsw );
#endif
   
   return (entry->self_user + entry->children_user) / 1000.0;
}

/* \doc Get the number of seconds of system CPU time. */

double tim_get_system( const char *name )
{
   tim_Entry entry;
   
   _tim_check();
   if (!(entry = (tim_Entry)hsh_retrieve( _tim_Hash, name ) ))
      err_internal ( __FUNCTION__, "No timer: %s\n", name );

   return (entry->self_system + entry->children_system) / 1000.0;
}

/* \doc Print the named timer values to |str|.  The format is similar to
   "time(1)". */

void tim_print_timer( FILE *str, const char *name )
{
   fprintf( str, "%-20s %0.3fr %0.3fu %0.3fs\n",
	    name,
	    tim_get_real( name ),
	    tim_get_user( name ),
	    tim_get_system( name ) );
}

static int _tim_iterator( const void *key, const void *datum, void *arg )
{
   FILE *str = (FILE *)arg;
   
   tim_print_timer( str, key );
   return 0;
}

/* \doc Print all the timers to |str|.  The order is arbitary. */

void tim_print_timers( FILE *str )
{
   if (_tim_Hash) hsh_iterate_arg( _tim_Hash, _tim_iterator, str );
}

static int _tim_freer( const void *key, const void *datum )
{
   xfree( (void *)datum ); /* Discard const */
   return 0;
}
   
/* \doc Free all memory associated with the timers.  This function is
   called automatically at program termination.  There should never be a
   need to call this function in user-level code. */

void _tim_shutdown( void )
{
   if (_tim_Hash) {
      hsh_iterate( _tim_Hash, _tim_freer );
      hsh_destroy( _tim_Hash );
   }
   _tim_Hash = NULL;
}
