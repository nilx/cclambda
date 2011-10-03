#!/bin/sh -e
#
# Check the expression support

_test_expr() {
    ./cclambda data/lena.png "$EXPR" > /dev/null
    CC=cc ./cclambda data/lena.png "$EXPR" > /dev/null
}

################################################

_log_init

echo "* check expressions"
_log make -B
for EXPR in \
    "(A > .5 ? 1 : 0)" \
    "(A / 2) + exp(-40. * R2)" \
    "A_(1,0) - A" \
    "(K == 1 ? 0 : A)" \
    "A2" \
#    "(A0 + A1 + A2) / 3" \
#    "hypot(2 * A_(1,0) + A_(1,1) + A_(1,-1) \
#     - 2 * A_(-1,0) + A_(-1,1) + A_(-1,-1), \
#     2 * A_(0,1) + A_(1,1) + A_(-1,1) \
#     - 2 * A_(0,-1) + A_(1,-1) + A_(-1,-1))" \
    do
    _log _test_expr "$EXPR"
done
_log make distclean

_log_clean
