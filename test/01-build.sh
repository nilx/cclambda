#!/bin/sh
#
# Test the code compilation and execution.

EXPR="(A > .5 ? 1 : 0)"
_test_run() {
    TEMPFILE=$(tempfile)
    CC=$1
    if [ "libtcc" = "$CC" ]; then
	./rw data/lena_gray.png - \
	    | ./cclambda - "$EXPR" > ${TEMPFILE}
    else
	./rw data/lena_gray.png - \
	    | CC=$CC ./cclambda - "$EXPR" > ${TEMPFILE}
    fi
}

################################################

_log_init

echo "* default build, test will all compilers, clean, rebuild"
_log make -B debug
_log _test_run libtcc
_log _test_run cc
_log make -B
_log _test_run libtcc
_log _test_run cc
_log make
_log make clean
_log make

echo "* compiler support"
#for CC in cc c++ c89 c99 gcc g++ tcc nwcc clang icc pathcc suncc; do
for CC in cc c++ icc suncc; do # this test is very long and not crucial
    which $CC || continue
    echo "* $CC compiler"
    _log make -B CC=$CC
    _log _test_run libtcc
    for _CC in cc c89 c99 gcc tcc nwcc clang icc pathcc suncc; do
	which $_CC > /dev/null || continue
	_log _test_run $_CC
    done
done

_log make distclean

_log_clean
