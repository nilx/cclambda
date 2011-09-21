/*
 * compile with cc -c -D__LAMBDA="..."
 */

#include <stdlib.h>
#include <math.h>

static void __lambda(const float **__in, float *__out,
                     size_t __nx, size_t __ny)
{

    size_t __n;
    const float *__a = __in[0];
    const float *__b = __in[1];
    const float *__c = __in[2];

    const size_t __nc = 3;

#define NX (__nx)
#define NY (__ny)
#define NC (__nc)

#define N (NX * NY)

#define I (__n % __nx)
#define J (__n / __nx % __ny)
#define K (__n / __nx / __ny)

#define R2 ((I - NX / 2.) * (I - NX / 2.)       \
            + (J - NY / 2.) * (J - NY / 2.))
#define T (atan())

#define A (__a[__n])
#define B (__b[__n])
#define C (__c[__n])

#define _A(DI, DJ) (I + (DI) < __nx && J + (DJ) < __ny  \
                    ? __a[__n + DI + DJ * __nx] : 0.)
#define _B(DI, DJ) (I + (DI) < __nx && J + (DJ) < __ny  \
                    ? __b[__n + DI + DJ * __nx] : 0.)
#define _C(DI, DJ) (I + (DI) < __nx && J + (DJ) < __ny  \
                    ? __c[__n + DI + DJ * __nx] : 0.)

#define A0 (__a[I + J * __nx])
#define A1 (__a[I + J * __nx + __nx * __ny])
#define A2 (__a[I + J * __nx + __nx * __ny * 2])

#define B0 (__b[I + J * __nx])
#define B1 (__b[I + J * __nx + __nx * __ny])
#define B2 (__b[I + J * __nx + __nx * __ny * 2])

#define C0 (__c[I + J * __nx])
#define C1 (__c[I + J * __nx + __nx * __ny])
#define C2 (__c[I + J * __nx + __nx * __ny * 2])

#ifndef __LAMBDA
#define __LAMBDA 0.
#endif

    for (__n = 0; __n < 3 * __nx * __ny; __n++) {
        __out[__n] = __LAMBDA;
    }
    return;
}
