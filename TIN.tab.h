<<<<<<< Updated upstream
/* A Bison parser, made by GNU Bison 2.7.  */
=======
/** \file TIN.tab.h Headers for a Bison parser, made by GNU Bison 2.7.  */
>>>>>>> Stashed changes

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_TIN_TAB_H_INCLUDED
# define YY_YY_TIN_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     UPLOAD = 258,
     AS = 259,
     NAME = 260,
     DELETE = 261,
     FIND = 262,
     ALL = 263,
     FIRST = 264,
     NEXT = 265,
     DOWNLOAD = 266,
     SIZE = 267,
     OWNER = 268,
     GUARD = 269,
     SIZE_UNIT = 270,
     GT = 271,
     GTE = 272,
     LT = 273,
     LTE = 274,
     EQUAL = 275,
     NONEQ = 276,
     MATCH = 277,
     AND = 278,
     OR = 279,
     NOT = 280,
     ONE = 281,
     BYTE = 282,
     KILOBYTE = 283,
     MEGABYTE = 284,
     GIGABYTE = 285,
     EXIT = 286,
     END = 287,
     OTHER = 288,
     CMD = 289,
     NATURAL = 290,
     PATH = 291,
     FILENAME = 292
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2058 of yacc.c  */
#line 31 "TIN.y"

	unsigned int inatural;
	const char* ipath;
	const char* iname;
	FileID id;
	FileIDs ids;


/* Line 2058 of yacc.c  */
#line 103 "TIN.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_TIN_TAB_H_INCLUDED  */
