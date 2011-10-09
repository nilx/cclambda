# Copyright 2011 Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
#
# Copying and distribution of this file, with or without
# modification, are permitted in any medium without royalty provided
# the copyright notice and this notice are preserved.  This file is
# offered as-is, without any warranty.

# source code, C language
SRC	= cclambda.c cclambda_lib.c io_png.c
# object files (partial compilation)
OBJ	= $(SRC:.c=.o)
# binary executable programs
BIN	= cclambda

# standard C compiler optimization options
COPT	= 
# complete C compiler options
CFLAGS	= -ansi -pedantic -Wall -Wextra -pipe $(COPT)
# preprocessor options
CPPFLAGS	= -DNDEBUG -DWITH_LIBTCC
# linker options
LDFLAGS	= -lpng -ltcc -ldl -lm

# static build
ifdef STATIC
# link options to use the local libraries
LDFLAGS	= /usr/lib/libpng.a /usr/lib/libz.a /usr/lib/libtcc.a -ldl -lm
CPPFLAGS	:= $(CPPFLAGS) -DSTATIC
endif

# openmp support
ifdef OMP
CPPFLAGS_OMP	= -DOMPCC=$(CC)
# for gcc and gcc-based compilers
CFLAGS_OMP	= -fopenmp
LDFLAGS_OMP	= -lgomp
# for icc
#CFLAGS_OMP	= -openmp
#LDFLAGS_OMP	= -liomp5 -lpthread -L/path/to/libiomp5.so
# for suncc
#CFLAGS_OMP	= -xopenmp
#LDFLAGS_OMP	= -lmtsk -L/path/to/libmtsk.so
CPPFLAGS	+= $(CPPFLAGS_OMP)
CFLAGS	+= $(CFLAGS_OMP)
LDFLAGS	+= $(LDFLAGS_OMP)
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
cclambda	: cclambda.o cclambda_lib.o io_png.o
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
