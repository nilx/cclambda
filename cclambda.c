/*
 * compile with cc -c -D__NBINPUT=X -D__LAMBDA="..."
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "io_png.h"


static void __lambda(float * const *__in, float *__out,
                     size_t __nx, size_t __ny)
{

    const size_t __nc = 3;
    size_t __n;


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

#if (__NBINPUT >= 1)
    const float *__a = __in[0];
#define A (__a[__n])
#define A_(DI, DJ) (I + (DI) < __nx && J + (DJ) < __ny		\
		    && I + (DI) >= 0 && J + (DJ) >= 0		\
		    ? __a[I + (DI) + (J + (DJ)) * __nx + K * __nx * __ny] : 0.)
#define A0 (__a[I + J * __nx])
#define A1 (__a[I + J * __nx + __nx * __ny])
#define A2 (__a[I + J * __nx + __nx * __ny * 2])
#endif

#if (__NBINPUT >= 2)
    const float *__b = __in[1];
#define B (__b[__n])
#define B_(DI, DJ) (I + (DI) < __nx && J + (DJ) < __ny		\
	            && I + (DI) >= 0 && J + (DJ) >= 0		\
		    ? __b[I + (DI) + (J + (DJ)) * __nx + K * __nx * __ny] : 0.)
#define B0 (__b[I + J * __nx])
#define B1 (__b[I + J * __nx + __nx * __ny])
#define B2 (__b[I + J * __nx + __nx * __ny * 2])
#endif

#if (__NBINPUT >= 3)
    const float *__c = __in[2];
#define C (__c[__n])
#define C_(DI, DJ) (I + (DI) < __nx && J + (DJ) < __ny		\
	            && I + (DI) >= 0 && J + (DJ) >= 0		\
		    ? __c[I + (DI) + (J + (DJ)) * __nx + K * __nx * __ny] : 0.)
#define C0 (__c[I + J * __nx])
#define C1 (__c[I + J * __nx + __nx * __ny])
#define C2 (__c[I + J * __nx + __nx * __ny * 2])
#endif

#if (__NBINPUT >= 3)
    const float *__d = __in[2];
#define D (__d[__n])
#define D_(DI, DJ) (I + (DI) < __nx && J + (DJ) < __ny		\
	            && I + (DI) >= 0 && J + (DJ) >= 0		\
		    ? __d[I + (DI) + (J + (DJ)) * __nx + K * __nx * __ny] : 0.)
#define D0 (__d[I + J * __nx])
#define D1 (__d[I + J * __nx + __nx * __ny])
#define D2 (__d[I + J * __nx + __nx * __ny * 2])
#endif

#ifndef __LAMBDA
#define __LAMBDA 0.
#endif

    for (__n = 0; __n < 3 * __nx * __ny; __n++) {
        __out[__n] = __LAMBDA;
    }
    return;
}

int main(int argc, char **argv)
{
    float * in[__NBINPUT];
    float * out;
    size_t i;
    size_t _nx, _ny;
    size_t nx, ny;

    if (__NBINPUT > 4) {
	fputs("max 4 input\n", stderr);
	abort();
    }

    if (__NBINPUT != argc-1) {
	fputs("inconsistent compilation and execution parameters\n", stderr);
	abort();
    }

    for (i = 0; i < __NBINPUT; i++) {
	in[i] = io_png_read_pp_flt(argv[i+1], &nx, &ny, NULL, IO_PNG_OPT_RGB);
	if (0 == i) {
	    _nx = nx;
	    _ny = ny;
	} else {
	    assert(nx == _nx && ny == _ny);
	}
    }
    out = (float *) malloc(3 * nx * ny * sizeof(float));

    __lambda(in, out, nx, ny);

    io_png_write_flt("-", out, nx, ny, 3);

    return EXIT_SUCCESS;
}
