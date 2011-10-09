#!/bin/sh -e
#
# Check the expression support

IN=data/lena.png
OUT=/dev/null

_test_expr() {
    EXPR=$1
    ./cclambda $IN "$EXPR" > $OUT
    CC=gcc ./cclambda $IN "$EXPR" > $OUT
    CC=gcc CFLAGS=-O3 ./cclambda $IN "$EXPR" > $OUT
}

_test_canny() {
    (./cclambda $IN "hypot(A_(1,0) - A, A_(0,1) - A)"; \
     ./cclambda $IN "atan2(A_(0,1) - A, A_(1,0) - A) / (2 * M_PI) + .5" ) \
     | ./cclambda - - \
	"((A > .1) && \
          ((((fabs(B - .5) < .125) || \
             (fabs(B - .5) > .375)) && \
            (A > A_(1,0)) && \
            (A > A_(-1,0))) || \
           (((fabs(B - .5) > .125) || \
             (fabs(B - .5) < .375)) && \
            (A > A_(0,1)) && \
            (A > A_(0,-1)))) \
          ? 1 : 0) " > $OUT
}

################################################

_log_init

echo "* check expressions"
_log make -B

_log _test_expr "(A > .5 ? 1 : 0)"
_log _test_expr "(A / 2) + exp(-40. * R2)"
_log _test_expr "A_(1,0) - A"
_log _test_canny
export CC=gcc
_log _test_canny
export CFLAGS=-O3
_log _test_canny
unset CC CFLAGS

_log make distclean

_log_clean
