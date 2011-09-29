#include <stdlib.h>
#include <stdio.h>

/** abort() wrapper macro with an error message */
#define ABORT(MSG) do {                                         \
    fprintf(stderr, "%s:%04u : %s\n", __FILE__, __LINE__, MSG); \
    fflush(NULL);                                               \
    abort();                                                    \
    } while (0);

/**
 * @brief printf()-like debug statements
 *
 * Variadic macros are not ANSI C89, so we define a set of macros with
 * a fixed number of arguments.
 *
 * Use like printf():
 * DBG_PRINTF("entering function foo()\n");
 * DBG_PRINTF2("i=%i, j=%i\n", i, j);
 */
#ifndef NDEBUG
#define DBG_PRINTF0(STR) {fprintf(stderr, STR);}
#define DBG_PRINTF1(STR, A1) {fprintf(stderr, STR, A1);}
#define DBG_PRINTF2(STR, A1, A2) {fprintf(stderr, STR, A1, A2);}
#define DBG_PRINTF3(STR, A1, A2, A3) {fprintf(stderr, STR, A1, A3, A3, A4);}
#define DBG_PRINTF4(STR, A1, A2, A3, A4) {fprintf(stderr, STR, A1, A2, A3, A4);}
#else
#define DBG_PRINTF0(STR) {}
#define DBG_PRINTF1(STR, A1) {}
#define DBG_PRINTF2(STR, A1, A2) {}
#define DBG_PRINTF3(STR, A1, A2, A3) {}
#define DBG_PRINTF4(STR, A1, A2, A3, A4) {}
#endif

/* defined in __lambda.h */
extern unsigned char __lambda_c[];
extern unsigned int __lambda_c_len;

/* cclambda_lib.c */
void loop_with_libtcc(const char *expr, int nbinput, float *const *in, float *out, size_t nx, size_t ny);
void loop_with_cc(const char *expr, int nbinput, float *const *in, float *out, size_t nx, size_t ny);
