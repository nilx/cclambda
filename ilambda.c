/*
 * Copyright 2011 Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file ilambda.c
 * @brief lambda filter on images
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 */




#include <stdlib.h>
#include <stdio.h>

#include "io_png.h"

/**
 * @brief main function call
 */
int main(int argc, char *const *argv)
{
    float *img;
    size_t nx, ny, nc;

    /*
     * the lambda expression is given in the command-line
     * inputs come from stdin, output goes on stdout
     */
    if (1 > argc) {
        fprintf(stderr, "syntax: %s 'expression'\n", argv[0]);
        fprintf(stderr, "usage : cat img1.png img2.png ... | "
		"%s 'expression' > out.png\n", argv[0]);
	return EXIT_FAILURE;
    }

    expr = argv[1];
    img_in = io_png_read_flt("-", &nx, &ny, &nc);

    return EXIT_SUCCESS;
}
