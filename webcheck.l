/* webcheck.l -- Perform some checking functions
 * Created: Thu Oct  3 00:51:04 1996 by faith@cs.unc.edu
 * Revised: Tue May 20 21:23:35 1997 by faith@acm.org
 * Copyright 1996, 1997 Rickard E. Faith (faith@acm.org)
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
 * $Id: webcheck.l,v 1.4 1997/05/21 23:52:04 faith Exp $
 */


%option stack
%{
#define ERROR 0
#define SEARCH 3
#include "maa.h"
#include <string.h>
extern int       yylex( void );
extern int       yydebug;
extern void      yyerror( const char *message );
static stk_Stack stk;
static void      push(const char *text, int length); 
static void      pop(const char *text, int length);

#define A          src_advance(yyleng)
#undef PRINTF
#define PRINTF(x,y)  src_advance(yyleng); printf(y)
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
   "-->"                    src_advance(yyleng); yy_pop_state();
   "-"                      src_advance(yyleng);
   [^-\n]+                  src_advance(yyleng);
   {NL}                     yy_push_state(SUBINIT);
}
<OTHER>{
"<--"                       src_advance(yyleng); yy_push_state(COMMENT);
\<Cced\/                    PRINTF( "[C cedilla]",    "\\c{C}" );
\\\'3e                      PRINTF( ">",              ".gt." );
\\\'80                      PRINTF( "[C cedilla]",    "\\c{C}" );
\<uum\/                     PRINTF( "[u umlau]",      "\\\"u" );
\\\'81                      PRINTF( "[u umlaut]",     "\\\"u" );
\\\'82                      PRINTF( "[e acute]",      "\\'e" );
\<acir\/                    PRINTF( "[a circumflex]", "\\^a" );
\\\'83                      PRINTF( "[a circumflex]", "\\^a" );
\<aum\/                     PRINTF( "[a umlaut]",     "\\\"a" );
\\\'84                      PRINTF( "[a umlaut]",     "\\\"a" );
\\\'85                      PRINTF( "[a grave]",      "\\`a" );
\<aring\/                   PRINTF( "[a ring]",       "\\aa" );
\\\'86                      PRINTF( "[a ring]",       "\\aa" );
\<cced\/                    PRINTF( "[c cedilla]",    "\\c{c}" );
\\\'87                      PRINTF( "[c cedilla]",    "\\c{c}" );
\<ecir\/                    PRINTF( "[e circumflex]", "\\^e" );
\\\'88                      PRINTF( "[e circumflex]", "\\^e" );
\<eum\/                     PRINTF( "[e umlaut]",     "\\\"e" );
\\\'89                      PRINTF( "[e umlaut]",     "\\\"e");
\<egrave\/                  PRINTF( "[e grave]",      "\\`e" );
\\\'8a                      PRINTF( "[e grave]",      "\\`e" );
\<ium\/                     PRINTF( "[i umlaut]",     "\\\"i" );
\\\'8b                      PRINTF( "[i umlaut]",     "\\\"i" );
\\\'8c                      PRINTF( "[i circumflex]", "\\^i" );
\\\'8d                      PRINTF( "[i grave]",      "\\`i" );
\\\'90                      PRINTF( "[E acute]",      "\\'E" );
\<ae\/                      PRINTF( "[ae]",           "\\ae" );
\\\'91                      PRINTF( "[ae]",           "\\ae" );
\<AE\/                      PRINTF( "[AE]",           "\\AE" );
\\\'92                      PRINTF( "[AE]",           "\\AE" );
\<ocir\/                    PRINTF( "[o circumflex]", "\\^o" );
\\\'93                      PRINTF( "[o circumflex]", "\\^o" );
\<oum\/                     PRINTF( "[o umlaut]",     "\\\"o" );
\\\'94                      PRINTF( "[o umlaut]",     "\\\"o" );
\\\'95                      PRINTF( "[o grave]",      "\\`o" );
\\\'96                      PRINTF( "[u circumflex]", "\\^u" );
\<pound\/                   PRINTF( "[pound]",        "[pound]" );
\\\'9c                      PRINTF( "[pound]",        "[pound]" );
\\\'97                      PRINTF( "[u grave]",      "\\`u" );
\\\'9a                      PRINTF( "[U umlaut]",     "\\\"U" );
\<aacute\/                  PRINTF( "[a acute]",      "\\'a");
\\\'a0                      PRINTF( "[a acute]",      "\\'a" );
\\\'a1                      PRINTF( "[i acute]",      "\\'i" );
\<oacute\/                  PRINTF( "[o acute]",      "\\'o" );
\\\'a2                      PRINTF( "[o acute]",      "\\'o" );
\<uacute\/                  PRINTF( "[u acute]",      "\\'u" );
\\\'a3                      PRINTF( "[u acute]",      "\\'u" );
\<ntil\/                    PRINTF( "\\~n",           "\\~n" );
\\\'a4                      PRINTF( "\\~n",           "\\~n" );
\<frac23\/                  PRINTF( "\\frac{2}{3}",   "\\frac{2}{3}" );
\\\'a6                      PRINTF( "\\frac{2}{3}",   "\\frac{2}{3}" );
\<frac13\/                  PRINTF( "\\frac{1}{3}",   "\\frac{1}{3}" );
\\\'a7                      PRINTF( "\\frac{1}{3}",   "\\frac{1}{3}" );
\<frac12\/                  PRINTF( "\\frac{1}{2}",   "\\frac{1}{2}" );
\\\'ab                      PRINTF( "\\frac{1}{2}",   "\\frac{1}{2}" );
\<frac14\/                  PRINTF( "\\frac{1}{4}",   "\\frac{1}{4}" );
\\\'ac                      PRINTF( "\\frac{1}{4}",   "\\frac{1}{4}" );
\<\?\/                      PRINTF( "[?]",            "[?]" );
\\\'b0                      PRINTF( "[?]",            "[?]" );
\\\'b4                      PRINTF( "\verticalarrow ","\verticalarrow " );
\<hand\/                    PRINTF( "[hand]",         "[hand]" );
\\\'b5                      PRINTF( "[hand]",         "[hand]" );
\\\'b6                      PRINTF( "\"",             "\"" );
\\\'b7                      PRINTF( "'",              "'" );
\<sec\/                     PRINTF( "'",              "'" );
\\\'b8                      PRINTF( "''",             "''" );
\\\'ba                      PRINTF( "||",             "||" );
\\\'bd                      PRINTF( "``",             "``" );
\<amac\/                    PRINTF( "[a macron]",     "\\=a" );
\\\'be                      PRINTF( "[a macron]",     "\\=a" );
\<lsquo\/                   PRINTF( "`",              "`" );
\\\'bf                      PRINTF( "`",              "`" );
\<sharp\/                   PRINTF( "[sharp]",        "[sharp]" );
\\\'c1                      PRINTF( "[sharp]",        "[sharp]" );
\<flat\/                    PRINTF( "[flat]",         "[flat]" );
\\\'c2                      PRINTF( "[flat]",         "[flat]" );
\\\'c3                      PRINTF( "--",             "--" );
\\\'c4                      PRINTF( "$-$",            "-" );
\<sect\/                    PRINTF( "[sect]",         "Sect. " );
\<th\/                      PRINTF( "[part]",         "Part " );
\\\'c5                      PRINTF( "[part]",         "Part " );
\<imac\/                    PRINTF( "[i macron]",     "\\=i" );
\\\'c6                      PRINTF( "[i macron]",     "\\=i" );
\<emac\/                    PRINTF( "[e macron]",     "\\=e" );
\\\'c7                      PRINTF( "[e macron]",     "\\=e" );
\<dsdot\/                   PRINTF( "[Sanskrit d dot]", ".d." );
\\\'c8                      PRINTF( "[Sanskrit d dot]", ".d." );
\<nsdot\/                   PRINTF( "[Sanskrit n dot]", ".n." );
\\\'c9                      PRINTF( "[Sanskrit n dot]", ".n." );
\<tsdot\/                   PRINTF( "[Sanskrit t dot]", ".t." );
\\\'ca                      PRINTF( "[Sanskrit t dot]", ".t." );
\<ecr\/                     PRINTF( "[e breve]",        "\\u{e}" );
\\\'cb                      PRINTF( "[e breve]",        "\\u{e}" );
\<icr\/                     PRINTF( "[i breve]",        "\\u{i}" );
\\\'cc                      PRINTF( "[i breve]",        "\\u{i}" );
\<ocr\/                     PRINTF( "[o breve]",        "\\u{o}" );
\\\'ce                      PRINTF( "[o breve]",        "\\u{o}" );
\\\'cf                      PRINTF( "--",               "--" );
\<mdash\/                   PRINTF( "---",              "---" );
\\\'d0                      PRINTF( "---",              "---" );
\\\'d1                      PRINTF( "[OE]",             "\\OE" );
\<oe\/                      PRINTF( "[oe]",             "\\oe" );
\\\'d2                      PRINTF( "[oe]",             "\\oe" );
\<omac\/                    PRINTF( "[o macron]",       "\\=o" );
\\\'d3                      PRINTF( "[o macron]",       "\\=o" );
\<umac\/                    PRINTF( "[u macron]",       "\\=u");
\\\'d4                      PRINTF( "[u macron]",       "\\=u" );
\<ocar\/                    PRINTF( "[o hacek]",        "\\v{o}" );
\\\'d5                      PRINTF( "[o hacek]",        "\\v{o}" );
\<aemac\/                   PRINTF( "[ae]",             "\\=\\ae" );
\\\'d6                      PRINTF( "[ae]",             "\\=\\ae" );
\<oemac\/                   PRINTF( "[oe]",             "\\=\\oe" );
\\\'d7                      PRINTF( "[oe]",             "\\=\\oe" );
\<par\/                     PRINTF( "||",               "||" );
\\\'d8                      PRINTF( "||",               "||" );
\<ucr\/                     PRINTF( "[u hacek]",        "\\u{u}" );
\\\'dc                      PRINTF( "[u hacek]",        "\\u{u}" );
\<acr\/                     PRINTF( "[a hacek]",        "\\u{a}" );
\\\'de                      PRINTF( "[a hacek]",        "\\u{a}" );
\<cre\/                     PRINTF( "[hacek]",          "\\u{}" );
\\\'dd                      PRINTF( "[hacek]",          "\\u{}" );
\<ymac\/                    PRINTF( "[y macron]",       "\\=y" );
\\\'df                      PRINTF( "[y macron]",       "\\=y" );
\<edh\/                     PRINTF( "[\\slash\\delta]", "[eth]" );
\\\'e3                      PRINTF( "[ols]",            "[osl]" );
\\\'eb                      PRINTF( "[\\slash\\delta]", "[eth]" );
\<thorn\/                   PRINTF( "[thorn]",          "[thorn]" );
\\\'ed                      PRINTF( "[thorn]",          "[thorn]" );
\<atil\/                    PRINTF( "\\~a",             "\\~a" );
\\\'ee                      PRINTF( "\\~a",             "\\~a" );
\\\'ef                      PRINTF( "[n dot]",           "\\d{n}" );
\\\'f0                      PRINTF( "[r dot]",          "\\d{r}" );
\<yogh\/                    PRINTF( "[3z]",             "[yogh]" );
\\\'f4                      PRINTF( "[3z]",             "[yogh]" );
\\\'f5                      PRINTF( "---",              "---" );
\\\'f6                      PRINTF( "[div]",            "/" );
\\\'f7                      PRINTF( "\\approx",         "[approx]" );
\<deg\/                     PRINTF( "[deg]",            "[deg]" );
\\\'f8                      PRINTF( "[deg]",            "[deg]" );
\\\'f9                      PRINTF( "\\bullet ",        "*" );
\\\'fa                      PRINTF( "\\cdot ",          "." );
\<root\/                    PRINTF( "[root]",           "[root]" );
\\\'fb                      PRINTF( "[root]",           "[root]" );

\<2dot\/                    PRINTF( "..",               ".." );
\<3dot\/                    PRINTF( "...",              "..." );
\<Dagger\/                  PRINTF( "[dag]",            "[dag]" );
\<ait\/                     PRINTF( "[ait]",            "[ait]" );
\<amc\/                     PRINTF( "[amc]",            "[amc]" );
\<colbreak\/                PRINTF( " ",                " " );
\<eit\/                     PRINTF( "[eit]",            "[eit]" );
\<epsilon\/                 PRINTF( "[epsilon]",        "[epsilon]" );
\<frac1x10\/                PRINTF( "\\frac{1}{10}",    "\\frac{1}{10}" );
\<frac1x24\/                PRINTF( "\\frac{1}{24}",    "\\frac{1}{24}" );
\<frac2x10\/                PRINTF( "\\frac{2}{10}",    "\\frac{2}{10}" );
\<fract25x100\/             PRINTF( "\\frac{25}{100}",  "\\frac{25}{100}" );
\<nsmac\/                   PRINTF( "",                 "" );
\<spasp\/                   PRINTF( "",                 "" );
\<asl\/                     PRINTF( "[a slash]",                 "" );
\<osl\/                     PRINTF( "[o slash]",                 "" );
\<adot\/                    PRINTF( "[a dot]",                 "" );
\<odot\/                    PRINTF( "[o dot]",                 "" );
\<or\/                      PRINTF( "[or]",                 "" );
\<alpha\/                   PRINTF( "[alpha]",                 "" );
\<a\/                       PRINTF( "[a]",                 "" );
\<keras\/                   PRINTF( "[keras]",                 "" );
\<min\/                     PRINTF( "[min]",                 "" );
\<asterism\/                PRINTF( "*",                 "" );
\<var\/                     PRINTF( "[var]",                 "" );
\<oomac\/                   PRINTF( "[oo macron]",                 "" );
\<breve\/                   PRINTF( "[breve]",                 "" );
\<and\/                     PRINTF( "&",                 "" );
\<ycr\/                     PRINTF( "[y breve]",                 "" );
\<GAMMA\/                   PRINTF( "[Gamma]",                 "" );
\<gamma\/                   PRINTF( "[gamma]",                 "" );
\<chi\/                     PRINTF( "[chi]",                 "" );
\<pause\/                   PRINTF( "[pause]",                 "" );
\<frac1x2500\/              PRINTF( "\\frac{1}{2500} ",                 "" );
\<frac1x6000\/              PRINTF( "\\frac{1}{6000} ",                 "" );
\<frac1x8000\/              PRINTF( "\\frac{1}{8000} ",                 "" );
\<frac15\/                  PRINTF( "\\frac{1}{5} ",                 "" );
\<dagger\/                  PRINTF( "[dagger]",                 "" );
\<ddagger\/                 PRINTF( "[ddagger]",                 "" );
\<DELTA\/                   PRINTF( "$\\delta$",                 "" );
\<frac35\/                  PRINTF( "\\frac{3}{5} ",                 "" );

\<0mac\/                    PRINTF( "",                 "" );
\<8star\/                   PRINTF( "",                 "" );
\<Aquarius\/                PRINTF( "",                 "" );
\<Aries\/                   PRINTF( "",                 "" );
\<CHI\/                     PRINTF( "",                 "" );
\<Cancer\/                  PRINTF( "",                 "" );
\<Capricorn\/               PRINTF( "",                 "" );
\<Crev\/                    PRINTF( "",                 "" );
\<EDH\/                     PRINTF( "",                 "" );
\<Jupiter\/                 PRINTF( "",                 "" );
\<LAMBDA\/                  PRINTF( "",                 "" );
\<Leo\/                     PRINTF( "",                 "" );
\<Libra\/                   PRINTF( "",                 "" );
\<OE\/                      PRINTF( "",                 "" );
\<OMEGA\/                   PRINTF( "",                 "" );
\<Omac\/                    PRINTF( "",                 "" );
\<Pisces\/                  PRINTF( "",                 "" );
\<SIGMA\/                   PRINTF( "",                 "" );
\<S\/                       PRINTF( "",                 "" );
\<Sagittarius\/             PRINTF( "",                 "" );
\<Scorpio\/                 PRINTF( "",                 "" );
\<Sun\/                     PRINTF( "",                 "" );
\<THETA\/                   PRINTF( "",                 "" );
\<Taurus\/                  PRINTF( "",                 "" );
\<UPSILON\/                 PRINTF( "",                 "" );
\<Virgo\/                   PRINTF( "",                 "" );
\<\?\?\?\/                  PRINTF( "",                 "" );
\<add\/                     PRINTF( "",                 "" );
\<adeg\/                    PRINTF( "",                 "" );
\<agrave\/                  PRINTF( "",                 "" );
\<asdd\/                    PRINTF( "",                 "" );
\<astascending\/            PRINTF( "",                 "" );
\<astdescending\/           PRINTF( "",                 "" );
\<bar\/                     PRINTF( "",                 "" );
\<beta\/                    PRINTF( "",                 "" );
\<br\/                      PRINTF( "",                 "" );
\<csdot\/                   PRINTF( "",                 "" );
\<cuberoot\/                PRINTF( "",                 "" );
\<dag\/                     PRINTF( "",                 "" );
\<dagr\/                    PRINTF( "",                 "" );
\<ddagr\/                   PRINTF( "",                 "" );
\<dele\/                    PRINTF( "",                 "" );
\<delta\/                   PRINTF( "",                 "" );
\<divby\/                   PRINTF( "",                 "" );
\<downslur\/                PRINTF( "",                 "" );
\<ecre\/                    PRINTF( "",                 "" );
\<edt\/                     PRINTF( "",                 "" );
\<emc\/                     PRINTF( "",                 "" );
\<esl\/                     PRINTF( "",                 "" );
\<eta\/                     PRINTF( "",                 "" );
\<etil\/                    PRINTF( "",                 "" );
\<ffllig\/                  PRINTF( "",                 "" );
\<filig\/                   PRINTF( "",                 "" );
\<fllig\/                   PRINTF( "",                 "" );
\<frac00\/                  PRINTF( "",                 "" );
\<frac1000x1434\/           PRINTF( "",                 "" );
\<frac110\/                 PRINTF( "",                 "" );
\<frac12x13\/               PRINTF( "",                 "" );
\<frac16\/                  PRINTF( "",                 "" );
\<frac17\/                  PRINTF( "",                 "" );
\<frac17x175\/              PRINTF( "",                 "" );
\<frac18\/                  PRINTF( "",                 "" );
\<frac1x10000\/             PRINTF( "",                 "" );
\<frac1x108719\/            PRINTF( "",                 "" );
\<frac1x12\/                PRINTF( "",                 "" );
\<frac1x20\/                PRINTF( "",                 "" );
\<frac1x216000\/            PRINTF( "",                 "" );
\<frac1x29966\/             PRINTF( "",                 "" );
\<frac1x3600\/              PRINTF( "",                 "" );
\<frac1x50000\/             PRINTF( "",                 "" );
\<frac1x60\/                PRINTF( "",                 "" );
\<frac25\/                  PRINTF( "",                 "" );
\<frac32\/                  PRINTF( "",                 "" );
\<frac34\/                  PRINTF( "",                 "" );
\<frac36\/                  PRINTF( "",                 "" );
\<frac56\/                  PRINTF( "",                 "" );
\<frac58\/                  PRINTF( "",                 "" );
\<frac59\/                  PRINTF( "",                 "" );
\<frac95\/                  PRINTF( "",                 "" );
\<gt\/                      PRINTF( "",                 "" );
\<icir\/                    PRINTF( "",                 "" );
\<integral2l\/              PRINTF( "",                 "" );
\<iota\/                    PRINTF( "",                 "" );
\<iques\/                   PRINTF( "",                 "" );
\<isl\/                     PRINTF( "",                 "" );
\<ismac\/                   PRINTF( "",                 "" );
\<kappa\/                   PRINTF( "",                 "" );
\<lambda\/                  PRINTF( "",                 "" );
\<lbrace2\/                 PRINTF( "",                 "" );
\<libra\/                   PRINTF( "",                 "" );
\<lt\/                      PRINTF( "",                 "" );
\<ltil\/                    PRINTF( "",                 "" );
\<male\/                    PRINTF( "",                 "" );
\<mercury\/                 PRINTF( "",                 "" );
\<msdot\/                   PRINTF( "",                 "" );
\<natural\/                 PRINTF( "",                 "" );
\<ndot\/                    PRINTF( "",                 "" );
\<nsm\/                     PRINTF( "",                 "" );
\<of\/                      PRINTF( "",                 "" );
\<omicron\/                 PRINTF( "",                 "" );
\<oocr\/                    PRINTF( "",                 "" );
\<para\/                    PRINTF( "",                 "" );
\<phi\/                     PRINTF( "",                 "" );
\<pi\/                      PRINTF( "",                 "" );
\<pisces\/                  PRINTF( "",                 "" );
\<rbrace2\/                 PRINTF( "",                 "" );
\<rho\/                     PRINTF( "",                 "" );
\<rot\/                     PRINTF( "",                 "" );
\<rsdot\/                   PRINTF( "",                 "" );
\<sagittarius\/             PRINTF( "",                 "" );
\<scorpio\/                 PRINTF( "",                 "" );
\<sigma\/                   PRINTF( "",                 "" );
\<sigmat\/                  PRINTF( "",                 "" );
\<tau\/                     PRINTF( "",                 "" );
\<taurus\/                  PRINTF( "",                 "" );
\<th\/                      PRINTF( "",                 "" );
\<theta\/                   PRINTF( "",                 "" );
\<times\/                   PRINTF( "",                 "" );
\<ucir\/                    PRINTF( "",                 "" );
\<udd\/                     PRINTF( "",                 "" );
\<umlaut\/                  PRINTF( "",                 "" );
\<upsilon\/                 PRINTF( "",                 "" );
\<upslur\/                  PRINTF( "",                 "" );
\<urcr\/                    PRINTF( "",                 "" );
\<uring\/                   PRINTF( "",                 "" );
\<usdot\/                   PRINTF( "",                 "" );
\<usl\/                     PRINTF( "",                 "" );
\<xi\/                      PRINTF( "",                 "" );
\<zeta\/                    PRINTF( "",                 "" );

\<p\>                   {A;}
\<\/p\>                 {A;}
\<\/[^>]+\>		{A; pop(yytext+2,yyleng-3);}
\<[^->/]+\>		{A; push(yytext+1,yyleng-2);}
\<[^->/]+\/		{A; fprintf(stderr,
				    "\nERROR in SPECIAL %*.*s\n",
				    yyleng,yyleng,yytext);
                        src_parse_error( stderr, src_get( yyleng ),
					 "Error in special" );
                        }
\\\'..			{A; fprintf(stderr,"\nERROR in ENTITY %*.*s\n",
				    yyleng,yyleng,yytext);}
[^<\\\n]+		{A; printf("%*.*s", yyleng, yyleng, yytext );}
{NL}                    printf("\n"); BEGIN(INITIAL);
.			{A; printf("%*.*s", yyleng, yyleng, yytext );}
}
<<EOF>>			printf( "\n" ); exit( 0 );
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

static void push( const char *text, int length )
{
#if ERROR
   const char *name = str_findn(text,length);

   if (!stk) {
      maa_init("filter");
      stk = stk_create();
   }
   stk_push(stk,(void *)name);
#else
   printf( "%*.*s",yyleng,yyleng,yytext);
#endif
}

static void pop( const char *text, int length )
{
#if ERROR
   const char *name = str_findn(text,length);
   char       *want;
   char       *want2;
#if SEARCH == 3
   char       *want3;
#endif
   char       buf[256];
   
   if (!stk) {
      maa_init("filter");
      stk = stk_create();
   }
   want = stk_pop(stk);

#if SEARCH == 3
   if (name != want) {
      want2 = stk_pop(stk);
      if (name != want2) {
	 want3 = stk_pop(stk);
	 if (name != want3) {
	    stk_push(stk,want3);
	    stk_push(stk,want2);
	    sprintf( buf, "\nExpected </%s>, but found </%s>", want, name );
	    src_parse_error( stderr, src_get( yyleng ), buf );
	 } else {
	    stk_push(stk,want2);
	    stk_push(stk,want);
	 }
      } else {
	 stk_push(stk,want);
      }
   }
#else
   if (name != want) {
#if SEARCH
      want2 = stk_pop(stk);
      if (name != want2) {
	 stk_push(stk,want2);
#endif
	 sprintf( buf, "\nExpected </%s>, but found </%s>", want, name );
	 src_parse_error( stderr, src_get( yyleng ), buf );
#if SEARCH
      } else {
	 stk_push(stk,want);
      }
#endif
   }
#endif
   
#else
   printf( "%*.*s",yyleng,yyleng,yytext);
#endif
}

int main( int argc, char **argv )
{
   char *pt;
   FILE *str;

   if (argc == 2) {
      fprintf( stderr, "Opening %s\n",argv[1]);
      if (!(str = fopen( argv[1], "r" ))) {
	 err_fatal_errno( __FUNCTION__,
			  "Cannot open \"%s\" for read\n", argv[1]);
      }
      if ((pt = strrchr(argv[1],'/'))) src_new_file(pt+1);
      else 	                       src_new_file(argv[1]);

      yyrestart(str);
      yylex();
      fclose(str);
   } else if (argc == 1) {
      src_new_file("[stdin]");
      yyrestart(stdin);
      yylex();
   }
   return 0;
}
