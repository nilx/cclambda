#! /bin/sh

set -e

for L in "A + B" \
    "(A / 2) + 200 * exp(-40. * R2)" \
    "A_(1,0) - A" \
    "hypot(2 * A_(1,0) + A_(1,1) + A_(1,-1) \
     - 2 * A_(-1,0) + A_(-1,1) + A_(-1,-1), \
     2 * A_(0,1) + A_(1,1) + A_(-1,1) \
     - 2 * A_(0,-1) + A_(1,-1) + A_(-1,-1))" \
    "(K == 1 ? 0 : A)" \
    "A2" \
    "(A0 + A1 + A2) / 3" \
    "(K == 0 ? A2 : A)" \
    "(K == 2 ? A0 : A)" \
    "(K == 0 ? A : B)" \
    ; do
    echo __LAMBDA="$L"
    cc -c -D__LAMBDA="$L" lambdacode.c
    tcc -c -D__LAMBDA="$L" lambdacode.c
    done
