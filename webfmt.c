/* webfmt.c -- Project Gutenberg Webster converter
 * Created: Sun Mar 16 09:29:39 1997 by faith@cs.unc.edu
 * Revised: Sun Feb 22 09:27:45 1998 by faith@acm.org
 * Copyright 1997, 1998 Rickard E. Faith (faith@acm.org)
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
 * $Id: webfmt.c,v 1.5 1998/02/22 15:14:58 faith Exp $
 * 
 */

#include "webfmt.h"
#include "parse.h"
#include <sys/utsname.h>

extern int        yy_flex_debug;
       int        database = 0;

extern FILE       *yyin;

static int        _prs_debug_flag   = 0;

static const char *id_string( const char *id )
{
   static char buffer[256];
   arg_List a = arg_argify( id, 0 );

   if (arg_count(a) >= 2)
      sprintf( buffer, "%s", arg_get( a, 2 ) );
   else
      buffer[0] = '\0';
   arg_destroy( a );
   return buffer;
}

static const char *pgw_get_banner( void )
{
   static char       *buffer= NULL;
   const char        *id = "$Id: webfmt.c,v 1.5 1998/02/22 15:14:58 faith Exp $";
   struct utsname    uts;
   
   if (buffer) return buffer;
   uname( &uts );
   buffer = xmalloc(256);
   sprintf( buffer,
	    "%s %s (%s %s)", err_program_name(), id_string( id ),
	    uts.sysname,
	    uts.release );
   return buffer;
}

static void banner( void )
{
   fprintf( stderr, "%s\n", pgw_get_banner() );
   fprintf( stderr,
	    "Copyright 1997 Rickard E. Faith (faith@cs.unc.edu)\n" );
}

static void license( void )
{
   static const char *license_msg[] = {
     "",
     "This program is free software; you can redistribute it and/or modify it",
     "under the terms of the GNU General Public License as published by the",
     "Free Software Foundation; either version 1, or (at your option) any",
     "later version.",
     "",
     "This program is distributed in the hope that it will be useful, but",
     "WITHOUT ANY WARRANTY; without even the implied warranty of",
     "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU",
     "General Public License for more details.",
     "",
     "You should have received a copy of the GNU General Public License along",
     "with this program; if not, write to the Free Software Foundation, Inc.,",
     "675 Mass Ave, Cambridge, MA 02139, USA.",
   };
   const char        **p = license_msg;
   
   banner();
   while (*p) fprintf( stderr, "   %s\n", *p++ );
}
    
static void help( void )
{
   static const char *help_msg[] = {
      "-h --help            give this help",
      "-L --license         display software license",
      "-v --verbose         verbose mode",
      "-V --version         display version number",
      "-d --debug <option>  select debug option",
      "-c                   GCIDE",
      0 };
   const char        **p = help_msg;

   banner();
   while (*p) fprintf( stderr, "%s\n", *p++ );
}

void prs_set_debug( int debug_flag )
{
   _prs_debug_flag = debug_flag;
}

void prs_stream( FILE *str, const char *name )
{
   yyin = str;
   src_new_file( name );
   yydebug = _prs_debug_flag;
   yyparse();
}

int main( int argc, char **argv )
{
   int                c;
   struct option      longopts[]  = {
      { "verbose", 0, 0, 'v' },
      { "version", 0, 0, 'V' },
      { "debug",   1, 0, 'd' },
      { "help",    0, 0, 'h' },
      { "license", 0, 0, 'L' },
      { "gcide",    0, 0, 'c' },
      { 0,         0, 0,  0  }
   };

   maa_init(argv[0]);
   dbg_register(DBG_VERBOSE, "verbose");
   dbg_register(DBG_PARSE,   "parse");
   dbg_register(DBG_SCAN,    "scan");
   dbg_register(DBG_NEWLINE, "newline");

   while ((c = getopt_long( argc, argv,
			    "vVd:hLc", longopts, NULL )) != EOF)
      switch (c) {
      case 'v': dbg_set( "verbose" ); break;
      case 'V': banner(); exit(1);    break;
      case 'd': dbg_set( optarg );    break;
      case 'L': license(); exit(1);   break;
      case 'c': database = 1;         break;
      case 'h':
      default:  help(); exit(1);      break;
      }

   if (dbg_test(DBG_PARSE))     prs_set_debug(1);
   if (dbg_test(DBG_SCAN))      yy_flex_debug = 1;
   else                         yy_flex_debug = 0;

   if (database == 1) fmt_open(dbg_test(DBG_VERBOSE) ? NULL : "gcide");
   else               fmt_open(dbg_test(DBG_VERBOSE) ? NULL : "web1913");
   prs_stream( stdin, "[stdin]" );
   fmt_close();
   return 0;
}
