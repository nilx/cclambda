#!/bin/sh -e
#
# Check there is no memory leak with valgrind/memcheck.

_test_memcheck() {
#libtcc has a memory leak problem
    echo "( CC=cc valgrind --tool=memcheck $@ > /dev/null ) 2>&1 \
                   | grep -c 'LEAK'"
    test "0" = "$( ( CC=cc valgrind --tool=memcheck \
                   ./rw data/lena_gray.png - | ./cclambda - '(A > .5 ? 1 : 0)' > /dev/null ) \
                   2>&1 | grep -c 'LEAK' )"
}

################################################

_log_init

echo "* check memory leaks"
_log make -B
_log _test_memcheck
_log make distclean

_log_clean
