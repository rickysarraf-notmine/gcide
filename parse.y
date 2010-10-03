/* parse.y -- Parser for Project Gutenberg Webster Converter
 * Created: Sun Mar 16 09:28:23 1997 by faith@cs.unc.edu
 * Revised: Sun Feb 22 12:59:41 1998 by faith@acm.org
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
 * $Id: parse.y,v 1.9 1998/02/22 18:24:14 faith Exp $
 * 
 */

%{
#include "webfmt.h"
#define YYDEBUG 1
#define YYERROR_VERBOSE
static void          other(const char *string);
static void          word(const char *string, int token);
static void          def(const char *string, int token);
static void          au(const char *string, int token);
static void          math(const char *string, int token);
static void          note(int token);
static void          quote(int token);
static int           haveDef, haveMCOL;
%}

%union {
   pgwToken    token;
}

				/* Non-terminals */
%type <token> Database Token TokenList

				/* Terminals */

%token <token> T_HW_START T_HW_STOP T_TAG_START T_TAG_STOP T_OTHER
%token <token> T_DEF_START T_DEF_STOP T_SN_START T_SN_STOP T_AU_START
%token <token> T_Q_START T_Q_STOP T_QAU_START T_QAU_STOP T_AU_STOP
%token <token> T_CS_START T_NOTE_START T_COL_START T_COL_STOP
%token <token> T_ALTNAME_START T_ALTNAME_STOP T_SD_START T_SD_STOP
%token <token> T_SYN_START T_USAGE_START T_SPN_START T_SPN_STOP
%token <token> T_STYPE_START T_STYPE_STOP T_PLW_START T_PLW_STOP
%token <token> T_SINGW_START T_SINGW_STOP T_CONJF_START T_CONJF_STOP
%token <token> T_ADJF_START T_ADJF_STOP T_WF_START T_WF_STOP
%token <token> T_DECF_START T_DECF_STOP T_ALTNPLUF_START T_ALTNPLUF_STOP
%token <token> T_ASP_START T_ASP_STOP T_CONTR_START T_CONTR_STOP
%token <token> T_ANT_START T_ANT_STOP T_EXP_START T_EXP_STOP
%token <token> T_ROOT_START T_ROOT_STOP T_VINC_START T_VINC_STOP
%token <token> T_SUPR_START T_SUPR_STOP T_SUPS_START T_SUPS_STOP
%token <token> T_SUBS_START T_SUBS_STOP T_MCOL_START T_MCOL_STOP
%token <token> T_BREAK T_PERSON_START T_PERSON_STOP
%token <token> T_ER_START T_ER_STOP T_CREF_START T_CREF_STOP
%token <token> T_PROD_START T_PROD_STOP T_CHFORM_START T_CHFORM_STOP
%token <token> T_ETSEP_START T_ETSEP_STOP T_UEX_START T_UEX_STOP

%%

Database : TokenList
         ;

Token    : T_HW_START T_OTHER T_HW_STOP   { word($2.string,T_HW_START); }
         | T_HW_START T_OTHER T_SUPR_START T_OTHER T_SUPR_STOP T_HW_STOP
                                          { other($2.string);
                                            math($4.string,T_SUPR_START); }
         | T_COL_START T_OTHER T_COL_STOP { word($2.string,T_COL_START); }
         | T_CHFORM_START T_OTHER T_CHFORM_STOP
                                          { word($2.string,T_CHFORM_START); }
         | T_ALTNAME_START T_OTHER T_ALTNAME_STOP
                                          { word($2.string,T_ALTNAME_START); }
         | T_ALTNPLUF_START T_OTHER T_ALTNPLUF_STOP
                                          { word($2.string,T_ALTNPLUF_START); }
         | T_SPN_START T_OTHER T_SPN_STOP { word($2.string,T_SPN_START); }
         | T_PROD_START T_OTHER T_PROD_STOP
                                          { word($2.string,T_PROD_START); }
         | T_PROD_START T_COL_START T_OTHER T_COL_STOP T_PROD_STOP
                                          { word($2.string,T_PROD_START); }
         | T_CREF_START T_OTHER T_CREF_STOP { math($2.string,T_CREF_START); }
         | T_PERSON_START T_OTHER T_PERSON_STOP
                                          { word($2.string,T_PERSON_START); }
         | T_PERSON_START
	 | T_PERSON_STOP
         | T_STYPE_START T_SPN_START T_OTHER T_SPN_STOP T_STYPE_STOP
                                          { word($3.string,T_SPN_START); }
         | T_MCOL_START                   { fmt_newline(2); ++haveMCOL; }
         | T_MCOL_STOP                    { haveMCOL = 0; }
         | T_BREAK                        { fmt_newline(1); }
         | T_STYPE_START
	   T_OTHER
	   T_SPN_START T_OTHER T_SPN_STOP
	   T_OTHER
	   T_SPN_START T_OTHER T_SPN_STOP
	   T_OTHER
	   T_STYPE_STOP
                                          { other($2.string);
					    word($4.string,T_SPN_START);
					    other($6.string);
                                            word($8.string,T_SPN_START);
                                       	    other($10.string);
					  }
         | T_ALTNAME_START T_SPN_START T_OTHER T_SPN_STOP T_ALTNAME_STOP
                                          { word($3.string,T_SPN_START); }
         | T_STYPE_START T_OTHER T_STYPE_STOP
                                          { word($2.string,T_STYPE_START); }
         | T_PLW_START T_OTHER T_PLW_STOP { word($2.string,T_PLW_START); }
         | T_SINGW_START T_OTHER T_SINGW_STOP
                                          { word($2.string,T_SINGW_START); }
         | T_CONJF_START T_OTHER T_CONJF_STOP
                                          { word($2.string,T_CONJF_START); }
         | T_ADJF_START T_OTHER T_ADJF_STOP
                                          { word($2.string,T_ADJF_START); }
         | T_DECF_START T_OTHER T_DECF_STOP
                                          { word($2.string,T_DECF_START); }
         | T_WF_START T_OTHER T_WF_STOP   { word($2.string,T_WF_START); }
         | T_ASP_START T_OTHER T_ASP_STOP { word($2.string,T_ASP_START); }
         | T_CONTR_START T_OTHER T_CONTR_STOP
                                          { math($2.string,T_CONTR_START); }
         | T_ANT_START T_OTHER T_ANT_STOP { math($2.string,T_ANT_START); }
         | T_UEX_START T_OTHER T_UEX_STOP { math($2.string,T_UEX_START); }
         | T_USAGE_START                  { note(T_USAGE_START); }
         | T_SYN_START                    { note(T_SYN_START); }
         | T_NOTE_START                   { note(T_NOTE_START); }
         | T_SN_START T_OTHER T_SN_STOP   { def($2.string,T_SN_START); }
         | T_SD_START T_OTHER T_SD_STOP   { def($2.string,T_SD_START); }
         | T_DEF_START                    { def(NULL,T_DEF_START); }
         | T_OTHER                        { other($1.string); }
         | T_TAG_START
	 | T_TAG_STOP
	 | T_AU_START T_OTHER T_AU_STOP   { au($2.string,T_AU_START); }
	 | T_QAU_START T_OTHER T_QAU_STOP { au($2.string,T_QAU_START); }
         | T_Q_START                      { quote(T_Q_START); }
         | T_Q_STOP                       { quote(T_Q_STOP); }
	 | T_EXP_START T_OTHER T_EXP_STOP   { math($2.string,T_EXP_START); }
	 | T_ROOT_START { math($1.string,-T_ROOT_START); }
           TokenList T_ROOT_STOP { math($3.string,-T_ROOT_STOP); }
	 | T_VINC_START T_OTHER T_VINC_STOP { math($2.string,T_VINC_START); }
	 | T_SUPR_START T_OTHER T_SUPR_STOP { math($2.string,T_SUPR_START); }
	 | T_SUPS_START T_OTHER T_SUPS_STOP { math($2.string,T_SUPS_START); }
	 | T_SUBS_START T_OTHER T_SUBS_STOP { math($2.string,T_SUBS_START); }
/* The following is for the web1913.new addendum and CIDE -- should probably be removed */
/* The idea here is to ignore COL between STYPE */
         | T_STYPE_START T_COL_START T_OTHER T_COL_STOP T_STYPE_STOP
                                            { word($3.string,T_STYPE_START); }
         | T_STYPE_START T_COL_START T_OTHER T_STYPE_STOP T_COL_STOP
/* for CIDE */
                                            { word($3.string,T_STYPE_START); }
         | T_STYPE_START T_COL_START T_OTHER T_STYPE_STOP T_OTHER T_COL_STOP
                                            { other($3.string); other($5.string); }
         | T_ALTNAME_START T_COL_START T_OTHER T_COL_STOP T_ALTNAME_STOP
                                            { word($3.string,T_ALTNAME_START); }
         | T_ALTNAME_START T_TAG_START T_OTHER T_TAG_STOP T_ALTNAME_STOP
                                            { word($3.string,T_ALTNAME_START); }
         | T_ALTNAME_START T_OTHER T_COL_START T_OTHER T_COL_STOP T_OTHER T_ALTNAME_STOP
                                            { other($2.string);
					      word($4.string,T_ALTNAME_START);
					      other($6.string); }
         | T_ALTNAME_START T_OTHER T_ASP_START T_OTHER T_ASP_STOP T_OTHER T_ALTNAME_STOP
                                            { other($2.string);
					      word($4.string,T_ASP_START);
					      other($6.string); }
         | T_ALTNAME_START T_OTHER
	 T_ASP_START T_OTHER T_ASP_STOP T_OTHER
	 T_ASP_START T_OTHER T_ASP_STOP T_OTHER T_ALTNAME_STOP
                                            { other($2.string);
					      word($4.string,T_ASP_START);
					      other($6.string);
					      word($8.string,T_ASP_START);
					      other($10.string);
					    }
         | T_STYPE_START T_MCOL_START IgnoreColList T_MCOL_STOP T_STYPE_STOP
         | T_ASP_START T_HW_START T_OTHER T_HW_STOP T_ASP_STOP
                                            { word($3.string,T_ASP_START); }
         | T_ALTNAME_START T_HW_START T_OTHER T_HW_STOP T_ALTNAME_STOP
                                          { word($3.string,T_ALTNAME_START); }
         ;

IgnoreCol : T_COL_START T_OTHER T_COL_STOP { word($2.string,T_STYPE_START); }
          | T_OTHER { other($1.string); }
          ;

IgnoreColList : IgnoreCol
              | IgnoreColList IgnoreCol
              ;

TokenList : Token
          | TokenList Token
          ;
%%
static void other(const char *string)
{
   if (!string) return;
   fmt_string("%s", string);
}


static void word(const char *string, int token)
{
   const char *clean = fmt_refmt(string);

   while (*string == '|') ++string;
   
   switch (token) {
   case T_HW_START:
      fmt_indent(0);
      if (haveDef) {
	 fmt_newline(1);
	 fmt_flush_index();
	 fmt_newline(1);
	 fmt_flush_index();
      }
      other(clean);
      other(" \\");
      other(string);
      other("\\");
      fmt_indent(3);
      haveDef      = 0;
      break;
   case T_COL_START:
      fmt_indent(3);
      if (!haveMCOL) fmt_newline(2);
      other("{");
      other(string);
      other("}");
      fmt_indent(6);
      break;
   case T_ALTNAME_START:
   case T_ALTNPLUF_START:
   case T_SPN_START:		/* Also <suborder>, <ord>er, <gen>us, <var>iety ? */
   case T_PERSON_START:
   case T_STYPE_START:
   case T_PLW_START:
   case T_SINGW_START:
   case T_CONJF_START:
   case T_ADJF_START:
   case T_DECF_START:
   case T_WF_START:
   case T_ASP_START:
   case T_PROD_START:
   case T_CHFORM_START:
				/* Check for length before storing. */
      other("{");
      other(string);
      other("}");
      break;
   default: err_internal( __FUNCTION__, "token = %d\n", token );
   }
   fmt_add_index(clean);
}

static void def(const char *string, int token)
{
   static int lastSN = 3;
   
   switch (token) {
   case T_DEF_START:		/* Also <def2>? */
      if (!haveDef) { fmt_indent(3); fmt_newline(1); lastSN = 3; }
      ++haveDef;
      other(string);
      break;
   case T_SN_START:
      fmt_indent(3);
      if (haveDef) fmt_newline(2);
      else         fmt_newline(1);
      ++haveDef;
      other(string);
      lastSN = fmt_indent_add(strlen(string)+1);
      break;
   case T_SD_START:
      fmt_indent(lastSN);
      fmt_newline(1);
      ++haveDef;
      other(string);
      fmt_indent_add(strlen(string)+1);
      break;
   default: err_internal( __FUNCTION__, "token = %d\n", token );
   }
}

static void au(const char *string, int token)
{
   int old;
   
   switch (token) {
   case T_QAU_START:
      old = fmt_indent_add(0);
      fmt_indent(50);
      other("--");
      other(string);
      fmt_indent(old);
      fmt_newline(1);
      break;
   case T_AU_START:
      other("--");
      other(string);
      break;
   default: err_internal( __FUNCTION__, "token = %d\n", token );
   }
}

static void quote(int token)
{
   switch (token) {
   case T_Q_START:
      fmt_indent_add(6);
      fmt_newline(2);
      break;
   case T_Q_STOP:
      fmt_indent_add(-6);
      break;
      fmt_newline(1);
   default: err_internal( __FUNCTION__, "token = %d\n", token );
   }
}

static void note(int token)
{
   switch (token) {
   case T_SYN_START:
      fmt_indent(3);
      fmt_newline(2);
      other("Syn: ");
      fmt_indent_add(5);
      break;
   case T_USAGE_START:
      fmt_indent(3);
      fmt_newline(2);
      other("Usage: ");
      fmt_indent_add(7);
      break;
   case T_NOTE_START:
      fmt_indent(3);
      fmt_newline(2);
      other("Note: ");
      fmt_indent_add(6);
      break;
   default: err_internal( __FUNCTION__, "token = %d\n", token );
   }
}

static void math(const char *string, int token)
{
   switch (token) {
   case T_CREF_START:
   case T_CONTR_START:
   case T_ANT_START:
   case T_ETSEP_START:
   case T_UEX_START:
      other("{");
      other(string);
      other("}");
      break;
   case T_EXP_START:
   case T_SUPR_START:
   case T_SUPS_START:
      other("^{");
      other(string);
      other("}");
      break;
   case T_SUBS_START:
      other("_{");
      other(string);
      other("}");
      break;
   case T_ROOT_START:
      other("root{");
      other(string);
      other("}");
      break;
   case -T_ROOT_START:
      other("root{");
      break;
   case -T_ROOT_STOP:
      other("}");
      break;
   case T_VINC_START:
      other("vinc{");
      other(string);
      other("}");
      break;
   default: err_internal( __FUNCTION__, "token = %d\n", token );
   }
}
