/* The lexical analyzer for the C-- Programming Language
 */

#include <stdlib.h>
#include <assert.h>
#include "lexer.h"
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// these are likely values that will be needed by the parser, so
// making them global variables is okay
char lexbuf[MAXLEXSIZE];  // stores current lexeme
int  tokenval=0;          // stores current token's value
                          // (might not be used for every token)
int  src_lineno=0;        // current line number in source code input
int lexbuf_count = 0;     // stores the number of items in lexbuf
bool is_lexeme = false;   // stores if the current token is a lexeme

const char * lex_symbol_table[LEXEME_COUNT] =
{
			  "STARTTOKEN",
			  "ID",
              "INT", "CHAR",
              "IF",
              "NUM",
              "RETURN",
              "READ",
              "WRITE",
              "WRITELN",
              "BREAK",
              "ELSE",
              "WHILE",
              "SEMICOLON",
              "COMMA",
              "LPAREN",
              "RPAREN",
              "LBRACKET",
              "RBRACKET",
              "LCURLY",
              "RCURLY",
              "NEG",
              "PLUS",
              "MINUS",
              "MULT",
              "DIV",
              "EQU",
              "NEQ",
              "LSS",
              "LEQ",
              "GTR",
              "GEQ",
              "AND",
              "OR",
              "ASSIGN",
              "DONE",
              "ENDTOKEN"
};

// function prototypes:
static void print_lineno();  // static limits its scope to only in this .c file
static int start(FILE * fd);
static int is_idchar(int c);
static int char_space(FILE * fd);
static void read_val(int c);
static int idchar(FILE * fd);
static int digit(FILE * fd);
static int lparen();
static int rparen();
static int si(FILE * fd);
static int state_in(FILE * fd);
static int sint(FILE * fd);
static int sif(FILE * fd);
static int sc(FILE * fd);
static int sch(FILE * fd);
static int scha(FILE * fd);
static int schar(FILE * fd);
static int se(FILE * fd);
static int sel(FILE * fd);
static int sels(FILE * fd);
static int selse(FILE * fd);
static int sb(FILE * fd);
static int sbr(FILE * fd);
static int sbre(FILE * fd);
static int sbrea(FILE * fd);
static int sbreak(FILE * fd);
static int sr(FILE * fd);
static int sre(FILE * fd);
static int sret(FILE * fd);
static int sretu(FILE * fd);
static int sretur(FILE * fd);
static int sreturn(FILE * fd);
static int srea(FILE * fd);
static int sread(FILE * fd);
static int sw(FILE * fd);
static int swr(FILE * fd);
static int swri(FILE * fd);
static int swrit(FILE * fd);
static int swrite(FILE * fd);
static int swritel(FILE * fd);
static int swriteln(FILE * fd);
static int swh(FILE * fd);
static int swhi(FILE * fd);
static int swhil(FILE * fd);
static int swhile(FILE * fd);
static int plus();
static int minus();
static int mult();
static int division(FILE * fd);
static int comment1(FILE * fd);
static int comment2(FILE * fd);
static int fullcomment(FILE * fd);
static int assign(FILE * fd);
static int equal();
static int neg(FILE * fd);
static int neq();
static int greater(FILE * fd);
static int geq(FILE * fd);
static int smaller(FILE * fd);
static int leq(FILE * fd);
static int and(FILE * fd);
static int and2(FILE * fd);
static int or(FILE * fd);
static int or2(FILE * fd);
static int semicolon(FILE * fd);
static int comma(FILE * fd);
static int lsqbracket();
static int rsqbracket();
static int lbracket();
static int rbracket();

/***************************************************************************/
/*
 *  Main lexer routine:  returns the next token in the input
 *
 *  param fd: file pointer for reading input file (source C-- program)
 *            TODO: you may want to add more parameters
 *
 *  returns: the next token, or
 *           DONE if there are no more tokens, or
 *           LEXERROR if there is a token parsing error
 *  note: num value and lexeme value will be each stored in the global variables,
 *        tokenval and lexbuf.
 */
token lexan(FILE *fd) {

  // initialize values
  lexbuf[0] = '\0';
  lexbuf_count = 0;
  tokenval = 0;
  is_lexeme = false;

  // need t.value and t.lexeme to be global varibles
  tokenT token_type = start(fd);
  token t;
  t.type = token_type;
  t.line = src_lineno;
  if (lexbuf_count > 0)
	{
		strcpy(t.lexeme, lexbuf);
		t.t_value = LEXEME;
	}
	else if (t.type == NUM)
	{
		t.t_value = VALUE;
	}
	else
	{
		t.t_value = EMPTY;
	}
	t.value = tokenval;
  return t;
}

/**
 * Increment the line number.
 */
static void line_inc()
{
	src_lineno = src_lineno + 1;
}

/**
 * Finds the next character from input, skips whitespaces
 * and advances line number if necessary.
 *
 * param fd: file pointer for reading input file (source C-- program)
 *
 * returns: next character
 */
static int next_char(FILE * fd)
{
	int c;
	while (isspace(c = fgetc(fd)))
	{
		if (c == '\n')
			line_inc();
	}
	read_val(c);
	return c;
}

/***************************************************************************/
// Below are the functions representing each state
// Each state in the DFA from part 3 corresponds to each function

// Start state
static int start(FILE * fd) {
  int c = next_char(fd);
  if(isdigit(c)) {
    return digit(fd);
  }
  switch(c) {
  case '(':
    return lparen(fd);
  case ')':
    return rparen(fd);
  case 'i':
    return si(fd);
  case 'c':
    return sc(fd);
  case 'e':
    return se(fd);
  case 'b':
    return sb(fd);
  case 'r':
    return sr(fd);
  case 'w':
    return sw(fd);
  case EOF:
    return DONE;
  case '+':
    return plus(fd);
  case '-':
    return minus(fd);
  case '*':
    return mult(fd);
  case '/':
    return division(fd);
  case '=':
    return assign(fd);
  case '!':
    return neg(fd);
  case '<':
    return greater(fd);
  case '>':
    return smaller(fd);
  case '&':
    return and(fd);
  case '|':
    return or(fd);
  case ';':
    return semicolon(fd);
  case ',':
    return comma(fd);
  case '[':
    return lsqbracket(fd);
  case ']':
    return rsqbracket(fd);
  case '{':
    return lbracket(fd);
  case '}':
    return rbracket(fd);
  }
  if(is_idchar(c)) {
    return idchar(fd);
  }
  return LEXERROR;
}

/**
 * Returns true if the character is a digit, a letter, or an underbar
 */
static int is_idchar(int c) {
	return isalnum(c) || c == '_';
}

/**
 * Gets the next character and moves the next line number
 */
static int char_space(FILE * fd) {
	int c = fgetc(fd);
	if (c == '\n') {
		line_inc();
		return -1;
	}
	if (c == EOF) {
		ungetc(c, fd);
		return -1;
	}
	if (isspace(c)) {
		return -1;
	}
	read_val(c);
	return c;
}

/**
 * Reads the lexeme value
 */
 static void read_val(int c)
 {
 	if (is_idchar(c))
 	{
 		if (lexbuf_count >= MAXLEXSIZE - 1) // max size of lexeme
 		{
 			lexer_error("lexeme too long\n", src_lineno);
 			return;
 		}
 		lexbuf[lexbuf_count++] = c;
 		lexbuf[lexbuf_count] = '\0';
 	}
 	if (isdigit(c))
 	{
 		int d = c - '0';
 		tokenval = tokenval * 10 + d;
 	}
 }

static int idchar(FILE * fd) {
	int c = char_space(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int digit(FILE * fd) {
  int c = char_space(fd);
  if (c < 0)
    return NUM;
  if (isdigit(c))
    return digit(fd);
  if (isalpha(c))
    return LEXERROR;
  ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
  return NUM;
}

static int lparen() {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
  return LPAREN;
}

static int rparen() {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
  return RPAREN;
}

// for further reference, s stands for state and i stands for a character
static int si(FILE * fd) {
  int c = char_space(fd);
  if (c < 0)
    return ID;
  switch (c) {
    case 'f':
      return sif(fd);
    case 'n':
      return state_in(fd);
  }
  if (is_idchar(c))
    return idchar(fd);
  ungetc(c, fd);
  return ID;
}

static int state_in(FILE * fd) {
  int c = char_space(fd);
  if (c < 0)
    return ID;
  if (c == 't')
    return sint(fd);
  if (is_idchar(c))
    return idchar(fd);
  ungetc(c, fd);
  return ID;
}

static int sint(FILE * fd) {
	int c = char_space(fd);
	if (is_idchar(c))
		return idchar(fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return INT;
}

static int sif(FILE * fd) {
	int c = char_space(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return IF;
}

static int sc(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	if (c == 'h')
		return sch(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sch(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	if (c == 'a')
		return scha(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int scha(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	if (c == 'r')
		return schar(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int schar(FILE * fd) {
	int c = char_space(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return CHAR;
}

static int se(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c){
	case 'l':
		return sel(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sel(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c) {
	case 's':
		return sels(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sels(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c) {
	case 'e':
		return selse(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int selse(FILE * fd) {
	int c = char_space(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return ELSE;
}

static int sb(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c) {
	case 'r':
		return sbr(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sbr(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c) {
	case 'e':
		return sbre(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sbre(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c) {
	case 'a':
		return sbrea(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sbrea(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c) {
	case 'k':
		return sbreak(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sbreak(FILE * fd) {
	int c = char_space(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return BREAK;
}

static int sr(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	if (c == 'e')
		return sre(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sre(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c) {
	case 't':
		return sret(fd);
	case 'a':
		return srea(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sret(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c) {
	case 'u':
		return sretu(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sretu(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c) {
	case 'r':
		return sretur(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sretur(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c) {
	case 'n':
		return sreturn(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sreturn(FILE * fd)
{
	int c = char_space(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	lexbuf[0] = '\0';
  lexbuf_count = 0;
	return RETURN;
}

static int srea(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c){
	case 'd':
		return sread(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int sread(FILE * fd)
{
	int c = char_space(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return READ;
}

static int sw(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c){
	case 'r':
		return swr(fd);
	case 'h':
		return swh(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int swr(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c){
	case 'i':
		return swri(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int swri(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c){
	case 't':
		return swrit(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int swrit(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c){
	case 'e':
		return swrite(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int swrite(FILE * fd) {
	int c = char_space(fd);
	if (c < 0) {
    lexbuf[0] = '\0';
    lexbuf_count = 0;
		return WRITE;
	}
	switch (c){
	case 'l':
		return swritel(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return WRITE;
}

static int swritel(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c){
	case 'n':
		return swriteln(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int swriteln(FILE * fd) {
	int c = char_space(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return WRITELN;
}

static int swh(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c){
	case 'i':
		return swhi(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int swhi(FILE * fd) {
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c)
	{
	case 'l':
		return swhil(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int swhil(FILE * fd)
{
	int c = char_space(fd);
	if (c < 0)
		return ID;
	switch (c)
	{
	case 'e':
		return swhile(fd);
	}
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
	return ID;
}

static int swhile(FILE * fd) {
	int c = char_space(fd);
	if (is_idchar(c))
		return idchar(fd);
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return WHILE;
}

static int plus() {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return PLUS;
}

static int minus() {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return MINUS;
}

static int mult() {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return MULT;
}

static int division(FILE * fd) {
	int c = char_space(fd);
	if (c < 0) {
    lexbuf[0] = '\0';
    lexbuf_count = 0;
		return DIV;
	}
	switch (c) {
	case '*':
		return comment1(fd);
	case '/':
		return comment2(fd);
	}
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return DIV;
}

static int comment1(FILE * fd) {
	int c = fgetc(fd);
	switch (c) {
	case '\n':
		line_inc();
		break;
	case '*':
		return fullcomment(fd);
	case EOF:
		return LEXERROR;
	}
	return comment1(fd);
}

static int fullcomment(FILE * fd) {
	int c = fgetc(fd);
	switch (c)
	{
	case '/':
		return start(fd);
	case EOF:
		return LEXERROR;
	}
	return comment1(fd);
}

  static int comment2(FILE * fd) {
  	int c = fgetc(fd);
  	switch (c) {
  	case '\n':
  		line_inc();
  		return start(fd);
  	case EOF:
  		return DONE;
  	}
  	return comment2(fd);
  }

  static int assign(FILE * fd)  {
	int c = char_space(fd);
	switch (c) {
	case '=':
		return equal();
	}
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return ASSIGN;
}

static int equal()
{
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return EQU;
}

static int neg(FILE * fd) {
	int c = char_space(fd);
	switch (c){
	case '=':
		return neq(fd);
	}
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return NEG;
}

static int neq() {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return NEQ;
}

static int greater(FILE * fd) {
	int c = char_space(fd);
	switch (c){
	case '=':
		return geq(fd);
	}
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return GTR;
}

static int geq(FILE * fd)
{
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return GEQ;
}

static int smaller(FILE * fd) {
	int c = char_space(fd);
	switch (c){
	case '=':
		return leq(fd);
	}
	ungetc(c, fd);
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return LSS;
}

static int leq(FILE * fd)
{
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return LEQ;
}

static int and(FILE * fd) {
	int c = char_space(fd);
	switch (c){
	case '&':
		return and2(fd);
	default:
		lexer_recovery('&', src_lineno);
		return and2(fd);
	}
	return LEXERROR;
}

static int and2(FILE * fd)
{
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return AND;
}

static int or(FILE * fd) {
	int c = char_space(fd);
	switch (c){
	case '|':
		return or2(fd);
	default:
		lexer_recovery('|', src_lineno);
		return or2(fd);
	}
	return LEXERROR;
}

static int or2(FILE * fd)
{
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return OR;
}

static int semicolon(FILE * fd) {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return SEMICOLON;
}

static int comma(FILE * fd) {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return COMMA;
}

static int lsqbracket() {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return LBRACKET;
}

static int rsqbracket() {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return RBRACKET;
}

static int lbracket() {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return LCURLY;
}

static int rbracket() {
  lexbuf[0] = '\0';
  lexbuf_count = 0;
	return RCURLY;
}

/***************************************************************************/
// A function for demonstrating that functions should be declared static
// if they are to be used only in the file in which they are defined.
// Static limits the scope to only this .c file
static void print_lineno() {

  printf("line no = %d\n", src_lineno);

}
