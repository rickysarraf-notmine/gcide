/* parse-concrete.c -- Support for calling parsers from Libmaa, continued
 * Created: Mon Apr 24 17:40:51 1995 by faith@cs.unc.edu
 * Revised: Fri Feb 28 09:21:43 1997 by faith@cs.unc.edu
 * Copyright 1995, 1997 Rickard E. Faith (faith@cs.unc.edu)
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
 * $Id: parse-concrete.c,v 1.1 1997/02/28 14:29:59 faith Exp $
 *
 */

#include "maaP.h"

static hsh_HashTable _prs_hash;

void prs_register_concrete( const char *symbol, const char *concrete )
{
   if (!_prs_hash) _prs_hash = hsh_create( NULL, NULL );

   hsh_insert( _prs_hash, str_find( symbol ), str_find( concrete ) );
}

const char *prs_concrete( const char *symbol )
{
   if (!_prs_hash) return NULL;
   return hsh_retrieve( _prs_hash, symbol );
}

void _prs_shutdown( void )
{
   if (_prs_hash) hsh_destroy( _prs_hash );
}
