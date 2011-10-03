#include <stdlib.h>
#include <stdio.h>

/** abort() wrapper macro with an error message */
#define ABORT(MSG) do {                                         \
    fprintf(stderr, "%s:%04u : %s\n", __FILE__, __LINE__, MSG); \
    fflush(NULL);                                               \
    abort();                                                    \
    } while (0);

/** printf()-like debug statements */
#ifndef NDEBUG
#define DBG_PRINTF0(STR) {fprintf(stderr, STR);}
#define DBG_PRINTF1(STR, A1) {fprintf(stderr, STR, A1);}
#else
#define DBG_PRINTF0(STR) {}
#define DBG_PRINTF1(STR, A1) {}
#endif

/* defined in __lambda.h */
extern unsigned char __lambda_c[];
extern unsigned int __lambda_c_len;

/* cclambda_lib.c */
#ifdef WITH_LIBTCC
void loop_with_libtcc(const char *expr, int nbinput, float *const *in, float *out, size_t nx, size_t ny);
#endif
void loop_with_cc(const char *expr, int nbinput, float *const *in, float *out, size_t nx, size_t ny);
