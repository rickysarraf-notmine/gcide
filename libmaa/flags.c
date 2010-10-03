/* flags.c -- Flag support for Khepera
 * Created: Sat Mar 23 10:11:52 1996 by faith@cs.unc.edu
 * Revised: Fri Jun 20 17:23:46 1997 by faith@acm.org
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
 * $Id: flags.c,v 1.3 1997/06/21 01:05:12 faith Exp $
 *
 * \section{Flag Support}
 *
 * \intro These routines provide low-level support for run-time program
 * flow control.  The mechanism used is similar to that used for debugging
 * messages.  A set of global flags are maintained using "#define"
 * statements.  These flags are assumed to be 32-bit integers, the top two
 * bits of which are used to select one of four sets of debugging lags.
 * Each set, therefore, has 30 bits of flag information.  For convenience,
 * each flag can be given a unique name, so that flags can be set easily
 * with command-line options.
 * */

#include "maaP.h"

#define TEST(flags,var) (((flags)>>31)                                     \
			 ? (((flags)>>30)                                  \
			    ? ((var[3] & (flags)) << 2)                    \
			    : ((var[2] & (flags)) << 2))                   \
			 : (((flags)>>30)                                  \
			    ? ((var[1] & (flags)) << 2)                    \
			    : ((var[0] & (flags)) << 2)))

static hsh_HashTable hash;
static flg_Type      setFlags[4];
static flg_Type      usedFlags[4];

/* |_flg_exists| returns non-zero if |flag| has been associated with a name
   (using the |flg_register| function). */

static int _flg_exists( flg_Type flag )
{
   return TEST( flag, usedFlags );
}

/* |flg_name| returns a pointer to the name that was associated with the
   |flag|. */

const char *flg_name( flg_Type flag )
{
   hsh_Position position;
   void         *key;
   void         *datum;

   HSH_ITERATE( hash, position, key, datum ) {
      if (flag == (flg_Type)datum) {
	 HSH_ITERATE_END( hash );
	 return key;
      }
   }

   return "unknown flag";
}

/* \doc |flg_register| is used to set up an asociated between a |flag| and
   a |name|.  After this association is made, calls to |flg_set| can use
   |name| to set the global flag.  */

void flg_register( flg_Type flag, const char *name )
{
   flg_Type tmp;
   
   for (tmp = flag & 0x3fffffff; tmp && !(tmp & 1); tmp >>= 1);
   if (!tmp || tmp >> 1)
	 err_fatal( __FUNCTION__,
		    "Malformed flag (%lx):"
		    " a single low-order bit must be set\n",
		    flag );
   
   if (!hash) hash = hsh_create( NULL, NULL );
   
   if (_flg_exists( flag ))
	 err_fatal( __FUNCTION__,
		    "The flag %lx has been used for \"%s\""
		    " and cannot be reused for \"%s\"\n",
		    flag,
		    flg_name( flag ),
		    name );

   hsh_insert( hash, name, (void *)flag );
}

/* \doc |flg_set| sets the |name| flag.  If |name| is ``none,'' then all
   flags are cleared.  */

void flg_set( const char *name )
{
   flg_Type flag;

   if (!name) err_internal( __FUNCTION__, "name is NULL\n" );
   if (!hash) err_fatal( __FUNCTION__, "No flag names registered\n" );
   if (!strcmp( name, "none" )) {
      setFlags[0] = setFlags[1] = setFlags[2] = setFlags[3] = 0;
      return;
   }
   if (!strcmp( name, "all" )) {
      setFlags[0] = setFlags[1] = setFlags[2] = setFlags[3] = ~0;
      return;
   }

   if (!(flag = (flg_Type)hsh_retrieve( hash, name ))) {
      if (!(flag = (flg_Type)hsh_retrieve( hash, name+1 ))
	  && *name != '-'
	  && *name != '+') {
	 
	 fprintf( stderr, "Valid flags are:\n" );
	 flg_list( stderr );
	 err_fatal( __FUNCTION__,
		    "\"%s\" is not a valid flag\n",
		    name );
      } else {
	 if (*name == '+') setFlags[ flag >> 30 ] |= flag;
	 else              setFlags[ flag >> 30 ] &= ~flag; /* - */
      }
   } else {
      setFlags[ flag >> 30 ] |= flag;
   }
}

/* \doc This inlined function tests the |flag|, returning non-zero if the
   |flag| is set, and zero otherwise. */

__inline__ int flg_test( flg_Type flag )
{
   return TEST( flag, setFlags );
}

/* \doc |flg_destroy| destroys the memory associated with the flag support
   routines.  This routine should \emph{never} be called by the programmer:
   it is automatically called at program termination on systems that
   support the |atexit| or |on_exit| calls. */

void flg_destroy( void )
{
   if (hash) hsh_destroy( hash );
   hash = NULL;
   setFlags[0] = setFlags[1] = setFlags[2] = setFlags[3] = 0;
   usedFlags[0] = usedFlags[1] = usedFlags[2] = usedFlags[3] = 0;
}


static int _flg_user( const void *key, const void *datum, void *arg )
{
   FILE     *stream = (FILE *)arg;
   
   fprintf( stream, "  %s\n", (char *)key );
   return 0;
}

/* |flg_list| lists all of the valid flags to the specified |stream|. */

void flg_list( FILE *stream )
{
   hsh_iterate_arg( hash, _flg_user, stream );
}
