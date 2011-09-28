#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "io_png.h"
#include "libtcc.h"

#include "__lambda.h"

int main(int argc, char **argv)
{
    float * in[4];
    float * out;
    size_t i;
    size_t _nx, _ny;
    size_t nx, ny;
    char * expr;
    int nbinput;
    char nbinput_str[2];
    void (*__lambda)(float * const *__in, float *__out,
		     size_t __nx, size_t __ny);
    TCCState *tcc;
    void *tccmem;

    /* validate params */
    nbinput = argc - 2;
    if (nbinput < 1 || nbinput > 4) {
	fprintf(stderr, "syntax:  %s img1.png img2.png ... 'espression'\n", argv[0]);
	fprintf(stderr, "         between 1 and 4 input images\n");
	fprintf(stderr, "         '-' for stdin\n");
	return EXIT_FAILURE;
    }
    strcpy(nbinput_str, (1 == nbinput ? "1" :
			 2 == nbinput ? "2" :
			 3 == nbinput ? "3" : "4"));
    expr = argv[argc-1];
    if (NULL != strstr(expr, "__")) {
	fprintf(stderr, "no '__' allowed in the C expression");
	return EXIT_FAILURE;
    }
    if (NULL != strchr(expr, ';')) {
	fprintf(stderr, "no ';' allowed in the C expression");
	return EXIT_FAILURE;
    }

    /* read input images */
    for (i = 0; i < nbinput; i++) {
	in[i] = io_png_read_pp_flt(argv[i+1], &nx, &ny, NULL, IO_PNG_OPT_RGB);
	if (0 == i) {
	    _nx = nx;
	    _ny = ny;
	} else {
	    assert(nx == _nx && ny == _ny);
	}
    }
    /* allocate output image */
    out = (float *) malloc(3 * nx * ny * sizeof(float));

    /* compile lambda */
    tcc = tcc_new();
    tcc_set_warning(tcc, "all", 1);
    tcc_define_symbol(tcc, "__NBINPUT", nbinput_str);
    tcc_define_symbol(tcc, "__EXPR", expr);
    tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);
    if (0 != tcc_compile_string(tcc, __lambda_c)) {
	fprintf(stderr, "compilation error\n");
	return EXIT_FAILURE;
    }
    tccmem = malloc(tcc_relocate(tcc, NULL));
    if (-1 == tcc_relocate(tcc, tccmem)) {
	fprintf(stderr, "relocation error\n");
	return EXIT_FAILURE;
    }
    if (NULL == (__lambda = tcc_get_symbol(tcc, "__lambda"))) {
	fprintf(stderr, "missing symbol\n");
	return EXIT_FAILURE;
    }
    /* run __lambda(in, out, nx, ny); */
    (*__lambda)(in, out, nx, ny);


    tcc_delete(tcc);
    free(tccmem);

    /* write output images */
    io_png_write_flt("-", out, nx, ny, 3);

    for (i = 0; i < nbinput; i++)
	free(in[i]);
    free(out);

    return EXIT_SUCCESS;
}
