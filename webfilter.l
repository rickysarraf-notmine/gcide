/* webfilter.l -- 
 * Created: Thu Oct  3 00:51:04 1996 by faith@cs.unc.edu
 * Revised: Sun Feb 22 11:26:25 1998 by faith@acm.org
 * Copyright 1996, 1997, 1998 Rickard E. Faith (faith@acm.org)
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
 * $Id: webfilter.l,v 1.9 1998/02/22 18:24:15 faith Exp $
 */


%option stack
%{
#include "maa.h"
#include <string.h>
#include <getopt.h>

#define DBG_VERBOSE 0x0001
#define DBG_DEBUG   0x0002
#define DBG_SEARCH2 0x0004
#define DBG_SEARCH3 0x0008 
   
extern int           yylex( void );
extern int           yydebug;
extern void          yyerror( const char *message );
static stk_Stack     stk;
static hsh_HashTable entityHash, hexHash;
static void          push(const char *text, int length); 
static void          pop(const char *text, int length);
static void          entity(const char *text, int length);
static void          hex(const char *text, int length);
static void          other(const char *text, int length);
static void          comment(const char *text, int length);

typedef struct trans {
   const char *name;
   const char *rep;
   int        count;
} *trans_t;
%}
%x OTHER COMMENT SUBINIT
NL                          \n
WS                          [[:blank:]]+
%%
<INITIAL>{
   .*{NL}                   src_line(yytext,yyleng); yyless(0); BEGIN(OTHER);
   .*                       src_line(yytext,yyleng); yyless(0); BEGIN(OTHER);
}
<SUBINIT>{
   .*{NL}                   src_line(yytext,yyleng); yyless(0); yy_pop_state();
   .*                       src_line(yytext,yyleng); yyless(0); yy_pop_state();
}
<COMMENT>{
   "-->"                    { src_advance(yyleng);
                              comment(yytext,yyleng);
			      yy_pop_state();
                            }
   "-"                      src_advance(yyleng); comment(yytext,yyleng);
   [^-\n]+                  src_advance(yyleng); comment(yytext,yyleng);
   {NL}                     comment(yytext,yyleng); yy_push_state(SUBINIT);
}
<OTHER>{
"<--"                       { src_advance(yyleng);
                              other(yytext,yyleng);
			      yy_push_state(COMMENT);
                            }
"<p>"                       src_advance(yyleng); /* ignore */
"</p>"                      src_advance(yyleng); /* ignore */
"<plain>"                   src_advance(yyleng); /* ignore */
"</plain>"                  src_advance(yyleng); /* ignore */
"<xex>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</xex>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<mark>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</mark>"                   { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<er>"                      { src_advance(yyleng); other("{",1);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</er>"                     { src_advance(yyleng); other("}",1);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<gen>"                     { src_advance(yyleng); other("{",1);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</gen>"                    { src_advance(yyleng); other("}",1);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<fam>"                     { src_advance(yyleng); other("{",1);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</fam>"                    { src_advance(yyleng); other("}",1);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<ord>"                     { src_advance(yyleng); other("{",1);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</ord>"                    { src_advance(yyleng); other("}",1);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<suborder>"                { src_advance(yyleng); other("{",1);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</suborder>"               { src_advance(yyleng); other("}",1);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<it>"                      { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</it>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<i>"                       { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</i>"                      { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<sub>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</sub>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<subs>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</subs>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<sups>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</sups>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<sup>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</sup>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<supr>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</supr>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<b>"                       { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</b>"                      { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<rj>"                      { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</rj>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<colf>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</colf>"                   { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<abbr>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</abbr>"                   { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<plu>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</plu>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</cd>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<ex>"                      { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</ex>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<ety>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</ety>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<ets>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</ets>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<pos>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</pos>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<as>"                      { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</as>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<grk>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</grk>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<fld>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</fld>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<qex>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</qex>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<vmorph>"                  { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</vmorph>"                 { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<pr>"                      { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</pr>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<member>"                  { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</member>"                 { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<cs>"                      { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</cs>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<altsp>"                   { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</altsp>"                  { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<pluf>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</pluf>"                   { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<specif>"                  { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</specif>"                 { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<wordforms>"               { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</wordforms>"              { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<amorph>"                  { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</amorph>"                 { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</note>"                   { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</def>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</def2>"                   { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</usage>"                  { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"</syn>"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
\ ?"<mhw>{"\ ?              { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<mhw>"                     { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
\ ?\}?"</mhw>"\ ?           { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
\ ?"<pr>(<?/)</pr>"\ ?      { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
\ ?"<pr>(?)</pr>"\ ?        { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
\ ?"<pr>(#)</pr>"\ ?        { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
\ ?"<pr>(-n<?/)</pr>"\ ?    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); }
"<br/"                      src_advance(yyleng);  other("<break>",7);
"&fist;"                    { src_advance(yyleng);
                              if (dbg_test(DBG_DEBUG)) other(yytext,yyleng); } 
\<\/[^>]+\>		    src_advance(yyleng);  pop(yytext+2,yyleng-3);
\<[^->/]+\>		    src_advance(yyleng);  push(yytext+1,yyleng-2);
\<[^->/]+\/		    src_advance(yyleng);  entity(yytext+1,yyleng-2);
\\\'..			    src_advance(yyleng);  hex(yytext+2,yyleng-2);
[^&{}<\\\n]+               src_advance(yyleng);  other(yytext,yyleng);
{NL}                        other(yytext,yyleng); BEGIN(INITIAL);
\{                          src_advance(yyleng);  other("[",1);
\}                          src_advance(yyleng);  other("]",1);
.			    src_advance(yyleng);  other(yytext,yyleng);
}
<<EOF>>			    return 0;
%%
					   
int yywrap( void )
{
   return 1;
}

void yyerror( const char *message )
{
   src_parse_error( stderr, src_get( yyleng ), message );
   err_fatal( __FUNCTION__, "parse error\n" );
   exit( 1 );
}

static void entity( const char *text, int length )
{
   const char *buf = str_findn(text,length);
   char       buf2[512];
   trans_t    t;

   if (!(t = (trans_t)hsh_retrieve(entityHash,buf))) {
      t = xmalloc(sizeof(struct trans));
      memset((char *)t,0,sizeof(struct trans));
      hsh_insert(entityHash,buf,t);
      if (dbg_test(DBG_DEBUG)) {
	       sprintf( buf2, "WARNING: Adding %s to entity table ******",buf);
	       src_parse_error( stderr, src_get( yyleng ), buf2 );
      }
   }
   ++t->count;

   if (!dbg_test(DBG_DEBUG)) {
      if (t->rep)
	 printf( "%s", t->rep);
      else
	 printf( "[%s]", buf);
   }
}
   
static void hex( const char *text, int length )
{
   const char *buf = str_findn(text,length);
   char       buf2[512];
   trans_t    t;

   if (!(t = (trans_t)hsh_retrieve(hexHash,buf))) {
      t = xmalloc(sizeof(struct trans));
      memset(t,0,sizeof(struct trans));
      hsh_insert(hexHash,buf,t);
      if (dbg_test(DBG_DEBUG)) {
	       sprintf( buf2, "WARNING: Adding %s to hex table ******",buf);
	       src_parse_error( stderr, src_get( yyleng ), buf2 );
      }
   }
   ++t->count;

   if (!dbg_test(DBG_DEBUG)) {
      if (t->rep)
	 printf( "%s", t->rep);
      else
	 printf( "[%s]", buf);
   }
}
   
static void other( const char *text, int length )
{
   char       *buf = alloca(length + 1);
   char       *d;
   const char *s;
   int        i;
   char       p = 0;

   if (!dbg_test(DBG_DEBUG)) {
      for (s = text, d = buf, i = 0; *s && i < length; i++, s++) {
	 if (i < length-1 && *s == ' '
	     && (s[1] == ',' || s[1] == ';' || s[1] == ' '))
	    continue;
	 *d++ = *s;
      }
      *d = '\0';
      printf( "%s", buf);
   }
}

static void comment( const char *text, int length )
{
   char       *buf = alloca(length + 1);
   char       *d;
   const char *s;
   int        i;
   char       p = 0;

   if (!dbg_test(DBG_DEBUG)) {
      for (s = text, d = buf, i = 0; *s && i < length; i++, s++) {
	 if (i < length-1 && *s == ' ' && (s[1] == ',' || s[1] == ';'))
	    continue;
	 *d++ = *s;
      }
      *d = '\0';
      printf( "%s", buf);
   }
}

static void push( const char *text, int length )
{
   const char *name;
   
   if (!dbg_test(DBG_DEBUG)) {
      printf( "%*.*s",yyleng,yyleng,yytext);
   } else {
      name = str_findn(text,length);
      stk_push(stk,(void *)name);
   }
}

static void pop( const char *text, int length )
{
   const char *name = str_findn(text,length);
   char       *want;
   char       *want2;
   char       *want3;
   char       buf[256];
   

   if (!dbg_test(DBG_DEBUG)) {
      printf( "%*.*s",yyleng,yyleng,yytext);
   } else {
      if (!(want = stk_pop(stk))) {
	 src_parse_error( stderr, src_get(yyleng), "ERROR: Stack underflow ******" );
	 return;
      }

      if (dbg_test(DBG_SEARCH2)) {
	 if (name != want) {
	    want2 = stk_pop(stk);
	    if (name != want2) {
	       stk_push(stk,want2);
	       sprintf( buf, "ERROR: Expected </%s>, but found </%s> ******",
			want, name );
	       src_parse_error( stderr, src_get( yyleng ), buf );
	    } else {
	       stk_push(stk,want);
	    }
	 }
      } else if (dbg_test(DBG_SEARCH3)) {
	 if (name != want) {
	    want2 = stk_pop(stk);
	    if (name != want2) {
	       want3 = stk_pop(stk);
	       if (name != want3) {
		  stk_push(stk,want3);
		  stk_push(stk,want2);
		  sprintf( buf, "ERROR: Expected </%s>, but found </%s> ******",
			   want, name );
		  src_parse_error( stderr, src_get( yyleng ), buf );
	       } else {
		  stk_push(stk,want2);
		  stk_push(stk,want);
	       }
	    } else {
	       stk_push(stk,want);
	    }
	 }
      } else if (name != want) {
	 sprintf( buf, "ERROR: Expected </%s>, but found </%s> ******",
		  want, name );
	 src_parse_error( stderr, src_get( yyleng ), buf );
      }
   }
}

static int printer( const void *name, const void *datum )
{
   const char *s     = (const char *)name;
   trans_t    t      = (trans_t)datum;

   if (t->rep)
      fprintf( stderr, "%10d %s => \"%s\"\n", t->count, s, t->rep );
   else
      fprintf( stderr, "%10d %s => *** NO TRANSLATION ***\n", t->count, s );
   
   return 0;
}

int main( int argc, char **argv )
{
   char       *pt;
   FILE       *str;
   char       buf[4096];
   arg_List   a;
   int        c;
   char       **v;
   trans_t    t;
   int        i;
   int        debug = 0;
   const char *filterFile = "filter.dat";

   maa_init(argv[0]);
   stk        = stk_create();
   entityHash = hsh_create(NULL,NULL);
   hexHash    = hsh_create(NULL,NULL);

   dbg_register( DBG_VERBOSE, "verbose" );
   dbg_register( DBG_DEBUG,   "debug" );
   dbg_register( DBG_SEARCH2, "search2" );
   dbg_register( DBG_SEARCH3, "search3" );

   while ((c = getopt( argc, argv, "vd:Df:" )) != EOF)
      switch(c) {
      case 'v': dbg_set( "verbose" ); break;
      case 'd': dbg_set( optarg );    break;
      case 'D': dbg_set( "debug" );   break;
      case 'f': filterFile = optarg;  break;
      }

   if ((str = fopen(filterFile,"r"))) {
      if (dbg_test(DBG_VERBOSE))
	 fprintf( stderr, "Reading data from %s\n", filterFile );
      while (fgets(buf,4096,str)) {
	 buf[strlen(buf)-1] = '\0';
	 if ((pt = strchr(buf,'#'))) *pt = '\0';
	 if (buf[0] == '\0') continue;

	 a = arg_argify(buf, ARG_NO_ESCAPE|ARG_NO_QUOTE);
	 arg_get_vector( a, &c, &v );
	 
	 if (c == 2) {
	    if (dbg_test(DBG_VERBOSE))
	       fprintf( stderr, "\"%s\" \"%s\" %d\n", v[0],v[1], c);
	    t = xmalloc(sizeof(struct trans));
	    memset((char *)t,0,sizeof(struct trans));
	    t->name = str_find(v[0]);
	    t->rep = str_find(v[1]);
	    hsh_insert(entityHash,t->name,t);
	 } else if (c == 3) {
	    if (dbg_test(DBG_VERBOSE))
	       fprintf( stderr, "\"%s\" \"%s\" \"%s\" %d\n",
			v[0],v[1], v[2], c);
	    t = xmalloc(sizeof(struct trans));
	    memset((char *)t,0,sizeof(struct trans));
	    t->name = str_find(v[0]);
	    t->rep = str_find(v[2]);
	    hsh_insert(hexHash,t->name,t);
	    
	    t = xmalloc(sizeof(struct trans));
	    memset((char *)t,0,sizeof(struct trans));
	    t->name = str_find(v[1]);
	    t->rep = str_find(v[2]);
	    hsh_insert(entityHash,t->name,t);
	 } else if (c) {
	    if (dbg_test(DBG_VERBOSE))
	       fprintf( stderr, "\"%s\" %d\n", v[0], c);
	    t = xmalloc(sizeof(struct trans));
	    memset((char *)t,0,sizeof(struct trans));
	    t->name = str_find(v[0]);
	    t->rep = NULL;
	    hsh_insert(entityHash,t->name,t);
	 }
	 arg_destroy(a);
      }
      fclose(str);
   }

   if (argc-optind >= 1) {
      for (i = optind; i < argc; i++) {
	 if (dbg_test(DBG_VERBOSE))
	    fprintf( stderr, "Opening %s\n",argv[i]);
	 if (!(str = fopen( argv[i], "r" ))) {
	    err_fatal_errno( __FUNCTION__,
			     "Cannot open \"%s\" for read\n", argv[1]);
	 }
	 if ((pt = strrchr(argv[i],'/'))) src_new_file(pt+1);
	 else 	                          src_new_file(argv[i]);
	 
	 yyrestart(str);
	 yylex();
	 fclose(str);
      }
   } else if (argc-optind == 0) {
      src_new_file("[stdin]");
      yyrestart(stdin);
      yylex();
   }

   if (dbg_test(DBG_VERBOSE)) {
      fflush(stdout);
      fprintf( stderr, "Entity table:\n" );
      hsh_iterate( entityHash, printer );
      fprintf( stderr, "Hex table:\n" );
      hsh_iterate( hexHash, printer );
   }
   return 0;
}
