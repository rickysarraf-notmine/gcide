/* scan.l -- Scanner for Project Gutenberg Webster converter
 * Created: Sun Mar 16 09:26:43 1997 by faith@cs.unc.edu
 * Revised: Sun Feb 22 13:07:39 1998 by faith@acm.org
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
 * $Id: scan.l,v 1.13 1998/02/22 18:24:15 faith Exp $
 *
 * HW-like: spn, plw
 */

%option stack debug
%{
#include "webfmt.h"
#include "parse.h"
#include <string.h>

#define DEBUG 0

#if !DEBUG
#define src_line(t,l)
#define src_get(x) NULL
#define src_advance(l)
#endif

extern int  yylex( void );
extern int  yydebug;
extern void yyerror( const char *message );
extern int  database; 
static int  match(const char *buf, int length);
static void comment( const char *text, int length ); 
#define RETURN(val) do {                  \
   yylval.token.src = src_get( yyleng );  \
   return val;                            \
} while (0)
%}
%x OTHER COMMENT SUBINIT
letter                      [a-zA-Z]
digit                       [0-9]
INTEGER                     {digit}({digit})*
comment                     <--.*-->
word                        ([[:alpha:]_/\-\.\*][[:alnum:]_/\-\.\*]*)
string                      \"(([^\"\n])|\"\")*\"
badstring                   \"(([^\"\n])|\"\")*
NL                          \n
ID                          ([[:alpha:]_][[:alnum:]_]*)
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
                              comment(NULL,0);
			      yy_pop_state();
                            }
   "-"                      src_advance(yyleng); comment(yytext,yyleng);
   [^-\n]+                  src_advance(yyleng); comment(yytext,yyleng);
   {NL}                     { src_advance(yyleng);
                              comment(yytext,yyleng);
			      yy_push_state(SUBINIT);
                            }
}
<OTHER>{
"<--"                       src_advance(yyleng); yy_push_state(COMMENT);
"<hw>"                      RETURN(T_HW_START);
"</hw>\."                   RETURN(T_HW_STOP);
"</hw>"                     RETURN(T_HW_STOP);
"<altname>"                 RETURN(T_ALTNAME_START);
"</altname>"                RETURN(T_ALTNAME_STOP);
"<altnpluf>"                RETURN(T_ALTNPLUF_START);
"</altnpluf>"               RETURN(T_ALTNPLUF_STOP);
"<def>"                     RETURN(T_DEF_START);
"<def2>"                    RETURN(T_DEF_START);
"<cd>"                      RETURN(T_DEF_START);
"<sn>"                      RETURN(T_SN_START);
"</sn>"                     RETURN(T_SN_STOP);
"<sd>"                      RETURN(T_SD_START);
"</sd>"                     RETURN(T_SD_STOP);
"<stype>"                   RETURN(T_STYPE_START);
"</stype>"                  RETURN(T_STYPE_STOP);
"<plw>"                     RETURN(T_PLW_START);
"</plw>"                    RETURN(T_PLW_STOP);
"<singw>"                   RETURN(T_SINGW_START);
"</singw>"                  RETURN(T_SINGW_STOP);
"<conjf>"                   RETURN(T_CONJF_START);
"</conjf>"                  RETURN(T_CONJF_STOP);
"<adjf>"                    RETURN(T_ADJF_START);
"</adjf>"                   RETURN(T_ADJF_STOP);
"<decf>"                    RETURN(T_DECF_START);
"</decf>"                   RETURN(T_DECF_STOP);
"<wf>"                      RETURN(T_WF_START);
"</wf>"                     RETURN(T_WF_STOP);
"<asp>"                     RETURN(T_ASP_START);
"</asp>"                    RETURN(T_ASP_STOP);
"<exp>"                     RETURN(T_EXP_START);
"</exp>"                    RETURN(T_EXP_STOP);
"<root>"                    RETURN(T_ROOT_START);
"</root>"                   RETURN(T_ROOT_STOP);
"<vinc>"                    RETURN(T_VINC_START);
"</vinc>"                   RETURN(T_VINC_STOP);
"<au>"                      RETURN(T_AU_START);
"</au>"                     RETURN(T_AU_STOP);
"<q>"                       RETURN(T_Q_START);
"</q>"                      RETURN(T_Q_STOP);
"<qau>"                     RETURN(T_QAU_START);
"</qau>"                    RETURN(T_QAU_STOP);
"<note>"                    RETURN(T_NOTE_START);
"<note>[hand]"              RETURN(T_NOTE_START);
" -- <col>"                 RETURN(T_COL_START);
"-- <col>"                  RETURN(T_COL_START);
"<col>"                     RETURN(T_COL_START);
"</col>"                    RETURN(T_COL_STOP);
"<mcol>"                    RETURN(T_MCOL_START);
"</mcol>"                   RETURN(T_MCOL_STOP);
"<syn>Syn. -- "             RETURN(T_SYN_START);
"<syn>"                     RETURN(T_SYN_START);
"<ant>"                     RETURN(T_ANT_START);
"</ant>"                    RETURN(T_ANT_STOP);
"<uex>"                     RETURN(T_UEX_START);
"</uex>"                    RETURN(T_UEX_STOP);
"<contr>"                   RETURN(T_CONTR_START);
"</contr>"                  RETURN(T_CONTR_STOP);
"<chform>"                  RETURN(T_CHFORM_START);
"</chform>"                 RETURN(T_CHFORM_STOP);
" -- <usage>"               RETURN(T_USAGE_START);
"-- <usage>"                RETURN(T_USAGE_START);
"<usage>"                   RETURN(T_USAGE_START);
"<usage> --"                RETURN(T_USAGE_START);
"<person"                   RETURN(T_PERSON_START);
"</person>"                 RETURN(T_PERSON_STOP);
"<spn>"                     RETURN(T_SPN_START); 
"</spn>"                    RETURN(T_SPN_STOP); 
"<prod>"                    RETURN(T_PROD_START); 
"</prod>"                   RETURN(T_PROD_STOP); 
"<supr>"                    RETURN(T_SUPR_START); 
"</supr>"                   RETURN(T_SUPR_STOP); 
"<sups>"                    RETURN(T_SUPS_START); 
"</sups>"                   RETURN(T_SUPS_STOP); 
"<subs>"                    RETURN(T_SUBS_START); 
"</subs>"                   RETURN(T_SUBS_STOP);
"<cref>"                    RETURN(T_CREF_START);
"</cref>"                   RETURN(T_CREF_STOP);
"<break>"                   RETURN(T_BREAK); /* line break command -- rik */
"</cd> -*"                  src_advance(yyleng); /* ignore */
"<fld>(Zo[\"o]l.)</fld>"    { src_advance(yyleng); 
                              yylval.token.string="(Zool.)";
			      RETURN(T_OTHER);
                            }
"<mhw>{"                    src_advance(yyleng); /* ignore */
\ ?\}?"</mhw>,"               { src_advance(yyleng);
                              yylval.token.string=",";
			      RETURN(T_OTHER);
                            }
\}?"</mhw> "                src_advance(yyleng); /* ignore */
\}?"</mhw> <pos>"           src_advance(yyleng); /* ignore */
"<centered><point26>".*"</point26></centered>" src_advance(yyleng); /*ignore*/
\<[^->/]+\>                 { yylval.token.string
				 = str_findn(yytext+1, yyleng-2);
                    /* fprintf( stderr, "Found <%s>\n", yylval.token.string );  */
                              RETURN(T_TAG_START);
                            }
\<\/[^>]+\>                 { yylval.token.string
				 = str_findn(yytext+2, yyleng-3);
                         /* fprintf( stderr, "Found </%s>\n", yylval.token.string );  */
                              RETURN(T_TAG_STOP);
                            }
[^<\n]+                     { yylval.token.string = str_copyn(yytext, yyleng);
                              RETURN(T_OTHER);
                            }
{NL}                        { yylval.token.string = " ";
                              BEGIN(INITIAL);
			      RETURN(T_OTHER);
                            }
.                           { yylval.token.string = str_findn(yytext,yyleng);
                              BEGIN(OTHER);
			      RETURN(OTHER);
                            }
}
<<EOF>>                     RETURN(0);
%%
int yywrap( void )
{
   return 1;
}

void yyerror( const char *message )
{
   fprintf( stderr, "\nError: %*.*s\n", yyleng, yyleng, yytext );
   src_parse_error( stderr, src_get( yyleng ), message );
   err_fatal( __FUNCTION__, "parse error\n" );
#if !DEBUG
   fprintf( stderr, "For line numbers, recompile with DEBUG set to 1\n" );
#endif
   exit( 1 );
}

void comment( const char *text, int length )
{
   static int have = 0;
   static int fin  = 0;
   time_t     t;

   if (!text && !length) {
      ++fin;
      fmt_indent(0);
      fmt_newline(1); fmt_flush_index(); fmt_newline(1); fmt_flush_index();
      return;
   }

   if (fin) {
#if 0
      fprintf( stderr, " %s", str_copyn(text,length) );
#endif
      return;
   }

   if (!have) {
      ++have;
      time(&t);
      
      fmt_indent(0);
      fmt_newline(1); fmt_flush_index(); fmt_newline(1); fmt_flush_index();
      fmt_add_index( "00-database-url" );
      if (database == 1) {
         fmt_add_index( "00-gcide-url" );
         fmt_string( "00-database-url" );
         fmt_newline(1);
         fmt_indent(3);
         fmt_string( "ftp://ftp.gnu.org/gnu/dictionary" );
      } else {
         fmt_add_index( "00-web1913-url" );
         fmt_string( "00-database-url" );
         fmt_newline(1);
         fmt_indent(3);
         fmt_string( "ftp://ftp.uga.edu/pub/misc/webster/" );
      }
      fmt_indent(0);
      fmt_newline(1); fmt_flush_index(); fmt_newline(1); fmt_flush_index();
      fmt_string( "00-database-short" );
      fmt_newline(1);
      fmt_indent(3);
      fmt_add_index( "00-database-short" );
      if (database == 1) {
	 fmt_add_index( "00-gcide-short" );
	 fmt_string( "The Collaborative International Dictionary of English v.0.46" );
      } else {
	 fmt_add_index( "00-web1913-short" );
	 fmt_string( "Webster's Revised Unabridged Dictionary (1913)" );
      }
      
      fmt_indent(0);
      fmt_newline(1); fmt_flush_index(); fmt_newline(1); fmt_flush_index();
      fmt_add_index( "00-database-long" );
      if (database == 1) fmt_add_index( "00-gcide-long" );
      else               fmt_add_index( "00-web1913-long" );
      fmt_string( "00-database-long" );
      fmt_newline(1);
      fmt_indent(3);
      if (database == 1) {
	 fmt_string(
	    "The Collaborative International Dictionary of English, derived from"
	    " Webster's Revised Unabridged Dictionary, 1913, C. & G. Merriam Co.,"
	    " Springfield, Mass., under the direction of Noah Porter, D.D., LL.D.;"
	    " and from WordNet(R), a semantic network created by the Cognitive"
	    " Science Department of Princeton University under the direction of"
	    " Prof. George Miller.  Online version prepared by MICRA,"
	    " Inc., Plainfield, N.J. and edited by Patrick Cassidy"
	    " <pc@worldsoul.org>." );
         fmt_newline(1);
         fmt_indent(3);
         fmt_string( "ftp://ftp.gnu.org/gnu/dictionary" );
      } else {
	 fmt_string(
	    "Webster's Revised Unabridged Dictionary (G & C. Merriam Co.,"
	    " 1913, edited by Noah Porter).  Online version prepared by MICRA,"
	    " Inc., Plainfield, N.J. and edited by Patrick Cassidy"
	    " <cassidy@micra.com>." );
         fmt_newline(1);
         fmt_string( "ftp://ftp.uga.edu/pub/misc/webster/" );
         fmt_newline(1);
         fmt_string(
	 "ftp://uiarchive.cso.uiuc.edu/pub/etext/gutenberg/etext96/pgw*" );
         fmt_newline(1);
         fmt_string(
	 "http://humanities.uchicago.edu/forms_unrest/webster.form.html" );
      }
      fmt_indent(0);
      fmt_newline(1); fmt_flush_index(); fmt_newline(1); fmt_flush_index();
      fmt_string( "00-database-info" );
      fmt_newline(1);
      fmt_indent(3);
      fmt_add_index( "00-database-info" );
      if (database == 1) fmt_add_index( "00-web1913-info" );
      else               fmt_add_index( "00-gcide-info" );
      fmt_string("This file was converted from the original database on:" );
      fmt_indent_add(10);
      fmt_newline(1);
      fmt_string("%25.25s", ctime(&t) );
      fmt_newline(1);
      fmt_indent_add(-10);
      fmt_newline(2);
      fmt_string( "The original data is available from:" );
      fmt_indent_add(10);
      fmt_newline(1);
      if (database == 1) {
         fmt_string( "ftp://ftp.gnu.org/gnu/gcide" );
         fmt_newline(1);
         fmt_indent_add(-10);
         fmt_string ( "(However, this archive does not always contain the most recent version of the dictionary.)" );  
      } else
      fmt_string( "ftp://ftp.uga.edu/pub/misc/webster/" );
      fmt_newline(1);
      fmt_indent_add(-10);
      fmt_newline(2);
      fmt_string(
	 "The original data was distributed with the notice shown below."
	 "  No additional restrictions are claimed.  Please redistribute"
	 " this changed version under the same conditions and restriction"
	 " that apply to the original version." );
      fmt_newline(2);
      fmt_string(
	 "===============================================================" );
      fmt_newline(2);
   }

#if 0
   fprintf( stderr, "GOT \"%s\"\n", str_copyn(text,length));
#endif
   fmt_indent(0);
   fmt_literal( "%s", str_copyn(text,length) );
}
