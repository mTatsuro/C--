#
# A simple makefile  
#
# make depend: will build dependency rules for you and add them to end of 
#   		this makefile
# make: build the target executable defined by MAIN 
#
#
.PHONY: depend clean backup setup

# target executable
MAIN=test_prog

# define the compiler
CC= gcc

# define compiler flags
CFLAGS= -Wall -g
#CFLAGS= -p

# paths to include directories for any header files not in (/usr/include)
# here, since I have a header file in this directory,
# I need to specify . as a path to search for .h files:
INCLUDES = -I../includes 

# paths to libraries (for any thing not in /usr/lib)
# ex: -L/home/newhall/mlibs  -L/lib
LFLAGS=

# libraries (-lm for example)
LIBS= -lreadline 

# source files: 
SRCS = main.c ast.c 

# target for shared library
SHARED_OBJ= libast.so

SHARED_OBJ_SRCS= ast.c

# the rest of this is generic and will work for any build of single
# executable (just define program-specific things above)

OBJS = $(SRCS:.c=.o)

SH_OBJS = $(SHARED_OBJ_SRCS:.c=.o)

all: $(MAIN)

$(MAIN):  $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(MAIN) $(LFLAGS) $(LIBS)

$(SHARED_OBJ): $(SH_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SH_OBJS) -shared -o $(SHARED_OBJ) 


.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<


depend: $(SRCS)
	makedepend $(INCLUDES) $^

clean:
	rm -f *.o  $(MAIN) $(SHARED_OBJ)

backup:
	mkdir bak
	cp Makefile *.[ch] bak

# DO NOT DELETE THIS LINE -- make depend needs it 

main.o: /usr/include/stdio.h /usr/include/features.h
main.o: /usr/include/stdc-predef.h /usr/include/libio.h
main.o: /usr/include/_G_config.h /usr/include/wchar.h /usr/include/stdlib.h
main.o: /usr/include/alloca.h /usr/include/string.h /usr/include/xlocale.h
main.o: ../includes/ast.h
ast.o: /usr/include/stdlib.h /usr/include/features.h
ast.o: /usr/include/stdc-predef.h /usr/include/alloca.h /usr/include/stdio.h
ast.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/wchar.h
ast.o: /usr/include/string.h /usr/include/xlocale.h ../includes/ast.h
