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
 * We have to get the compiled loop function via double pointer
 * indirection because of conflicts between ISO C and POSIX standards.
 * cf. "RATIONALE" section of
 *   http://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html
 * and "Solving the function pointer problem on POSIX systems" section of
 *   http://en.wikipedia.org/wiki/Dynamic_loading
 *
 * @todo: use POSIX execvp() to check exec result
 */

#define _XOPEN_SOURCE 500       /* for snprintf(), fdopen(), mkstemp() */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef S_SPLINT_S              /* see http://bugs.debian.org/476228 */
#include <unistd.h>
#else
int mkstemp(const char *);
FILE *fdopen(int, const char *);
#endif

#include <dlfcn.h>              /* dlopen() */

#ifdef WITH_LIBTCC
#include <libtcc.h>
#endif

#include "__lambda.h"           /* __lambda_c and __lambda_c_len */

/* ensure consistency */
#include "cclambda_lib.h"

/** pointer to the compiled __lambda() function */
typedef void (*lambda_fp) (float *const *, float *);

#ifdef WITH_LIBTCC
/**
 * use the embedded libtcc compiler to build the lambda loop
 */
void loop_with_libtcc(const char *expr, int nbinput,
                      float *const *in, float *out, size_t nx, size_t ny)
{
    TCCState *tcc;
    void *tccmem;
    char nbinput_s[16], nx_s[16], ny_s[16];
    /* get the loop function via double pointer indirection */
    lambda_fp *funcpp;

    DBG_PRINTF0("compile with embedded libtcc\n");

    /* format the cpp macros as text */
    snprintf(nbinput_s, 16, "%i", nbinput);
    snprintf(nx_s, 16, "%lu", nx);
    snprintf(ny_s, 16, "%lu", ny);

    /* compile with libtcc */
    DBG_CLOCK_START();
    tcc = tcc_new();
    tcc_set_warning(tcc, "all", 1);
    tcc_define_symbol(tcc, "__EXPR", expr);
    DBG_PRINTF1("__EXPR\t'%s'\n", expr);
    tcc_define_symbol(tcc, "__NBINPUT", nbinput_s);
    DBG_PRINTF1("__NBINPUT\t'%s'\n", nbinput_s);
    tcc_define_symbol(tcc, "__NX", nx_s);
    DBG_PRINTF1("__NX\t'%s'\n", nx_s);
    tcc_define_symbol(tcc, "__NY", ny_s);
    DBG_PRINTF1("__NY\t'%s'\n", ny_s);
#ifdef NDEBUG
    tcc_define_symbol(tcc, "NDEBUG", "1");
    DBG_PRINTF1("NDEBUG\t'%s'\n", "1");
#else
    /*
     * missing in libtcc, see
     * http://repo.or.cz/w/tinycc.git/commit/5f99fe2f
     */
    /* tcc_enable_debug(tcc); */
#endif
    tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);
    if (0 != tcc_compile_string(tcc, (const char *) __lambda_c))
        ABORT("compilation error");

    /* get the compiled symbol */
    tccmem = malloc((size_t) tcc_relocate(tcc, NULL));
    if (NULL == tccmem)
        ABORT("allocation error");
    if (-1 == tcc_relocate(tcc, tccmem))
        ABORT("relocation error");
    /* get the loop function via double pointer indirection */
    funcpp = (lambda_fp *) tcc_get_symbol(tcc, "__lambda_fp");
    if (NULL == funcpp)
        ABORT("missing __lambda_fp() symbol");
    DBG_CLOCK_TOGGLE();
    DBG_PRINTF1("%0.3fs\tcompiling the code with libtcc\n", DBG_CLOCK_S());

    /* run __lambda(in, out); */
    DBG_CLOCK_START();
    (**funcpp) (in, out);
    DBG_CLOCK_TOGGLE();
    DBG_PRINTF1("%0.3fs\tprocessing the loop\n", DBG_CLOCK_S());

    tcc_delete(tcc);
    free(tccmem);
    return;
}
#endif                          /* WITH_LIBTCC */

/** compile with the local CC */
static void _compile_with_cc(const char *cc, const char *cflags,
                             const char *expr, int nbinput,
                             size_t nx, size_t ny,
                             const char *fname_c, const char *fname_o)
{
    FILE *fd;
    char cmd[512];

    /* compile */
    /* TODO: insert warnings */
    if (0 == system(NULL))
        ABORT("no command processor");
    snprintf(cmd, 512, "%s %s "
             "-D__EXPR='%s' -D__NBINPUT=%i -D__NX=%lu -D__NY=%lu "
             "-fPIC -c -o %s %s",
             cc, cflags, expr, nbinput, nx, ny, fname_o, fname_c);
    DBG_PRINTF1("cmd\t'%s'\n", cmd);
    system(cmd);
    /* try to open the raw object file */
    fd = fopen(fname_o, "r");
    if (NULL == fd)
        ABORT("compilation error");
    fclose(fd);

    return;
}

/** link with the local linker */
static void _link_with_cc(const char *fname_o, const char *fname_so)
{
    FILE *fd;
    char cmd[512];

    /* link */
    snprintf(cmd, 512, "ld -shared -o %s %s", fname_so, fname_o);
    DBG_PRINTF1("cmd\t'%s'\n", cmd);
    system(cmd);
    /* try to open the shared object file */
    fd = fopen(fname_so, "r");
    if (NULL == fd)
        ABORT("compilation error");
    fclose(fd);

    return;
}

/** run the loop with dlopen */
static void _run_with_cc(const char *fname_so, float *const *in, float *out)
{
    void *dl;
    lambda_fp *funcpp;

    /* dynamic load */
    dl = dlopen(fname_so, RTLD_NOW);
    if (NULL == dl)
        ABORT(dlerror());
    /* get the loop function via double pointer indirection */
    funcpp = (lambda_fp *) dlsym(dl, "__lambda_fp");
    if (NULL == funcpp)
        ABORT(dlerror());
    /* run __lambda(in, out); */
    (**funcpp) (in, out);
    dlclose(dl);

    return;
}

/** get the compiler name */
static char *_get_cc(void)
{
    char *_cc, *cc;

    _cc = getenv("CC");
    if (NULL == _cc || '\0' == _cc[0])
        ABORT("missing CC environment variable");

    cc = (char *) malloc((strlen(_cc) + 1) * sizeof(char));
    strcpy(cc, _cc);
    return cc;
}

/** get the compiler flags */
static char *_get_cflags(void)
{
    char *_cflags, *cflags;
    char cflags_empty[] = "";

    _cflags = getenv("CFLAGS");
    if (NULL == _cflags)
        _cflags = cflags_empty;
#ifdef NDEBUG
    /* add "-DNDEBUG" */
    cflags = (char *) malloc((strlen(_cflags) +
                              +strlen(" -DNDEBUG") + 1) * sizeof(char));
    strcpy(cflags, _cflags);
    strcat(cflags, " -DNDEBUG");
#else
    /* add "-g" in debug mode */
    cflags = (char *) malloc((strlen(_cflags)
                              + strlen(" -g") + 1) * sizeof(char));
    strcpy(cflags, _cflags);
    strcat(cflags, " -g");
#endif

    return cflags;
}

/** use the local CC compiler to build and run the lambda loop */
void loop_with_cc(const char *expr, int nbinput,
                  float *const *in, float *out, size_t nx, size_t ny)
{
    char fname[] = "/tmp/cclambda_XXXXXX";
    char fname_c[] = "/tmp/cclambda_XXXXXX.c";
    char fname_o[] = "/tmp/cclambda_XXXXXX.o";
    char fname_so[] = "/tmp/cclambda_XXXXXX.so";
    FILE *fd;
    char *cc, *cflags;

    DBG_PRINTF0("compile with external compiler\n");

    /* temporary source file */
    fd = fdopen(mkstemp(fname), "w");
    (void) fwrite((void *) __lambda_c, sizeof(char), __lambda_c_len, fd);
    fclose(fd);
    /* add suffixes for compiler comfort */
    strcpy(fname_c, fname);
    strcat(fname_c, ".c");
    rename(fname, fname_c);
    strcpy(fname_o, fname);
    strcat(fname_o, ".o");
    strcpy(fname_so, fname);
    strcat(fname_so, ".so");

    /* gather local compiler settings */
    cc = _get_cc();
    DBG_PRINTF1("CC\t'%s'\n", cc);
    cflags = _get_cflags();
    DBG_PRINTF1("CFLAGS\t'%s'\n", cflags);

    /* compile the C code */
    DBG_CLOCK_START();
    _compile_with_cc(cc, cflags, expr, nbinput, nx, ny, fname_c, fname_o);
    /* link into a shared library */
    _link_with_cc(fname_o, fname_so);
    DBG_CLOCK_TOGGLE();
    DBG_PRINTF2("%0.3fs\tcompiling the code with %s\n", DBG_CLOCK_S(), cc);
    /* run the loop */
    DBG_CLOCK_START();
    _run_with_cc(fname_so, in, out);
    DBG_CLOCK_TOGGLE();
    DBG_PRINTF1("%0.3fs\tprocessing the loop\n", DBG_CLOCK_S());

    free(cc);
    free(cflags);
    remove(fname_c);
    remove(fname_o);
    remove(fname_so);
    return;
}
