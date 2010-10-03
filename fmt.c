/* fmt.c -- 
 * Created: Sun Mar 16 11:38:57 1997 by faith@cs.unc.edu
 * Revised: Fri Jul 11 21:13:38 1997 by faith@acm.org
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
 * $Id: fmt.c,v 1.6 1997/07/12 02:59:42 faith Exp $
 * 
 */

#include "webfmt.h"
#include <ctype.h>

static FILE          *dct;
static FILE          *idx;
static int           indent;
static int           offset;
static stk_Stack     stk;
static unsigned long filePos;
static int           hwcount, entrycount;

static struct line {
   int  c;
   int  indent;
   int  special;
} line[1024];

#define MAXLINE      65
#define FMT_CHAR     0
#define FMT_NEWLINE  1
#define FMT_LITERAL  2

static void _fmt_shift( int end )
{
   int i;

   for (i = 0; i < offset-end-1; i++) {
      line[i].c       = line[i+end+1].c;
      line[i].indent  = line[i+end+1].indent;
      line[i].special = line[i+end+1].special;
   }
   offset = offset - end - 1;
}

static int _fmt_check( int flag )
{
   int        i, j;
   static int curpos = 0;
   int        next;
   
   if (!flag && offset < 2 * MAXLINE) return 0;

   for (next = 1; next < offset; next++) {
      if (line[next].c == ' ' || line[next].special == FMT_NEWLINE)
	 break;
   }
   if (next+curpos >= MAXLINE) {
      fputc('\n',dct);
      curpos = 0;
   }
	 
   for (i = 0; i < offset; i++) {
      if (line[i].special == FMT_LITERAL) {
	 for (next = i+1; next < offset; next++) {
	    if (line[next].c == ' ' || line[next].special == FMT_NEWLINE)
	       break;
	 }
	 if (next-i+curpos >= MAXLINE || line[i].c == '\n') {
	    fputc('\n',dct);
	    goto newline;
	 }
	 while (curpos < line[i].indent) {
	    fputc(' ',dct);
	    ++curpos;
	 }
	 fputc(line[i].c,dct);
	 ++curpos;
      } else if (line[i].c == ' ') {
	 for (next = i+1; next < offset; next++) {
	    if (line[next].c == ' ' || line[next].special == FMT_NEWLINE)
	       break;
	 }
	 if (next == i+1) continue;
	 if (next-i+curpos >= MAXLINE) {
	    fputc('\n',dct);
	    goto newline;
	 } else if (curpos && curpos >= line[i].indent) {
	    if (!line[i].c) err_internal( __FUNCTION__, "Null\n" );
	    if (i<offset-1 && (line[i+1].c == ',' || line[i+1].c == ' '))
	       continue;
	    if (i<offset-3
		&& line[i+1].c == '-' && line[i+2].c == '-'
		&& line[i+3].c == ' '
		&& line[i+4].special == FMT_NEWLINE) continue;
	    fputc(' ',dct);
	    ++curpos;
	 }
      } else if (line[i].special == FMT_NEWLINE) {
	 if (dbg_test(DBG_NEWLINE))
	    printf("[%d,%dn]",line[i].c,line[i].indent);
	 for (j = 0; j < line[i].c; j++) fputc('\n',dct);
	 goto newline;
      } else {
	 if (line[i].c != ',' && line[i].c != ';') {
	    for (j = curpos; j < line[i].indent; j++) {
	       fputc(' ',dct);
	       ++curpos;
	    }
	 }
	 if (!line[i].c) err_internal( __FUNCTION__, "Null\n" );
	 if (line[i].c == '}'
	     && (!i || line[i-1].c == ' ' || line[i-1].c == ',')) continue;
	 if (line[i].c == '-') {
	    if (i<offset-1 && line[i+1].special == FMT_NEWLINE) continue;
	    if (i<offset-2) {
	       if (line[i+1].c == '-' && line[i+2].special == FMT_NEWLINE)
		  continue;
	       if (line[i+1].c == ' ' && line[i+2].special == FMT_NEWLINE)
		  continue;
	    }
	    if (i<offset-3
		&& line[i+1].c == '-' && line[i+2].c == ' '
		&& line[i+3].special == FMT_NEWLINE) continue;
	 }
	 fputc(line[i].c,dct);
	 ++curpos;
      }
   }
   offset = 0;
   return 1;

 newline:
   curpos = 0;
   _fmt_shift(i);
   return 0;
}

static void _fmt_flush( void )
{
   _fmt_check(0);
   while (!_fmt_check(1));
   fflush(dct);
   assert(!offset);
}

static void _fmt_line( int c, int ind, int special )
{
   if (!c) err_internal( __FUNCTION__, "Null\n" );

   switch (special) {
   case FMT_NEWLINE:
      if (offset && line[offset-1].special == FMT_NEWLINE) {
				/* Condense */
	 if (line[offset-1].c < c) line[offset-1].c = c;
	 line[offset-1].indent = ind;
      } else {
	 line[offset].c       = c;
	 line[offset].indent  = ind;
	 line[offset].special = special;
	 ++offset;
      }
      break;
   case FMT_LITERAL:
      line[offset].c       = (isspace(c) && c != '\n') ? ' ' : c;
      line[offset].indent  = ind;
      line[offset].special = special;
      ++offset;
      break;
   default:
      if (isspace(c) && offset
	  && (line[offset-1].c == ' '
	      || line[offset-1].special == FMT_NEWLINE)) {
	    line[offset-1].indent  = ind;
      } else {
	 line[offset].c       = isspace(c) ? ' ' : c;
	 line[offset].indent  = ind;
	 line[offset].special = special;
	 ++offset;
      }
      break;
   }
   
   _fmt_check(0);
}

void fmt_newline( int count )
{
   _fmt_line( count, indent, FMT_NEWLINE );
}

void fmt_string( const char *format, ... )
{
   va_list     ap;
   char        buf[BUFFERSIZE];
   char        *pt;

   va_start(ap, format);
   vsprintf(buf, format, ap);
   va_end(ap);

   for (pt = buf; *pt; ++pt) _fmt_line( *pt, indent, FMT_CHAR );
}

void fmt_literal( const char *format, ... )
{
   va_list     ap;
   char        buf[BUFFERSIZE];
   char        *pt;

   va_start(ap, format);
   vsprintf(buf, format, ap);
   va_end(ap);

   for (pt = buf; *pt; ++pt) _fmt_line( *pt, indent, FMT_LITERAL );
}

void fmt_open( const char *name )
{
   char buf[1024];
   
   if (!name) {
      dct = stdout;
      idx = stdout;
   } else {
      sprintf( buf, "%s.dict", name );
      if (!(dct = fopen( buf, "w" ))) {
	 fprintf( stderr, "Cannot open \"%s\" for write\n", buf );
	 exit( 1 );
      }
      
      sprintf( buf, "sort -df > %s.index", name );
      if (!(idx = popen( buf, "w" ))) {
	 fprintf( stderr, "Cannot open \"%s\" for write\n", buf );
	 exit( 1 );
      }
   }
}

void fmt_close( void )
{
   _fmt_flush();
   if (dct && dct != stdout) fclose(dct);
   if (idx && idx != stdout) fclose(idx);
   dct = idx = NULL;
   printf( "%12d headwords words for %12d entries, total\n", hwcount, entrycount );
   fflush( stdout );
}

int fmt_indent( int i )
{
   return indent = i;
}

int fmt_indent_add( int i )
{
   return indent += i;
}

void fmt_flush_index( void )
{
   unsigned long current;
   const char    *next;
   int           counted = 0;

   _fmt_flush();
   if (!stk) {
      filePos = ftell(dct);
      return;
   }
   current = ftell(dct);
   while ((next=stk_pop(stk))) {
      if (idx) {
	 fprintf( idx, "%s\t%s\t", next, b64_encode(filePos) );
	 fprintf( idx, "%s\n", b64_encode(current-filePos) );
#if 0
	 fprintf( idx, "%s\t%lu\t%lu\n",
		  next, filePos,
		  current-filePos);
#endif
	 if (!counted) {
	    ++entrycount;
	    ++counted;
	 }
	 ++hwcount;
	 if (hwcount && !(hwcount % 1000)) {
	    printf( "%10d headwords words for %10d entries\r", hwcount, entrycount );
	    fflush( stdout );
	 }
      }
   }
   filePos = current;
}

void fmt_add_index( const char *string )
{
   const char *pt = string;
   
   if (!stk) {
      fmt_flush_index();
      stk = stk_create();
   }
   while (isspace(*pt)) ++pt;	/* skip leading spaces */

   if (!*pt) return;		/* skip empties */
   stk_push(stk, (void *)pt);
}

const char *fmt_refmt( const char *string )
{
   char       *buf = alloca(strlen(string) * 2);
   const char *s;
   char       *d;

   for (s = string, d = buf; *s; s++) {
      if (isalnum(*s) || *s == ' ' || *s == '-' || *s == '\'') *d++ = *s;
   }
   *d = '\0';
   return str_find(buf);
}
