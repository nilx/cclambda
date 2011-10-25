# Copyright 2011 Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
#
# Copying and distribution of this file, with or without
# modification, are permitted in any medium without royalty provided
# the copyright notice and this notice are preserved.  This file is
# offered as-is, without any warranty.

# source code, C language
SRC	= cclambda.c cclambda_lib.c io_bds.c
# object files (partial compilation)
OBJ	= $(SRC:.c=.o)
# binary executable programs
BIN	= cclambda

# C compiler optimization options
COPT	= 
# complete C compiler options
CFLAGS	= $(COPT)
# preprocessor options
CPPFLAGS	= -I. -DNDEBUG -DWITH_LIBTCC
# linker options
LDFLAGS	=
# libraries
LDLIBS	= -ltcc -ldl -lm

# openmp support
ifdef OMP
-include	makefile.openmp
endif

# default target: the binary executable programs
default: $(BIN)

# dependencies
-include makefile.dep

# __lambda.h formatting
__lambda.h	: __lambda.c
	echo "/* xxd -i $< */" > $@
	xxd -i $< >> $@

# partial C compilation xxx.c -> xxx.o
%.o	: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

# final link
cclambda	: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# cleanup
.PHONY	: clean distclean
clean	:
	$(RM) $(OBJ) *.plist
distclean	: clean
	$(RM) $(BIN)
	$(RM) -r srcdoc

################################################
# dev tasks
PROJECT	= cclambda
-include	makefile.dev
