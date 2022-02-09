#
# compiles mycc, a compiler for C--
#
# make depend: automatically build .o file dependencies
# make: build mycc
# make clean: removes all .o and executable files
#

CC = gcc
CFLAGS = -Wall -g

LFLAGS =
LIBS =

INCLUDES =  -I../includes

# add additional source files here
SRCS = ../ast/ast.c ../lexer/lexer.c ../parser/parser.c \
       codegen.c codetable.c main.c ../lexer/lexerror.c

OBJS = $(SRCS:.c=.o)


MAIN = mycc

# generic part of makefile...don't change this
all:    $(MAIN)
	@echo  Simple parser named parser has been compiled

$(MAIN):  $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

../ast/ast.o: /usr/include/stdlib.h /usr/include/features.h
../ast/ast.o: /usr/include/stdc-predef.h /usr/include/alloca.h
../ast/ast.o: /usr/include/stdio.h /usr/include/libio.h
../ast/ast.o: /usr/include/_G_config.h /usr/include/wchar.h
../ast/ast.o: /usr/include/string.h /usr/include/xlocale.h ../includes/ast.h
../lexer/lexer.o: /usr/include/stdlib.h /usr/include/features.h
../lexer/lexer.o: /usr/include/stdc-predef.h /usr/include/alloca.h
../lexer/lexer.o: /usr/include/assert.h ../includes/lexer.h
../lexer/lexer.o: /usr/include/stdio.h /usr/include/libio.h
../lexer/lexer.o: /usr/include/_G_config.h /usr/include/wchar.h
../parser/parser.o: /usr/include/stdlib.h /usr/include/features.h
../parser/parser.o: /usr/include/stdc-predef.h /usr/include/alloca.h
../parser/parser.o: /usr/include/stdio.h /usr/include/libio.h
../parser/parser.o: /usr/include/_G_config.h /usr/include/wchar.h
../parser/parser.o: /usr/include/assert.h ../includes/parser.h
../parser/parser.o: /usr/include/ctype.h /usr/include/endian.h
../parser/parser.o: /usr/include/xlocale.h /usr/include/string.h
../parser/parser.o: ../includes/ast.h ../includes/lexer.h
codegen.o: /usr/include/stdio.h /usr/include/features.h
codegen.o: /usr/include/stdc-predef.h /usr/include/libio.h
codegen.o: /usr/include/_G_config.h /usr/include/wchar.h
codegen.o: /usr/include/assert.h ../includes/parser.h /usr/include/stdlib.h
codegen.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/endian.h
codegen.o: /usr/include/xlocale.h /usr/include/string.h ../includes/ast.h
codegen.o: ../includes/codegen.h
main.o: /usr/include/stdio.h /usr/include/features.h
main.o: /usr/include/stdc-predef.h /usr/include/libio.h
main.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
main.o: /usr/include/alloca.h /usr/include/strings.h /usr/include/xlocale.h
main.o: ../includes/codegen.h ../includes/parser.h /usr/include/ctype.h
main.o: /usr/include/endian.h /usr/include/string.h ../includes/ast.h
