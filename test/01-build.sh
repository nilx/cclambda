#!/bin/sh
#
# Test the code compilation and execution.

EXPR="(A > .5 ? 1 : 0)"
_test_run() {
    TEMPFILE=$(tempfile)
    ./cclambda - "$EXPR" < data/lena.png > ${TEMPFILE}
    for CC in cc gcc tcc nwcc clang icc pathcc suncc; do
	which $CC || continue
	CC=${CC} ./cclambda - "$EXPR" < data/lena.png > ${TEMPFILE}
    done
}

_test_ldd() {
    test "0" = "$(ldd cclambda | grep -c png)"
}

################################################

_log_init

echo "* default build, test will all compilers, clean, rebuild"
_log make -B debug
_log _test_run
_log make -B
_log _test_run
_log make
_log make clean
_log make

echo "* compiler support"
#for CC in cc c++ gcc g++ tcc nwcc clang icc pathcc suncc; do
for CC in ; do # this test is very long and not crucial 
    which $CC || continue
    echo "* $CC compiler"
    case $CC in
	"gcc"|"g++")
	    _log make -B CC=$CC ;;
	*)
	    _log make -B CC=$CC CFLAGS= ;;
    esac
    _log _test_run
done

# static build
_log make distclean
_log make -B STATIC=1
_log _test_run
_log _test_ldd

_log make distclean

_log_clean
