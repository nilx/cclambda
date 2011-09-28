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
 * @todo: dump __lambda_c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libtcc.h>

#include "io_png.h"

#include "__lambda.h"

/** pointer to the compiled __lambda() function */
typedef void (*lambda_fp) (float *const *, float *, size_t, size_t);

/** abort() wrapper macro with an error message */
#define ABORT(MSG) do {                                         \
    fprintf(stderr, "%s:%04u : %s\n", __FILE__, __LINE__, MSG); \
    fflush(NULL);                                               \
    abort();                                                    \
    } while (0);

/** exit() wrapper macro with an error message */
#define ERROR(MSG) do {         \
    fprintf(stderr, MSG);       \
    fflush(NULL);               \
    exit(EXIT_FAILURE);         \
    } while (0);

/** use the embedded libtcc compiler to build the lambda loop */
static void run_with_libtcc(const char *expr, int nbinput,
                            float *const *in, float *out,
                            size_t nx, size_t ny)
{
    TCCState *tcc;
    void *tccmem;
    char nb[2];
    /* TODO: typedef */
    void (*funcp) (float *const *, float *, size_t, size_t);

    strcpy(nb, (1 == nbinput ? "1" :
                2 == nbinput ? "2" : 3 == nbinput ? "3" : "4"));
    /* compile lambda */
    tcc = tcc_new();
    tcc_set_warning(tcc, "all", 1);
    tcc_define_symbol(tcc, "__EXPR", expr);
    tcc_define_symbol(tcc, "__NBINPUT", nb);
    tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);
    if (0 != tcc_compile_string(tcc, (const char *) __lambda_c))
        ABORT("compilation error");
    /* get the compiled symbols */
    tccmem = malloc((size_t) tcc_relocate(tcc, NULL));
    if (NULL == tccmem)
        ABORT("relocation error");
    if (-1 == tcc_relocate(tcc, tccmem))
        ABORT("relocation error");
    /*
     * see the RATIONALE section of
     * http://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html
     */
    funcp = (void (*)(float *const *, float *, size_t, size_t))
        tcc_get_symbol(tcc, "__lambda");
    if (NULL == funcp)
        ABORT("missing __lambda symbol");
    /* run __lambda(in, out, nx, ny); */
    (*funcp) (in, out, nx, ny);
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
    float *in[4];
    float *out;
    size_t _nx, _ny;
    size_t nx, ny;
    char *expr;
    int nbinput;
    int i;

    /* validate params */
    nbinput = argc - 2;
    if (nbinput < 1 || nbinput > 4)
        ERROR("syntax:  cclambda img1.png img2.png ... 'espression'\n"
              "         between 1 and 4 input images\n"
              "         '-' for stdin");
    expr = argv[argc - 1];
    if (NULL != strstr(expr, "__")
        || NULL != strchr(expr, ';')
        || NULL != strchr(expr, '"'))
        ERROR("no '__', ';' or '\"' allowed in the C expression");

    /* keep the compiler happy */
    _nx = 0;
    _ny = 0;
    /* read input images */
    for (i = 0; i < nbinput; i++) {
        in[i] =
            io_png_read_pp_flt(argv[i + 1], &nx, &ny, NULL, IO_PNG_OPT_RGB);
        if (0 == i) {
            /* store thje first size */
            _nx = nx;
            _ny = ny;
        }
        else {
            /* check the size */
            if (nx != _nx || ny != _ny) {
                fprintf(stderr, "input image sizes do not match\n");
                return EXIT_FAILURE;
            }
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
