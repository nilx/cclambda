#include <math.h>

void __lambda(const size_t __nx, const size_t __ny, const size_t __nc)
{

    size_t __n;

#define NX (__nx)
#define NY (__ny)
#define NC (__nc)

#define N (NX * NY)

#define I (__n % __nx)
#define J (__n / __nx % __ny)
#define K (__n / __nx / __ny)

#define R2 ((_i - _x / 2.) * (_i - _x / 2.) + (_j - _y / 2.) * (_j - _y / 2.))
#define T (atan...)

#define A (__a[__n])
#define B (__b[__n])
#define C (__c[__n])

#define _A(DI, DJ) (I + (DI) < __nx && J + (DJ) < __ny	\
		    ? __a[__n + DI + DJ * __nx] : 0.)
#define _B(DI, DJ) (I + (DI) < __nx && J + (DJ) < __ny	\
		    ? __b[__n + DI + DJ * __nx] : 0.)
#define _C(DI, DJ) (I + (DI) < __nx && J + (DJ) < __ny	\
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

    for (__n=0; __n < __size; __n ++) {
	__out[__n] = <EXPRESSION>;
    }    
}
