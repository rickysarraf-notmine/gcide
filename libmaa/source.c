/* source.c -- Source code management for Libmaa
 * Created: Mon Dec 26 19:42:03 1994 by faith@cs.unc.edu
 * Revised: Sun Feb 22 06:51:24 1998 by faith@acm.org
 * Copyright 1994, 1995, 1996, 1997, 1998 Rickard E. Faith (faith@acm.org)
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
 * $Id: source.c,v 1.7 1998/02/22 15:13:52 faith Exp $
 *
 * \section{Source Code Management}
 *
 * \intro Most of the source code management routines are designed to be
 * called from within a lexical analyzer.  The routines support the storage
 * and retrieval of source code lines and source code token information.  A
 * single, global, source code management object is supported.
 *
 * \textbf{This documentation is incomplete.}
 *
 * \textbf{These routines have not been fully implemented.}
 *
 */

#include "maaP.h"

typedef struct source {
   const char *file;
   int        line;
   int        offset;
   int        length;
   int        index;
} sourceType;

#define INCREMENT 1000

static const char **Lines;
static int        Count;
static int        Used;
static sourceType Info;
static mem_String StringHeap;
static mem_Object InfoHeap;

/* \doc The |src_create| functions initializes the global source code
   management object.  This routine should only be called once, if at all.
   If the other routines are used as expected, |src_create| will be called
   automatically when it is needed. */

void src_create( void )
{
   if (Lines)
	 err_fatal( __FUNCTION__, "Source manager already created\n" );

   Lines      = xmalloc( (Count = INCREMENT) * sizeof( char * ) );
   StringHeap = mem_create_strings();
   InfoHeap   = mem_create_objects( sizeof( struct source ) );
}

/* \doc |src_destroy| frees all memory associated with the global source
   code management object. */

void src_destroy( void )
{
   if (!Lines) return;
   mem_destroy_objects( InfoHeap );
   mem_destroy_strings( StringHeap );
   xfree( Lines );		/* terminal */
   Lines = StringHeap = InfoHeap = NULL;
}

/* \doc |src_line| stores a |line| of length |len|. */

const char *src_line( const char *line, int len )
{
   char *pt;
   
   if (!Lines) src_create();
   
   ++Info.line;
   Info.index  = Used;
   Info.offset = 0;
   
   Lines[Used] = mem_strncpy( StringHeap, line, len );
   for (pt = (char *)Lines[Used]; *pt; pt++) if (*pt == '\t') *pt = ' ';
   
   PRINTF(MAA_SRC,("Line %d: %s",Used,Lines[Used]));
   
   if (++Used >= Count)
	 Lines = xrealloc( Lines, (Count += INCREMENT) * sizeof( char * ) );
   
   return Lines[ Used - 1 ];
}

/* \doc |src_new_file| specifies that |filename| is the name of the current
   file being read by the lexer. */

void src_new_file( const char *filename )
{
   Info.file = str_find( filename );
}

/* \doc |src_new_line| specifies that |line| is the number of the current
   line being read by the lexer.  Line numbers start at 1. */

void src_new_line( int line )
{
   Info.line = line;
}

/* \doc |src_advance| is used to advance the token offset pointer |length|
   bytes. */

void src_advance( int length )
{
   Info.offset += length;
}

void src_cpp_line( const char *line, int length )
{
   arg_List args;
   char     *tmp = alloca( length + 1 );
   int      lineno;

   strncpy( tmp, line, length );
   tmp[ length ] = '\0';

   args = arg_argify( tmp, 0 );

   if ((lineno = atoi( arg_get( args, 1 ) )) > 0) --lineno;
   else                                           lineno = 1;
   src_new_line( lineno );

				/* FIXME! This is debugging cruft to be
				   removed. */
   if (arg_count( args ) == 2) {
      PRINTF(MAA_SRC,( "lineno %s\n", arg_get( args, 1 ) ));
   } else {
      PRINTF(MAA_SRC,( "lineno %s in %s\n",
		      arg_get( args, 1 ), arg_get( args, 2 ) ));
      src_new_file( arg_get( args, 2 ) );
   }
   
   arg_destroy( args );
}

/* \doc |src_get| is used to get a |src_Type| object for the current token.
   This object contains file, line, offset, and length information which
   can be used to print parse error messages and provide detailed token
   tracking. */

src_Type src_get( int length )
{
   sourceType *new;

   if (!Lines)
	 err_fatal( __FUNCTION__, "Source manager does not exist\n" );
   
   Info.length = length;
   new = mem_get_object( InfoHeap );

   new->file   = Info.file;
   new->line   = Info.line;
   new->offset = Info.offset;
   new->length = Info.length;
   new->index  = Info.index;	/* Index into Lines array. */

   if (dbg_test(MAA_SRC)) {
      printf( "%s:%d @ %d, %d; %d\n",
	      new->file,
	      new->line,
	      new->offset,
	      new->length,
	      new->index );
   }

   src_advance( length );

   return new;
}

/* \doc |src_filename| returns the |file| associated with the specified
   |source| information. */

const char *src_filename( src_Type source )
{
   sourceType *s = (sourceType *)source;

   if (!Lines)
	 err_fatal( __FUNCTION__, "Source manager never created\n" );

   return s ? s->file : "";
}

/* \doc |src_linenumber| returns the |line| associated with the specified
   |source| information. */

int src_linenumber( src_Type source )
{
   sourceType *s = (sourceType *)source;

   if (!Lines)
	 err_fatal( __FUNCTION__, "Source manager never created\n" );

   return s ? s->line : 0;
}

/* \doc |src_offset| returns the token |offset| associated with the
   specified |source| information. */

int src_offset( src_Type source )
{
   sourceType *s = (sourceType *)source;

   if (!Lines)
	 err_fatal( __FUNCTION__, "Source manager never created\n" );

   return s ? s->offset : 0;
}

/* \doc |src_length| returns the token |length| associated with the
   specified |source| information. */

int src_length( src_Type source )
{
   sourceType *s = (sourceType *)source;

   if (!Lines)
	 err_fatal( __FUNCTION__, "Source manager never created\n" );

   return s ? s->length : 0;
}

/* \doc |src_source_line| returns the full source line associated with the
   specified |source| information. */

const char *src_source_line( src_Type source )
{
   sourceType *s = (sourceType *)source;

   if (!Lines)
	 err_fatal( __FUNCTION__, "Source manager never created\n" );

   return s ? Lines[ s->index ] : "";
}

/* \doc |src_get_stats| returns the statistics associated with the source
   code manager.  The |src_Stats| structure is shown in
   \grind{src_Stats}. */

src_Stats src_get_stats( void )
{
   src_Stats s = xmalloc( sizeof( struct src_Stats ) );

   if (Lines) {
      mem_StringStats ms = mem_get_string_stats( StringHeap );
      mem_ObjectStats mo = mem_get_object_stats( InfoHeap );
      
      s->lines_used      = Used;
      s->lines_allocated = Count;
      s->lines_bytes     = ms->bytes;
      s->tokens_total    = mo->total;
      s->tokens_reused   = mo->reused;
      s->tokens_size     = mo->size;

      xfree( ms );		/* rare */
      xfree( mo );		/* rare */
   } else {
      s->lines_used      = 0;
      s->lines_allocated = 0;
      s->lines_bytes     = 0;
      s->tokens_total    = 0;
      s->tokens_reused   = 0;
      s->tokens_size     = 0;
   }

   return s;
}

/* \doc |src_print_stats| prints the statistics about the source code
   manager to the specified |stream|.  If |stream| is "NULL", then "stdout"
   will be used. */

void src_print_stats( FILE *stream )
{
   FILE      *str = stream ? stream : stdout;
   src_Stats s    = src_get_stats();

   fprintf( str, "Statistics for source manager:\n" );
   fprintf( str, "   %d lines using %d bytes (%d allocated)\n",
	    s->lines_used, s->lines_bytes, s->lines_allocated );
   fprintf( str, "   %d tokens using %d bytes (%d reused)\n",
	    s->tokens_total, s->tokens_total * s->tokens_size,
	    s->tokens_reused );
   xfree( s );			/* rare */
}

static void _src_print_yyerror( FILE *str, const char *message )
{
   const char *pt;
   char       buf[1024];
   char       *b;
   const char *concrete;

   assert( str );
   
   if (!message) {
      fprintf( str, "(null)" );
      return;
   }

   for (pt = message; *pt; pt++) {
      if (*pt == '`') {		/* clean up character constants */
	 if (pt[1] == '`' && pt[2] && pt[3] == '\'' && pt[4] == '\'') {
	    fprintf( str, "`%c'", pt[2] );
	    pt += 4;
	 } else if (pt[1] == 'T' && pt[2] == '_') { /* replace symbols */
	    for (b = buf, ++pt; *pt && *pt != '\''; b++, pt++) *b = *pt;
	    *b = '\0';
	    if ((concrete = prs_concrete( buf )))
	       fprintf( str, "`%s'", concrete );
	    else
	       fprintf( str, "`%s'", buf );
	 } else
	    putc( *pt, str );
      } else
	 putc( *pt, str );
   }
}

/* \doc |src_print_line| prints a line of source code, as described by
   |source| to the specified |stream|.  If |stream| is "NULL", then
   "stdout" will be used. */

void src_print_line( FILE *stream, src_Type source )
{
   sourceType *s   = (sourceType *)source;
   FILE       *str = stream ? stream : stdout;

   if (s)
      fprintf( str, "%s:%d: %s", s->file, s->line, Lines[ s->index ] );
   else
      fprintf( str, "?:?: <source line not available>\n" );
}

static void _src_print_error( FILE *str, sourceType *s, int fudge )
{
   int        i;

   assert( str );

   src_print_line( str, s );
   if (s) {
      PRINTF(MAA_SRC,("s->offset = %d, fudge = %d, s->length = %d\n",
		     s->offset, fudge, s->length ));
      fprintf( str, "%s:%d: ", s->file, s->line );
      for (i = 0; i < s->offset - fudge; i++) putc( ' ', str );
      for (i = 0; i < s->length; i++) putc( '^', str );
   } else {
      fprintf( str, "?:?: " );
   }
   putc( '\n', str );
}

/* \doc |src_parse_error| will print the error |message| to the specified
   |stream|, followed by the offending line of source code, as specified by
   |source|.  If |stream| is "NULL", then "stdout" will be used.

   It is assumed that |message| has the format of |yyerror| so that
   massaging of the string can be performed to make it more readable (token
   names are assumed to start with ``T\_'' and will be changed to more
   readable names).  This massaging should be done by a user-defined
   function, since it is relatively specific to this author's coding
   conventions. */

void src_parse_error( FILE *stream, src_Type source, const char *message )
{
   sourceType *s   = (sourceType *)source;
   FILE       *str = stream ? stream : stdout;

   if (s) fprintf( str, "%s:%d: ", s->file, s->line );
   else   fprintf( str, "?:?: " );
   _src_print_yyerror( str, message );
   putc( '\n', str );

   _src_print_error( str, s, 0 );
}

/* \doc |src_print_error| will print an arbitrary error, specified as for
   |printf| by the |format| variable, to the specified |stream|, followed
   by the line of source code specified by |source|.  If |stream| is
   "NULL", then "stdout" will be used. */

void src_print_error( FILE *str, src_Type source, const char *format, ... )
{
   va_list    ap;
   sourceType *s = (sourceType *)source;

   fflush( str );
   if (format) {
      if (s)
         fprintf( str, "%s:%d: ", s->file, s->line );
      else
         fprintf( str, "?:?: " );
      va_start( ap, format );
      vfprintf( str, format, ap );
      va_end( ap );
      putc( '\n', str );
   }
   
   _src_print_error( str, s, 0 );
}

/* \doc |src_print_message| will just print a message labeled with a line. */

void src_print_message( FILE *str, src_Type source, const char *format, ... )
{
   va_list    ap;
   sourceType *s = (sourceType *)source;

   fflush( str );
   if (format) {
      if (s)
         fprintf( str, "%s:%d: ", s->file, s->line );
      else
         fprintf( str, "?:?: " );
      va_start( ap, format );
      vfprintf( str, format, ap );
      va_end( ap );
      putc( '\n', str );
   }
}
