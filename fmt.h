/* fmt.h -- 
 * Created: Sun Mar 16 11:42:20 1997 by faith@cs.unc.edu
 * Revised: Tue May 20 15:40:05 1997 by faith@acm.org
 * Copyright 1997 Rickard E. Faith (faith@cs.unc.edu)
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 1, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * $Id: fmt.h,v 1.3 1997/05/21 01:07:21 faith Exp $
 * 
 */

extern void       fmt_open( const char *name );
extern void       fmt_close( void );
extern int        fmt_indent( int indent );
extern int        fmt_indent_add( int offset );
extern void       fmt_newline( int count );
extern void       fmt_string( const char *format, ... );
extern void       fmt_literal( const char *format, ... );
extern void       fmt_flush_index( void );
extern void       fmt_add_index( const char *string );
extern const char *fmt_refmt( const char *string );
