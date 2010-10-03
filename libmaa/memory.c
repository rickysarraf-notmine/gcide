/* memory.c -- Memory management for Khepera
 * Created: Thu Dec 22 09:58:38 1994 by faith@cs.unc.edu
 * Revised: Sun Nov 10 12:37:24 1996 by faith@cs.unc.edu
 * Copyright 1994, 1995, 1996 Rickard E. Faith (faith@cs.unc.edu)
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
 * $Id: memory.c,v 1.7 1996/11/10 20:20:50 faith Exp $
 *
 * \section{Memory Management Routines}
 *
 * \intro The memory management routines provide simple support for string
 * object storage.  These routines are generally used as building blocks by
 * other parts of the \khepera library (e.g., string pools and abstract
 * syntax trees).
 *
 */

#include "maaP.h"
#include "obstack.h"

#ifdef DMALLOC_FUNC_CHECK
				/* Must be true functions */
#define obstack_chunk_alloc malloc
#define obstack_chunk_free  free
#else
#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free  xfree
#endif

typedef struct stringInfo {
#if MAA_MAGIC
   int            magic;
#endif   
   int            count;
   int            bytes;
   struct obstack *obstack;
} *stringInfo;

typedef struct objectInfo {
#if MAA_MAGIC
   int            magic;
#endif
   int            total;
   int            used;
   int            reused;
   int            size;
   stk_Stack      stack;	/* for free list */
   struct obstack *obstack;
} *objectInfo;


#if !MAA_MAGIC
#define _mem_magic_strings(i,function)                                        \
   do {                                                                       \
      if (!i) err_internal( function, "mem_String is null\n" );               \
   } while (0);
#else
static void _mem_magic_strings( stringInfo i, const char *function )
{
   if (!i) err_internal( function, "mem_String is null\n" );
   
   if (i->magic != MEM_STRINGS_MAGIC)
      err_internal( function,
		    "Incorrect magic: 0x%08x (should be 0x%08x)\n",
		    i->magic,
		    MEM_STRINGS_MAGIC );
}
#endif

/* \doc |mem_create_strings| creates a memory object for storing strings. */

mem_String mem_create_strings( void )
{
   stringInfo info = xmalloc( sizeof( struct stringInfo ) );

#if MAA_MAGIC
   info->magic   = MEM_STRINGS_MAGIC;
#endif
   info->count   = 0;
   info->bytes   = 0;
   info->obstack = xmalloc( sizeof( struct obstack ) );
   obstack_init( info->obstack );

   obstack_alignment_mask( info->obstack ) = 0; /* no alignment for chars */

   return info;
}

/* \doc |mem_destroy_strings| destroys the memory object returned from
   |mem_create_strings|.  All memory if freed, including that used for the
   strings.  Therefore, any pointers to strings in the table will be left
   dangling. */

void mem_destroy_strings( mem_String info )
{
   stringInfo i = (stringInfo)info;

   _mem_magic_strings( i, __FUNCTION__ );
#if MAA_MAGIC
   i->magic = MEM_STRINGS_MAGIC_FREED;
#endif
   
   obstack_free( i->obstack, NULL );
   xfree( i->obstack );		/* terminal */
   xfree( i );			/* terminal */
}

/* \doc |mem_strcpy| copies a |string| into the memory object pointed to by
   |info|. */

const char *mem_strcpy( mem_String info, const char *string )
{
   stringInfo i   = (stringInfo)info;
   int        len = strlen( string );

   _mem_magic_strings( i, __FUNCTION__ );
   
   ++i->count;
   i->bytes += len + 1;
   
   return obstack_copy0( i->obstack, string, len );
}

/* \doc |mem_strncpy| copies |len| bytes of |string| into the memory object
   pointed to by |info|.  A null is added to the end of the copied
   sequence. */

const char *mem_strncpy( mem_String info,
			 const char *string, int len )
{
   stringInfo i = (stringInfo)info;

   _mem_magic_strings( i, __FUNCTION__ );
   
   ++i->count;
   i->bytes += len + 1;
   
   return obstack_copy0( i->obstack, string, len );
}

/* \doc |mem_grow| copies |len| of |string| onto the top of the memory
   object pointed to by |info|.  Several calls to |mem_grow| should be
   followed by a single call to |mem_finish| without any intervening calls
   to other functions which modify |info|. */

void mem_grow( mem_String info, const char *string, int len )
{
   stringInfo i = (stringInfo)info;
   
   _mem_magic_strings( i, __FUNCTION__ );
   
   i->bytes += len;
   obstack_grow( i->obstack, string, len );
}

/* \doc |mem_finish| finishes the growth of the object performed by
   |mem_grow|. */

const char *mem_finish( mem_String info )
{
   stringInfo i = (stringInfo)info;
   
   _mem_magic_strings( i, __FUNCTION__ );
   
   ++i->count;
   i->bytes += 1;
   obstack_grow0( i->obstack, "", 0 );
   return obstack_finish( i->obstack );
}

/* \doc |mem_get_string_stats| returns statistics about the memory object
   pointed to by |info|.  The |mem_StringStats| structure is shown in
   \grind{mem_StringStats}. */

mem_StringStats mem_get_string_stats( mem_String info )
{
   stringInfo      i = (stringInfo)info;
   mem_StringStats s = xmalloc( sizeof( struct mem_StringStats ) );

   _mem_magic_strings( i, __FUNCTION__ );
   
   s->count = i->count;
   s->bytes = i->bytes;

   return s;
}

/* \doc |mem_print_string_stats| prints the statistics for the memory
   object pointed to by |info| on the specified |stream|.  If |stream| is
   "NULL", then "stdout" will be used. */

void mem_print_string_stats( mem_String info, FILE *stream )
{
   FILE            *str = stream ? stream : stdout;
   mem_StringStats s    = mem_get_string_stats( info );

   _mem_magic_strings( info, __FUNCTION__ );
   
   fprintf( str, "Statistics for string memory manager at %p:\n", info );
   fprintf( str, "   %d strings, using %d bytes\n", s->count, s->bytes );

   xfree( s );			/* rare */
}

#if !MAA_MAGIC
#define _mem_magic_objects(i,function) /*  */
#else
static void _mem_magic_objects( objectInfo i, const char *function )
{
   if (!i) err_internal( function, "mem_Object is null\n" );
   
   if (i->magic != MEM_OBJECTS_MAGIC)
      err_internal( function,
		    "Incorrect magic: 0x%08x (should be 0x%08x)\n",
		    i->magic,
		    MEM_OBJECTS_MAGIC );
}
#endif

/* \doc |mem_create_objects| creates a memory storage object for object of
   |size| bytes.  */

mem_Object mem_create_objects( int size )
{
   objectInfo info = xmalloc( sizeof ( struct objectInfo ) );

#if MAA_MAGIC
   info->magic   = MEM_OBJECTS_MAGIC;
#endif
   info->total   = 0;
   info->used    = 0;
   info->reused  = 0;
   info->size    = size;
   info->stack   = stk_create();
   info->obstack = xmalloc( sizeof( struct obstack ) );
   obstack_init( info->obstack );

   return info;
}

/* \doc |mem_destroy_objects| destroys the memory object returned from
   |mem_create_objects|.  All memory if freed, including that used for the
   object.  Therefore, any pointers to objects stored by |info| will be
   left dangling. */

void mem_destroy_objects( mem_Object info )
{
   objectInfo i = (objectInfo)info;
   
   _mem_magic_objects( i, __FUNCTION__ );
#if MAA_MAGIC
   i->magic = MEM_OBJECTS_MAGIC_FREED;
#endif
   
   stk_destroy( i->stack );
   obstack_free( i->obstack, NULL );
   xfree( i->obstack );		/* terminal */
   xfree( i );			/* terminal */
}

/* \doc |mem_get_object| returns a pointer to a block of memory which is
   |size| bytes long (as specified in the call to |mem_create_objects|).
   This block is either newly allocated memory, or is memory which was
   previously allocated by |mem_get_object| and subsequently freed by
   |mem_free_object|. */

void *mem_get_object( mem_Object info )
{
   objectInfo  i   = (objectInfo)info;
   void       *obj = stk_pop( i->stack );

   _mem_magic_objects( i, __FUNCTION__ );

   if (!obj) {
      obj = obstack_alloc( i->obstack, i->size );
      ++i->total;
   } else ++i->reused;

   ++i->used;
   
   return obj;
}

/* \doc |mem_get_empty_object| is exactly like |mem_get_object|, except the
   memory associated with the object is set to all zeros. */

void *mem_get_empty_object( mem_Object info )
{
   objectInfo  i    = (objectInfo)info;
   void        *obj = mem_get_object( info );

   memset( obj, 0, i->size );
   
   return obj;
}

/* \doc |mem_free_object| ``frees'' the object, |obj|, which was previously
   obtained from |mem_get_object|.  The memory associated with the object
   is not actually freed, but the object pointer is stored on a stack, and
   is available for subsequent calls to |mem_get_object|. */

void mem_free_object( mem_Object info, void *obj )
{
   objectInfo i = (objectInfo)info;

   _mem_magic_objects( i, __FUNCTION__ );

   stk_push( i->stack, obj );
   --i->used;
}

/* \doc |mem_get_object_stats| returns statistics about the memory object
   pointed to by |info|.  The |mem_ObjectStats| structure is shown in
   \grind{mem_ObjectStats}. */

mem_ObjectStats mem_get_object_stats( mem_Object info )
{
   objectInfo      i = (objectInfo)info;
   mem_ObjectStats s = xmalloc( sizeof( struct mem_ObjectStats ) );

   _mem_magic_objects( i, __FUNCTION__ );

   if (info) {
      s->total  = i->total;
      s->used   = i->used;
      s->reused = i->reused;
      s->size   = i->size;
   } else {
      s->total  = 0;
      s->used   = 0;
      s->reused = 0;
      s->size   = 0;
   }

   return s;
}

/* \doc |mem_print_object_stats| prints the statistics for the memory
   object pointed to by |info| on the specified |stream|.  If |stream| is
   "NULL", then "stdout" will be used. */

void mem_print_object_stats( mem_Object info, FILE *stream )
{
   FILE            *str = stream ? stream : stdout;
   mem_ObjectStats s    = mem_get_object_stats( info );

   _mem_magic_objects( info, __FUNCTION__ );

   fprintf( str, "Statistics for object memory manager at %p:\n", info );
   fprintf( str,
	    "   %d objects allocated, of which %d are in use\n",
	    s->total, s->used );
   fprintf( str, "   %d objects have been reused\n", s->reused );

   xfree( s );			/* rare */
}
