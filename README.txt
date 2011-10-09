% cclambda: C compiled lambda filter on images

* synopsis
* description
* requirements
* files
* compilation
* usage
* expression syntax
* examples
* advanced use
* bugs
* todo
* copyright

# SYNOPSIS

    ./cclambda a.png b.png "expression" > out.png
    CC=gcc CFLAGS="-O3 -ffast-math" ./cclambda a.png b.png "expr" > out.png

# DESCRIPTION

cclambda applies an expression to all the pixels of a collection of
images. The goal of cclambda is similar to Enric Meinhardt's
plambda[1], and cclambda was started after thinking about possible
variations around plambda. The main differences between the two codes
is that cclambda implements no expression parser, stack or dynamic
instruction handling; instead, cclambda writes the expression in a
plain C code context, compiles it and execute the result.

cclambda is less formal and elegant than plambda, it's on the "dirty
hack" side of the force. I hope cclambda can be faster than plambda, or
at least be the proof of concept of another implementation.

cclambda a very early development stage, which means the
features are limited and the implementation is hacky and full
of bugs. Further development may or may not improve it, depending on
the usefulness of cclambda and the interest of the author.

[1] http://dev.ipol.im/git/?p=coco/imscript.git

# REQUIREMENTS

- a POSIX system (for dlopen(), mkstemp(), ...)
- a C compiler to build cclambda
- libtcc for dynamic run-time compilation,
  if WITHOUT_LIBTCC is not defined at compile time
- libpng+zlib to read and write PNG images

# FILES

- cclambda.c     cli handler
- cclambda_lib.c main code
- __lambda.h     dynamically compiled code template, from __lambda.c
- io_png.{c,h}   libpng wrapper

# COMPILATION

Use the makefile, with `make`.

Alternatively, you can manually compile cclambda with:

    cc cclambda.c cclambda_lib.c io_png.c -DWITH_LIBTCC -DNDEBUG \
    -ltcc -lpng -ldl -o cclambda

# USAGE

    cclambda [-c|-h]
    cclambda img1.png img2.png ... 'expr'

        -c       dump loop code
        -h       this help
        imgN.png 1 to 4 input files
                 '-' for stdin
        no '__', ';' or ''' allowed in expr

The default C compiler is the embedded libtcc. It is a fast C
compiling tool with few optimizations and some bugs (see BUGS).
For complex expression on large images, you can benefit from compiler
optimization by specifying an external compiler and the compiling
flags in the CC and CFLAGS environment variables:

    CC=gcc CFLAGS="-O3" ./cclambda a.png b.png "expr" > out.png

External compilation been tested on linux 2.6.32, amd64 architecture,
with GNU ld 2.20.1 and the following compilers:
- gcc    4.4.5
- tcc    0.9.25
- nwcc   0.8.1
- pathcc 4.0.10
- suncc  5.11
- icc    12.0.4

# EXPRESSION SYNTAX

The "expression" is a C expression, inserted in a pixel loop similar to:

    for (i=0, i<..; i++)
        for (j=0, i<..; i++)
	    output[i, j] = <expression>;

The expression can contain any C code valid in this context. The libc
math.h header is included, so all the constants and functions defined
in this header (on your system) are available.

The input images are read as grayscale float arrays with values in
[0,1]. They must all have the same size. The output image is written
from a grayscale float array with values in [0,1]. You can refer to the
input image values as A, B, C and D. No more than 4 input images are
(currently) supported.

The image width and height are available as the NX and NY macros. You
van also access the current horizontal and vertical coordinates via
the I and J macros, in [0,NX[ ans [0,NY[.

Some other convenience macros are available:
- N  number of pixels in the image
- R2 square normalized distance to the center of the image
- T  angle from the center of the image

For every image, some pixel position modifiers are available:
A_(dx, dy) is the value of the image A on the pixel I+dx, _J+dy.

A value of pixel (I,J)
A_(0,0) value of pixel (I,J)
A_(1,0) value of pixel (I+1,J)
A_(-1,-1) value of pixel (I-1,J-1)

Access to pixels out of the image gives a 0 value.

# EXAMPLES

Sum two images:

  cclambda a.png b.png "(A + B) / 2" > c.png

Add a gaussian to half of lena:

  cclambda lena.png "(A / 2) + exp(-40. * sqrt(R2))" > out.png

Forward differences to compute the derivative in horizontal direction:

  cclambda "A_(1,0) - A" < lena.png > out.png

Sobel edge detector:

  cclambda lena.png "hypot(2 * A_(1,0) + A_(1,1) + A_(1,-1) 
                 - 2 * A_(-1,0) + A_(-1,1) + A_(-1,-1),
                 2 * A_(0,1) + A_(1,1) + A_(-1,1)
                 - 2 * A_(0,-1) + A_(1,-1) + A_(-1,-1))" > sobel.png

# ADVANCED USE

## MULTI-THREADING

The cclambda loop processes every pixel with the same operations. This
is well adapted to parallel processing, and cclambda can use OpenMP
multi-threading to process large images faster.

To use OpenMP, you need to compile cclambda with OpenMP compiler
options. The makefile contains presets for gcc compilers, enabled with
`make OMP=1`. Options for other compilers (icc, suncc) are commented
in makefile.openmp.

Alternatively, you can manually compile cclambda with gcc and OpenMP
support with:

    gcc cclambda.c cclambda_lib.c io_png.c -DWITH_LIBTCC -DNDEBUG \
    -fopenmp -ltcc -lpng -ldl -lgomp -o cclambda

Then you must use cclambda with the same compiler and compiler options:

    CC=gcc CFLAGS="-O3 -fopenmp" ./cclambda a.png b.png "expr" > out.png

OpenMP will only work when cclambda is compiled and invoked with the
same compiler and compiler options. But you can use an OpenMP-enabled
cclambda with any compiler (including the internal libtcc) if you
don't use parallel processing.

## DEBUG MODE

Compiling cclambda in debug mode (without the NDEBUG macro) activates
debugging options:
* the compilation, execution and input/output time are measured;
* the compiler choice (libtcc or an external compiler) is reported,
  with the preprocessor symbols and the full compiler and linker
  command-line when an external compiler is used;
* the -g option is automatically added to the compiler flags;
* the number of OpenMP threads is reported.

You can compile in debug mode with the special make target
"debug". This will also compile cclambda with the debug symbols and
link with the Electric Fence memory debugging library.

Alternatively, you can do it manually with:

    cc -g cclambda.c cclambda_lib.c io_png.c -DWITH_LIBTCC -UNDEBUG \
    -ltcc -lpng -ldl -o cclambda

# BUGS

Some expressions trigger a segmentation fault on tcc_compile() or
tcc_delete(). This complex bug has not been investigated yet, and it
may or may not happen depending on the compilation
settings. Meanwhile, we recommend to compile cclambda without
optimization, as it seems to "solve" some of these errors. You can
still use compiler optimizations for the loop via CFLAGS at run time.

# TODO

Float file format
Compare compiler speed
Compare speed with plambda
More macros
RGB solution

# COPYRIGHT

Copyright 2011 Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>

Copying and distribution of this README file, with or without
modification, are permitted in any medium without royalty provided
the copyright notice and this notice are preserved.  This file is
offered as-is, without any warranty.
