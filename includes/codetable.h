#ifndef _CODETABLE_H
#define _CODETABLE_H

typedef enum {
    LI,
    LA,
    MOVE,
    SYSCALL,
    ADD,
    ADDI,
    SW,
    LW,
    SUB,
    MUL,
    DIV_I,
    MFLO,
    OR_I,
    AND_I,
    NOT_I,
    SLT,
    XOR,
    LABEL,
    MOVZ,
    BEQZ,
    B_I,
    BGE,
    BLE,
    J_I,
    BNEZ,
    SB,
    LB,
    TEXT,
    JAL
} Instruction_type;

extern const char * instruction_type_string[];
extern const int instruction_reg_count[];

typedef enum {
    LABEL_IF,
    LABEL_ELSE,
    LABEL_IF_ELSE_END,
    LABEL_WHILE,
    LABEL_WHILE_END,
    LABEL_COMPARE,
    LABEL_COMPARE_END,
    FUNCTION,
    FUN_PREAMBLE,
    FUN_EPILOG
} Label_type;

typedef struct {
    Instruction_type type;
    int dest_reg;
    int reg1;
    int reg2;
    int offset;
    Label_type label;
    int label_sn;
    const char * label_name;
} Instruction_line;

typedef enum {
    ZERO,
    at,
    v0,
    v1,
    a0,
    a1,
    a2,
    a3,
    t0,
    t1,
    t2,
    t3,
    t4,
    t5,
    t6,
    t7,
    s0,
    s1,
    s2,
    s3,
    s4,
    s5,
    s6,
    s7,
    t8,
    t9,
    k0,
    k1,
    gp,
    sp,
    fp,
    ra
} REG;

void stack_push(int reg);
void stack_pop(int reg);
void codetable_init();
void codetable_destroy();
int get_next_label_sn(Label_type label);
int get_last_label_sn(Label_type label);
Instruction_line * create_instruction(Instruction_type type, int dest_reg, int reg1, int reg2);
Instruction_line * create_jump_instruction(Instruction_type type, int dest_reg, int reg1, Label_type label, int label_sn);
Instruction_line * create_instruction_label(Label_type label, int label_sn);
Instruction_line * create_instruction_offset(Instruction_type type, int dest_reg, int reg1, int reg2, int offset);
Instruction_line * create_instruction_named_label(Label_type label, const char * name);
Instruction_line * create_jump_label_instruction(Instruction_type type, int dest_reg, int reg1, const char * name);
void stack_push(int reg);
void add_instruction(Instruction_line * line);
int codetable_print(FILE * out);
Instruction_line * create_instruction_text(Label_type label);

#endif
