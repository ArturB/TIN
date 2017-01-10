/*
 * Plik wejściowy generatora lekserów flex
 * Nie edytować! Procedury wykonywane przez komendy znajdują się w pliku p2p-actions.hpp
 * Artur M. Brodzki, Kalisz 2016
 */

%option noyywrap

%{
#include <cstdio>
#include <iostream>
using namespace std;

#include "structs.h"
#include "TIN.tab.h"  // to get the token types that we return

int line_num = 1;

#define YY_DECL extern "C" int yylex()
%}

%%
[ \t]             ;
upload            { return UPLOAD; }
as                { return AS; }
delete            { return DELETE; }
find              { return FIND; }
all               { return ALL; }
first             { return FIRST; }
one               { return ONE; }
next              { return NEXT; }
download          { return DOWNLOAD; }
size              { return SIZE; }
owner             { return OWNER; }
name              { return NAME; }
\||where          { return GUARD; }
b|B               { return BYTE; }
kb|KB             { return KILOBYTE; }
mb|MB             { return MEGABYTE; }
gb|GB             { return GIGABYTE; }
\:\!              { return CMD; }

exit|quit         { return EXIT; }

\n                { line_num++; return END; }

0|[1-9][0-9]*     { 
						yylval.inatural = atoi(yytext);
						return NATURAL; }

\"[a-zA-Z][a-zA-Z0-9\-\_\ \.]*\" { 
						yylval.iname = strdup(yytext);
						int length = strlen(yylval.iname);
						char* res = new char[length - 1];
						for(unsigned i = 1; i < length - 1; ++i)
							res[i - 1] = yylval.iname[i];
						res[length - 2] = '\0';
						yylval.iname = res;
						return FILENAME; }

						
\"[a-zA-Z0-9\-\_\=\+\.\,\;\:\[\]\{\}\!\@\#\$\%\^\&\*\(\)\ \\\/]+\" { 
						yylval.ipath = strdup(yytext);
						int length = strlen(yylval.ipath);
						char* res = new char[length - 1];
						for(unsigned i = 1; i < length - 1; ++i)
							res[i - 1] = yylval.ipath[i];
						res[length - 2] = '\0';
						yylval.ipath = res;
						return PATH; }
					
\>                { return GT; }
\<                { return LT; }
\>\=              { return GTE; }
\<\=              { return LTE; }
\=\=              { return EQUAL; }
\!\=              { return NONEQ; }
\&\&              { return AND; }
\|\|              { return OR; }
\!\=\=\!          { return MATCH; }
\~|not            { return NOT; }



%%

