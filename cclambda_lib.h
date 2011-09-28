#include <stdlib.h>
#include <stdio.h>

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

/* defined in __lambda.h */
unsigned char __lambda_c[];
unsigned int __lambda_c_len;

/* cclambda_lib.c */
void loop_with_libtcc(const char *expr, int nbinput, float *const *in, float *out, size_t nx, size_t ny);
void loop_with_cc(const char *expr, int nbinput, float *const *in, float *out, size_t nx, size_t ny);
