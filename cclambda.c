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
 * @todo: use safe POSIX functions (execvp, snprintf, mkstemp, ...)
 * @todo: pass image size as cpp macro for unrolled loops
 * @todo: multiple io formats
 * @todo: external compilers
 * @todo: optional libtcc
 * @todo: dump __lambda_c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dlfcn.h>

#include <libtcc.h>

#include "io_png.h"

#include "__lambda.h"           /* __lambda_c and __lambda_c_len */

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
    lambda_fp funcp;

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
    funcp = (lambda_fp) tcc_get_symbol(tcc, "__lambda");
    if (NULL == funcp)
        ABORT("missing __lambda symbol");
    /* run __lambda(in, out, nx, ny); */
    (*funcp) (in, out, nx, ny);
    /* cleanup */
    tcc_delete(tcc);
    free(tccmem);
    return;
}

/** use the local CC compiler to build the lambda loop
 *
 * hdl = dlopen(tmpdir/lambda.so)
 * lambda = dlsym(hdl, "__lambda")
 * (*lambda)(in, out, nx, ny)
 * rm -rf tmp
 */
static void run_with_cc(const char *expr, int nbinput,
                        float *const *in, float *out, size_t nx, size_t ny)
{
    char *cc, *cflags;
    char cflags_empty[] = "";
    char fname_src[L_tmpnam + 2], fname_obj[L_tmpnam + 3];
    FILE *file_src;
    char cmd[512];
    void *dl;
    lambda_fp funcp;

    /* gather local settings */
    cc = getenv("CC");
    if (NULL == cc)
        ABORT("missing CC environment variable");
    cflags = getenv("CFLAGS");
    if (NULL == cflags)
        cflags = cflags_empty;
    /* temporary source file */
    /* TODO: use mkstemp */
    (void) tmpnam(fname_src);
    strcat(fname_src, ".c");
    file_src = fopen(fname_src, "w");
    (void) fwrite((void *) __lambda_c,
                  sizeof(char), __lambda_c_len, file_src);
    fclose(file_src);
    /* build the command line */
    (void) tmpnam(fname_obj);
    strcat(fname_obj, ".so");
    /* TODO: use snprintf() or check the length */
    sprintf(cmd, "%s %s -D__EXPR=\"%s\" -D__NBINPUT=%i -shared -o %s %s",
            cc, cflags, expr, nbinput, fname_obj, fname_src);
    /* compile */
    if (0 == system(NULL))
        ABORT("no command processor");
    system(cmd);
    /* dynamic load */
    dl = dlopen(fname_obj, RTLD_NOW);
    /*
     * see the RATIONALE section of
     * http://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html
     */
    funcp = (lambda_fp) dlsym(dl, "__lambda");
    if (NULL == funcp)
        ABORT("missing __lambda symbol");
    /* run __lambda(in, out, nx, ny); */
    (*funcp) (in, out, nx, ny);
    /* cleanup */
    dlclose(dl);
    remove(fname_obj);
    remove(fname_src);
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
    if (NULL == getenv("CC"))
        /* no CC, use libtcc */
        run_with_libtcc(expr, nbinput, in, out, nx, ny);
    else
        /* use local CC */
        run_with_cc(expr, nbinput, in, out, nx, ny);

    /* write output images */
    io_png_write_flt("-", out, nx, ny, 3);

    /* cleanup */
    for (i = 0; i < nbinput; i++)
        free(in[i]);
    free(out);

    return EXIT_SUCCESS;
}
