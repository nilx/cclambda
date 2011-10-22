#!/bin/sh -e
#
# Check the OpenMP support

_test_openmp_support() {
    LIB=$1
    test "1" = "$(./cclambda -h | grep -c with\ OpenMP)"
    test "1" = "$(ldd cclambda | grep -c $LIB)"
}

_test_openmp_run() {
    CC=$1
    case $CC in
	"gcc")
	    CFLAGS="-O3 -fopenmp"
	    ;;
	"icc")
	    CFLAGS="-O3 -openmp"
	    ;;
	"suncc")
	    CFLAGS="-O3 -xopenmp"
	    ;;
    esac
    export CC=$CC
    export CFLAGS="$CFLAGS"
    export OMP_NUM_THREADS=4
    test "1" = "$( ( ./test/rw data/lena_gray.png - \
                     | ./cclambda '(A > .5 ? 1 : 0)' > /dev/null ) 2>&1 \
                  | grep -c 'using 4 OpenMP threads')"
    unset CC CFLAGS OMP_NUM_THREADS
}

################################################

_log_init

LPATH_ICC=/opt/intel/composerxe-2011.4.191/compiler/lib/intel64/
LPATH_SUNCC=/opt/solstudio12.2/prod/lib/compilers/rtlibs/usr/lib/amd64/
export LD_LIBRARY_PATH="$LPATH_ICC:$LPATH_SUNCC"

echo "* check openmp"
_log make -B OMP=1
_log _test_openmp_support libgomp
_log make -B OMP=1 debug
_log _test_openmp_run gcc

_log make -B OMP=1 CC=gcc CFLAGS_OMP="-fopenmp" LDFLAGS_OMP="-lgomp"
_log _test_openmp_support libgomp
_log make -B OMP=1 CC=gcc CFLAGS_OMP="-fopenmp" LDFLAGS_OMP="-lgomp" debug
_log _test_openmp_run gcc

_log make -B OMP=1 CC=icc CFLAGS_OMP="-openmp" \
    LDFLAGS_OMP="-liomp5 -lpthread -L$LPATH_ICC"
_log _test_openmp_support libiomp5
_log make -B OMP=1 CC=icc CFLAGS_OMP="-openmp" \
    LDFLAGS_OMP="-liomp5 -lpthread -L$LPATH_ICC" debug
_log _test_openmp_run icc

_log make -B OMP=1 CC=suncc CFLAGS_OMP="-xopenmp" \
    LDFLAGS_OMP="-lmtsk -L$LPATH_SUNCC"
_log _test_openmp_support libmtsk
_log make -B OMP=1 CC=suncc CFLAGS_OMP="-xopenmp" \
    LDFLAGS_OMP="-lmtsk -L$LPATH_SUNCC" debug
_log _test_openmp_run suncc

_log make distclean

_log_clean
