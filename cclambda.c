/*
 * Copyright (c) 2011, Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under, at your option, the terms of the GNU General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version, or
 * the terms of the simplified BSD license.
 *
 * You should have received a copy of these licenses along this
 * program. If not, see <http://www.gnu.org/licenses/> and
 * <http://www.opensource.org/licenses/bsd-license.html>.
 */

/**
 * @file cclambda.c
 * @brief cclambda main code
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 *
 * @todo: multiple io formats
 * @todo: external compilers
 * @todo: optional libtcc
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "io_png.h"
#include "libtcc.h"

#include "__lambda.h"

/** pointer to the compiled __lambda() function */
static void (*__lambda)(float * const *__in, float *__out,
			size_t __nx, size_t __ny);

/** use the embedded libtcc compiler to build the lambda loop */
void run_with_libtcc(const char* expr, int nbinput,
		     float * const * in, float * out,
		     size_t nx, size_t ny)
{
    TCCState *tcc;
    void *tccmem;
    char nb[2];

    strcpy(nb, (1 == nbinput ? "1" :
		2 == nbinput ? "2" :
		3 == nbinput ? "3" : "4"));
    /* compile lambda */
    tcc = tcc_new();
    tcc_set_warning(tcc, "all", 1);
    tcc_define_symbol(tcc, "__EXPR", expr);
    tcc_define_symbol(tcc, "__NBINPUT", nb);
    tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);
    if (0 != tcc_compile_string(tcc, __lambda_c)) {
	fprintf(stderr, "compilation error\n");
	exit(EXIT_FAILURE);
    }
    /* get the compiled symbols */
    tccmem = malloc(tcc_relocate(tcc, NULL));
    if (-1 == tcc_relocate(tcc, tccmem)) {
	fprintf(stderr, "relocation error\n");
	exit(EXIT_FAILURE);
    }
    if (NULL == (__lambda = tcc_get_symbol(tcc, "__lambda"))) {
	fprintf(stderr, "missing symbol\n");
	exit(EXIT_FAILURE);
    }
    /* run __lambda(in, out, nx, ny); */
    (*__lambda)(in, out, nx, ny);
    /* cleanup */
    tcc_delete(tcc);
    free(tccmem);
    return;
}

/**
 * command-line handler
 */
int main(int argc, char **argv)
{
    float * in[4];
    float * out;
    size_t i;
    size_t _nx, _ny;
    size_t nx, ny;
    char * expr;
    int nbinput;

    /* validate params */
    nbinput = argc - 2;
    if (nbinput < 1 || nbinput > 4) {
	fprintf(stderr, "syntax:  %s img1.png img2.png ... 'espression'\n",
		argv[0]);
	fprintf(stderr, "         between 1 and 4 input images\n");
	fprintf(stderr, "         '-' for stdin\n");
	return EXIT_FAILURE;
    }
    expr = argv[argc-1];
    if (NULL != strstr(expr, "__")) {
	fprintf(stderr, "no '__' allowed in the C expression");
	return EXIT_FAILURE;
    }
    if (NULL != strchr(expr, ';')) {
	fprintf(stderr, "no ';' allowed in the C expression");
	return EXIT_FAILURE;
    }
    if (NULL != strchr(expr, '"')) {
	fprintf(stderr, "no '\"' allowed in the C expression");
	return EXIT_FAILURE;
    }

    /* read input images */
    for (i = 0; i < nbinput; i++) {
	in[i] = io_png_read_pp_flt(argv[i+1], &nx, &ny, NULL, IO_PNG_OPT_RGB);
	if (0 == i) {
	    /* store thje first size */
	    _nx = nx;
	    _ny = ny;
	} else {
	    /* check the size */
	    assert(nx == _nx && ny == _ny);
	}
    }
    /* allocate output image */
    out = (float *) malloc(3 * nx * ny * sizeof(float));
    
    /* compile and run the lambda loop */
    run_with_libtcc(expr, nbinput, in, out, nx, ny);

    /* write output images */
    io_png_write_flt("-", out, nx, ny, 3);

    /* cleanup */
    for (i = 0; i < nbinput; i++)
	free(in[i]);
    free(out);

    return EXIT_SUCCESS;
}
