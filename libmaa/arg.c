/* arg.c -- Argument list support
 * Created: Sun Jan  7 13:39:29 1996 by faith@acm.org
 * Revised: Fri Aug 15 07:55:05 1997 by faith@acm.org
 * Copyright 1996, 1997 Rickard E. Faith (faith@acm.org)
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
 * $Id: arg.c,v 1.9 1997/08/15 12:10:53 faith Exp $
 *
 * \section{Argument List Routines}
 *
 * \intro Argument lists are vectors of pointers to strings (e.g., the
 * standard |char **argv|).  These routines manage the efficient creation,
 * manipulation, and deletion of these sorts of lists.
 *
 */

#include "maaP.h"

typedef struct Arg {
#if MAA_MAGIC
   int        magic;
#endif
   int        argc;		/* Current count */
   int        argm;		/* Maximum count */
   char       **argv;		/* Vector */
   mem_String object;
} *Arg;

static void _arg_check( arg_List arg, const char *function )
{
   Arg a = (Arg)arg;
   
   if (!a) err_internal( function, "arg is null\n" );
#if MAA_MAGIC
   if (a->magic != ARG_MAGIC)
      err_internal( function,
		    "Magic match failed: 0x%08x (should be 0x%08x)\n",
		    a->magic,
		    ARG_MAGIC );
#endif
}   

/* \doc Create an |arg_List| object. */

arg_List arg_create( void )
{
   Arg a = xmalloc( sizeof( struct Arg ) );

#if MAA_MAGIC
   a->magic   = ARG_MAGIC;
#endif
   a->argc    = 0;
   a->argm    = 2;
   a->argv    = xmalloc( a->argm * sizeof( char ** ) );
   a->argv[0] = NULL;
   a->object  = mem_create_strings();

   return a;
}

/* \doc Free all memory associated with |arg|, including the memory used
   for the strings. */

void arg_destroy( arg_List arg )
{
   Arg a = (Arg)arg;
   
   _arg_check( a, __FUNCTION__ );
   mem_destroy_strings( a->object );
   xfree( a->argv );
#if MAA_MAGIC
   a->magic = ARG_MAGIC_FREED;
#endif
   xfree( a );
}

/* \doc Add |string| as the next item in |arg|. */

arg_List arg_add( arg_List arg, const char *string )
{
   Arg        a = (Arg)arg;
   const char *new;
   
   _arg_check( a, __FUNCTION__ );
   new = mem_strcpy( a->object, string );
   if (a->argm <= a->argc + 2 )
      a->argv = xrealloc( a->argv, sizeof( char **) * (a->argm *= 2) );
   
   a->argv[a->argc++] = (char *)new; /* discard const */
   a->argv[a->argc]   = NULL;

   return a;
}

/* \doc Add |length| characters of |string| as the next item in |arg|.  A
   terminating "NULL" is added to the copied |string|. */

arg_List arg_addn( arg_List arg, const char *string, int length )
{
   Arg        a = (Arg)arg;
   const char *new;
   
   _arg_check( a, __FUNCTION__ );
   new = mem_strncpy( a->object, string, length );
   if (a->argm <= a->argc + 2 )
      a->argv = xrealloc( a->argv, sizeof( char **) * (a->argm *= 2) );
   
   a->argv[a->argc++] = (char *)new; /* discard const */
   a->argv[a->argc]   = NULL;

   return a;
}

/* \doc Grow the next item of |arg| with |length| characters of |string|.
   Several calls to |arg_grow| should be followed by a single call to
   |arg_finish| without any intervening calls to other functions which
   modify |arg|. */

void arg_grow( arg_List arg, const char *string, int length )
{
   Arg        a = (Arg)arg;

   _arg_check( a, __FUNCTION__ );
   mem_grow( a->object, string, length );
}

/* \doc Finish the growth of the next item in |arg| started by
   |arg_grow|. */

arg_List arg_finish( arg_List arg )
{
   Arg        a = (Arg)arg;
   const char *new;
   
   _arg_check( a, __FUNCTION__ );
   new = mem_finish( a->object );
   if (a->argm <= a->argc + 2 )
      a->argv = xrealloc( a->argv, sizeof( char **) * (a->argm *= 2) );
   
   a->argv[a->argc++] = (char *)new; /* discard const */
   a->argv[a->argc]   = NULL;

   return a;
}

/* \doc Return |item| from |arg|.  |arg| is 0-based. */

const char *arg_get( arg_List arg, int item ) /* FIXME! inline? */
{
   Arg a = (Arg)arg;
   
   _arg_check( a, __FUNCTION__ );
   if (item < 0 || item >= a->argc)
      err_internal( __FUNCTION__,
		    "Request for item %d in list containing %d items\n",
		    item,
		    a->argc );
   return a->argv[ item ];
}

/* \doc Return the number of items in |arg|. */

int arg_count( arg_List arg )	/* FIXME! inline? */
{
   _arg_check( arg, __FUNCTION__ );
   return ((Arg)arg)->argc;
}

/* \doc Get an |argc| and |argv| from |arg|.  These are suitable for use in
   calls to |exec|.  The |argc|+1 item in |argv| is "NULL". */

void arg_get_vector( arg_List arg, int *argc, char ***argv )
{
   Arg        a = (Arg)arg;

   _arg_check( a, __FUNCTION__ );
   *argc = a->argc;
   *argv = a->argv;
}

/* \doc Break up |string| into arguments, placing them as items in |arg|.
   Items within single or double quotes may contain spaces.  The quotes are
   stripped as in shell argument processing.  In this version, backslash is
   "not" a special quoting character. */

arg_List arg_argify( const char *string, int quoteStyle )
{
   Arg        a = arg_create();
   const char *last;
   const char *pt = string;
   int        len;
   int        quote = 0;

   if (!string)
      err_internal( __FUNCTION__, "Cannot argify NULL pointer\n" );

   while (*pt && (*pt == ' '
		  || *pt == '\t'
		  || *pt == '\n'
		  || *pt == '\r'
		  || *pt == '\v'
		  || *pt == '\f'))
      ++pt;

   for (last = pt, len = 0; *pt; ++pt, ++len) {
      switch (*pt) {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
      case '\v':
      case '\f':
	 if (!quote) {
	    while (pt[1] == ' '
		   || pt[1] == '\t'
		   || pt[1] == '\n'
		   || pt[1] == '\r'
		   || pt[1] == '\v'
		   || pt[1] == '\f')
	       ++pt;
	    arg_grow( a, last, len );
	    arg_finish( a );
	    last = pt + 1;
	    len = -1;
	 }
	 break;
      case '"':
      case '\'':
	 if (!(quoteStyle & ARG_NO_QUOTE)) {
	    if (quote == *pt) {
	       arg_grow( a, last, len );
	       quote = 0;
/* 	       last  = 0; */
	       len   = -1;
	    } else if (!quote) {
	       arg_grow( a, last, len );
	       quote = *pt;
	       last  = pt + 1;
	       len   = -1;
	    }
	 }
	 break;
      case '\\':
	 if (!(quoteStyle & ARG_NO_ESCAPE)) {
	    if (pt[1]) {
	       arg_grow( a, last, len );
	       arg_grow( a, ++pt, 1 );
	       last = pt + 1;
	       len  = -1;
	    }
	 }
	 break;
      }
   }
   if (*last
       && *last != ' '
       && *last != '\t'
       && *last != '\n'
       && *last != '\r'
       && *last != '\v'
       && *last != '\f') {
      arg_grow( a, last, len );
      arg_finish( a );
   }

   return a;
}
