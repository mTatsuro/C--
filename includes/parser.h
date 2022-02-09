/****** parser.h ********************************************************/
// this should contain definitions that are defined by the parser and
// that are shared across files and modules
//
#ifndef _PARSER_H_
#define _PARSER_H_


// TODO: you may remove everything that is in here if you'd like; the contents
//       are just to give you an example of some of the types of things
//       that may appear in this file

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "ast.h"


// a special value for the token field of an AST node to signify that
// this AST node corresponds to syntax that is not represented by a terminal
// in the grammar
#define NONTERMINAL   400       // set to 400 so that it does not conflict
                                // with a valid token value that is defined
                                // as an enum type in lexer.h
                                // (it would be better to add this to the
                                // enum def, but is separated out since
                                // it's appearing for the first time in part2
                                // of the project

// symbols for AST nodes corresponding to non-terminals
// ... add more, or feel free to change these completely
// also, you could use an enum type here instead of #defines
//
#define START_AST_SYM 	401       // used to specify start of valid range
#define ROOT       	    401       // Program
#define DECL  		  	402       // Decl
#define PROGRAM_ 	  	403       // Program'
#define FUN_DECL	  	404       // FunDecl
#define FUN_DECL_TAIL 	405       // FunDeclTail
#define VAR_DECL_LIST 	406       // VarDeclList
#define VAR_DECL_LIST_	407       // VarDeclList'
#define VAR_DECL		408       // VarDecl
#define VAR_DECL_		409       // VarDecl'
#define FUN_DECL_LIST	410       // FunDeclList
#define FUN_DECL_LIST_	411       // FunDeclList'
#define FUN_DECL_		412       // FunDecl'
#define PARAM_DECL_LIST 413  	  // ParamDeclList
#define PARAM_DECL		414       // ParamDecl
#define BLOCK_N			415       // Block
#define STMT			416       // Stmt
#define STMT_LIST		417       // StmtList
#define EXPR			418       // Expr
#define EXPR_LIST		419       // ExprList
#define END_AST_SYM   	419       // Specify end of valid range


// add global variable definitions:
extern ast ast_tree;        // the abstract syntax tree

// add any function prototypes that are shared across files:
extern void parse(FILE *fd);

// uncomment DEBUG_PARSER #define to enable debug output
//#define DEBUG_PARSER     1
#ifdef DEBUG_PARSER  // DEBUG_PARSER on:
#define parser_debug0(str)            printf(str)           // 1 string arg
#define parser_debug1(fmtstr, arg1)   printf(fmtstr,arg1)   // format str & arg
#define parser_debug2(fmtstr, arg1, arg2)  printf(fmtstr, arg1, arg2)
#else  // DEBUG_LEXER off:
#define parser_debug0(str)
#define parser_debug1(fmtstr, arg1)
#define parser_debug2(fmtstr, arg1, arg2)
#endif


#endif
