/**
 * @file __lambda.c
 * @brief lambda loop code template
 *
 * some macros must be defined at compile time:
 *  __NBINPUT
 *  __EXPR
 *  __NX
 *  __NY
 */

#include <stdlib.h>
#include <math.h>

#ifndef NDEBUG
#include <stdio.h>
/** printf()-like debug statements */
#define DBG_PRINTF1(STR, A1) {fprintf(stderr, STR, A1);}
#else
#define DBG_PRINTF1(STR, A1) {}
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

/*
 * default values
 */

#ifndef __EXPR
#define __EXPR 0.
#endif

#ifndef __NBINPUT
#define __NBINPUT 0
#endif

#ifndef __NX
#define __NX 0
#endif

#ifndef __NY
#define __NY 0
#endif

/*
 * indices
 */

#define NX (__NX)
#define NY (__NY)

#define N (NX * NY)

#define I (__n % NX)
#define J (__n / NX % NY)

/*
 * position
 */

#define R2 ((I / (float) NX - .5) * (I / (float) NX - .5) \
            + (J / (float) NY - .5) * (J / (float) NY - .5))
/* TODO: define the angle */
#define T (atan())

/*
 * input arrays
 */

#if (__NBINPUT >= 1)
#define A (__a[__n])
#define A_(DI, DJ) (I + (DI) < NX && J + (DJ) < NY              \
                    && I + (DI) >= 0 && J + (DJ) >= 0           \
                    ? __a[I + (DI) + (J + (DJ)) * NX] : 0.)
#endif

#if (__NBINPUT >= 2)
#define B (__b[__n])
#define B_(DI, DJ) (I + (DI) < NX && J + (DJ) < NY              \
                    && I + (DI) >= 0 && J + (DJ) >= 0           \
                    ? __b[I + (DI) + (J + (DJ)) * NX] : 0.)
#endif

#if (__NBINPUT >= 3)
#define C (__c[__n])
#define C_(DI, DJ) (I + (DI) < NX && J + (DJ) < NY              \
                    && I + (DI) >= 0 && J + (DJ) >= 0           \
                    ? __c[I + (DI) + (J + (DJ)) * NX] : 0.)
#endif

#if (__NBINPUT >= 4)
#define D (__d[__n])
#define D_(DI, DJ) (I + (DI) < NX && J + (DJ) < NY              \
                    && I + (DI) >= 0 && J + (DJ) >= 0           \
                    ? __d[I + (DI) + (J + (DJ)) * NX] : 0.)
#endif

/*
 * use "restrict" in C99 only
 */
#if (defined(__STDC__) && defined(__STDC_VERSION__) \
     && __STDC_VERSION__ >= 199901L)
#define __RSTRCT restrict
#else
#define __RSTRCT
#endif

void __lambda(float *const *__in, float *__RSTRCT __out)
{

    size_t __n;

#if (__NBINPUT >= 1)
    const float *__RSTRCT __a = __in[0];
#endif
#if (__NBINPUT >= 2)
    const float *__RSTRCT __b = __in[1];
#endif
#if (__NBINPUT >= 3)
    const float *__RSTRCT __c = __in[2];
#endif
#if (__NBINPUT >= 4)
    const float *__RSTRCT __d = __in[3];
#endif

#ifdef _OPENMP
    DBG_PRINTF1("using %i OpenMP threads\n", omp_get_max_threads());
#pragma omp parallel for default(shared) private(__n)
#endif
    for (__n = 0; __n < NX * NY; __n++) {
        __out[__n] = (__EXPR);
    }
    return;
}

/**
 * @brief pointer to the compiled __lambda() function
 *
 * We have to maintain this pointer because of conflicts between ISO C
 * and POSIX standards.
 * cf. "RATIONALE" section of
 *   http://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html
 * and "Solving the function pointer problem on POSIX systems" section of
 *   http://en.wikipedia.org/wiki/Dynamic_loading
 */
void (*__lambda_fp) (float *const *, float *) = &__lambda;
