#include <stdlib.h>

/* defined in __lambda.h */
extern unsigned char __lambda_c[];
extern unsigned int __lambda_c_len;

/* cclambda_lib.c */
#ifdef WITH_LIBTCC
void loop_with_libtcc(const char *expr, int nbinput, float *const *in, float *out, size_t nx, size_t ny);
#endif
void loop_with_cc(const char *expr, int nbinput, float *const *in, float *out, size_t nx, size_t ny);
