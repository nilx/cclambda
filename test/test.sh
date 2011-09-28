#! /bin/sh

set -e

for L in "(A / 2) + exp(-40. * R2)" \
    "A_(1,0) - A" \
    "hypot(2 * A_(1,0) + A_(1,1) + A_(1,-1) \
     - 2 * A_(-1,0) + A_(-1,1) + A_(-1,-1), \
     2 * A_(0,1) + A_(1,1) + A_(-1,1) \
     - 2 * A_(0,-1) + A_(1,-1) + A_(-1,-1))" \
    "(K == 1 ? 0 : A)" \
    "A2" \
    "(A0 + A1 + A2) / 3" \
    "(K == 0 ? A2 : A)" \
    ; do
    echo __LAMBDA="$L"
    ./cclambda ./data/lena.png "$L" > /dev/null
    done
