/*
 * parser.c: A Recursive Descent Parser for C--
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "parser.h"
#include "lexer.h"
#include "ast.h"

// TODO: you may completely wipe out or change the contents of this file; it
//       is just an example of how to get started on the structure of the
//       parser program.

// function decls
static void program(FILE *fd, ast_node *parent);
static void parser_error(char *err_string);
static void expr_list(FILE * fd, ast_node * expr_list_node);
static ast_node * expr(FILE * fd);

token lookahead;  // stores next token returned by lexer
                // you may need to change its type to match your implementation

ast ast_tree;   // the abstract syntax tree

static void print_nonterminal(char * nonterminal) {
	printf("%s\n", nonterminal);
}

static void comp_error(int expected) {
	printf("Line %d: Comparison error, expected %s\n",
			lookahead.line, lex_symbol_table[expected]);
	exit(1);
}

static void expansion_error() {
	printf("Line %d: Unexpected %s\n", lookahead.line,
			lex_symbol_table[lookahead.type]);
	exit(1);
}

/**
 * Prints the currently matched lookahead.
 */
static void print_match() {
	if (lookahead.t_value == EMPTY)
		printf("MATCH: %s\n", lex_symbol_table[lookahead.type]);
	else if (lookahead.t_value == LEXEME)
		printf("MATCH: %s.%s\n", lex_symbol_table[lookahead.type], lookahead.lexeme);
	else
		printf("MATCH: %s.%d\n", lex_symbol_table[lookahead.type], lookahead.value);
}

static void next(FILE * fd)
{
	lookahead = lexan(fd);
}

/**
 * Compares the lookahead token with expected token
 * and either gets another token if the two match
 * or issues an error.
 * param fd: FILE pointer
 * param expected: expected token
 * return: the old lookahead token
 */
static token comp(FILE * fd, int expected, int ignore) {
	token old = lookahead;
	if (lookahead.type == expected)
	{
		print_match();
		next(fd);
	}
	else
	{
		if (ignore == 0)
		{
			comp_error(expected);
		}
		else
		{
			printf("Missing %s at line %d\n", lex_symbol_table[expected], lookahead.line);
			token t;
			t.type = expected;
			t.t_value = NONE;
			t.line = lookahead.line;
			t.value = 0;
			old = t;
		}
	}
	return old;
}

/**
 * Create new ast_info structure for a terminal
 * param t: token with info
 * return: ast_info structure with info from a given token
 */
static ast_info * new_ast_terminal_info(token t) {
	return create_new_ast_node_info(t.type, t.value, t.type, t.lexeme, t.line);
}

/**
 * Create new ast_info structure for a nonterminal
 * param grammar_sym: a nonterminal constant
 * return: ast_info structure with no info and a given nonterminal constant
 */
static ast_info * new_ast_nonterminal_info(int grammar_sym) {
	return create_new_ast_node_info(NONTERMINAL, 0, grammar_sym, 0, 0);
}

/**
 * Create new ast_node structure
 * param ast_info: info for the ast_node
 * return: ast_node structure with a given info
 */
static ast_node * new_ast_node(ast_info * info) {
	return create_ast_node(info);
}

/**************************************************************************/
/*
 *  Main parser routine: parses the C-- program in input file pt'ed to by fd,
 *                       calls the function corresponding to the start state,
 *                       checks to see that the last token is DONE,
 *                       prints a success msg if there were no parsing errors
 *  param fd: file pointer for input
 */
void parse(FILE *fd)  {

  // TODO: here is an example of what this function might look like,
  //       you may completely change this:
  ast_info *s;
  ast_node *n;

  // create the root AST node
  s = create_new_ast_node_info(NONTERMINAL, 0, ROOT, 0, 0);
  n = create_ast_node(s);
  if(init_ast(&ast_tree, n)) {
        parser_error("ERROR: bad AST\n");
  }

  // lookahead is a global variable holding the next token
  // you could also use a local variable and then pass it to program
  lookahead = lexan(fd);
  program(fd, ast_tree.root);  // program corresponds to the start state

  // the last token should be DONE
  if (lookahead.type != DONE) {
    parser_error("expected end of file");
  } else {
     comp(fd, DONE, 0);
  }

}
/**************************************************************************/
static void parser_error(char *err_string) {
  if(err_string) {
    printf("%s\n", err_string);
  }
  exit(1);
}

static ast_node * h_(FILE * fd)
{
	print_nonterminal("H'");
	switch (lookahead.type)
	{
	case LPAREN:
	{
		comp(fd, LPAREN, 0);
		ast_node * expr_list_node = new_ast_node(new_ast_nonterminal_info(EXPR_LIST)); // create an ExprList node
		expr_list(fd, expr_list_node); // ExprList node
		comp(fd, RPAREN, 1);
		return expr_list_node;
	}
	case LBRACKET:
	{
		comp(fd, LBRACKET, 0);
		ast_node * expr_node = expr(fd); // Expr node
		comp(fd, RBRACKET, 1);
		return expr_node;
	}
	case MULT:
	case DIV:
	case PLUS:
	case MINUS:
	case LSS:
	case LEQ:
	case GEQ:
	case GTR:
	case EQU:
	case NEQ:
	case AND:
	case OR:
	case ASSIGN:
	case RPAREN:
	case RBRACKET:
	case COMMA:
	case SEMICOLON:
	{
		return NULL;
	}
	default:
		return NULL;
	}
	return NULL;
}

static ast_node * h(FILE * fd)
{
	print_nonterminal("H");
	switch (lookahead.type)
	{
	case ID:
	{
		ast_node * id_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an id node
		comp(fd, ID, 0);
		ast_node * h__node = h_(fd); // H' node
		if (h__node != NULL)
			add_child_node(id_node, h__node);
		return id_node;
	}
	case LPAREN:
	{
		comp(fd, LPAREN, 0);
    ast_node * expr_node = expr(fd); // Expr node
		comp(fd, RPAREN, 1);
		return expr_node;
	}
	case NUM:
	{
		ast_node * num_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a num node
		comp(fd, NUM, 0);
		return num_node;
	}
	default:
		return NULL;
	}
	return NULL;
}

static ast_node * g(FILE * fd, ast_node * left_node)
{
	print_nonterminal("G");
	switch (lookahead.type)
	{
	case ID:
	case LPAREN:
	case NUM:
	{
		return h(fd);
	}
	case NEG:
	{
		ast_node * neg_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an ! node
		comp(fd, NEG, 0);
		ast_node * h_node = h(fd); // H node
		add_child_node(neg_node, h_node);
		return neg_node;
	}
	case MINUS:
	{
		ast_node * minus_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an - node
		comp(fd, MINUS, 0);
		ast_node * h_node = h(fd); // H node
		add_child_node(minus_node, h_node);
		return minus_node;
	}
	default:
		return NULL;
	}
	return NULL;
}

static ast_node * f_(FILE * fd, ast_node * left_node)
{
	print_nonterminal("F'");
	switch (lookahead.type)
	{
	case MULT:
	{
		ast_node * mult_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a * node
		comp(fd, MULT, 0);
		add_child_node(mult_node, left_node);
		ast_node * g_node = g(fd, NULL);
		add_child_node(mult_node, g_node);

		ast_node * f__node = f_(fd, mult_node); // F' node
		//add_child_node(mult_node, f__node);
		return f__node;
	}
	case DIV:
	{
		ast_node * div_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a / node
		comp(fd, DIV, 0);
		add_child_node(div_node, left_node);
		ast_node * g_node = g(fd, NULL);
		add_child_node(div_node, g_node);

		ast_node * f__node = f_(fd, div_node); // F' node
		//add_child_node(div_node, f__node);
		return f__node;
	}
	case PLUS:
	case MINUS:
	case LSS:
	case LEQ:
	case GEQ:
	case GTR:
	case EQU:
	case NEQ:
	case AND:
	case OR:
	case ASSIGN:
	case RPAREN:
	case RBRACKET:
	case COMMA:
	case SEMICOLON:
	{
		return left_node;
	}
	default:
		return left_node;
	}
	return NULL;
}

static ast_node * f(FILE * fd)
{
	print_nonterminal("F");
	ast_node * g_node = g(fd, NULL);
	return f_(fd, g_node);
}

static ast_node * e_(FILE * fd, ast_node * left_node)
{
	print_nonterminal("E'");
	switch (lookahead.type)
	{
	case PLUS:
	{
		ast_node * plus_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a + node
		comp(fd, PLUS, 0);
		add_child_node(plus_node, left_node);
		ast_node * f_node = f(fd);
		add_child_node(plus_node, f_node);

		ast_node * e__node = e_(fd, plus_node); // E' node
		//add_child_node(plus_node, e__node);
		return e__node;
	}
	case MINUS:
	{
		ast_node * minus_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a - node
		comp(fd, MINUS, 0);
		add_child_node(minus_node, left_node);
		ast_node * f_node = f(fd);
		add_child_node(minus_node, f_node);

		ast_node * e__node = e_(fd, minus_node); // E' node
		//add_child_node(minus_node, e__node);
		return e__node;
	}
	case LSS:
	case LEQ:
	case GEQ:
	case GTR:
	case EQU:
	case NEQ:
	case AND:
	case OR:
	case ASSIGN:
	case RPAREN:
	case RBRACKET:
	case COMMA:
	case SEMICOLON:
	{
		return left_node;
	}
	default:
		return left_node;
	}
	return NULL;
}

static ast_node * e(FILE * fd)
{
	print_nonterminal("E");
	ast_node * f_node = f(fd);
	return e_(fd, f_node);
}

static ast_node * d_(FILE * fd, ast_node * left_node)
{
	print_nonterminal("D'");
	switch (lookahead.type)
	{
	case LSS:
	{
		ast_node * lss_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an < node
		comp(fd, LSS, 0);
		add_child_node(lss_node, left_node);
		ast_node * e_node = e(fd);
		add_child_node(lss_node, e_node);

		ast_node * d__node = d_(fd, lss_node); // D' node
		//add_child_node(lss_node, e_node);
		return d__node;
	}
	case LEQ:
	{
		ast_node * leq_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an <= node
		comp(fd, LEQ, 0);
		add_child_node(leq_node, left_node);
		ast_node * e_node = e(fd);
		add_child_node(leq_node, e_node);

		ast_node * d__node = d_(fd, leq_node); // D' node
		//add_child_node(leq_node, e_node);
		return d__node;
	}
	case GEQ:
	{
		ast_node * geq_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an >= node
		comp(fd, GEQ, 0);
		add_child_node(geq_node, left_node);
		ast_node * e_node = e(fd);
		add_child_node(geq_node, e_node);

		ast_node * d__node = d_(fd, geq_node); // D' node
		//add_child_node(geq_node, e_node);
		return d__node;
	}
	case GTR:
	{
		ast_node * gtr_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an > node
		comp(fd, GTR, 0);
		add_child_node(gtr_node, left_node);
		ast_node * e_node = e(fd);
		add_child_node(gtr_node, e_node);

		ast_node * d__node = d_(fd, gtr_node); // D' node
		//add_child_node(gtr_node, e_node);
		return d__node;
	}
	case EQU:
	case NEQ:
	case AND:
	case OR:
	case ASSIGN:
	case RPAREN:
	case RBRACKET:
	case COMMA:
	case SEMICOLON:
	{
		return left_node;
	}
	default:
		return left_node;
	}
	return NULL;
}

static ast_node * d(FILE * fd)
{
	print_nonterminal("D");
	ast_node * e_node = e(fd);
	return d_(fd, e_node);
}

static ast_node * c_(FILE * fd, ast_node * left_node)
{
	print_nonterminal("C'");
	switch (lookahead.type)
	{
	case EQU:
	{
		ast_node * equ_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an == node
		comp(fd, EQU, 0);
		add_child_node(equ_node, left_node);
		ast_node * d_node = d(fd);
		add_child_node(equ_node, d_node);

		ast_node * c__node = c_(fd, equ_node); // C' node
		//add_child_node(equ_node, d_node);
		return c__node;
	}
	case NEQ:
	{
		ast_node * neq_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a != node
		comp(fd, NEQ, 0);
		add_child_node(neq_node, left_node);
		ast_node * d_node = d(fd);
		add_child_node(neq_node, d_node);

		ast_node * c__node = c_(fd, neq_node); // C' node
		//add_child_node(neq_node, d_node);
		return c__node;
	}
	case AND:
	case OR:
	case ASSIGN:
	case RPAREN:
	case RBRACKET:
	case COMMA:
	case SEMICOLON:
	{
		return left_node;
	}
	default:
		return left_node;
	}
	return NULL;
}

static ast_node * c(FILE * fd)
{
	print_nonterminal("C");
	ast_node * d_node = d(fd);
	return c_(fd, d_node);
}

static ast_node * b_(FILE * fd, ast_node * left_node)
{
	print_nonterminal("B'");
	switch (lookahead.type)
	{
	case AND:
	{
		ast_node * and_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an && node
		comp(fd, AND, 0);
		add_child_node(and_node, left_node);
		ast_node * c_node = c(fd);
		add_child_node(and_node, c_node);

		ast_node * b__node = b_(fd, and_node); // B' node
		//add_child_node(and_node, b__node);
		return b__node;
	}
	case OR:
	case ASSIGN:
	case RPAREN:
	case RBRACKET:
	case COMMA:
	case SEMICOLON:
	{
		return left_node;
	}
	default:
		return left_node;
	}
	return NULL;
}

static ast_node * b(FILE * fd)
{
	print_nonterminal("B");
	ast_node * c_node = c(fd);
	return b_(fd, c_node);
}

static ast_node * a_(FILE * fd, ast_node * left_node)
{
	print_nonterminal("A'");
	switch (lookahead.type)
	{
	case OR:
	{
		ast_node * or_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an || node
		comp(fd, OR, 0);
		add_child_node(or_node, left_node);
		ast_node * b_node = b(fd);
		add_child_node(or_node, b_node);

		ast_node * a__node = a_(fd, or_node); // A' node
		//add_child_node(or_node, a__node);
		return a__node;
	}
	case ASSIGN:
	case RPAREN:
	case RBRACKET:
	case COMMA:
	case SEMICOLON:
		return left_node;
	default:
		return left_node;
	}
	return NULL;
}

static ast_node * a(FILE * fd)
{
	print_nonterminal("A");
	ast_node * b_node = b(fd);
	return a_(fd, b_node);
}

static void expr_list_(FILE * fd, ast_node * expr_list_node)
{
	print_nonterminal("ExprList'");
	switch (lookahead.type)
	{
	case COMMA:
	{
		comp(fd, COMMA, 0);
		expr_list(fd, expr_list_node);
		return;
	}
	case RPAREN:
	{
		return;
	}
	default:
		return;
	}
}

static void expr_list(FILE * fd, ast_node * expr_list_node)
{
	print_nonterminal("ExprList");
	switch (lookahead.type)
	{
	case NEG:
	case MINUS:
	case ID:
	case LPAREN:
	case NUM:
	{
		ast_node * expr_node = expr(fd); // Expr node
		add_child_node(expr_list_node, expr_node);
		expr_list_(fd, expr_list_node);
		return;
	}
	case RPAREN:
	{
		return;
	}
	default:
		return;
	}
}


static ast_node * expr_(FILE * fd, ast_node * left_node)
{
	print_nonterminal("Expr'");
	switch (lookahead.type)
	{
	case ASSIGN:
	{
		ast_node * assign_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an = node
		comp(fd, ASSIGN, 0);
		add_child_node(assign_node, left_node);
		ast_node * a_node = a(fd);

		ast_node * expr__node = expr_(fd, a_node); // Expr' node
		add_child_node(assign_node, expr__node);
		return assign_node;
	}
	case RBRACKET:
	case COMMA:
	case SEMICOLON:
	case RPAREN:
	{
		return left_node;
	}
	default:
		return left_node;
	}
	return NULL;
}

static ast_node * expr(FILE * fd)
{
	print_nonterminal("Expr");
	ast_node * a_node = a(fd);
	return expr_(fd, a_node);
}

static ast_node * block(FILE * fd);
static void stmt(FILE * fd, ast_node * stmt_list_node)
{
	print_nonterminal("Stmt");
	switch (lookahead.type)
	{
	case SEMICOLON:
	{
		comp(fd, SEMICOLON, 0);
		return;
	}
	case NEG:
	case MINUS:
	case ID:
	case LPAREN:
	case NUM:
	{
		ast_node * expr_node = expr(fd); // Expr node
		comp(fd, SEMICOLON, 1);
		add_child_node(stmt_list_node, expr_node);
		return;
	}
	case RETURN:
	{
		ast_node * return_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a return node
		comp(fd, RETURN, 0);
		ast_node * expr_node = expr(fd); // Expr node
		comp(fd, SEMICOLON, 1);

		add_child_node(return_node, expr_node);
		add_child_node(stmt_list_node, return_node);
		return;
	}
	case READ:
	{
		ast_node * read_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a read node
		comp(fd, READ, 0);
		token t = comp(fd, ID, 0);
		ast_node * id_node = new_ast_node(new_ast_terminal_info(t)); // create an id node
		comp(fd, SEMICOLON, 1);

		add_child_node(read_node, id_node);
		add_child_node(stmt_list_node, read_node);
		return;
	}
	case WRITE:
	{
		ast_node * write_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a write node
		comp(fd, WRITE, 0);
		ast_node * expr_node = expr(fd);
		comp(fd, SEMICOLON, 1);

		add_child_node(write_node, expr_node);
		add_child_node(stmt_list_node, write_node);
		return;
	}
	case WRITELN:
	{
		ast_node * writeln_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a writeln node
		add_child_node(stmt_list_node, writeln_node);
		comp(fd, WRITELN, 0);
		comp(fd, SEMICOLON, 1);
		return;
	}
	case BREAK:
	{
		ast_node * break_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a break node
		add_child_node(stmt_list_node, break_node);
		comp(fd, BREAK, 0);
		comp(fd, SEMICOLON, 1);
		return;
	}
	case IF:
	{
		ast_node * if_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an if node
		comp(fd, IF, 0);
		comp(fd, LPAREN, 1);
		ast_node * expr_node = expr(fd); // Expr node
		add_child_node(if_node, expr_node);

		comp(fd, RPAREN, 1);
		stmt(fd, if_node); // Stmt

		token t = comp(fd, ELSE, 1);
		ast_node * else_node = new_ast_node(new_ast_terminal_info(t)); // create an else node
		stmt(fd, else_node); // Stmt

		add_child_node(stmt_list_node, if_node);
		add_child_node(if_node, else_node);
		return;
	}
	case WHILE:
	{
		ast_node * while_node = new_ast_node(new_ast_terminal_info(lookahead)); // create a while node
		comp(fd, WHILE, 0);
		comp(fd, LPAREN, 1);
		ast_node * expr_node = expr(fd);
		add_child_node(while_node, expr_node);

		comp(fd, RPAREN, 1);
		stmt(fd, while_node); // Stmt

		add_child_node(stmt_list_node, while_node);
		return;
	}
	case LCURLY:
	{
		ast_node * block_node = block(fd); // Block node
		add_child_node(stmt_list_node, block_node);
		return;
	}
	default:
		return;
	}
}

static void stmt_list(FILE * fd, ast_node * stmt_list_node);
static void stmt_list_(FILE * fd, ast_node * stmt_list_node)
{
	print_nonterminal("StmtList'");
	switch (lookahead.type)
	{
	case SEMICOLON:
	case RETURN:
	case READ:
	case WRITE:
	case WRITELN:
	case BREAK:
	case IF:
	case WHILE:
	case NEG:
	case MINUS:
	case ID:
	case LPAREN:
	case NUM:
	case LCURLY:
	{
		stmt_list(fd, stmt_list_node);
	}
	case RCURLY:
	case DONE:
	case INT:
	case CHAR:
	{
		return;
	}
	default:
		return;
	}
}

static void stmt_list(FILE * fd, ast_node * stmt_list_node)
{
	print_nonterminal("StmtList");
	stmt(fd, stmt_list_node); // Stmt node
	stmt_list_(fd, stmt_list_node); // StmtList' node
}

static ast_node * type(FILE * fd);
static ast_node * fun_type(FILE * fd)
{
	print_nonterminal("FunType");
	switch (lookahead.type)
	{
	case CHAR:
	case INT:
	{
		return type(fd);
	}
	case ID:
	{
		return NULL;
	}
	default:
		expansion_error();
	}
	return NULL;
}

static ast_node * type(FILE * fd)
{
	print_nonterminal("Type");
	switch (lookahead.type)
	{
	case CHAR:
	{
		ast_node * char_node = new_ast_node(new_ast_terminal_info(lookahead));
		comp(fd, CHAR, 0);
		return char_node;
	}
	case INT:
	{
		ast_node * int_node = new_ast_node(new_ast_terminal_info(lookahead));
		comp(fd, INT, 0);
		return int_node;
	}
	default:
		expansion_error();
	}
	return NULL;
}

static void var_decl_list(FILE * fd, ast_node * var_decl_list_node);
static ast_node * block(FILE * fd)
{
	print_nonterminal("Block");
	comp(fd, LCURLY, 0);

	ast_node * var_decl_list_node = new_ast_node(new_ast_nonterminal_info(VAR_DECL_LIST)); // create a VarDeclList node
	var_decl_list(fd, var_decl_list_node); // VarDeclList node

	ast_node * stmt_list_node = new_ast_node(new_ast_nonterminal_info(STMT_LIST)); // create a StmtList node
	stmt_list(fd, stmt_list_node); // StmtList node
	comp(fd, RCURLY, 1);

	ast_node * this_node = new_ast_node(new_ast_nonterminal_info(BLOCK_N)); // create a Block node
	add_child_node(this_node, var_decl_list_node);
	add_child_node(this_node, stmt_list_node);
	return this_node;
}

static void param_decl_(FILE * fd)
{
	print_nonterminal("ParamDecl'");
	switch (lookahead.type)
	{
	case LBRACKET:
	{
		comp(fd, LBRACKET, 0);
		comp(fd, RBRACKET, 1);
		return;
	}
	case COMMA:
	case RPAREN:
	{
		return;
	}
	default:
		expansion_error();
	}
}

static ast_node * param_decl(FILE * fd)
{
	print_nonterminal("ParamDecl");
	ast_node * type_node = type(fd); // Type node
	token t = comp(fd, ID, 0);
	ast_node * id_node = new_ast_node(new_ast_terminal_info(t)); // create an id node
	param_decl_(fd);

	ast_node * this_node = new_ast_node(new_ast_nonterminal_info(PARAM_DECL)); // create a ParamDecl node
	add_child_node(this_node, type_node);
	add_child_node(this_node, id_node);
	return this_node;
}

static void param_decl_list_tail(FILE * fd, ast_node * param_decl_list_node);
static void param_decl_list_tail_(FILE * fd, ast_node * param_decl_list_node)
{
	print_nonterminal("ParamDeclListTail'");
	switch (lookahead.type)
	{
	case COMMA:
	{
		comp(fd, COMMA, 0);
		param_decl_list_tail(fd, param_decl_list_node);
	}
	case RPAREN:
	case LCURLY:
	{
		return;
	}
	default:
		return;
	}
}

static void param_decl_list_tail(FILE * fd, ast_node * param_decl_list_node)
{
	print_nonterminal("ParamDeclListTail");
	ast_node * param_decl_node = param_decl(fd); // ParamDecl node
	param_decl_list_tail_(fd, param_decl_list_node); // ParamDeclListTail' node

	add_child_node(param_decl_list_node, param_decl_node);
}

static ast_node * param_decl_list(FILE * fd)
{
	print_nonterminal("ParamDeclList");
	switch (lookahead.type)
	{
	case INT:
	case CHAR:
	{
		ast_node * this_node = new_ast_node(new_ast_nonterminal_info(PARAM_DECL_LIST)); // create a ParamDeclList node
		param_decl_list_tail(fd, this_node);
		return this_node;
	}
	case RPAREN:
	case LCURLY:
	{
		ast_node * this_node = new_ast_node(new_ast_nonterminal_info(PARAM_DECL_LIST)); // create a ParamDeclList node
		return this_node;
	}
	default:
		expansion_error();
	}
	return NULL;
}

static ast_node * fun_decl_list(FILE * fd, ast_node * fun_decl_list_node);
static ast_node * fun_decl_(FILE * fd, ast_node * fun_decl_list_node)
{
	print_nonterminal("FunDecl'");
	switch (lookahead.type)
	{
	case INT:
	case CHAR:
	{
		return fun_decl_list(fd, fun_decl_list_node);
	}
	case DONE:
	{
		return NULL;
	}
	default:
		expansion_error();
	}
	return NULL;
}

static void fun_decl_tail(FILE * fd, ast_node * fun_decl_list_node, ast_node * type_node, ast_node * id_node);
static void fun_decl_list_(FILE * fd, ast_node * program_node, ast_node * type_node, ast_node * id_node)
{
	print_nonterminal("FunDeclList'");
	ast_node * this_node = new_ast_node(new_ast_nonterminal_info(FUN_DECL_LIST)); // create a FunDeclList node
	fun_decl_tail(fd, this_node, type_node, id_node); // FunDeclTail node
	fun_decl_list(fd, this_node); // FunDeclList node
	add_child_node(program_node, this_node);
}

static void fun_decl(FILE * fd, ast_node * fun_decl_list_node);
static ast_node * fun_decl_list(FILE * fd, ast_node * fun_decl_list_node)
{
	print_nonterminal("FunDeclList");
	switch (lookahead.type)
	{
	case CHAR:
	case INT:
	{
		fun_decl(fd, fun_decl_list_node); // FunDecl node
		ast_node * fun_decl__node = fun_decl_(fd, fun_decl_list_node); // FunDecl' node
		return fun_decl_list_node;
	}
	case DONE:
	{
		return NULL;
	}
	default:
		expansion_error();
	}
	return NULL;
}

static ast_node * var_decl_(FILE * fd)
{
	print_nonterminal("VarDecl'");
	switch (lookahead.type)
	{
	case SEMICOLON:
	{
		comp(fd, SEMICOLON, 0);
		return NULL;
	}
	case LBRACKET:
	{
		comp(fd, LBRACKET, 0);
		token t = comp(fd, NUM, 0);
		ast_node * num_node = new_ast_node(new_ast_terminal_info(t)); // create a num node
		comp(fd, RBRACKET, 1);
		comp(fd, SEMICOLON, 1);
		return num_node;
	}
	default:
		expansion_error();
	}

	return NULL;
}

static ast_node * var_decl(FILE * fd)
{
	print_nonterminal("VarDecl");
	ast_node * type_node = type(fd); // Type node
	token t = comp(fd, ID, 0);
	ast_node * id_node = new_ast_node(new_ast_terminal_info(t)); // create an id node
	ast_node * var_decl_node = var_decl_(fd); // VarDecl' node

	ast_node * this_node = new_ast_node(new_ast_nonterminal_info(VAR_DECL)); // create a VarDecl node
	add_child_node(this_node, type_node);
	add_child_node(this_node, id_node);
	if (var_decl_node != NULL)
		add_child_node(this_node, var_decl_node);
	return this_node;
}

static void decl(FILE * fd, ast_node * program_node);
static void var_decl_list_(FILE * fd, ast_node * program_node, ast_node * type_node, ast_node * id_node)
{
	print_nonterminal("VarDeclList'");
	ast_node * var_decl_node = var_decl_(fd); // VarDecl' node
	decl(fd, program_node); // Decl node

	ast_node * this_node = new_ast_node(new_ast_nonterminal_info(VAR_DECL)); // create a VarDecl node

	add_child_node(this_node, type_node);
	add_child_node(this_node, id_node);
	if (var_decl_node != NULL) // could be just SEMICOLON
		add_child_node(this_node, var_decl_node);
	add_child_node(program_node, this_node);
}

static void var_decl_list(FILE * fd, ast_node * var_decl_list_node)
{
	print_nonterminal("VarDeclList");
	switch (lookahead.type)
	{
	case SEMICOLON:
	case RETURN:
	case READ:
	case WRITE:
	case WRITELN:
	case BREAK:
	case IF:
	case WHILE:
	case NEG:
	case MINUS:
	case ID:
	case LPAREN:
	case NUM:
	case LCURLY:
	case DONE:
	{
		return;
	}
	case INT:
	case CHAR:
	{
		ast_node * var_decl_node = var_decl(fd); // VarDecl node
		add_child_node(var_decl_list_node, var_decl_node);
		var_decl_list(fd, var_decl_list_node); // VarDeclList
		return;
	}
	default:
		expansion_error();
	}
}

static void fun_decl_tail(FILE * fd, ast_node * fun_decl_list_node, ast_node * type_node, ast_node * id_node)
{
	print_nonterminal("FunDeclTail");
	comp(fd, LPAREN, 0);
	ast_node * param_decl_list_node = param_decl_list(fd); // ParamDeclList node
	comp(fd, RPAREN, 1);
	ast_node * block_node = block(fd); // Block node

	ast_node * this_node = new_ast_node(new_ast_nonterminal_info(FUN_DECL)); // create a FunDecl node

	add_child_node(this_node, type_node);
	add_child_node(this_node, id_node);
	add_child_node(this_node, param_decl_list_node);
	add_child_node(this_node, block_node);
	add_child_node(fun_decl_list_node, this_node);
}

static void fun_decl(FILE * fd, ast_node * fun_decl_list_node)
{
	print_nonterminal("FunDecl");
	ast_node * fun_type_node = fun_type(fd); // FunType node
	token t = comp(fd, ID, 0);
	ast_node * id_node = new_ast_node(new_ast_terminal_info(t)); // create an id node
	fun_decl_tail(fd, fun_decl_list_node, fun_type_node, id_node); // FunDeclTail node
}

static void program_(FILE * fd, ast_node * program_node, ast_node * type_node, ast_node * id_node)
{
	print_nonterminal("Program'");
	switch (lookahead.type)
	{
	case SEMICOLON:
	case LBRACKET:
	{
		var_decl_list_(fd, program_node, type_node, id_node); // VarDeclList node
		return;
	}
	case LPAREN:
	{
		fun_decl_list_(fd, program_node, type_node, id_node); // FunDeclList' node
		return;
	}
	default:
		expansion_error();
	}
}

static void decl(FILE * fd, ast_node * program_node)
{
	print_nonterminal("Decl");
	switch (lookahead.type)
	{
	case CHAR:
	case INT:
	{
		ast_node * type_node = type(fd); // Type node
		token t = comp(fd, ID, 0);
		ast_node * id_node = new_ast_node(new_ast_terminal_info(t)); // create an id node
		program_(fd, program_node, type_node, id_node); // Program' node
		return;
	}
	case ID:
	{
		ast_node * id_node = new_ast_node(new_ast_terminal_info(lookahead)); // create an id node
		comp(fd, ID, 0);
		fun_decl_list_(fd, program_node, NULL, id_node); // FunDeclList' node

		add_child_node(program_node, id_node);
		return;
	}
	default:
		expansion_error();
	}
}

/**************************************************************************/
/*
 *  this function corresponds to the start symbol in the LL(1) grammar
 *  when this function returns, the full AST tree with parent node "parent"
 *  will be constructed
 *         fd: the input stream
 *     parent: the parent ast node  (it should be a ROOT)
 */
static void program(FILE *fd, ast_node *parent) {

  print_nonterminal("Program");

  parser_debug0("in program\n");

  // assert is useful for testing a function's pre and post conditions
  assert(parent->symbol->token == NONTERMINAL);
  assert(parent->symbol->grammar_symbol == ROOT);

  decl(fd, parent);

}
