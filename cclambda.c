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
 * @todo: empty image built on-demand
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "io_bds.h"
#include "cclambda_lib.h"

/*
 * macro stringification,
 * cf. http://gcc.gnu.org/onlinedocs/cpp/Stringification.html
 */
#define xstr(s) str(s)
#define str(s) #s

#ifdef _OPENMP
#ifdef OMPCC
#define OMP_STR ", with OpenMP/" xstr(OMPCC)
#else
#define OMP_STR ", with OpenMP"
#endif
#else
#define OMP_STR ""
#endif

#ifndef WITH_LIBTCC
#define LIBTCC_STR ", without libtcc"
#define LIBTCC_SYNTAX ""
#else
#define LIBTCC_STR ", with libtcc"
#define LIBTCC_SYNTAX "        cclambda 'expr'\n"
#endif

#ifndef NDEBUG
#define DEBUG_STR ", debug mode"
#else
#define DEBUG_STR ""
#endif

#define BUILD_STR ", compiled " __DATE__ \
    OMP_STR LIBTCC_STR DEBUG_STR

#define USAGE \
    "cclambda" BUILD_STR "\n"                                           \
    "\n"                                                                \
    "syntax: cclambda [-c|-h]\n"                                        \
    LIBTCC_SYNTAX                                                       \
    "        CC=cc CFLAGS=-O3 cclambda 'expr'\n"                        \
    "\n"                                                                \
    "        -c       dump loop code\n"                                 \
    "        -h       show this help\n"                                 \
    "        expr     C expression, no '__', ';' or '\'' allowed\n"     \
    "\n"                                                                \
    "usage:  cat foo bar | cclambda 'expr' > baz\n"

/**
 * command-line handler
 */
int main(int argc, char **argv)
{
    float *in[4];
    float *out;
    size_t _nx, _ny;
    size_t nx, ny, nc;
    char *expr;
    int nbinput;
    int i;
    int c;

    /* validate params */
    if (2 != argc) {
        fprintf(stderr, USAGE);
        exit(EXIT_FAILURE);
    }
    if (0 == strcmp(argv[1], "-h")) {
        fprintf(stdout, USAGE);
        exit(EXIT_SUCCESS);
    }
    if (0 == strcmp(argv[1], "-c")) {
        (void) fwrite((void *) __lambda_c, sizeof(char), __lambda_c_len,
                      stdout);
        exit(EXIT_SUCCESS);
    }

    /* get anc check expr */
    expr = argv[1];
    if (NULL != strstr(expr, "__")
        || NULL != strchr(expr, ';')
        || NULL != strchr(expr, '\'')) {
        fprintf(stderr, USAGE);
        exit(EXIT_FAILURE);
    }

    /* keep the compiler happy */
    _nx = 0;
    _ny = 0;
    /* read input images */
    DBG_CLOCK_START();
    i = 0;
    /* read max 4 inputs */
    while (4 > i && EOF != (c = getc(stdin))) {
        ungetc(c, stdin);
        in[i] = io_bds_read_flt("-", &nx, &ny, &nc);
        if (1 != nc) {
            fprintf(stderr, "only one channel per image is handled\n");
            return EXIT_FAILURE;
        }
        if (0 == i) {
            /* store the first size */
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
        i++;
    }
    DBG_CLOCK_TOGGLE();
    nbinput = i;
    if (0 == nbinput) {
        fprintf(stderr, "input is missing\n");
        return EXIT_FAILURE;
    }
    /* allocate output image */
    out = (float *) malloc(nx * ny * sizeof(float));

    /* compile and run the lambda loop */
#ifndef WITH_LIBTCC
    loop_with_cc(expr, nbinput, in, out, nx, ny);
#else
    {
        char *cc = getenv("CC");
        if (NULL == cc || '\0' == cc[0] || 0 == strcmp("libtcc", cc))
            /* no CC, use libtcc */
            loop_with_libtcc(expr, nbinput, in, out, nx, ny);
        else
            /* use local CC */
            loop_with_cc(expr, nbinput, in, out, nx, ny);
    }
#endif

    /* write output images */
    DBG_CLOCK_TOGGLE();
    io_bds_write_flt("-", out, nx, ny, 1);
    DBG_CLOCK_TOGGLE();
    DBG_PRINTF1("%0.3fs\treading and writing the files\n", DBG_CLOCK_S());

    /* cleanup */
    for (i = 0; i < nbinput; i++)
        free(in[i]);
    free(out);

    return EXIT_SUCCESS;
}
