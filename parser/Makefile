#
# compiles parser, a parser for C--
#
# make depend: automatically build .o file dependencies
# make: build parser
# make pure: build a purified version of parser
# make clean: removes all .o and executable files
#

CC = gcc
CFLAGS = -Wall -g

LFLAGS =
LIBS =

INCLUDES =  -I../includes

# add additional source files here
SRCS = ../ast/ast.c ../lexer/lexer.c ../lexer/lexerror.c parser.c main.c

OBJS = $(SRCS:.c=.o)


MAIN = parser

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
../lexer/lexer.o: /usr/include/string.h /usr/include/xlocale.h
../lexer/lexer.o: /usr/include/ctype.h /usr/include/endian.h
parser.o: /usr/include/stdlib.h /usr/include/features.h
parser.o: /usr/include/stdc-predef.h /usr/include/alloca.h
parser.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
parser.o: /usr/include/wchar.h /usr/include/assert.h ../includes/parser.h
parser.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/xlocale.h
parser.o: /usr/include/string.h ../includes/ast.h ../includes/lexer.h
main.o: /usr/include/stdio.h /usr/include/features.h
main.o: /usr/include/stdc-predef.h /usr/include/libio.h
main.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
main.o: /usr/include/alloca.h /usr/include/strings.h /usr/include/xlocale.h
main.o: ../includes/lexer.h ../includes/parser.h /usr/include/ctype.h
main.o: /usr/include/endian.h /usr/include/string.h ../includes/ast.h
