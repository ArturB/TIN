/*
 * Plik wejściowy generatora parserów bison
 * Nie edytować! Procedury wykonywane przez komendy znajdują się w pliku p2p-actions.hpp
 * Artur M. Brodzki, Kalisz 2016
 */

%{
#include <cstdio>
#include <iostream>
#include <string.h>
#include <errno.h>

#include "structs.h"
#include "p2p-actions.hpp"

#define SYNTAX_ERR -10

using namespace std;

// stuff from flex that bison needs to know about:
extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;
extern int line_num;
extern "C" char* yytext;

void yyerror(const char *s);

%}

%union {
	unsigned int inatural;
	const char* ipath;
	const char* iname;
	FileID id;
	FileIDs ids;
}


// define the "terminal symbol" token types I'm going to use (in CAPS
// by convention), and associate each with a field of the union:
%token UPLOAD
%token AS
%token NAME
%token DELETE
%token FIND
%token ALL
%token FIRST
%token NEXT
%token DOWNLOAD
%token SIZE
%token OWNER
%token GUARD
%token SIZE_UNIT
%token GT
%token GTE
%token LT
%token LTE
%token EQUAL
%token NONEQ
%token MATCH
%token AND
%token OR
%token NOT
%token ONE
%token BYTE
%token KILOBYTE
%token MEGABYTE
%token GIGABYTE

%token EXIT
%token END
%token OTHER
%token CMD

%token <inatural> NATURAL
%token <ipath> PATH
%token <iname> FILENAME

%type <id> BoolTerm;
%type <id> BoolClause;
%type <ids> BoolExpr;
%type <inatural> SizeUnit;
%type <inatural> SizeFooter;

%%
S:
	  UploadInst END   { upload_action(); errno = 0; YYACCEPT; }
	| DownInst END     { download_action(); errno = 0; YYACCEPT; }
	| DeleteInst END   { delete_action(); errno = 0; YYACCEPT; }
	| FindInst END     { find_action(); errno = 0;   YYACCEPT; }
	| CMD PATH END     { system($2); safe_cout(prompt); errno = 0; }
	| OTHER            { yyerror(""); errno = SYNTAX_ERR; return -1; } 
	| EXIT             { main_destroy(); exit(0); }
	;

UploadInst:
	UPLOAD PATH AS FILENAME { 
		pthread_mutex_lock(&cmtx);
		cmdata.filePath = $2;
		cmdata.fileName = $4;
		pthread_mutex_unlock(&cmtx);
	}
	;

DeleteInst:
	DELETE PATH {
		pthread_mutex_lock(&cmtx);
		cmdata.fileName = $2;
		cmdata.filePath = "";
		pthread_mutex_unlock(&cmtx);
	}
	;

FindInst:
	FIND FindHead GUARD BoolExpr { 
		pthread_mutex_lock(&cmtx);
		cmdata.fileName = "";
		cmdata.filePath = "";
		cmdata.ids = $4;
		pthread_mutex_unlock(&cmtx);
	}
	;
	
DownInst:
	DOWNLOAD FILENAME SIZE SizeFooter {
		pthread_mutex_lock(&cmtx);
		cmdata.fileName = $2;
		cmdata.fileSize = $4;
		pthread_mutex_unlock(&cmtx);
	}
	;
	
FindHead:
	  ONE { 
		  pthread_mutex_lock(&cmtx);
		  cmdata.find_one = true;
		  cmdata.find_all = false;
		  cmdata.find_first = false;
		  pthread_mutex_unlock(&cmtx);
	  }
	| ALL { 
		  pthread_mutex_lock(&cmtx);
		  cmdata.find_one = false;
		  cmdata.find_all = true;
		  cmdata.find_first = false;
		  pthread_mutex_unlock(&cmtx);
	}
	| FIRST NATURAL { 
		  pthread_mutex_lock(&cmtx);
		  cmdata.find_one = false;
		  cmdata.find_all = false;
		  cmdata.find_first = true;
		  cmdata.find_first_count = $2;
		  pthread_mutex_unlock(&cmtx);
	}
	;
	
BoolTerm:
	  SIZE SizeFooter { 
		  FileID res;
		  res.name = NULL;
		  res.owner = NULL;
		  res.size = $2;
		  res.time = 0;
		  $$ = res;
	  }
	| OWNER PATH { 
		  FileID res;
		  res.name = NULL;
		  res.owner = $2;
		  res.size = 0;
		  res.time = 0;
		  $$ = res;
	  }
	| NAME FILENAME { 
		  FileID res;
		  res.name = $2;
		  res.owner = NULL;
		  res.size = 0;
		  res.time = 0;
		  $$ = res;
	  }
	;

SizeFooter:
	NATURAL SizeUnit { $$ = $1 * $2; }
	;
	
SizeUnit:
	  BYTE { $$ = 1; }
	| KILOBYTE { $$ = 1024; }
	| MEGABYTE { $$ = 1024 * 1024; }
	| GIGABYTE { $$ = 1024 * 1024 * 1024; }
	;
	
BoolClause:
	  BoolTerm { 
		  $$ = $1;
	  }
	| BoolTerm AND BoolClause { 
		  FileID res = $1;
		  if($3.name != NULL)
			  res.name = strdup($3.name);
		  if($3.owner != NULL)
			  res.owner = strdup($3.owner);
		  if($3.size != 0)
			  res.size = $3.size;
		  if($3.time != 0)
			  res.time = $3.time;
		  $$ = res;
	}
	;

BoolExpr:
	  BoolClause { 
		  FileIDs res;
		  res.ids = new FileID[1];
		  res.ids[0] = $1;
		  res.size = 1;
		  $$ = res;
	  }
	  | BoolClause OR BoolExpr { 
		  FileIDs res;
		  res.ids = new FileID[$3.size + 1];
		  for(unsigned i = 0; i < $3.size; ++i)
			  res.ids[i] = $3.ids[i];
		  res.ids[$3.size] = $1;
		  res.size = $3.size + 1;
		  
		  $$ = res;
	  }
	;

%%

int main(int argv, char** argc) {
	main_init();
	safe_cout(welcome);
	while(1) {
		safe_cout(prompt);
		yyparse();
	}
	main_destroy();
}

void yyerror(const char *s) {
	string errMsgS(errMsg);
	safe_cout(" - " + errMsgS + "\n");
	errno = SYNTAX_ERR;
}




