% cclambda: C compiled lambda filter on images

# USAGE

  ./cclambda a.png b.png "expression" > out.png
  cat a.png b.png | ./cclambda.sh - - "expression" > out.png

# DESCRIPTION

cclambda applies an expression to all the pixels of a collection of
images. The goal of cclambda is almost identical to Enric Meinhardt's
plambda[1], and cclambda was started after thinking about possible
variations around plambda. The main differences between the two codes
is that cclambda implements no expression parser or dynamic RPN
execution; instead, cclambda writes the expression in a C code
context, compiles it and execute the result.

cclambda is less formal and elegant than plambda, it's on the "dirty
hack" side of the force. I hope cclambda can be faster than plambda, or
at least be the proof of concept of another implementation.

cclambda a very early development stage, which means the
functionnalities are limited and the implementation is hacky and full
of bugs. Further development may or may not improve it, depending on
the usefullness of cclambda and the interest of the author.

[1] http://dev.ipol.im/git/?p=coco/imscript.git

# SYNTAX

The "expression" is a C expression, inserted in a pixel loop similar to:

    for (i=0, i<..; i++)
        for (j=0, i<..; i++)
	    output[i, j] = <expression>;

The expression can contain any C code valid in this context. The libc
math.h header is included, so all the constants and fonctions defined
in this header (on your system) are available.

The inputt images are read as RGB float arrays with values in
[0,1]. They must all have the same size. The output image is written
from a RGB floar array with values in [0,1]. You can refer to the
input image values as A, B, C and D. no more than 4 input images are
(currently) supported.

For each input image, you can explicitely select a color channel: A0,
A1, A2, are the values of the red, green and blue channels of the
first image.

The image width and height are available as the NX and NY macros. You
van also access the current horizontal and vertical coordinates via
the I and J macros, in [0,NX[ ans [0,NY[. The current channel is K in
[0,3[.

Some other convenience macros are available:
- N  number of pixels in the image
- R2 square normalized distance to the center of the image
- T  angle from the center of the image

For ecery image, some pixel position modifiers are available:
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

Set to 0 the green component of a RGB image:

  cclambda - "(K == 1 ? 0 : A)" < lena.png > nogreen.png

Pick the blue channel of a RGB image:

  cclambda "A2"

Color to gray:

  cclambda "(A0 + A1 + A2) / 3"

Swap the red an green channels of a RGB image:

  ( cclambda - "(K == 0 ? A2 : A)" < lena.png; 
    cclambda - "(K == 2 ? A0 : A)" < lena.png ) \
    | cclambda - - "(K == 0 ? A : B)" > lena_bgr.png

Display the three RGB channels:

  ./cclambda - "(K == 0 ? A : 0)" < data/lena.png | display
  ./cclambda - "(K == 1 ? A : 0)" < data/lena.png | display
  ./cclambda - "(K == 2 ? A : 0)" < data/lena.png | display

# REQUIREMENTS

A C compiler is needed. tcc is a good choice because it is light and
fast. If tcc is not available, the default C compiler (cc) wiill be
used.

You also need libpng (binary and headers).

# FILES

- cclambda     shell wrapper
- cclambda.c   lambda template
- io_png.{c,h} libpng wrapper

# TODO

Detect and filter "__" and ";" in the expression.

Replace the shell+cc prototype by a full compiled C stand-alone
implementation:
- generate the cclambda.c code as a string
- compile in-memory with libtcc
- run the compiled function
- libpng and libtcc can be embedded in a static build

Compare speed with plambda.
