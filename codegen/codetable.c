// functions to add, remove, modify entries in code table and
// to write codetable to MIPS output file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codetable.h"

#define FUNCTION_BEGIN "addiu\t$sp,\t$sp,\t-8\nsw\t$ra,\t8($sp)\nsw\t$fp,\t4($sp)\n"

#define FUNCTION_END "\nlw\t$ra,\t8($sp)\nlw\t$fp,\t4($sp)\naddiu\t$sp,\t$sp,\t8\njr\t$ra\n"

const char * label_string[] = {
    "if",
    "else",
    "if_else_end",
    "while",
    "while_end",
    "compare",
    "compare_end",
    "function",
    "text"
};

// Define MIPS instructions
const char * instruction_type_string[] = {
    "li",
    "la",
    "move",
    "syscall",
    "add",
    "addi",
    "sw",
    "lw",
    "sub",
    "mul",
    "div",
    "mflo",
    "or",
    "and",
    "not",
    "slt",
    "xor",
    "label",
    "movz",
    "beqz",
    "b",
    "bge",
    "ble",
    "j",
    "bnez",
    "sb",
    "lb",
    "text",
    "jal"
};

// Define instruction counts
const int instruction_reg_count[] = {
    2,
    2,
    2,
    0,
    3,
    3,
    2,
    2,
    3,
    3,
    2,
    1,
    3,
    3,
    2,
    3,
    3,
    0,
    3,
    1,
    0,
    2,
    2,
    0,
    1,
    2,
    2,
    0,
    0
};

int instruction_capacity = 1000;
int instruction_count;
Instruction_line ** instructions = NULL;
int label_sn_if = 0;
int label_sn_else = 0;
int label_sn_if_else_end = 0;
int label_sn_while = 0;
int label_sn_while_end = 0;
int label_sn_compare = 0;
int label_sn_compare_end = 0;

void codetable_init() {
    instruction_count = 0;
    instructions = malloc(sizeof (Instruction_line*) * instruction_capacity);
}

void codetable_destroy() {
    int i = 0;
    if (instructions != NULL) {
        for (i = 0; i < instruction_count; i++) {
            free(instructions[i]);
        }
        free(instructions);
    }
    instructions = NULL;
    instruction_count = 0;
}

int get_next_label_sn(Label_type label) {
    switch (label) {
        case LABEL_IF:
            return label_sn_if++;
        case LABEL_ELSE:
            return label_sn_else++;
        case LABEL_IF_ELSE_END:
            return label_sn_if_else_end++;
        case LABEL_WHILE:
            return label_sn_while++;
        case LABEL_WHILE_END:
            return label_sn_while_end++;
        case LABEL_COMPARE:
            return label_sn_compare++;
        case LABEL_COMPARE_END:
            return label_sn_compare_end++;
        case FUNCTION:
        case FUN_PREAMBLE:
        case FUN_EPILOG:
            return 0;


    }
    printf("Unknown label\n");
    return -1;
}

int get_last_label_sn(Label_type label) {
    switch (label) {
        case LABEL_WHILE_END:
            return label_sn_while_end - 1;
        default:
            return 0;
    }
}

Instruction_line * create_instruction(Instruction_type type, int dest_reg, int reg1, int reg2) {
    Instruction_line * line = NULL;
    line = malloc(sizeof (Instruction_line));
    if (line == NULL) return NULL; //ERROR!!!

    line->type = type;
    line->dest_reg = dest_reg;
    line->reg1 = reg1;
    line->reg2 = reg2;
    line->offset = -1;
    line->label = 0;
    line->label_sn = -1;

    return line;
}

Instruction_line * create_jump_instruction(Instruction_type type, int dest_reg, int reg1, Label_type label, int label_sn) {
    Instruction_line * line = NULL;
    line = malloc(sizeof (Instruction_line));
    if (line == NULL) return NULL; //ERROR!!!

    line->type = type;
    line->dest_reg = dest_reg;
    line->reg1 = reg1;
    line->reg2 = 0;
    line->offset = -1;
    line->label = label;
    line->label_sn = label_sn;

    return line;
}

Instruction_line * create_instruction_label(Label_type label, int label_sn) {
    Instruction_line * line = NULL;
    line = malloc(sizeof (Instruction_line));
    if (line == NULL) return NULL; //ERROR!!!

    line->type = LABEL;
    line->dest_reg = 0;
    line->reg1 = 0;
    line->reg2 = 0;
    line->offset = -1;
    line->label = label;
    line->label_sn = label_sn;

    return line;
}

Instruction_line * create_jump_label_instruction(Instruction_type type, int dest_reg, int reg1, const char * name) {
    Instruction_line * line = NULL;
    line = malloc(sizeof (Instruction_line));
    if (line == NULL) return NULL; //ERROR!!!

    line->type = type;
    line->dest_reg = dest_reg;
    line->reg1 = reg1;
    line->reg2 = 0;
    line->offset = -1;
    line->label = 0;
    line->label_sn = 0;
    line->label_name = name;

    return line;
}

Instruction_line * create_instruction_named_label(Label_type label, const char * name) {
    Instruction_line * line = NULL;
    line = malloc(sizeof (Instruction_line));
    if (line == NULL) return NULL; //ERROR!!!

    line->type = LABEL;
    line->dest_reg = 0;
    line->reg1 = 0;
    line->reg2 = 0;
    line->offset = -1;
    line->label = label;
    line->label_sn = 0;
    line->label_name = name;

    return line;
}

Instruction_line * create_instruction_text(Label_type label) {
    Instruction_line * line = NULL;
    line = malloc(sizeof (Instruction_line));
    if (line == NULL) return NULL; //ERROR!!!

    line->type = TEXT;
    line->dest_reg = 0;
    line->reg1 = 0;
    line->reg2 = 0;
    line->offset = -1;
    line->label = label;
    line->label_sn = 0;

    return line;
}

Instruction_line * create_instruction_offset(Instruction_type type, int dest_reg, int reg1, int reg2, int offset) {
    Instruction_line * line = NULL;
    line = malloc(sizeof (Instruction_line));
    if (line == NULL) return NULL; //ERROR!!!

    line->type = type;
    line->dest_reg = dest_reg;
    line->reg1 = reg1;
    line->reg2 = reg2;
    line->offset = offset;
    line->label = 0;
    line->label_sn = -1;

    return line;
}

void add_instruction(Instruction_line * line) {
    // TODO: check for overflow
    instructions[instruction_count++] = line;
}

void stack_push(int reg) {
    add_instruction(create_instruction(ADDI, sp, sp, -4));
    add_instruction(create_instruction_offset(SW, reg, sp, 0, 0));
    //addi $sp, $sp, -4  # Decrement stack pointer by 4
    //sw   $r3, 0($sp)   # Save $r3 to stack
}

void stack_pop(int reg) {
    add_instruction(create_instruction_offset(LW, reg, sp, 0, 0));
    add_instruction(create_instruction(ADDI, sp, sp, 4));
    //lw   $r3, 0($sp)   # Copy from stack to $r3
    //addi $sp, $sp, 4   # Increment stack pointer by 4
}

void print_preamble(FILE * out) {
    fprintf(out, ".data\n"
            "_newline_:\n"
            ".asciiz	\"\\n\"\n"
            ".text\n"
            ".globl main\n\n");
}

int codetable_print(FILE * out) {
    print_preamble(out);

    int i = 0;
    Instruction_line * l;
    char dollar = '$'; // no comment
    for (i = 0; i < instruction_count; i++) {
        dollar = '$';
        l = instructions[i];

        if (l->type == LABEL) {
            if (l->label == FUNCTION)
                fprintf(out, "%s:\n", l->label_name);
            else
                fprintf(out, "%s.%d:\n", label_string[l->label], l->label_sn);
            continue;
        } else if (l->type == TEXT) {
            if (l->label == FUN_PREAMBLE)
                fprintf(out, FUNCTION_BEGIN);
            else if (l->label == FUN_EPILOG)
                fprintf(out, FUNCTION_END);
        } else if (l->type == BEQZ || l->type == BNEZ) {
            fprintf(out, "%s\t$%d,\t%s.%d\n", instruction_type_string[l->type], l->dest_reg, label_string[l->label], l->label_sn);
            continue;
        } else if ((l->type == B_I) || (l->type == J_I)) {
            fprintf(out, "%s\t%s.%d\n", instruction_type_string[l->type], label_string[l->label], l->label_sn);
            continue;
        } else if ((l->type == JAL)) {
            fprintf(out, "%s\t%s\n", instruction_type_string[l->type], l->label_name);
            continue;
        } else if ((l->type == BGE) || (l->type == BLE)) {
            fprintf(out, "%s\t$%d,\t$%d,\t%s.%d\n", instruction_type_string[l->type], l->dest_reg, l->reg1, label_string[l->label], l->label_sn);
            continue;
        } else {
            fprintf(out, "%s", instruction_type_string[l->type]);
        }

        if (l->type == LA) {
            fprintf(out, "\t$%d,\t_newline_", l->dest_reg);
        } else if (l->offset >= 0) {
            switch (instruction_reg_count[l->type]) {
                case 2:
                    fprintf(out, "\t$%d,\t%d($%d)", l->dest_reg, l->offset, l->reg1);
                    break;
                case 3:
                    fprintf(out, "\t$%d,\t$%d,\t%d($%d)", l->dest_reg, l->reg1, l->offset, l->reg2);
                    break;
            }

        } else {
            if ((l->type == LI) || (l->type == ADDI)) {
                dollar = ' ';
            }
            switch (instruction_reg_count[l->type]) {
                case 3:
                    fprintf(out, "\t$%d,\t$%d,\t%c%d", l->dest_reg, l->reg1, dollar, l->reg2);
                    break;
                case 2:
                    fprintf(out, "\t$%d,\t%c%d", l->dest_reg, dollar, l->reg1);
                    break;
                case 1:
                    fprintf(out, "\t$%d", l->dest_reg);
                    break;
            }
        }
        fprintf(out, "\n");
    }
    return 0;
}
