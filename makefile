LDFLAGS	= -lpng -ltcc -ldl -lm

default	: cclambda

__lambda.h	: __lambda.c
	xxd -i $< > $@

cclambda.o	: cclambda.c __lambda.h
	$(CC) $(CFLAGS) -c $< -o $@

io_png.o	: io_png.c
	$(CC) $(CFLAGS) -c $< -o $@

cclambda	: cclambda.o io_png.o
	$(CC) $^ $(LDFLAGS) -o $@


clean	:
	$(RM) *.o

distclean	: clean
	$(RM) cclambda __lambda.h
