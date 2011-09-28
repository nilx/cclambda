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
 * @todo: optional libtcc
 * @todo: dump __lambda_c
 */

#include <stdlib.h>
#include <string.h>

#include "io_png.h"
#include "cclambda_lib.h"

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
    if (NULL == getenv("CC"))
        /* no CC, use libtcc */
        loop_with_libtcc(expr, nbinput, in, out, nx, ny);
    else
        /* use local CC */
        loop_with_cc(expr, nbinput, in, out, nx, ny);

    /* write output images */
    io_png_write_flt("-", out, nx, ny, 3);

    /* cleanup */
    for (i = 0; i < nbinput; i++)
        free(in[i]);
    free(out);

    return EXIT_SUCCESS;
}
