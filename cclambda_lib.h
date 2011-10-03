#include <stdlib.h>

/** abort() wrapper macro with an error message */
#define ABORT(MSG) do {                                         \
    fprintf(stderr, "%s:%04u : %s\n", __FILE__, __LINE__, MSG); \
    fflush(NULL);                                               \
    abort();                                                    \
    } while (0);

#ifndef NDEBUG
/** printf()-like debug statements */
#define DBG_PRINTF0(STR) {fprintf(stderr, STR);}
#define DBG_PRINTF1(STR, A1) {fprintf(stderr, STR, A1);}
#define DBG_PRINTF2(STR, A1, A2) {fprintf(stderr, STR, A1, A2);}
/** clock counter, initialized to 0 */
#include <time.h>
static clock_t _dbg_clock_counter;
/** reset a CPU clock counter */
#define DBG_CLOCK_RESET() { _dbg_clock_counter = 0; }
/** toggle (start/stop) a CPU clock counter */
#define DBG_CLOCK_TOGGLE() { \
        _dbg_clock_counter = clock() - _dbg_clock_counter; }
/** reset and toggle the CPU clock counter */
#define DBG_CLOCK_START() { DBG_CLOCK_RESET(); DBG_CLOCK_TOGGLE(); }
/** CPU clock counter */
#define DBG_CLOCK() ((long) _dbg_clock_counter)
/** CPU clock time in seconds */
#define DBG_CLOCK_S() ((float) _dbg_clock_counter / CLOCKS_PER_SEC)
#else
#define DBG_PRINTF0(STR) {}
#define DBG_PRINTF1(STR, A1) {}
#define DBG_PRINTF2(STR, A1, A2) {}
#define DBG_CLOCK_RESET() {}
#define DBG_CLOCK_TOGGLE() {}
#define DBG_CLOCK_START() {}
#define DBG_CLOCK() (-1)
#define DBG_CLOCK_S() (-1.)
#endif

/* defined in __lambda.h */
extern unsigned char __lambda_c[];
extern unsigned int __lambda_c_len;

/* cclambda_lib.c */
#ifdef WITH_LIBTCC
void loop_with_libtcc(const char *expr, int nbinput, float *const *in, float *out, size_t nx, size_t ny);
#endif
void loop_with_cc(const char *expr, int nbinput, float *const *in, float *out, size_t nx, size_t ny);
