/* stack.c -- Stack routines for Khepera
 * Created: Wed Nov  9 19:40:00 1994 by faith@cs.unc.edu
 * Revised: Sun Nov 19 13:30:19 1995 by faith@cs.unc.edu
 * Copyright 1994, 1995 Rickard E. Faith (faith@cs.unc.edu)
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
 * $Id: stack.c,v 1.4 1996/02/26 15:39:12 faith Exp $
 *
 * \section{Stack Routines}
 *
 * \intro The stack routines provide support for a general stack of
 * pointers to "void".  Because of the simplicity of the stack object, no
 * statistics are maintained.  (Althought the list routines can also be used
 * as a stack, the stack implemented here is more efficient.)
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

typedef struct data {
   const void  *datum;
   struct data *prev;
} *dataType;

typedef struct stack {
   struct data    *data;
   struct obstack *obstack;
} *stackType;

/* \doc |stk_create| initializes a stack object. */

stk_Stack stk_create( void )
{
   stackType s;

   s          = xmalloc( sizeof( struct stack ) );
   s->data    = NULL;
   s->obstack = xmalloc( sizeof( struct obstack ) );
   
   obstack_init( s->obstack );

   return s;
}

/* \doc |stk_destroy| destroys all memory associated with the |stack|.  The
   memory used by data is \emph{not} freed---this memory is the
   responsibility of the user. */

void stk_destroy( stk_Stack stack )
{
   stackType s = (stackType)stack;

   obstack_free( s->obstack, NULL );
   xfree( s->obstack );
   xfree( stack );		/* terminal */
}

/* \doc |stk_push| places |datum| on the top of the |stack|. */

void stk_push( stk_Stack stack, void *datum )
{
   stackType s = (stackType)stack;
   dataType  d = (dataType)obstack_alloc( s->obstack, sizeof( struct data ) );

   d->datum = datum;
   d->prev  = s->data;
   s->data  = d;
}

/* \doc |stk_pop| removes the top of the |stack| and returns the pointer.
   If the |stack| is empty, |stk_pop| returns "NULL". */

void *stk_pop( stk_Stack stack )
{
   stackType  s     = (stackType)stack;
   void      *datum = NULL;

   if (s->data) {
      dataType old = s->data;

      datum = (void *)old->datum; /* Discard const */
      s->data = s->data->prev;
      obstack_free( s->obstack, old );
   }
   
   return datum;
}

/* \doc |stk_top| returns a pointer to the datum on the top of the |stack|,
   but does \emph{not} remove this datum from the |stack|.  If the |stack|
   is empty, |stk_pop| returns "NULL". */

void *stk_top( stk_Stack stack )
{
   stackType s = (stackType)stack;

   if (s->data)
	 return (void *)s->data->datum;	/* Discard const */
   
   return NULL;
}
