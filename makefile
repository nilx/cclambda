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
LDFLAGS	+= -lpng -ltcc -ldl -lm

# static build
ifdef STATIC
# link options to use the local libraries
LDFLAGS	= /usr/lib/libpng.a /usr/lib/libz.a /usr/lib/libtcc.a -ldl -lm
CPPFLAGS	:= $(CPPFLAGS) -DSTATIC
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
# extra tasks

PROJECT	= cclambda
DATE	= $(shell date -u +%Y%m%d)
RELEASE_TAG   = 0.$(DATE)

.PHONY	: srcdoc lint beautify debug test release
# source documentation
srcdoc	: $(SRC) __lambda.c __lambda.h
	doxygen doc/doxygen.conf
# code cleanup
beautify	: $(SRC) __lambda.c
	for FILE in $^; do \
		expand $$FILE | sed 's/[ \t]*$$//' > $$FILE.$$$$ \
		&& indent -kr -i4 -l78 -nut -nce -sob -sc \
			$$FILE.$$$$ -o $$FILE \
		&& rm $$FILE.$$$$; \
	done
	$(MAKE) __lambda.h
# static code analysis
CLANG_OPTS	= --analyze -ansi
SPLINT_OPTS	= -ansi-lib -weak -castfcnptr
CPPFLAGS2	= -D__NBINPUT=4 -D__EXPR=A+B+C+D -D__NX=512 -D__NY=512
lint	: $(SRC) __lambda.c
	for FILE in $^; do \
		echo clang $$FILE; \
		clang $(CLANG_OPTS) $(CPPFLAGS) $(CPPFLAGS2) -UNDEBUG \
			-I. $$FILE || exit 1; done;
	for FILE in $^; do \
		echo splint $$FILE; \
		splint $(SPLINT_OPTS) $(CPPFLAGS) $(CPPFLAGS2) -UNDEBUG \
			-I. $$FILE || exit 1; done;
	$(RM) *.plist
# debug build
debug	: $(SRC)
	$(MAKE) CFLAGS="$(CFLAGS) -g" CPPFLAGS="$(CPPFLAGS) -UNDEBUG" \
		LDFLAGS="$(LDFLAGS) -lefence"
# code tests
test	: $(SRC) $(HDR)
	sh -e test/run.sh && echo SUCCESS || ( echo ERROR; return 1)
# release tarball
release	: beautify lint test distclean
	git archive --format=tar --prefix=$(PROJECT)-$(RELEASE_TAG)/ HEAD \
	        | gzip > ../$(PROJECT)-$(RELEASE_TAG).tar.gz
