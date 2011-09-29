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
 * @file cclambda_lib.c
 * @brief cclambda routines
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 *
 * @todo: use safe POSIX functions (execvp, snprintf, mkstemp, ...)
 * @todo: pass image size as cpp macro for unrolled loops
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <dlfcn.h>

#ifndef WITHOUT_LIBTCC
#include <libtcc.h>
#endif

#include "__lambda.h"           /* __lambda_c and __lambda_c_len */

/* ensure consistency */
#include "cclambda_lib.h"

/** pointer to the compiled __lambda() function */
typedef void (*lambda_fp) (float *const *, float *);

#ifndef WITHOUT_LIBTCC
/**
 * use the embedded libtcc compiler to build the lambda loop
 */
void loop_with_libtcc(const char *expr, int nbinput,
                      float *const *in, float *out, size_t nx, size_t ny)
{
    TCCState *tcc;
    void *tccmem;
    char nbinput_s[16], nx_s[16], ny_s[16];
    lambda_fp funcp;

    DBG_PRINTF0("compile with embedded libtcc\n");
    sprintf(nbinput_s, "%i", nbinput);
    sprintf(nx_s, "%lu", nx);
    sprintf(ny_s, "%lu", ny);
    /* compile lambda */
    tcc = tcc_new();
    tcc_set_warning(tcc, "all", 1);
    tcc_define_symbol(tcc, "__EXPR", expr);
    tcc_define_symbol(tcc, "__NBINPUT", nbinput_s);
    tcc_define_symbol(tcc, "__NX", nx_s);
    tcc_define_symbol(tcc, "__NY", ny_s);
    DBG_PRINTF1("__EXPR\t'%s'\n", expr);
    DBG_PRINTF1("__NBINPUT\t'%s'\n", nbinput_s);
    DBG_PRINTF1("__NX\t'%s'\n", nx_s);
    DBG_PRINTF1("__NY\t'%s'\n", ny_s);
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
    /* run __lambda(in, out); */
    (*funcp) (in, out);
    /* cleanup */
    tcc_delete(tcc);
    free(tccmem);
    return;
}
#endif                          /* WITH_LIBTCC */

/**
 * use the local CC compiler to build the lambda loop
 */
void loop_with_cc(const char *expr, int nbinput,
                  float *const *in, float *out, size_t nx, size_t ny)
{
    char *cc, *cflags;
    char cflags_empty[] = "";
    char fname_src[L_tmpnam + 2], fname_obj[L_tmpnam + 3];
    FILE *fd;
    char cmd[512];
    void *dl;
    lambda_fp funcp;

    DBG_PRINTF0("compile with external compiler\n");
    /* gather local settings */
    cc = getenv("CC");
    if (NULL == cc)
        ABORT("missing CC environment variable");
    cflags = getenv("CFLAGS");
    if (NULL == cflags)
        cflags = cflags_empty;
    /* TODO: add -g in debug mode */
    DBG_PRINTF1("CC\t'%s'\n", cc);
    DBG_PRINTF1("CFLAGS\t'%s'\n", cflags);
    /* temporary source file */
    /* TODO: use mkstemp */
    (void) tmpnam(fname_src);
    strcat(fname_src, ".c");
    fd = fopen(fname_src, "w");
    (void) fwrite((void *) __lambda_c, sizeof(char), __lambda_c_len, fd);
    fclose(fd);
    /* build the command line */
    (void) tmpnam(fname_obj);
    strcat(fname_obj, ".so");
    /* TODO: use snprintf() */
    /* TODO: insert warnings */
    sprintf(cmd, "%s %s "
            "-D__EXPR='%s' -D__NBINPUT=%i -D__NX=%lu -D__NY=%lu "
            "-shared -o %s %s",
            cc, cflags, expr, nbinput, nx, ny, fname_obj, fname_src);
    DBG_PRINTF1("cmd\t'%s'\n", cmd);
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
    /* run __lambda(in, out); */
    (*funcp) (in, out);
    /* cleanup */
    dlclose(dl);
    remove(fname_obj);
    remove(fname_src);
    return;
}
