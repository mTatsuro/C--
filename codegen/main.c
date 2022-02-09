/*
 *  Main function for C-- compiler
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "codegen.h"
#include "parser.h"


int main(int argc, char *argv[]) {

  FILE *in = 0, *out = 0;

  if(argc != 3) {
    printf("usage: mycc  filename.c--  filename.mips\n");
    exit(1);
  }
  if(!(in = fopen(argv[1], "rw")) ) {
    perror("no such file\n");
    exit(1);
  }
  if(!(out = fopen(argv[2], "w")) ) {
    perror("opening output file faild\n");
    exit(1);
  }

  // TODO: these calls will need to be fixed to match the prototypes in
  //       your compiler
  //
  // init_symtab(); ...   // call any initialization routines here
  parse(in);   // call your main parse routine
  codegen(out, ast_tree.root);   // call your main code generation routine to fill codetable 
  //generate_code_from_codetable(out);   // write MIPS code from codetable to
                                       // output file
  fclose(in);
  fclose(out);

  exit(0);
}
