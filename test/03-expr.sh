#!/bin/sh -e
#
# Check the expression support

_test_expr() {
    IN=data/lena.png
    OUT=/dev/null
    ./cclambda $IN "$EXPR" > $OUT
}

_test_canny() {
    IN=data/lena.png
    OUT=/dev/null
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
for EXPR in \
    "(A > .5 ? 1 : 0)" \
    "(A / 2) + exp(-40. * R2)" \
    "A_(1,0) - A" \
#    "hypot(2 * A_(1,0) + A_(1,1) + A_(1,-1) \
#     - 2 * A_(-1,0) + A_(-1,1) + A_(-1,-1), \
#     2 * A_(0,1) + A_(1,1) + A_(-1,1) \
#     - 2 * A_(0,-1) + A_(1,-1) + A_(-1,-1))" \
    do
    unset CC
    unset CFLAGS
    _log _test_expr "$EXPR"
    export CC=gcc
    _log _test_expr "$EXPR"
    export CFLAGS=-O3
    _log _test_expr "$EXPR"
done
unset CC
unset CFLAGS
_log _test_canny
export CC=gcc
_log _test_canny
export CFLAGS=-O3
_log _test_canny
unset CC
unset CFLAGS

_log make distclean

_log_clean
