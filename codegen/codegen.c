// functions for generating MIPS code given an AST of a C-- program
// from the parser

// TODO: add more includes files here as necessary
#include <stdio.h>
#include <assert.h>
#include "parser.h"
#include "codegen.h"
#include "codetable.h"
#include "lexer.h"

// constants for "and" and "or" labels
int and_label = -1;
int or_label = -1;

// function declarations
VarAddress lookup_variable(const char * name);
VarAddress lookup_array(const char * name);
FunDef lookup_function(const char* name);

// this function will be called after your parse function
// depending on how you are storing the AST (a global or a return
// value from parse, you may need to add some parameters to this function
void codegen(FILE * out, ast_node * root) {
  codetable_init();
  init_registers();
  handle_program(root);
  if (codetable_print(out) == 0) {
    printf("Success\n");
  } else {
    printf("Error writing instructions\n");
  }
  codetable_destroy();
}

void free_register(int reg) {
    if (reg < REGISTER_T_OFFSET || reg >= REGISTER_COUNT + REGISTER_T_OFFSET)
        return; // error
    registers[reg - REGISTER_T_OFFSET] = 1;
}

void init_registers() {
    int i = 0;
    for (i = 0; i < REGISTER_COUNT; i++) {
        registers[i] = 1;
    }
}

int allocate_register() {
    int i = 0;
    for (i = 0; i < REGISTER_COUNT; i++) {
        if (registers[i] == 1) {
            registers[i] = 0;
            return i + REGISTER_T_OFFSET;
        }
    }
    printf("Could not allocate a register\n");
    return -1;
}

handle_ptr get_handle_function(ast_node * node) {
    printf("token: %d\n", node->symbol->token);

    if (node->symbol->token != NONTERMINAL) {
        switch (node->symbol->token) {
            case ASSIGN:
                return handle_assign;
            case WRITELN:
                return handle_writeln;
            case WRITE:
                return handle_write;
            case READ:
                return handle_read;
            case PLUS:
                return handle_plus;
            case MULT:
                return handle_mult;
            case DIV:
                return handle_div;
            case MINUS:
                return handle_minus;
            case OR:
                return handle_or;
            case AND:
                return handle_and;
            case NEG:
                return handle_not;
            case EQU:
                return handle_equ;
            case NEQ:
                return handle_neq;
            case LSS:
                return handle_lss;
            case LEQ:
                return handle_leq;
            case GTR:
                return handle_gtr;
            case GEQ:
                return handle_geq;
            case IF:
                return handle_if;
            case WHILE:
                return handle_while;
            case NUM:
                return handle_num;
            case ID:
                return handle_id;
            case BREAK:
                return handle_break;
            case RETURN:
                return handle_return;
        }
    } else {
        switch (node->symbol->grammar_symbol) {
            case BLOCK_N:
                return handle_block;
        }
    }
    printf("No handle function found\n");
    return NULL;
}


// handle functions for each state in AST
int handle_return(ast_node * node) {
    printf("Handle Return\n");
    int reg;
    reg = get_handle_function(get_childlist(node)[0])(get_childlist(node)[0]);
    add_instruction(create_instruction(MOVE, v0, reg, 0));
    free_register(reg);
    return 0;
}

int handle_num(ast_node * node) {
    int dest_reg = allocate_register();
    add_instruction(create_instruction(LI, dest_reg, node->symbol->value, 0));

    return dest_reg;
}

void compute_index(VarAddress var, int index_reg) {
	int var_size_reg = allocate_register();
	add_instruction(create_instruction(LI, var_size_reg, var.size, 0));
	add_instruction(create_instruction(MUL, index_reg, index_reg, var_size_reg));
	free_register(var_size_reg);
	add_instruction(create_instruction(SUB, index_reg, sp, index_reg));
	add_instruction(create_instruction(ADDI, index_reg, index_reg, var.offset));
}

int handle_id(ast_node * node) {
	printf("Handle ID\n");
    int dest_reg = allocate_register();
    ast_info * info = NULL;
    ast_node ** args = get_childlist(node);
    int num_args = get_num_children(node);
    VarAddress var;

    info = node->symbol;
    if (num_args == 0) {
    	var = lookup_variable(info->lexeme);
        if (var.offset < 0) {
            handle_error("error: variable undeclared (first use in this function)", info->line_no);
        }
        if (var.size == 1)
            add_instruction(create_instruction_offset(LB, dest_reg, sp, 0, var.offset));
        else
            add_instruction(create_instruction_offset(LW, dest_reg, sp, 0, var.offset));
    } else {
        if (args[0]->symbol->grammar_symbol == EXPR_LIST) {
            call_function(node);
            add_instruction(create_instruction(MOVE, dest_reg, v0, 0));
        }
        else {
    		var = lookup_array(info->lexeme);
		    if (var.offset < 0) {
		        handle_error("error: array undeclared (first use in this function)", info->line_no);
		    }

        	int index_reg = get_handle_function(args[0])(args[0]);
        	compute_index(var, index_reg);
        	if (var.size == 1)
		        add_instruction(create_instruction_offset(LB, dest_reg, index_reg, 0, 0)); // load array element address
		    else
		        add_instruction(create_instruction_offset(LW, dest_reg, index_reg, 0, 0)); // load array element address
		    free_register(index_reg);
        }
    }

    return dest_reg;
}

int handle_while(ast_node * node) {
    printf("Handle WHILE\n");

    ast_node ** args = get_childlist(node);
    int while_cond_reg;
    int while_label_sn = get_next_label_sn(LABEL_WHILE);
    int while_end_label_sn = get_next_label_sn(LABEL_WHILE_END);
    int reg = 0;

    // create while condition label
    add_instruction(create_instruction_label(LABEL_WHILE, while_label_sn));
    // jump based on the condition
    while_cond_reg = get_handle_function(args[0])(args[0]);
    add_instruction(create_jump_instruction(BEQZ, while_cond_reg, 0, LABEL_WHILE_END, while_end_label_sn));
    // while body
    reg = get_handle_function(args[1])(args[1]);
    if (reg != 0) free_register(reg);
    // jump to while condition
    add_instruction(create_jump_instruction(J_I, 0, 0, LABEL_WHILE, while_label_sn));
    // add while_end label
    add_instruction(create_instruction_label(LABEL_WHILE_END, while_end_label_sn));

    return 0;
}

int handle_break(ast_node * node) {
    printf("Handle BREAK\n");
    add_instruction(create_jump_instruction(J_I, 0, 0, LABEL_WHILE_END, get_last_label_sn(LABEL_WHILE_END)));
    return 0;
}

int handle_if(ast_node * node) {
    printf("Handle IF\n");

    ast_node ** args = get_childlist(node);
    int if_reg;
    int else_label_sn = get_next_label_sn(LABEL_ELSE);
    int if_else_end_sn = get_next_label_sn(LABEL_IF_ELSE_END);

    if_reg = get_handle_function(args[0])(args[0]);
    // jump to else label based on the condition
    add_instruction(create_jump_instruction(BEQZ, if_reg, 0, LABEL_ELSE, else_label_sn));
    // if body
    get_handle_function(args[1])(args[1]);
    // jump to if else end
    add_instruction(create_jump_instruction(J_I, 0, 0, LABEL_IF_ELSE_END, if_else_end_sn));
    // handle else body
    handle_else(args[2], else_label_sn);
    // add if else end label
    add_instruction(create_instruction_label(LABEL_IF_ELSE_END, if_else_end_sn));
    return 0;
}

int handle_else(ast_node * node, int label_sn) {
    printf("Handle ELSE\n");
    ast_node ** args = get_childlist(node);
    // add else label
    add_instruction(create_instruction_label(LABEL_ELSE, label_sn));
    get_handle_function(args[0])(args[0]);

    return 0;
}

int handle_geq(ast_node * node) {
    printf("Handle GEQ\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;
    int compare_sn = get_next_label_sn(LABEL_COMPARE);
    int compare_end_sn = get_next_label_sn(LABEL_COMPARE_END);

    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }


    add_instruction(create_jump_instruction(BGE, arg0_reg, arg1_reg, LABEL_COMPARE, compare_sn));

    add_instruction(create_instruction(XOR, arg0_reg, arg0_reg, arg0_reg));
    add_instruction(create_jump_instruction(J_I, 0, 0, LABEL_COMPARE_END, compare_end_sn));

    add_instruction(create_instruction_label(LABEL_COMPARE, compare_sn));
    add_instruction(create_instruction(LI, arg0_reg, 1, 0));
    add_instruction(create_instruction_label(LABEL_COMPARE_END, compare_end_sn));

    free_register(arg1_reg);

    return arg0_reg;
}

int handle_gtr(ast_node * node) {
    printf("Handle GTR\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;


    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }

    add_instruction(create_instruction(SLT, arg0_reg, arg1_reg, arg0_reg));
    free_register(arg1_reg);

    return arg0_reg;
}

int handle_leq(ast_node * node) {
    printf("Handle LEQ\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;
    int compare_sn = get_next_label_sn(LABEL_COMPARE);
    int compare_end_sn = get_next_label_sn(LABEL_COMPARE_END);

    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }


    add_instruction(create_jump_instruction(BLE, arg0_reg, arg1_reg, LABEL_COMPARE, compare_sn));

    add_instruction(create_instruction(XOR, arg0_reg, arg0_reg, arg0_reg));
    add_instruction(create_jump_instruction(J_I, 0, 0, LABEL_COMPARE_END, compare_end_sn));

    add_instruction(create_instruction_label(LABEL_COMPARE, compare_sn));
    add_instruction(create_instruction(LI, arg0_reg, 1, 0));
    add_instruction(create_instruction_label(LABEL_COMPARE_END, compare_end_sn));

    free_register(arg1_reg);

    return arg0_reg;
}

int handle_lss(ast_node * node) {
    printf("Handle LSS\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;

    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }

    add_instruction(create_instruction(SLT, arg0_reg, arg0_reg, arg1_reg));
    free_register(arg1_reg);

    return arg0_reg;
}

int handle_neq(ast_node * node) {
    printf("Handle NEQ\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;

    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }

    add_instruction(create_instruction(XOR, arg0_reg, arg0_reg, arg1_reg));
    free_register(arg1_reg);

    return arg0_reg;
}

int handle_equ(ast_node * node) {
    printf("Handle EQU\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;
    int zero_register;
    int one_register;

    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }

    add_instruction(create_instruction(XOR, arg0_reg, arg0_reg, arg1_reg));
    // negate the logical value
    zero_register = allocate_register();
    one_register = allocate_register();
    add_instruction(create_instruction(AND_I, zero_register, zero_register, ZERO));
    add_instruction(create_instruction(LI, one_register, 1, 0));
    add_instruction(create_instruction(MOVZ, zero_register, one_register, arg0_reg));
    free_register(one_register);
    free_register(arg1_reg);
    free_register(arg0_reg);

    return zero_register;
}

int handle_not(ast_node * node) {
    int arg_reg;
    printf("Handle NEG\n");

    ast_node * arg = get_childlist(node)[0];
    arg_reg = get_handle_function(arg)(arg);
    add_instruction(create_instruction(NOT_I, arg_reg, arg_reg, 0));
    return arg_reg;
}

int handle_and(ast_node * node) {
    printf("Handle AND\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;
    int first_and = 0;
    int last_label;

    if (and_label == -1) {
        and_label = get_next_label_sn(LABEL_COMPARE_END);
        first_and = 1;
    } else {
    }

    //add_instruction(create_jump_instruction(BEQZ, if_reg, LABEL_ELSE, else_label_sn));
    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }

    add_instruction(create_jump_instruction(BEQZ, arg0_reg, 0, LABEL_COMPARE_END, and_label));
    add_instruction(create_jump_instruction(BEQZ, arg1_reg, 0, LABEL_COMPARE_END, and_label));
    add_instruction(create_instruction(LI, arg0_reg, 1, 0));

    if (first_and) {
        last_label = get_next_label_sn(LABEL_COMPARE_END);
        add_instruction(create_jump_instruction(J_I, 0, 0, LABEL_COMPARE_END, last_label));
        add_instruction(create_instruction_label(LABEL_COMPARE_END, and_label));
        add_instruction(create_instruction(LI, arg0_reg, 0, 0));
        add_instruction(create_instruction_label(LABEL_COMPARE_END, last_label));
        and_label = -1;
    }

    free_register(arg1_reg);

    return arg0_reg;
}

int handle_or(ast_node * node) {
    printf("Handle OR\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;
    int first_or = 0;
    int last_label;

    if (or_label == -1) {
        or_label = get_next_label_sn(LABEL_COMPARE_END);
        first_or = 1;
    } else {
    }

    //add_instruction(create_jump_instruction(BEQZ, if_reg, LABEL_ELSE, else_label_sn));
    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }

    add_instruction(create_jump_instruction(BNEZ, arg0_reg, 0, LABEL_COMPARE_END, or_label));
    add_instruction(create_jump_instruction(BNEZ, arg1_reg, 0, LABEL_COMPARE_END, or_label));
    add_instruction(create_instruction(LI, arg0_reg, 0, 0));

    if (first_or) {
        last_label = get_next_label_sn(LABEL_COMPARE_END);
        add_instruction(create_jump_instruction(J_I, 0, 0, LABEL_COMPARE_END, last_label));
        add_instruction(create_instruction_label(LABEL_COMPARE_END, or_label));
        add_instruction(create_instruction(LI, arg0_reg, 1, 0));
        add_instruction(create_instruction_label(LABEL_COMPARE_END, last_label));
        or_label = -1;
    }

    free_register(arg1_reg);

    return arg0_reg;
}

int handle_div(ast_node * node) {
    printf("Handle Div\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;


    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }

    add_instruction(create_instruction(DIV_I, arg0_reg, arg1_reg, 0));
    add_instruction(create_instruction(MFLO, arg0_reg, 0, 0));
    free_register(arg1_reg);

    return arg0_reg;
}

int handle_plus(ast_node * node) {
    printf("Handle Plus\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;


    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }

    add_instruction(create_instruction(ADD, arg0_reg, arg0_reg, arg1_reg));
    free_register(arg1_reg);

    return arg0_reg;
}

int handle_minus(ast_node * node) {
    printf("Handle Minus\n");

    ast_node ** args = get_childlist(node);
    int num_args = get_num_children(node);
    int arg0_reg;
    int arg1_reg;

    if (num_args == 2) {
        if (args[0]->num_children == 0) {
            arg1_reg = get_handle_function(args[1])(args[1]);
            arg0_reg = get_handle_function(args[0])(args[0]);
        } else {
            arg0_reg = get_handle_function(args[0])(args[0]);
            arg1_reg = get_handle_function(args[1])(args[1]);
        }
        add_instruction(create_instruction(SUB, arg0_reg, arg0_reg, arg1_reg));
        free_register(arg1_reg);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        add_instruction(create_instruction(SUB, arg0_reg, ZERO, arg0_reg));
    }
    return arg0_reg;
}

int handle_mult(ast_node * node) {
    printf("Handle Mult\n");

    ast_node ** args = get_childlist(node);
    int arg0_reg;
    int arg1_reg;


    if (args[0]->num_children == 0) {
        arg1_reg = get_handle_function(args[1])(args[1]);
        arg0_reg = get_handle_function(args[0])(args[0]);
    } else {
        arg0_reg = get_handle_function(args[0])(args[0]);
        arg1_reg = get_handle_function(args[1])(args[1]);
    }

    add_instruction(create_instruction(MUL, arg0_reg, arg0_reg, arg1_reg));
    free_register(arg1_reg);

    return arg0_reg;
}

int handle_write(ast_node * node) {
    printf("Handle Write\n");
    ast_node * arg = get_childlist(node)[0];
    int result_reg = get_handle_function(arg)(arg);
    free_register(result_reg);
    add_instruction(create_instruction(MOVE, a0, result_reg, 0));
    add_instruction(create_instruction(LI, v0, 1, 0));
    add_instruction(create_instruction(SYSCALL, 0, 0, 0));

    return 0;
}

int handle_writeln(ast_node * node) {
    printf("Handle Writeln\n");
    add_instruction(create_instruction(LI, v0, 4, 0));
    add_instruction(create_instruction(LA, a0, 0, 0));
    add_instruction(create_instruction(SYSCALL, 0, 0, 0));

    return 0;
}

void handle_stmt_list(ast_node * node) {
    int i = 0;
    ast_node ** statements = get_childlist(node);
    int num_statements = get_num_children(node);
    int reg = 0;
    printf("Handle StmtList\n");
    for (i = 0; i < num_statements; i++) {
        handle_ptr handle_fun = get_handle_function(statements[i]);

        if (handle_fun != NULL) {
            reg = handle_fun(statements[i]);
            if (reg != 0) free_register(reg);
        }

    }
}

void handle_var_decl_list(ast_node * node) {
    printf("Handle VarDeclList\n");
    int scope_size = 0;
    int i = 0;
    ast_node ** vars = get_childlist(node);
    int num_vars = get_num_children(node);

    for (i = 0; i < num_vars; i++) {
        handle_var_decl(vars[i]);
    }

    if ((scope_size = get_scope_size()) > 0)
        add_instruction(create_instruction(ADDI, sp, sp, -1 * scope_size)); //allocate space for the variables
}

void handle_var_decl(ast_node * node) {
    printf("Handle VarDecl\n");
    VARTYPE type;
    ast_node ** args = get_childlist(node);
    int num_args = get_num_children(node);

	type = (args[0]->symbol->token == INT) ? T_INT : T_CHAR;
    if (num_args == 2) {
        add_variable(type, (const char *) args[1]->symbol->lexeme);
    } else {
    	int arr_size = args[2]->symbol->value;
        add_array(type, (const char *) args[1]->symbol->lexeme, arr_size, 0);
    }
}

int handle_block(ast_node * node) {
    printf("Handle Block\n");
    int scope_size = 0;

    add_scope();
    handle_var_decl_list(get_childlist(node)[0]);
    handle_stmt_list(get_childlist(node)[1]);

    if ((scope_size = get_scope_size()) > 0)
        add_instruction(create_instruction(ADDI, sp, sp, scope_size));
    destroy_scope(1);

    return 0;
}

void handle_fun_decl(ast_node * node) {

    printf("Handle FunDecl\n");
    ast_node ** args = get_childlist(node);
    int type;
    FunDef dummy;
    int scope_size = 0;

    dummy = lookup_function(args[1]->symbol->lexeme);
    if (dummy.name != NULL) handle_error("error: function already defined.", node->symbol->line_no);
    add_instruction(create_instruction_named_label(FUNCTION, (const char *) args[1]->symbol->lexeme));
    add_instruction(create_instruction_text(FUN_PREAMBLE));
    adjust_stack_height(8);

    add_scope(); //a scope for parameters
    type = (args[0]->symbol->token == INT) ? T_INT : T_CHAR;
    define_function(type, (const char *) args[1]->symbol->lexeme);
    handle_param_decl_list(args[2]);

    copy_parameters();
    handle_block(args[3]);

    if ((scope_size = get_scope_size()) > 0)
        add_instruction(create_instruction(ADDI, sp, sp, scope_size)); //destroy block
    destroy_scope(1);
    add_instruction(create_instruction_text(FUN_EPILOG));
    adjust_stack_height(-8);
}

void handle_fun_decl_list(ast_node * node) {
    printf("Handle FunDeclList\n");
    int i = 0;
    ast_node ** functions = get_childlist(node);
    int num_functions = get_num_children(node);
    for (i = 0; i < num_functions; i++) {
        handle_fun_decl(functions[i]);
    }
}

void handle_param_decl_list(ast_node * node) {
    printf("Handle ParamDeclList\n");
    int i = 0;
    int scope_size = 0;

    ast_node ** params = get_childlist(node);
    int num_params = get_num_children(node);
    for (i = num_params-1; i >= 0 ; i--) {
        handle_param_decl(params[i]);
    }

    if ((scope_size = get_scope_size()) > 0)
        add_instruction(create_instruction(ADDI, sp, sp, -1 * scope_size)); //allocate space for the variables
}

void handle_param_decl(ast_node * node) {
    printf("Handle ParamDecl\n");
    VARTYPE type;
    ast_node ** args = get_childlist(node);
    int num_args = get_num_children(node);

	type = (args[0]->symbol->token == INT) ? T_INT : T_CHAR;
    if (num_args == 2) {
        add_parameter(type, 0, (const char *) args[1]->symbol->lexeme); // variable
    } else {
    	add_parameter(type, 1, (const char *) args[1]->symbol->lexeme); // array
    }

}

int handle_assign(ast_node * node) {
    printf("Handle Assign\n");
    int arg1_reg = -1;
    ast_info * info = NULL;
    VarAddress var;

    ast_node ** args = get_childlist(node); // a  = b = c
    info = args[0]->symbol;
    if (info->token != ID) handle_error("error: incompatible lvalue.", info->line_no);

    if (get_num_children(args[0]) == 0) {
    	var = lookup_variable(info->lexeme); // var
    	if (var.offset < 0) {
     	   handle_error("error: variable undeclared (first use in this function)", info->line_no);
    	}
	}
	else {
		var = lookup_array(info->lexeme); // array element
		if (var.offset < 0) {
    	    handle_error("error: array undeclared (first use in this function)", info->line_no);
    	}
	}

    arg1_reg = get_handle_function(args[1])(args[1]);

	if (get_num_children(args[0]) == 0) {
		if (var.size == 1)
		    add_instruction(create_instruction_offset(SB, arg1_reg, sp, 0, var.offset));
		else
		    add_instruction(create_instruction_offset(SW, arg1_reg, sp, 0, var.offset));
    }
    else {
    	ast_node * index_node = get_childlist(args[0])[0];
    	int index_reg = get_handle_function(index_node)(index_node);
    	compute_index(var, index_reg);
    	if (var.size == 1)
    		add_instruction(create_instruction_offset(SB, arg1_reg, index_reg, 0, 0)); // save to array element address
		else
    		add_instruction(create_instruction_offset(SW, arg1_reg, index_reg, 0, 0)); // save to array element address
    	free_register(index_reg);
    }

    return arg1_reg;

}

int handle_read(ast_node * node) {
    printf("Handle Read\n");
    ast_info * info = NULL;
    VarAddress var;

    ast_node ** args = get_childlist(node); // a  = b = c
    info = args[0]->symbol;
    if (info->token != ID) handle_error("error: incompatible rvalue.", info->line_no);
    if ((var = lookup_variable(info->lexeme)).offset < 0) {
        handle_error("error: variable undeclared (first use in this function)", info->line_no);
    }


    if (var.size == 1) {
        add_instruction(create_instruction(LI, v0, 12, 0));
        add_instruction(create_instruction(SYSCALL, 0, 0, 0));
        add_instruction(create_instruction_offset(SB, v0, sp, 0, var.offset));
    } else {
        add_instruction(create_instruction(LI, v0, 5, 0));
        add_instruction(create_instruction(SYSCALL, 0, 0, 0));
        add_instruction(create_instruction_offset(SW, v0, sp, 0, var.offset));
    }

    return 0;

}

int handle_expr_list(ast_node * node, FunDef fun) {
    printf("Handle ExprList\n");
    ast_node ** args = get_childlist(node);
    int num_args = get_num_children(node);
    int reg;
    int i;
    int is_array = 0;

    if (num_args < fun.param_count) handle_error("error: too few arguments to function", node->symbol->line_no);
    if (num_args > fun.param_count) handle_error("error: too many arguments to function", node->symbol->line_no);

    for (i = 0; i < fun.param_count; i++) {
        VarAddress var;
        var = lookup_variable(args[i]->symbol->lexeme);
        if (var.offset < 0) {
        	var = lookup_array(args[i]->symbol->lexeme);
        	if (var.offset >= 0)
        		is_array = 1;
		}
        if (var.offset >= 0) {
            if (var.size != (fun.param_type[i] == T_INT ? 4 : 1)) {
                handle_error("error: non-matching argument types", node->symbol->line_no);
            }
        }

        if (is_array) {
        	if (var.offset < 0) {
		        handle_error("error: array undeclared (first use in this function)", node->symbol->line_no);
		    }
        } else {
        	reg = get_handle_function(args[i])(args[i]);
        	add_instruction(create_instruction(MOVE, 4 + i, reg, 0));
        }
    }
    free_register(reg);
    return 0;
}

void handle_program(ast_node * node) {
    printf("Handle Program\n");
    int i = 0;
    ast_node ** args = get_childlist(node);
    int num_children = get_num_children(node);
    add_scope();
    for (i = num_children - 1; i > 0; i--) {
        handle_var_decl(args[i]);
    }
    handle_fun_decl_list(args[i]);
    destroy_scope(1);
}

void handle_error(const char * msg, int line) {
    fprintf(stderr, "Line %d: %s\n", line + 1, msg);
    while (destroy_scope(0) >= 0); //cleanup
    exit(1);
}

int call_function(ast_node * node) {
    FunDef fun = lookup_function(node->symbol->lexeme);

    if (fun.name == NULL) handle_error("error: function not declared.", node->symbol->line_no);
    //backup_params(&fun);

    handle_expr_list(get_childlist(node)[0], fun);
    save_registers();
    add_instruction(create_jump_label_instruction(JAL, 0, 0, fun.name));
    restore_registers();
    return 0;
}

void save_registers() {
    int i;
    add_instruction(create_instruction(ADDI, sp, sp, -1 * 32)); //allocate space for the registers
    for (i = 0; i < REGISTER_COUNT; i++) {
        add_instruction(create_instruction_offset(SW, i + REGISTER_T_OFFSET, sp, 0, 4+i * 4));
    }
    adjust_stack_height(32);
}

void restore_registers() {
    int i;
    for (i = 0; i < REGISTER_COUNT; i++) {
        add_instruction(create_instruction_offset(LW, i + REGISTER_T_OFFSET, sp, 0, 4+i * 4));
    }
    add_instruction(create_instruction(ADDI, sp, sp, 1 * 32)); //restore the stack pointer
    adjust_stack_height(-32);
}

void backup_params(FunDef * fun) {
}

#define FUNCTIONS_SIZE 50

typedef struct Scope {
    Symentry * variables;
    int variables_count;
    int variables_max;
    struct Scope * parent;
} Scope;


int current_stack_height = 0;
Scope * current_scope = NULL;
FunDef * current_function = NULL;
FunDef functions[FUNCTIONS_SIZE];
int functions_count = 0;

void add_scope() {
    Scope * scope = (Scope *) malloc(sizeof (Scope));

    scope->variables = NULL;
    scope->variables_count = 0;
    scope->variables_max = 0;
    scope->parent = current_scope;
    current_scope = scope;
}

int destroy_scope(int verbose) {
    if (verbose) ;
    Scope * scope = current_scope;
    if (scope == NULL) return -1;
    current_scope = scope->parent;
    free(scope->variables);
    free(scope);
    return 0;
}

static void extend_scope() {
    int new_size;
    if (current_scope->variables_max > 0) {
        new_size = current_scope->variables_max * 2;
        current_scope->variables = (Symentry *) realloc(current_scope->variables, new_size * sizeof (Symentry));
    } else {
        new_size = 2;
        current_scope->variables = (Symentry *) malloc(new_size * sizeof (Symentry));
    }
    current_scope->variables_max = new_size;
}

static int get_var_size(VARTYPE type)
{
	return (type == T_INT) ? 4 : 1;
}

static void add_variable_to_scope(Symentry entry) {
    int x = current_scope->variables_count++;
    current_scope->variables[x] = entry;
}

static Symentry create_symentry(const char * name, int size, int stack_height, int is_array, int count)
{
	Symentry entry;
	entry.name = name;
    entry.size = size;
    entry.stack_height = stack_height;
    entry.is_array = is_array;
    entry.count = count;
    return entry;
}

static void extend_scope_if_needed()
{
	if (current_scope->variables_count >= current_scope->variables_max)
        extend_scope();
}

void add_variable(VARTYPE type, const char * name) {
    int size = get_var_size(type);
    extend_scope_if_needed();
    add_variable_to_scope(create_symentry(name, size, current_stack_height, 0, 1));
    current_stack_height += size;
}

void add_array(VARTYPE type, const char * name, int arr_size, int reference)
{
	int size = get_var_size(type);
    extend_scope_if_needed();
    add_variable_to_scope(create_symentry(name, size, current_stack_height, 1, arr_size));
    current_stack_height += size * arr_size;
}

int get_scope_size() {
    int size, padding;
    if (current_scope->variables_count == 0) return 0;
    size = current_stack_height - current_scope->variables[0].stack_height;
    padding = (4 - size % 4) % 4;
    current_stack_height += padding;
    return size + padding;
}

void adjust_stack_height(int offset) {
    current_stack_height += offset;
}

int get_current_stack_height() {
	return current_stack_height;
}

/**
 * @return: offset of the variable from the top of the stack
 */
VarAddress lookup_variable(const char * name) {
    Scope * current = current_scope;
    int i;
    VarAddress var;

    while (current != NULL) {
        for (i = 0; i < current->variables_count; i++) {
            if (!strcmp(current->variables[i].name, name) && !current->variables[i].is_array) {
                var.offset = current_stack_height - current->variables[i].stack_height;
                var.size = current->variables[i].size;
                var.count = current->variables[i].count;

                return var;
            }
        }
        current = current->parent;
    }
    var.offset = -1;
    var.size = -1;

    return var;
}

/**
 * @return: offset of the array from the top of the stack
 */
VarAddress lookup_array(const char * name) {
    Scope * current = current_scope;
    int i;
    VarAddress arr;

    while (current != NULL) {
        for (i = 0; i < current->variables_count; i++) {
            if (!strcmp(current->variables[i].name, name) && current->variables[i].is_array) {
                arr.offset = current_stack_height - current->variables[i].stack_height;
                arr.size = current->variables[i].size;
                arr.count = current->variables[i].count;

                return arr;
            }
        }
        current = current->parent;
    }
    arr.offset = -1;
    arr.size = -1;

    return arr;
}

VarAddress lookup_variable_in_current_scope(const char * name) {
    int i;
    VarAddress var;

    for (i = 0; i < current_scope->variables_count; i++) {
        if (!strcmp(current_scope->variables[i].name, name) && !current_scope->variables[i].is_array) {
            var.offset = current_stack_height - current_scope->variables[i].stack_height;
            var.size = current_scope->variables[i].size;
            var.count = current_scope->variables[i].count;

            return var;
        }
    }
    var.offset = -1;
    var.size = -1;

    return var;
}

VarAddress lookup_array_in_current_scope(const char * name) {
    int i;
    VarAddress arr;

    for (i = 0; i < current_scope->variables_count; i++) {
        if (!strcmp(current_scope->variables[i].name, name) && current_scope->variables[i].is_array) {
            arr.offset = current_stack_height - current_scope->variables[i].stack_height;
            arr.size = current_scope->variables[i].size;
            arr.count = current_scope->variables[i].count;

            return arr;
        }
    }
    arr.offset = -1;
    arr.size = -1;

    return arr;
}

void set_array_stack_height_in_current_scope(const char * name, int stack_height) {
	int i;
	for (i = 0; i < current_scope->variables_count; i++) {
        if (!strcmp(current_scope->variables[i].name, name) && current_scope->variables[i].is_array) {
        	current_scope->variables[i].stack_height = stack_height;
        	return;
        }
    }
}

/***FUNCTIONS***/
FunDef define_function(VARTYPE type, const char * name) {
    FunDef function;

    function.name = name;
    function.type = type;
    function.param_count = 0;
    functions[functions_count] = function;
    set_current_function(&(functions[functions_count++]));

    return function;
}

FunDef lookup_function(const char* name) {
    int i;
    FunDef dummy;
    for (i = 0; i < functions_count; i++) {
        if (!strcmp(functions[i].name, name)) {
            return functions[i];
        }
    }
    dummy.name = NULL;
    return dummy;
}

void set_current_function(FunDef * fun) {
    current_function = fun;
}

void add_parameter(VARTYPE type, int is_array, const char * name) {
    current_function->param_type[current_function->param_count] = type;
    current_function->is_array[current_function->param_count] = is_array;
    current_function->param_name[current_function->param_count++] = name;
    if (is_array)
    	add_array(type, name, 0, 1);
    else
    	add_variable(type, name);

}

void copy_parameters() {
    int i;
    VarAddress var;

    for (i = 0; i < current_function->param_count; i++) {
    	if (current_function->is_array[i]) {
        	var = lookup_array_in_current_scope(current_function->param_name[i]);
        } else {
        	var = lookup_variable_in_current_scope(current_function->param_name[i]);
		    if (var.size == 1)
		        add_instruction(create_instruction_offset(SB, 4 + i, sp, 0, var.offset));
		    else
		        add_instruction(create_instruction_offset(SW, 4 + i, sp, 0, var.offset));
        }
    }
}
