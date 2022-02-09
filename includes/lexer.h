/****** lexer.h ********************************************************/
// this file contain definitions that are used by the lexer, and are
// exported by the lexer to other modules like parser and code gen
//
// extern  means that this variable or function is declared somewhere else
//         extern int x;  // x is a global variable declared somewhere else
//                        // type information about the global name x
//                        // this appears in .h files included by every .c
//                        // file that wants to use x
//
//         int x;         // declares a global variable x...alloc space for it
//                        // this appears in exactly one .c file
//
// TODO: you may remove everything that is in here if you'd like; the contents
//       are just to give you an example of some of the types of things
//       that may appear in this file
#ifndef _LEXER_H
#define _LEXER_H

// INCLUDES:
#include <stdio.h>
#include <stdbool.h>

// CONSTANT DEFS:
// constants used by lexer
#define MAXLEXSIZE    128            // max length of a lexeme
#define NONE           -1
#define LEXERROR       -1
#define LEXEME_COUNT   37

// TYPEDEFS:
// enum and struct defs
//
typedef enum {EMPTY,
              LEXEME,
              VALUE } token_value;

// token defs:
// either use an enumerated type def or #defines to define token values:
// (i.e. use IF in your code rather than 3 for the token value of keyword if)
typedef enum {STARTTOKEN,
			  ID,
              INT, CHAR,        // types
              IF,               // keyword
              NUM,              // integer literal
              RETURN,
              READ,
              WRITE,
              WRITELN,
              BREAK,
              ELSE,
              WHILE,
              SEMICOLON,
              COMMA,
              LPAREN,
              RPAREN,
              LBRACKET,
              RBRACKET,
              LCURLY,
              RCURLY,
              NEG,
              PLUS,
              MINUS,
              MULT,
              DIV,
              EQU,
              NEQ,
              LSS,
              LEQ,
              GTR,
              GEQ,
              AND,
              OR,
              ASSIGN,
              DONE,             // special "token" indicates LA is done
              ENDTOKEN } tokenT;

typedef struct token
{
tokenT type;
token_value t_value;
int line;
char lexeme[MAXLEXSIZE];
int value;
} token;

extern const char * lex_symbol_table[LEXEME_COUNT];


// GLOBAL VARIABLE DEFS: for global variable that are used in more than one .c:
// "extern" means they are declared somewhere else (in exactly one .c file)
// as a rule, you should avoid using global variables, but if you have state
// that is shared between modules, you often need to use globals
//

// information about the current token
// (the next call to lexan changes their value):
extern char lexbuf[];  // its lexeme
extern int  tokenval;  // its value (for a numeric literal it could be its
                       // value, for an identifier it could be its entry in
                       // symbol table, ...)
// the current source code line number
extern int  src_lineno;
extern bool is_lexeme;

// MACROS DEFINITIONS:  in general, use functions rather than macros as the
//      compiler can check parameter type for functions
// macros for debugging output

// uncomment DEBUG_LEXER #define to enable debug output
//#define DEBUG_LEXER     1
#ifdef DEBUG_LEXER  // DEBUG_LEXER on:
#define lexer_debug0(str)            printf(str)           // 1 string arg
#define lexer_debug1(fmtstr, arg1)   printf(fmtstr,arg1)   // format str & 1 arg
#else  // DEBUG_LEXER off:
#define lexer_debug0(str)
#define lexer_debug1(fmtstr, arg1)
#endif

// FUNCTION PROTOTYPES: prototypes for any functions exported by the lexer
// (ones defined in one lexer.c file, that are used in other modules)
// "extern" means that the function's definition is somewhere else
extern token lexan(FILE *fd);
extern void lexer_emit(token t);
void lexer_error(char *m, int lineno);
void lexer_recovery(char expected, int lineno);
void char_error(int c);

#endif
