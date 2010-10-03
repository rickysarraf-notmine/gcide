/* hash.c -- Hash table routines for Khepera
 * Created: Thu Nov  3 20:07:29 1994 by faith@cs.unc.edu
 * Revised: Tue May 20 14:32:58 1997 by faith@acm.org
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
 * $Id: hash.c,v 1.17 1997/05/20 21:30:24 faith Exp $
 *
 * \section{Hash Table Routines}
 *
 * \intro Generic hash table support is provided for storing generic data
 * associated with keys.  The hash table has prime length, with
 * self-organizing linked lists \cite[pp.~398--9]{faith:Knuth73c} used for
 * collision resolution. The hash table automatically grows as necessary to
 * preserve efficient access.
 *
 */

#include "maaP.h"

typedef struct bucket {
   const void    *key;
   unsigned long hash;
   const void    *datum;
   struct bucket *next;
} *bucketType;

typedef struct table {
#if MAA_MAGIC
   int           magic;
#endif
   unsigned long prime;
   unsigned long entries;
   bucketType    *buckets;
   unsigned long resizings;
   unsigned long retrievals;
   unsigned long hits;
   unsigned long misses;
   unsigned long (*hash)( const void * );
   int           (*compare)( const void *, const void * );
   int           readonly;
} *tableType;
   
static void _hsh_check( tableType t, const char *function )
{
   if (!t) err_internal( function, "table is null\n" );
#if MAA_MAGIC
   if (t->magic != HSH_MAGIC)
      err_internal( function,
		    "Magic match failed: 0x%08x (should be 0x%08x)\n",
		    t->magic,
		    HSH_MAGIC );
#endif
   if (!t->buckets)
      err_internal( function, "no buckets\n" );
}

static hsh_HashTable _hsh_create( unsigned long seed,
				  unsigned long (*hash)( const void * ),
				  int (*compare)( const void *,
						  const void * ))
{
   tableType     t;
   unsigned long i;
   unsigned long prime = prm_next_prime( seed );
   
   t             = xmalloc( sizeof( struct table ) );
#if MAA_MAGIC
   t->magic      = HSH_MAGIC;
#endif
   t->prime      = prime;
   t->entries    = 0;
   t->buckets    = xmalloc( prime * sizeof( struct bucket ) );
   t->resizings  = 0;
   t->retrievals = 0;
   t->hits       = 0;
   t->misses     = 0;
   t->hash       = hash ? hash : hsh_string_hash;
   t->compare    = compare ? compare : hsh_string_compare;
   t->readonly   = 0;

   for (i = 0; i < prime; i++) t->buckets[i] = NULL;

   return t;
}

/* \doc The |hsh_create| function initilizes a generic hash table.  Keys
   and data are pointers to "void".

   The internal representation of the hash table will grow automatically
   when an insertion is performed and the table is more than half full.

   The |hash| function should take a pointer to a |key| and return an
   "unsigned long".  If |hash| is "NULL", then the |key| is assumed to be a
   pointer to a null-terminated string, and the function shown in
   \grind{hsh_string_hash} will be used for |hash| (the algorithm for this
   function is from \cite[p.~435]{faith:Aho88}).

   The |compare| function should take a pair of pointers to keys and return
   zero if the keys are equal and non-zero if the keys are not equal.  If
   |compare| is "NULL", then the keys are assumed to point to
   null-terminated strings, and the |strcmp| function will be used for
   |compare|.

   Additionally, the |hsh_pointer_hash| and |hsh_pointer_compare| functions
   are available and can be used to treat the \emph{value} of the "void"
   pointer as the key.  These functions are often useful for maintaining
   sets of objects. */

hsh_HashTable hsh_create( unsigned long (*hash)( const void * ),
			  int (*compare)( const void *,
					  const void * ) )
{
   return _hsh_create( 0, hash, compare );
}

static void _hsh_destroy_buckets( hsh_HashTable table )
{
   unsigned long i;
   tableType     t    = (tableType)table;

   _hsh_check( t, __FUNCTION__ );
   for (i = 0; i < t->prime; i++) {
      bucketType b = t->buckets[i];

      while (b) {
	 bucketType next = b->next;

	 xfree( b );		/* terminal */
	 b = next;
      }
   }

   xfree( t->buckets );		/* terminal */
   t->buckets = NULL;
}

static void _hsh_destroy_table( hsh_HashTable table )
{
   tableType t = (tableType)table;
   
#if MAA_MAGIC
   t->magic = HSH_MAGIC_FREED;
#endif
   xfree( t );			/* terminal */
}

/* \doc |hsh_destroy| frees all of the memory associated with the hash
   table.

   The memory used by keys and data is \emph{not} freed---this memory is
   the responsibility of the user.  However, a call to |hsh_iterate|
   can be used to free this memory \emph{immediately} before a call to
   |hsh_destroy|. */

void hsh_destroy( hsh_HashTable table )
{
   _hsh_check( table, __FUNCTION__ );
   if (((tableType)table)->readonly)
      err_internal( __FUNCTION__, "Attempt to destroy readonly table\n" );
   _hsh_destroy_buckets( table );
   _hsh_destroy_table( table );
}

static void _hsh_insert( hsh_HashTable table,
			 unsigned long hash,
			 const void *key,
			 const void *datum )
{
   tableType     t = (tableType)table;
   unsigned long h = hash % t->prime;
   bucketType    b;

   _hsh_check( t, __FUNCTION__ );
   
   b        = xmalloc( sizeof( struct bucket ) );
   b->key   = key;
   b->hash  = hash;
   b->datum = datum;
   b->next  = NULL;
   
   if (t->buckets[h]) b->next = t->buckets[h];
   t->buckets[h] = b;
   ++t->entries;
}

/* \doc |hsh_insert| inserts a new |key| into the |table|.  If the
   insertion is successful, zero is returned.  If the |key| already exists,
   1 is returned.  Hence, the way to change the |datum| associated with a
   |key| is first to call |hsh_delete|.

   If the internal representation of the hash table becomes more than half
   full, its size is increased automatically.  At present, this requires
   that all of the key pointers are copied into a new table.  Rehashing is
   not required, however, since the hash values are stored for each key. */

int hsh_insert( hsh_HashTable table,
		const void *key,
		const void *datum )
{
   tableType     t         = (tableType)table;
   unsigned long hashValue = t->hash( key );
   unsigned long h;

   _hsh_check( t, __FUNCTION__ );
   if (t->readonly)
      err_internal( __FUNCTION__, "Attempt to insert into readonly table\n" );
   
				/* Keep table less than half full */
   if (t->entries * 2 > t->prime) {
      tableType     new = _hsh_create( t->prime * 3, t->hash, t->compare );
      unsigned long i;

      for (i = 0; i < t->prime; i++) {
	 if (t->buckets[i]) {
	    bucketType pt;

	    for (pt = t->buckets[i]; pt; pt = pt->next)
		  _hsh_insert( new, pt->hash, pt->key, pt->datum );
	 }
      }

				/* fixup values */
      _hsh_destroy_buckets( t );
      t->prime   = new->prime;
      t->buckets = new->buckets;
      _hsh_destroy_table( new );
      ++t->resizings;
   }

   h = hashValue % t->prime;

   if (t->buckets[h]) {		/* Assert uniqueness */
      bucketType pt;
      
      for (pt = t->buckets[h]; pt; pt = pt->next)
	    if (!t->compare( pt->key, key )) return 1;
   }

   _hsh_insert( t, hashValue, key, datum );
   return 0;
}

/* \doc |hsh_delete| removes a |key| and the associated datum from the
   |table|.  Zero is returned if the |key| was present.  Otherwise, 1 is
   returned. */

int hsh_delete( hsh_HashTable table, const void *key )
{
   tableType     t = (tableType)table;
   unsigned long h = t->hash( key ) % t->prime;

   _hsh_check( t, __FUNCTION__ );
   if (t->readonly)
      err_internal( __FUNCTION__, "Attempt to delete from readonly table\n" );
   
   if (t->buckets[h]) {
      bucketType pt;
      bucketType prev;
      
      for (prev = NULL, pt = t->buckets[h]; pt; prev = pt, pt = pt->next)
	    if (!t->compare( pt->key, key )) {
	       --t->entries;
	       
	       if (!prev) t->buckets[h] = pt->next;
	       else       prev->next = pt->next;
	       
	       xfree( pt );
	       return 0;
	    }
   }
   
   return 1;
}


/* \doc |hsh_retrieve| retrieves the datum associated with a |key|.  If the
   |key| is not present in the |table|, then "NULL" is returned. */

const void *hsh_retrieve( hsh_HashTable table,
			  const void *key )
{
   tableType     t = (tableType)table;
   unsigned long h = t->hash( key ) % t->prime;

   _hsh_check( t, __FUNCTION__ );
   
   ++t->retrievals;
   if (t->buckets[h]) {
      bucketType pt;
      bucketType prev;
      
      for (prev = NULL, pt = t->buckets[h]; pt; prev = pt, pt = pt->next)
	    if (!t->compare( pt->key, key )) {
	       if (!prev) {
		  ++t->hits;
	       } else if (!t->readonly) {
				/* Self organize */
		  prev->next    = pt->next;
		  pt->next      = t->buckets[h];
		  t->buckets[h] = pt;
	       }
	       return pt->datum;
	    }
   }

   ++t->misses;
   return NULL;
}

/* \doc |hsh_iterate| is used to iterate a function over every value in the
   |table|.  The function, |iterator|, is passed the |key| and |datum| pair
   for each entry in the table.  If |iterator| returns a non-zero value,
   the iterations stop, and |hsh_iterate| returns non-zero.  Note that the
   keys are in some arbitrary order, and that this order may change between
   two successive calls to |hsh_iterate|. */

int hsh_iterate( hsh_HashTable table,
		 int (*iterator)( const void *key,
				  const void *datum ) )
{
   tableType     t = (tableType)table;
   unsigned long i;
   bucketType    pt;
   bucketType    next;		/* Save, because pt might vanish. */

   _hsh_check( t, __FUNCTION__ );
   
   for (i = 0; i < t->prime; i++) {
      if (t->buckets[i]) {
	 for (pt = t->buckets[i]; pt; pt = next) {
	    next = pt->next;
	    if (iterator( pt->key, pt->datum ))
	       return 1;
	 }
      }
   }
   return 0;
}

/* \doc |hsh_iterate_arg| is used to iterate a function over every value in
   the |table|.  The function, |iterator|, is passed the |key| and |datum|
   pair for each entry in the table.  If |iterator| returns a non-zero
   value, the iterations stop, and |hsh_iterate| returns non-zero.  Note
   that the keys are in some arbitrary order, and that this order may
   change between two successive calls to |hsh_iterate|. */

int hsh_iterate_arg( hsh_HashTable table,
		     int (*iterator)( const void *key,
				      const void *datum,
				      void *arg ),
		     void *arg )
{
   tableType     t = (tableType)table;
   unsigned long i;
   bucketType    pt;
   bucketType    next;		/* Save, because pt might vanish. */

   _hsh_check( t, __FUNCTION__ );
   
   for (i = 0; i < t->prime; i++) {
      if (t->buckets[i]) {
	 for (pt = t->buckets[i]; pt; pt = next) {
	    next = pt->next;
	    if (iterator( pt->key, pt->datum, arg ))
	       return 1;
	 }
      }
   }
   return 0;
}

/* a function callable from hsh_iterate() to print key values */
static int _hsh_key_strings( const void *k, const void *d ) {
   const char *s;
   static int i = 0;
   
   if (k == NULL) { i=0; return 0; }
   
   s = k;
   printf("%s  ",s);
   if ((i += strlen(s)+2) >= 60) { i=0; printf("\n"); }
   return 0;
}

/* print all keys in table t as strings */
void hsh_key_strings(hsh_HashTable t) {
   _hsh_key_strings(NULL,NULL);
   hsh_iterate(t,_hsh_key_strings);
   printf("\n");
}

/* \doc |hsh_get_stats| returns statistics about the |table|.  The
   |hsh_Stats| structure is shown in \grind{hsh_Stats}. */

hsh_Stats hsh_get_stats( hsh_HashTable table )
{
   tableType     t = (tableType)table;
   hsh_Stats     s = xmalloc( sizeof( struct hsh_Stats ) );
   unsigned long i;
   int           count;

   _hsh_check( t, __FUNCTION__ );
   
   s->size           = t->prime;
   s->resizings      = t->resizings;
   s->entries        = 0;
   s->buckets_used   = 0;
   s->singletons     = 0;
   s->maximum_length = 0;
   s->retrievals     = t->retrievals;
   s->hits           = t->hits;
   s->misses         = t->misses;
   
   for (i = 0; i < t->prime; i++) {
      if (t->buckets[i]) {
	 bucketType pt;
	 
	 ++s->buckets_used;
	 for (count = 0, pt = t->buckets[i]; pt; ++count, pt = pt->next);
	 if (count == 1) ++s->singletons;
	 s->maximum_length = max( s->maximum_length, count );
	 s->entries += count;
      }
   }
   if (t->entries != s->entries )
	 err_internal( __FUNCTION__,
		       "Incorrect count for entries: %lu vs. %lu\n",
		       t->entries,
		       s->entries );

   return s;
}

/* \doc |hsh_print_stats| prints the statistics for |table| on the
   specified |stream|.  If |stream| is "NULL", then "stdout" will be
   used. */

void hsh_print_stats( hsh_HashTable table, FILE *stream )
{
   FILE      *str = stream ? stream : stdout;
   hsh_Stats s    = hsh_get_stats( table );

   _hsh_check( table, __FUNCTION__ );
   
   fprintf( str, "Statistics for hash table at %p:\n", table );
   fprintf( str, "   %lu resizings to %lu total\n", s->resizings, s->size );
   fprintf( str, "   %lu entries (%lu buckets used, %lu without overflow)\n",
	    s->entries, s->buckets_used, s->singletons );
   fprintf( str, "   maximum list length is %lu", s->maximum_length );
   if (s->buckets_used)
	 fprintf( str, " (optimal is %.1f)\n",
		  (double)s->entries / (double)s->buckets_used );
   else
	 fprintf( str, "\n" );
   fprintf( str, "   %lu retrievals (%lu from top, %lu failed)\n",
	    s->retrievals, s->hits, s->misses );

   xfree( s );			/* rare */
}

unsigned long hsh_string_hash( const void *key )
{
   const char           *pt = (const char *)key;
   unsigned long        h  = 0;
   static const char    *prev_pt = NULL;
   static unsigned long prev_h = 0;

   if (!pt)
      err_internal( __FUNCTION__, "String-valued keys may not be NULL\n" );

   if (prev_pt == pt) return prev_h;

   while (*pt) {
      h += *pt++;
#if 0
      h *= 65599L;		/* prime near %$2^{16}$% */
#else
      h *= 2654435789U;		/* prime near %$\frac{\sqrt{5}-1}{2}2^{32}$% */
#endif
   }

   prev_pt = pt;
   return prev_h = h;
}

unsigned long hsh_pointer_hash( const void *key )
{
   const char           *pt;
   unsigned long        h   = 0;
   int                  i;
   static const char    *prev_pt = NULL;
   static unsigned long prev_h = 0;

#ifdef WORDS_BIGENDIAN
   pt = ((const char *)&key) + SIZEOF_VOID_P - 1;
#else
   pt = (const char *)&key;
#endif
   
   if (key == prev_pt) return prev_h;
   
   for (i = 0; i < SIZEOF_VOID_P; i++) {
#ifdef WORDS_BIGENDIAN
      h += *pt--;
#else
      h += *pt++;
#endif
#if 0
      h *= 65599L;		/* prime near %$2^{16}$% */
#else
      h *= 2654435789U;		/* prime near %$\frac{\sqrt{5}-1}{2}2^{32}$% */
#endif
   }

   prev_pt = pt;
   return prev_h = h;
}

int hsh_string_compare( const void *key1, const void *key2 )
{
   if (!key1 || !key2)
      err_internal( __FUNCTION__,
		    "String-valued keys may not be NULL: key1=%p, key2=%p\n",
		    key1, key2 );
   return strcmp( (const char*)key1, (const char*)key2 );
}

int hsh_pointer_compare( const void *key1, const void *key2 )
{
   const char *p1 = (const char *)&key1;
   const char *p2 = (const char *)&key2;
   int  i;

   for (i = 0; i < SIZEOF_VOID_P; i++) if (*p1++ != *p2++) return 1;
   return 0;
}

/* \doc |hsh_init_position| returns a position marker for some arbitary
   first element in the table.  This marker can be used with
   |hsh_next_position| and |hsh_get_position|. */

hsh_Position hsh_init_position( hsh_HashTable table )
{
   tableType     t = (tableType)table;
   unsigned long i;

   _hsh_check( t, __FUNCTION__ );
   for (i = 0; i < t->prime; i++) if (t->buckets[i]) {
      t->readonly = 1;
      return t->buckets[i];
   }
   return NULL;
}

/* \doc |hsh_next_position| returns a position marker for the next element
   in the table.  Elements are in arbitrary order based on their positions
   in the hash table. */

hsh_Position hsh_next_position( hsh_HashTable table, hsh_Position position )
{
   tableType     t = (tableType)table;
   bucketType    b = (bucketType)position;
   unsigned long i;
   unsigned long h;

   _hsh_check( t, __FUNCTION__ );
   
   if (!b) {
      t->readonly = 0;
      return NULL;
   }
   
   if (b->next) return b->next;

   for (h = b->hash % t->prime, i = h + 1; i < t->prime; i++)
      if (t->buckets[i]) return t->buckets[i];

   t->readonly = 0;
   return NULL;
}

/* \doc |hsh_get_position| returns the datum associated with the |position|
   marker, or "NULL" if there is no such datum.  |key| is set to the key
   associated with this datum, or "NULL" is there is no such datum. */

void *hsh_get_position( hsh_Position position, void **key )
{
   bucketType b = (bucketType)position;

   *key = NULL;
   if (!b) return NULL;

   *key = (void *)b->key;	/* Discard const */
   return (void *)b->datum;	/* Discard const */
}

/* \doc |hsh_readonly| sets the |readonly| flag for the |table| to |flag|.
   |flag| should be 0 or 1.  The value of the previous flag is returned.
   When a hash table is marked as readonly, self-organization of the
   bucket-overflow lists will not take place, and any attempt to modify the
   list (e.g., insertion or deletion) will result in an error. */

int hsh_readonly( hsh_HashTable table, int flag )
{
   tableType t = (tableType)table;
   int       current;

   _hsh_check( t, __FUNCTION__ );

   current     = t->readonly;
   t->readonly = flag;
   return current;
}
