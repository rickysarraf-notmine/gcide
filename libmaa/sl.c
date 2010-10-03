/* sl.c -- Skip lists
 * Created: Sun Feb 18 11:51:06 1996 by faith@cs.unc.edu
 * Revised: Sat Nov 29 12:04:02 1997 by faith@acm.org
 * Copyright 1996, 1997 Rickard E. Faith (faith@acm.org)
 * Copyright 1996 Lars Nyland (nyland@cs.unc.edu)
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
 * $Id: sl.c,v 1.10 1997/11/30 13:37:33 faith Exp $
 *
 * \section{Skip List Routines}
 *
 * \intro Skip list support is provided as an alternative to balanced
 * trees.  Skip lists have the advantage that an inorder walk through the
 * list is possible in the face of additions and deletions from the list.
 * Balanced tree, algorithms, in contrast, make this sort of traversal
 * impossible because of the rotations that are necessary for the
 * balancing.
 *
 * For these lists, the |key| is derivable from the |datum| that is stored
 * in the list.  This makes it possible for the actualy keys to change, as
 * long as the ordering of the data stay the same.  This is essential for
 * the use of skip lists for \khepera trees.
 *
 * This code is derived from \cite{faith:Pugh90} and from a skip list
 * implementation by Lars Nyland.
 *
 */

#include "maaP.h"

#define SL_DEBUG 0		/* Debug like crazy */

typedef struct _sl_Entry {
#if MAA_MAGIC
   int              magic;
#endif
   const void       *datum;
#if SL_DEBUG
   int              levels;	/* levels for this entry */
#endif
   struct _sl_Entry *forward[1]; /* variable sized array */
} *_sl_Entry;

typedef struct _sl_List {
#if MAA_MAGIC
   int              magic;
#endif
   int              level;
   int              count;	/* number of data */
   struct _sl_Entry *head;
   int              (*compare)( const void *key1, const void *key2 );
   const void       *(*key)( const void *datum );
   const char       *(*print)( const void *datum );
} *_sl_List;

static mem_Object _sl_Memory;

#define _sl_MaxLevel  16

#define PRINT(l,d) ((l)->print ? (l)->print(d) : _sl_print(d))

static void _sl_check_list( _sl_List l, const char *function )
{
   if (!l) err_internal( function, "skip list is null\n" );
#if MAA_MAGIC
   if (l->magic != SL_LIST_MAGIC)
      err_internal( function,
		    "Bad magic: 0x%08x (should be 0x%08x)\n",
		    l->magic,
		    SL_LIST_MAGIC );
#endif
}

#if !SL_DEBUG
#define _sl_check_entry(e,f)	/*  */
#else
static int _sl_check_entry( _sl_Entry e, const char *function )
{
   if (!e) err_internal( function, "entry is null\n" );
#if MAA_MAGIC
   if (e->magic != SL_ENTRY_MAGIC) {
      err_warning( function,
		   "Bad magic: 0x%08x (should be 0x%08x)\n",
		   e->magic,
		   SL_ENTRY_MAGIC );
      return 1;
   }
#endif
   return 0;
}
#endif

#if !SL_DEBUG
#define _sl_check(x) /* */
#else
static void _sl_check( sl_List list )
{
   _sl_List  l     = (_sl_List)list;
   int       count = 0;
   _sl_Entry pt;
   
   _sl_check_list( list, __FUNCTION__ );
   for (pt = l->head->forward[0]; pt; pt = pt->forward[0] ) {
      ++count;
      if (pt && pt->forward[0]
	  && l->compare( l->key( pt->datum ),
			 l->key( pt->forward[0]->datum ) ) >= 0) {
	 _sl_dump( list );
	 err_internal( __FUNCTION__,
		       "Datum 0x%p=%lu >= 0x%p=%lu\n",
		       l->key( pt->datum ),
		       (unsigned long)l->key( pt->datum ),
		       l->key( pt->forward[0]->datum ),
		       (unsigned long)l->key( pt->forward[0]->datum ) );
      }
   }
   if (count != l->count) {
      err_internal( __FUNCTION__,
		    "Count should be %d instead of %d\n", count, l->count );
   }
}
#endif

static _sl_Entry _sl_create_entry( int maxLevel, const void *datum )
{
   _sl_Entry e;

   if (maxLevel > _sl_MaxLevel)
      err_internal( __FUNCTION__,
		    "level %d > %d\n", maxLevel, _sl_MaxLevel );

   e = xmalloc( sizeof( struct _sl_Entry )
		+ (maxLevel + 1) * sizeof( _sl_Entry ) );
#if MAA_MAGIC
   e->magic  = SL_ENTRY_MAGIC;
#endif
   e->datum  = datum;
#if SL_DEBUG
   e->levels = maxLevel + 1;
#endif

   return e;
}

/* \doc |sl_create| initializes a skip list.  The |compare| function
   returns -1, 0, or 1 depending on the ordering of |key1| and |key2|.  The
   |key| function converts a |datum| into a |key|.  The |print| function
   returns a string representation of |datum|, and is allowed to always
   return a pointer to the same static buffer.

   |compare| must be provided.  If |key| is not provided, then |datum| will
   be used as the key.  If |print| is not provided, then the |datum|
   pointer will be printed when necessary. */


sl_List sl_create( int (*compare)( const void *key1, const void *key2 ),
		   const void *(*key)( const void *datum ),
		   const char *(*print)( const void *datum ) )
{
   _sl_List l;
   int      i;

   if (!_sl_Memory) {
      _sl_Memory = mem_create_objects( sizeof( struct _sl_List ) );
   }

   if (!compare)
      err_internal( __FUNCTION__, "compare fuction is NULL\n" );
   if (!key)
      err_internal( __FUNCTION__, "key fuction is NULL\n" );

   l          = mem_get_object( _sl_Memory );
#if MAA_MAGIC
   l->magic   = SL_LIST_MAGIC;
#endif
   l->level   = 0;
   l->head    = _sl_create_entry( _sl_MaxLevel, NULL );
   l->compare = compare;
   l->key     = key;
   l->print   = print;
   l->count   = 0;

   for (i = 0; i <= _sl_MaxLevel; i++) l->head->forward[i] = NULL;

   return l;
}

/* \doc |sl_destroy| removes all of the memory associated with the
   maintenance of the specified skip |list|.  The pointer to the
   user-defined |datum| is "not" freed -- this is the responsibility of the
   user. */

void sl_destroy( sl_List list )
{
   _sl_List  l = (_sl_List)list;
   _sl_Entry e;
   _sl_Entry next;

   _sl_check_list( list, __FUNCTION__ );
   for (e = l->head; e; e = next) {
      next = e->forward[0];
#if MAA_MAGIC
      e->magic = SL_ENTRY_MAGIC_FREED;
#endif
      xfree( e );
   }
#if MAA_MAGIC
   l->magic = SL_LIST_MAGIC_FREED;
#endif
   mem_free_object( _sl_Memory, l );
}

/* \doc |_sl_shutdown| is used to free the internal data structures used by
   the skip list package.  Since it is called automatically by \libmaa, it
   should not be called explicitly by the user. */

void _sl_shutdown( void )
{
   if (_sl_Memory) mem_destroy_objects( _sl_Memory );
   _sl_Memory = NULL;
}

#if 0
static int rnd( void )		/* generate bit stream */
{
   static int i = 0;

   i = !i;
   return i;
}
#endif

static int _sl_random_level( void )
{
   int level = 1;
				/* FIXME! Assumes random() is.  We should
                                   use our own random() to make sure.  This
                                   also assumes that p == 0.5, which is
                                   probably reasonable, but maybe should be
                                   a user-defined parameter. */
   while ((random() & 0x80) &&  level < _sl_MaxLevel) ++level;
   return level;
}

static const char *_sl_print( const void *datum )
{
   static char buf[1024];

   sprintf( buf, "%p", datum );

   return buf;
}

static _sl_Entry _sl_locate( _sl_List l, const void *key, _sl_Entry update[] )
{
   int       i;
   _sl_Entry pt;
   
   _sl_check( l );
   for (i = l->level, pt = l->head; i >= 0; i--) {
      while (pt->forward[i]
	     && l->compare( l->key( pt->forward[i]->datum ), key ) < 0)
	 pt = pt->forward[i];
      update[i] = pt;
   }
   
   return pt->forward[0];
}


/* \doc Insert |datum| into |list|. */

void sl_insert( sl_List list, const void *datum )
{
   _sl_List         l = (_sl_List)list;
   _sl_Entry        update[_sl_MaxLevel + 1];
   _sl_Entry        pt;
   const void       *key;
   int              i;
   int              level = _sl_random_level();
   _sl_Entry        entry;

   _sl_check_list( list, __FUNCTION__ );
   
   key = l->key( datum );

   pt = _sl_locate( l, key, update );

   if (pt && !l->compare( l->key( pt->datum ), key ))
      err_internal( __FUNCTION__,
		    "Datum \"%s\" is already in list\n", PRINT(l,datum) );

   if (level > l->level) {
      level = ++l->level;
      update[level] = l->head;
   }
   
   entry = _sl_create_entry( level, datum );

				/* Fixup forward pointers */
   for (i = 0; i <= level; i++) {
      entry->forward[i]     = update[i]->forward[i];
      update[i]->forward[i] = entry;
   }

   ++l->count;
   _sl_check( list );
}

/* \doc Delete |datum| from |list|. */

void sl_delete( sl_List list, const void *datum )
{
   _sl_List         l = (_sl_List)list;
   _sl_Entry        update[_sl_MaxLevel + 1];
   _sl_Entry        pt;
   const void       *key;
   int              i;

   _sl_check_list( list, __FUNCTION__ );
   
   key = l->key( datum );

   pt = _sl_locate( l, key, update );

   if (!pt || l->compare( l->key( pt->datum ), key )) {
      _sl_dump( list );
      err_internal( __FUNCTION__,
		    "Datum \"%s\" is not in list\n", PRINT(l,datum) );
   }

				/* Fixup forward pointers */
   for (i = 0; i <= l->level; i++) {
      if (update[i]->forward[i] == pt)
	 update[i]->forward[i] = pt->forward[i];
   }
   
   xfree( pt );
   while (l->level && !l->head->forward[ l->level ])
      --l->level;
   --l->count;
   _sl_check( list );
}

/* \doc Find the datum in |list| that has an associated value of |key|.
   Return that datum (a pointer), or "NULL" if the |key| is not found. */

const void *sl_find( sl_List list, const void *key )
{
   _sl_List         l = (_sl_List)list;
   _sl_Entry        update[_sl_MaxLevel + 1];
   _sl_Entry        pt;

   _sl_check_list( list, __FUNCTION__ );

   pt = _sl_locate( l, key, update );

   if (pt && !l->compare( l->key( pt->datum ), key )) return pt->datum;
   return NULL;
}

/* \doc Iterate |f| over every datum in |list|.  If |f| returns non-zero,
   then abort the remainder of the iteration.  Iterations are designed to
   do something appropriate in the face of arbitrary insertions and
   deletions performed by |f|. */

int sl_iterate( sl_List list, sl_Iterator f )
{
   _sl_List   l = (_sl_List)list;
   _sl_Entry  pt;
   int        retcode;
   int        count;
   int        i;
   const void **copy;
   

   if (!list) return 0;
   _sl_check_list( list, __FUNCTION__ );

				/* WARNING! This *ASSUMES* that the data to
                                   the right of the current point will
                                   remain at its memory location during the
                                   walk.  Only memory locations for data to
                                   the left of the point may change! */
   count = l->count;
   copy = alloca( count * sizeof( void * ) );
   for (i = 0, pt = l->head->forward[0]; pt; i++, pt = pt->forward[0]) {
      copy[i] = pt->datum;
   }
   for (i = 0; i < count; i++) {
      if ((retcode = f( copy[i] ))) return retcode;
   }

   _sl_check( list );
   
   return 0;
}

/* \doc Iterate |f| over every datum in |list|.  If |f| returns non-zero,
   then abort the remainder of the iteration.  Iterations are designed to
   do something appropriate in the face of arbitrary insertions and
   deletions performed by |f|. */

int sl_iterate_arg( sl_List list, sl_IteratorArg f, void *arg )
{
   _sl_List   l = (_sl_List)list;
   _sl_Entry  pt;
   int        retcode;
   int        count;
   int        i;
   const void **copy;
   

   if (!list) return 0;
   _sl_check_list( list, __FUNCTION__ );

				/* WARNING! This *ASSUMES* that the data to
                                   the right of the current point will
                                   remain at its memory location during the
                                   walk.  Only memory locations for data to
                                   the left of the point may change! */
   count = l->count;
   copy = alloca( count * sizeof( void * ) );
   for (i = 0, pt = l->head->forward[0]; pt; i++, pt = pt->forward[0]) {
      _sl_check_entry( pt, __FUNCTION__ );
      copy[i] = pt->datum;
   }
   for (i = 0; i < count; i++) {
      if ((retcode = f( copy[i], arg ))) return retcode;
   }

   _sl_check( list );
   
   return 0;
}

/* \doc Dump the internal data structures associated with |list|.  This is
   purely for debugging. */

void _sl_dump( sl_List list )
{
   _sl_List  l = (_sl_List)list;
   _sl_Entry pt;
   int       count = 0;

   _sl_check_list( list, __FUNCTION__ );

   printf( "Level = %d, count = %d\n", l->level, l->count );
   for (pt = l->head; pt; pt = pt->forward[0]) {
#if SL_DEBUG
      int       i;
      
      printf( "  Entry %p (%d/%p/0x%p=%lu) has 0x%x levels:\n",
	      pt, count++, pt->datum,
	      pt->datum ? l->key( pt->datum ) : 0,
	      (unsigned long)(pt->datum ? l->key( pt->datum ) : 0),
	      pt->levels );
      for (i = 0; i < pt->levels; i++)
	 printf( "    %p\n", pt->forward[i] );
#else
      printf( "  Entry %p (%d/%p/0x%p=%lu)\n",
	      pt, count++, pt->datum,
	      pt->datum ? l->key( pt->datum ) : 0,
	      (unsigned long)(pt->datum ? l->key( pt->datum ) : 0) );
#endif
   }
}
