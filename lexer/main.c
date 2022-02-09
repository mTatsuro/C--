/*
 *  Main function for testing the C-- lexical analyzer.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

int main(int argc, char *argv[]) {

  token t;
  FILE *fd;

  if(argc <= 1) {
      printf("usage: lexer infile.c--\n");
      exit(1);
  }
  fd = fopen(argv[1], "r");
  if(fd == 0) {
      printf("error opening file: %s\n", argv[1]);
      exit(1);
  }
  t.type = IF;
  while(t.type != DONE && t.type != LEXERROR) {
      t = lexan(fd);
      if (t.type != LEXERROR)
	      lexer_emit(t);
  }
  if( t.type == LEXERROR ) {
      lexer_error("invalid symbol", src_lineno + 1);
      exit(1);     /*  unsuccessful termination  */
  }
  fclose(fd);
  exit(0);     /*  successful termination  */
}
