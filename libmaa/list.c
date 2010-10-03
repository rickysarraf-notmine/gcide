/* list.c -- List routines for Khepera
 * Created: Wed Nov  9 19:40:00 1994 by faith@cs.unc.edu as stack.c
 * Updated: Tue Jul 25 13:04:50 1995 by faith@cs.unc.edu as list.c
 * Revised: Fri Nov  7 19:20:43 1997 by faith@acm.org
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
 * $Id: list.c,v 1.14 1997/11/08 01:33:09 faith Exp $
 *
 * \section{List Routines}
 *
 * \intro The list routines provide support for a general linked list
 * containting objects that are pointers to "void".  The list can be viewed
 * as a stack or queue -- data can be added to the head or the tail, but
 * can only be removed from the head (this data structure is sometimes
 * called a ``qstack'').  (If only a stack is needed, the stack routines are
 * more efficient.)
 *
 */

#include "maaP.h"

typedef struct data {
   const void  *datum;
   struct data *next;
} *dataType;

typedef struct list {
#if MAA_MAGIC
   int          magic;
#endif
   struct data  *head;
   struct data  *tail;
   unsigned int count;
} *listType;

static mem_Object mem;
static long int   _lst_allocated;

static void _lst_check( listType l, const char *function )
{
   if (!l) err_internal( function, "list is null\n" );
#if MAA_MAGIC
   if (l->magic != LST_MAGIC)
      err_internal( function,
		    "Incorrect magic: 0x%08x (should be 0x%08x)\n",
		    l->magic,
		    LST_MAGIC );
#endif
}

long int lst_total_allocated( void )
{
   return _lst_allocated;
}

/* \doc |lst_create| initializes a list object. */

lst_List lst_create( void )
{
   listType l = xmalloc( sizeof( struct list ) );

   _lst_allocated += sizeof(struct list);
#if MAA_MAGIC
   l->magic = LST_MAGIC;
#endif
   l->head  = NULL;
   l->tail  = NULL;
   l->count = 0;

   if (!mem) mem = mem_create_objects( sizeof( struct data ) );
   
   return l;
}

void _lst_shutdown( void )
{
   if (mem) mem_destroy_objects( mem );
   mem = NULL;
}

/* \doc |lst_destroy| destroys all memory associated with the |list|.  The
   memory used by data is \emph{not} freed---this memory is the
   responsibility of the user. */

void lst_destroy( lst_List list )
{
   listType l = (listType)list;
   dataType d;

   _lst_check( l, __FUNCTION__ );
   
   for (d = l->head; d;) {
      dataType next = d->next;
      mem_free_object( mem, d );
      d = next;
   }

#if MAA_MAGIC
   l->magic = LST_MAGIC_FREED;
#endif
   xfree( list );
}

/* \doc |lst_append| appends |datum| on the |list|. */

void lst_append( lst_List list, const void *datum )
{
   listType l = (listType)list;
   dataType d = mem_get_object( mem );

   _lst_allocated += sizeof(struct data);
   _lst_check( l, __FUNCTION__ );
   
   d->datum = datum;
   d->next  = NULL;
   if (l->tail) {
      assert( l->tail->next == NULL );
      l->tail->next = d;
   }
   l->tail = d;
   if (!l->head) l->head = d;
   ++l->count;
}

/* \doc |lst_push| prepends |datum| on the |list|. */

void lst_push( lst_List list, const void *datum )
{
   listType l = (listType)list;
   dataType d = mem_get_object( mem );

   _lst_allocated += sizeof(struct data);
   _lst_check( l, __FUNCTION__ );
   
   d->datum = datum;
   d->next  = l->head;
   l->head  = d;
   if (!l->tail) l->tail = d;
   ++l->count;
}

/* \doc |lst_pop| removes the first datum on the |list| and returns the
   pointer.  If the |list| is empty, |lst_pop| returns "NULL". */

void *lst_pop( lst_List list )
{
   listType l     = (listType)list;
   void     *datum = NULL;

   _lst_check( l, __FUNCTION__ );
   
   if (l->head) {
      dataType old = l->head;

      datum = (void *)old->datum; /* Discard const */
      l->head = l->head->next;
      if (!l->head) l->tail = NULL;
      --l->count;
      mem_free_object( mem, old );
   }
   
   return datum;
}

/* \doc |lst_top| returns a pointer to the datum that is the first element
   of the |list|, but does \emph{not} remove this datum from the |list|.
   If the |list| is empty, |lst_top| returns "NULL". */

void *lst_top( lst_List list )
{
   listType l = (listType)list;

   _lst_check( l, __FUNCTION__ );
   
   if (l->head)
	 return (void *)l->head->datum;	/* Discard const */
   
   return NULL;
}

/* \doc |lst_nth_get| returns a pointer to the $n$-th datum in the |list|,
   or "NULL" if the $n$th element does not exist. */

void *lst_nth_get( lst_List list, unsigned int n )
{
   listType     l = (listType)list;
   dataType     d;
   unsigned int i;
   
   _lst_check( l, __FUNCTION__ );
   
   if (n < 1 || n > l->count) return NULL;
   for (i = 1, d = l->head; i < n && d; i++, d = d->next);
   if (i != n)
      err_internal( __FUNCTION__, "Can't find element %d of %d\n",
		    n, l->count );
   return (void *)d->datum;	/* Discard const. */
}

/* \doc |lst_nth_set| locates the $n$-th datum in the |list| and replaces
   that datum with |datum|.  If the $n$th element does not exist, the
   program will halt with an error. (I.e., it is the programmer's
   responsibility to check |lst_length| and only pass valid values of |n|.) */

void lst_nth_set( lst_List list, unsigned int n, const void *datum )
{
   listType     l = (listType)list;
   dataType     d;
   unsigned int i;
   
   _lst_check( l, __FUNCTION__ );
   
   if (n < 1 || n > l->count)
      err_fatal( __FUNCTION__, "Attempt to change element %d of %d elements\n",
		 n, l->count );
   for (i = 1, d = l->head; i < n && d; i++, d = d->next);
   if (i != n)
      err_internal( __FUNCTION__, "Can't find element %d of %d\n",
		    n, l->count );
   
   d->datum = datum;
}

/* \doc |lst_member| returns non-zero if the pointer to |datum| is also a
   pointer on the list, and zero otherwise.  Note that only pointers are
   compared, so identical copies of data structures will be viewed as
   non-equal. */

int lst_member( lst_List list, const void *datum )
{
   listType l = (listType)list;
   dataType d;

   _lst_check( l, __FUNCTION__ );
   
   for (d = l->head; d; d = d->next)
      if (d->datum == datum) return 1;

   return 0;
}

/* \doc |lst_length| returns the number of elements in the list. */

unsigned int lst_length( lst_List list )
{
   listType l = (listType)list;
   
   _lst_check( l, __FUNCTION__ );
   return l->count;
}

/* \doc |lst_truncate| truncates a list to |length| elements.  If the list
   is not longer than |length|, nothing it done. */

void lst_truncate( lst_List list, unsigned int length )
{
   listType     l = (listType)list;
   dataType     d;
   dataType     next;
   unsigned int i;

   _lst_check( l, __FUNCTION__ );
   
   if (l->count <= length) return;

   if (!length) {
      next = l->head;
      l->head = l->tail = NULL;
   } else {
				/* Find new end of list */
      for (i = 1, d = l->head; i < length && d; d = d->next);
   
				/* Remember start of remainder of list */   
      next = d->next;		

				/* Truncate list */
      d->next = NULL;
      l->tail = d;
   }

				/* Free truncated portion of list */
   while (next) {
      dataType tmp = next->next;
      mem_free_object( mem, next );
      next = tmp;
      --l->count;
   }

   assert( l->count == length );
}

/* \doc |lst_truncate_position| truncates a list beyond |position| (i.e.,
   |position| is always left in the list.  If |postition| is "NULL", then
   the list is emptied.  This convention is useful when using
   |lst_last_postition| to get a marker allowing an older state of a list
   to be restored. */

void lst_truncate_position( lst_List list, lst_Position position )
{
   listType     l = (listType)list;
   dataType     d;
   dataType     next;

   _lst_check( l, __FUNCTION__ );
   
   if (!position) {
      next = l->head;
      l->head = l->tail = NULL;
   } else {
      d = position;		/* New end of list */
      next = d->next;		/* Start of remainder of list */
      
				/* Truncate */
      d->next = NULL;
      l->tail = d;
   }

				/* Free truncated portion of list */
   while (next) {
      dataType tmp = next->next;
      mem_free_object( mem, next );
      next = tmp;
      --l->count;
   }
}

/* \doc |lst_iterate| is used to iterate a function over every element in
   the |list|.  The function, |iterator|, is passed a pointer to each
   element.  If |iterator| returns a non-zero value, the iterations stop,
   and |lst_iterate| returns.  */

int lst_iterate( lst_List list, int (*iterator)( const void *datum ) )
{
   listType l = (listType)list;
   dataType d;

   _lst_check( l, __FUNCTION__ );
   
   for (d = l->head; d; d = d->next) if (iterator( d->datum )) return 1;
   return 0;
}

/* \doc |lst_iterate_arg| is used to iterate a function over every element
   in the |list|.  The function, |iterator|, is passed a pointer to each
   element.  If |iterator| returns a non-zero value, the iterations stop,
   and |lst_iterate| returns.  */

int lst_iterate_arg( lst_List list,
		     int (*iterator)( const void *datum, void *arg ),
		     void *arg )
{
   listType l = (listType)list;
   dataType d;

   _lst_check( l, __FUNCTION__ );
   
   for (d = l->head; d; d = d->next) if (iterator( d->datum, arg )) return 1;
   return 0;
}

/* \doc |lst_init_position| returns a position marker for the head of the
   list.  This marker can be used with |lst_next_position| and
   |lst_get_position|. */

lst_Position lst_init_position( lst_List list )
{
   listType l = (listType)list;

   _lst_check( l, __FUNCTION__ );
   return l->head;
}

/* \doc |lst_last_position| returns a position marker for the tail of the
   list.  This marker can be used with |lst_truncate_position| to restore a
   previous state of the list. */

lst_Position lst_last_position( lst_List list )
{
   listType l = (listType)list;

   _lst_check( l, __FUNCTION__ );
   return l->tail;
}

/* \doc |lst_next_position| returns a position marker for the element after
   the element marked by |position|, or "NULL" if |position| is the last
   element in the list. */

lst_Position lst_next_position( lst_Position position )
{
   dataType d = (dataType)position;

   if (!d) return NULL;
   return d->next;
}

/* \doc |lst_nth_position| returns a position marker for the $n$th element
   in the list, or "NULL" if the $n$th element does not exist. */

lst_Position lst_nth_position( lst_List list, unsigned int n )
{
   listType     l = (listType)list;
   dataType     d;
   unsigned int i;
   
   _lst_check( l, __FUNCTION__ );
   
   if (n < 1 || n > l->count) return NULL;
   for (i = 1, d = l->head; i < n && d; i++, d = d->next);
   if (i != n)
      err_internal( __FUNCTION__, "Can't find element %d of %d\n",
		    n, l->count );
   return d;
}

/* \doc |lst_get_position| returns the datum associated with the |position|
   marker, or "NULL" if there is no such element. */

void *lst_get_position( lst_Position position )
{
   dataType d = (dataType)position;

   if (!d) return NULL;
   return (void *)d->datum;	/* Discard const */
}

/* \doc |lst_set_position| sets the |datum| associated with the |position|
   marker. */

void lst_set_position( lst_Position position, const void *datum )
{
   dataType d = (dataType)position;

   if (d) d->datum = datum;
}



/* \doc |lst_dump| prints each |datum| on the list in hex */

static int _lst_dump_node( const void *datum )
{
   printf(" 0x%08x\n",(unsigned int)datum);
   return 0;
}

void lst_dump( lst_List l )
{
   lst_iterate(l,_lst_dump_node);
}
