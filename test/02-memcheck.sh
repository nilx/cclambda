#!/bin/sh -e
#
# Check there is no memory leak with valgrind/memcheck.

_test_memcheck() {
#libtcc has a memory leak problem
    test "0" = "$( ( ./test/rw data/lena_gray.png - \
                     | CC=$1 valgrind --tool=memcheck \
                       ./cclambda - '(A > .5 ? 1 : 0)' > /dev/null ) 2>&1 \
                   | grep -c 'LEAK' )"
}

################################################

_log_init

echo "* check memory leaks"
_log make -B
#_log _test_memcheck libtcc # libtcc has a memory leak
_log _test_memcheck cc
_log _test_memcheck icc
_log _test_memcheck suncc
_log make distclean

_log_clean
