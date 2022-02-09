#ifndef _CODEGEN_H
#define _CODEGEN_H


// you likely will need to include other
// header files from your compiler here
#include "parser.h"

#define REGISTER_T_OFFSET 8
#define REGISTER_COUNT 8

// add all definitions exported by your code gen modules here
extern void codegen();

int registers[REGISTER_COUNT];
void init_registers();
void free_register(int reg);
int allocate_register();

typedef enum {
    T_INT,
    T_CHAR,
    T_INTARR,
    T_CHARARR
} VARTYPE;

typedef struct {
    int size;
    int offset;
    int count;
} VarAddress;

typedef struct {
    const char * name;
    VARTYPE type;
    VARTYPE param_type[4];
    int is_array[4];
    const char * param_name[4];
    int param_count;

} FunDef;

typedef struct {
    const char * name;
    VARTYPE type;
    int size;
    int stack_height;
    int is_array;
    int count;
} Symentry;

typedef int (*handle_ptr)(ast_node *);
handle_ptr get_handle_function(ast_node * node);

int handle_return(ast_node * node);
int handle_num(ast_node * node);
int handle_while(ast_node * node);
int handle_break(ast_node * node);
int handle_if(ast_node * node);
int handle_id(ast_node * node);
int handle_else(ast_node * node, int label_sn);
int handle_geq(ast_node * node);
int handle_gtr(ast_node * node);
int handle_leq(ast_node * node);
int handle_lss(ast_node * node);
int handle_neq(ast_node * node);
int handle_equ(ast_node * node);
int handle_not(ast_node * node);
int handle_and(ast_node * node);
int handle_or(ast_node * node);
int handle_div(ast_node * node);
int handle_plus(ast_node * node);
int handle_minus(ast_node * node);
int handle_mult(ast_node * node);
int handle_write(ast_node * node);
int handle_writeln(ast_node * node);
int handle_read(ast_node * node);
void handle_stmt_list(ast_node * node);
int handle_expr_list(ast_node * node, FunDef fun);
void handle_var_decl_list(ast_node * node);
void handle_var_decl(ast_node * node);
void handle_param_decl_list(ast_node * node);
void handle_param_decl(ast_node * node);
int handle_block(ast_node * node);
void handle_fun_decl(ast_node * node);
void handle_fun_decl_list(ast_node * node);
void handle_program(ast_node * node);
int handle_assign(ast_node * node);
int call_function(ast_node * node);
void save_registers();
void restore_registers();
void handle_error(const char * msg, int line);
void add_variable(VARTYPE type, const char * name);
void add_array(VARTYPE type, const char * name, int arr_size, int zero_size);
int get_padding();
int get_scope_size();
VarAddress lookup_variable(const char * name);
VarAddress lookup_array(const char * name);
int destroy_scope(int verbose);
void add_scope();
FunDef define_function(VARTYPE type, const char * name);
void add_parameter(VARTYPE type, int is_array, const char * name);
void set_current_function(FunDef * fun);
FunDef lookup_function(const char* name);
void copy_parameters();
void adjust_stack_height(int offset);
int get_current_stack_height();


#endif
