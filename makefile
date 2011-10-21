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
CPPFLAGS	= -DNDEBUG -DWITH_LIBTCC
# linker options
LDFLAGS	= -ltcc -ldl -lm

# openmp support
ifdef OMP
-include	makefile.openmp
endif

# default target: the binary executable programs
default: $(BIN)

# __lambda.h formatting
__lambda.h	: __lambda.c
	echo "/* xxd -i $< */" > $@
	xxd -i $< >> $@

# dependencies
cclambda.o	: cclambda.c __lambda.h

# partial C compilation xxx.c -> xxx.o
%.o	: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

# final link
cclambda	: cclambda.o cclambda_lib.o io_bds.o
	$(CC) $^ $(LDFLAGS) -o $@

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
