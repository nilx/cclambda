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

#define NX (__NX)
#define NY (__NY)
#define NC (3)

#define N (NX * NY)

#define I (__n % NX)
#define J (__n / NX % NY)
#define K (__n / NX / NY)

#define R2 ((I / (float) NX - .5) * (I / (float) NX - .5) \
            + (J / (float) NY - .5) * (J / (float) NY - .5))
#define T (atan())

#if (__NBINPUT >= 1)
#define A (__a[__n])
#define A_(DI, DJ) (I + (DI) < NX && J + (DJ) < NY                      \
                    && I + (DI) >= 0 && J + (DJ) >= 0                   \
                    ? __a[I + (DI) + (J + (DJ)) * NX + K * NX * NY] : 0.)
#define A0 (__a[I + J * NX])
#define A1 (__a[I + J * NX + NX * NY])
#define A2 (__a[I + J * NX + NX * NY * 2])
#endif

#if (__NBINPUT >= 2)
#define B (__b[__n])
#define B_(DI, DJ) (I + (DI) < NX && J + (DJ) < NY                      \
                    && I + (DI) >= 0 && J + (DJ) >= 0                   \
                    ? __b[I + (DI) + (J + (DJ)) * NX + K * NX * NY] : 0.)
#define B0 (__b[I + J * NX])
#define B1 (__b[I + J * NX + NX * NY])
#define B2 (__b[I + J * NX + NX * NY * 2])
#endif

#if (__NBINPUT >= 3)
#define C (__c[__n])
#define C_(DI, DJ) (I + (DI) < NX && J + (DJ) < NY                      \
                    && I + (DI) >= 0 && J + (DJ) >= 0                   \
                    ? __c[I + (DI) + (J + (DJ)) * NX + K * NX * NY] : 0.)
#define C0 (__c[I + J * NX])
#define C1 (__c[I + J * NX + NX * NY])
#define C2 (__c[I + J * NX + NX * NY * 2])
#endif

#if (__NBINPUT >= 4)
#define D (__d[__n])
#define D_(DI, DJ) (I + (DI) < NX && J + (DJ) < NY                      \
                    && I + (DI) >= 0 && J + (DJ) >= 0                   \
                    ? __d[I + (DI) + (J + (DJ)) * NX + K * NX * NY] : 0.)
#define D0 (__d[I + J * NX])
#define D1 (__d[I + J * NX + NX * NY])
#define D2 (__d[I + J * NX + NX * NY * 2])
#endif

void __lambda(float *const *__in, float *__out)
{

    size_t __n;
#if (__NBINPUT >= 1)
    const float *__a = __in[0];
#endif
#if (__NBINPUT >= 2)
    const float *__b = __in[1];
#endif
#if (__NBINPUT >= 3)
    const float *__c = __in[2];
#endif
#if (__NBINPUT >= 4)
    const float *__d = __in[2];
#endif

    for (__n = 0; __n < NC * NX * NY; __n++) {
        __out[__n] = (__EXPR);
    }
    return;
}
