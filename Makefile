allmodules=lexer parser ast codegen

all:
	+@for submodule in $(allmodules); do
		if [ -f $$submodule/Makefile ]; then
			$(MAKE) -C $$submodule ;
		else
			true;
		fi
	done

clean depend:
	+@for submodule in $(allmodules); do
		if [ -f $$submodule/Makefile ]; then
			$(MAKE) -C $$submodule $@;
		else
			true;
		fi
	done


