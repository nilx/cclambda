% ilambda: lambda filter on images

# USAGE

  cat a.png b.png c.png ... | ilambda "expression" out.png

# DESCRIPTION

Ilambda applies an expression to all the pixels of a collection of
images. The goal of ilambda is almost identical to Enric Meinhardt's
plambda[1], and ilambda was started after thinking about possible
variations around plambda. The main differences between the two codes
are:
- ilambda works as a filter, it reads data from stdin and writes to
  stdout
- ilambda implements no expression parser or dynamic RPN
  execution; instead, ilambda writes the expression in a C code
  context, compiles it and loads the object code

ilambda is less formal and elegant than plambda, it's on the "dirty
hack" side of the force. I hope ilambda can be faster than plambda, or
at least be the proof of concept of another implementation.

[1] http://dev.ipol.im/git/?p=coco/imscript.git

# SYNTAX

The "expression" is a C expression, inserted in a (pseudo) pixel loop like:

    for (i=0, i<..; i++)
        for (j=0, i<..; i++)
	    output[i, j] = <expression>;

The expression can contain any C code valid in this context. The
math.h fonctions are available.

The libc math.h header is included, so all the constants and fonctions
defined in this header (on your system) are available.

A collection of macros is abvailable to access the image values:

A, B and C are the single-precision floating-point values of the three
input images. No more than 3 input images are curently handled. The
images are read as RGB with values in [0,1]. 

Channel selectors are available for every image.
A0, A1, A2, are the values of the red, green and blue channels.

Pixel position modifiers are available for every image.
A_(dx, dy) is the value of the image a on the pixel I+dx, _J+dy.

A value of pixel (i,j)
A_(0,0) value of pixel (i,j)
A_(1,0) value of pixel (i+1,j)
A_(-1,-1) value of pixel (i-1,j-1)

TODO: handle borders

A value of pixel (i,j), current channel
A0 value of pixel (i,j), first channel
A0 value of pixel (i,j), second channel
A2_(1,-1) value of third channel of pixel (i+1,j-1)

CONSTANTS

NX width of the image
NY heigth of the image
NC number of channels of the image
I  horizontal coordinate of the pixel in [0..NX[
J  vertical coordinate of the pixel in [0..NY[
K  channel number in [0..NC[

N  number of pixels
R2 square distance to the center of the image
T  angle from the center of the image




All variables in the surrounding code are prefixed with "__" and no

"__" is not allowed in the expression, to avoid side-effects. 

The loop is on x, y, then c

# EXAMPLES

Sum two images:

  cat a.png b.png | ilambda "A + B" > c.png

Add a gaussian to half of lena:

  ilambda "(A / 2) + 200 * exp(-40. * R2)" < lena.png > out.png

Forward differences to compute the derivative in horizontal direction:

  ilambda "A_(1,0) - A" < lena.png > out.png

Sobel edge detector:

  ilambda "hypot(2 * A_(1,0) + A_(1,1) + A_(1,-1) 
                 - 2 * A_(-1,0) + A_(-1,1) + A_(-1,-1),
                 2 * A_(0,1) + A_(1,1) + A_(-1,1)
                 - 2 * A_(0,-1) + A_(1,-1) + A_(-1,-1))" < lena.png > sobel.png

Set to 0 the green component of a RGB image:

  ilambda "(K == 1 ? 0 : A)" < lena.png > nogreen.png

Pick the blue channel of a RGB image:
  ilambda "A2"

Color to gray:

  ilambda "(A0 + A1 + A2) / 3"

Swap the blue an green channels of a RGB image:

  ( ilambda "(K == 0 ? A2 : A)" < lena.png; 
    ilambda "(K == 2 ? A0 : A)" < lena.png ) | ilambda "(K == 0 ? A : B)"
