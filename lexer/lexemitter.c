#include "lexer.h"

//
// generates the lexer's output
//   t: the token
//   tval: token value
//
void lexer_emit(token t) {
  if (t.type >= LEXEME_COUNT || t.type < 0)
  	{
      	printf("unknown %c\n", t.type);
  	}
  	else
  	{
  		if (t.t_value == EMPTY)
  			printf("%s\n", lex_symbol_table[t.type]);
  		else if (t.t_value == LEXEME)
  			printf("%s.%s\n", lex_symbol_table[t.type], t.lexeme);
  		else
  			printf("%s.%d\n", lex_symbol_table[t.type], t.value);
  	}
}
