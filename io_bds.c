/*
 * Copyright (c) 2010-2011, Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under, at your option, the terms of the GNU General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version, or
 * the terms of the simplified BSD license.
 *
 * You should have received a copy of these licenses along this
 * program. If not, see <http://www.gnu.org/licenses/> and
 * <http://www.opensource.org/licenses/bsd-license.html>.
 */

/**
 * @file io_bds.c
 * @brief raw memory read/write stream routines
 *
 * @author Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
 */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* unified Windows detection */
#if (defined(_WIN32) || defined(__WIN32__) \
     || defined(__TOS_WIN__) || defined(__WINDOWS__))
/* from http://predef.sourceforge.net/preos.html#sec25 */
#ifndef WIN32
#define WIN32
#endif
/*
 * On windows systems, the streams mist be reset to binary mode with
 * setmode() to avoid CRLF translation and other nasty side-effects.
 */
#include <io.h>
#include <fcntl.h>
#endif

/* ensure consistency */
#include "io_bds.h"

/*
 * INFO
 */

/** @brief string tag inserted into the binary */
static char _io_bds_tag[] = "using io_bds " IO_BDS_VERSION;
/** @brief helps tracking versions with the string tag */
char *io_bds_info(void)
{
    return _io_bds_tag;
}

#define _IO_BDS_FILE_WARNING \
    "Warning: binary data stream format is not safe for file storage.\n"

/*
 * UTILS
 */

/** @brief abort() wrapper macro with an error message */
#define _IO_BDS_ABORT(MSG) do {                                 \
    fprintf(stderr, "%s:%04u : %s\n", __FILE__, __LINE__, MSG); \
    fflush(NULL);                                               \
    abort();                                                    \
    } while (0);

/** @brief abort() wrapper macro plus the system error */
#define _IO_BDS_ABORT_ERR(MSG) do {                             \
    fprintf(stderr, "%s:%04u : %s\nsystem message : %s\n",      \
            __FILE__, __LINE__, MSG, strerror(errno));          \
    fflush(NULL);                                               \
    abort();                                                    \
    } while (0);

/** @brief safe malloc wrapper */
static void *_io_bds_safe_malloc(size_t size)
{
    void *memptr;

    if (NULL == (memptr = malloc(size)))
        _IO_BDS_ABORT("not enough memory");
    return memptr;
}

/** @brief safe malloc wrapper macro with safe casting */
#define _IO_BDS_SAFE_MALLOC(NB, TYPE)                                   \
    ((TYPE *) _io_bds_safe_malloc((size_t) (NB) * sizeof(TYPE)))

/* @todo: add a _IO_BDS_SAFE_FWRITE() macro */

/**
 * Compute the integer log base 10 of an integer,
 * ie the number of digits.
 *
 * Use a simple plain stupid test method, cf
 * http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10Obvious
 */
static size_t ndigits(size_t n)
{
    int digits;

    digits = -1;

#if (ULONG_MAX >= (1 << 32))
    /* (wrongly) suppose n <= 2^64; 2^64 has 20 digits */
    /* *INDENT-OFF* */
    digits = ((  n >= 10000000000000000000UL) ? 20
              : (n >= 1000000000000000000UL) ? 19
              : (n >= 100000000000000000UL) ? 18
              : (n >= 10000000000000000UL) ? 17
              : (n >= 1000000000000000UL) ? 16
              : (n >= 100000000000000UL) ? 15
              : (n >= 10000000000000UL) ? 14
              : (n >= 1000000000000UL) ? 13
              : (n >= 100000000000UL) ? 12
              : (n >= 10000000000UL) ? 11
              : -1);
    /* *INDENT-ON* */
#endif
    if (digits > 0)
        return (size_t) digits;

    /* (wrongly) suppose n <= 2^32; 2^32 has 20 digits */
    /* *INDENT-OFF* */
    digits = ((  n >= 1000000000UL) ? 10
              : (n >= 100000000UL) ? 9
              : (n >= 10000000UL) ? 8
              : (n >= 1000000UL) ? 7
              : (n >= 100000UL) ? 6
              : (n >= 10000UL) ? 5
              : (n >= 1000UL) ? 4
              : (n >= 100UL) ? 3
              : (n >= 10UL) ? 2
              : 1);
    /* *INDENT-ON* */
    return (size_t) digits;
}

/*
 * SIGNATURE AND HEADER
 */

/*
 * The binary data stream format is:
 * - a 12 bytes signature
 * - a 52 bytes header
 * - the binary data
 */

/** BDS signature length */
#define _IO_BDS_SIG_LEN 12
#define _IO_BDS_SIG_BASE_LEN 8
/**
 * @brief fixed BDS signature
 *
 * The BDS signature is made of 12 bytes: a 8 byte signature base
 * similar to the PNG signature and a 4 byte version info.
 * Whenever the ABI changes, the version info is incremented.
 */
static const char _io_bds_sig[_IO_BDS_SIG_LEN] = {
    '\211', 'B', 'D', 'S', '\r', '\n', '\032', '\n',
/*  ------  -------------  ----------  ------  ----
 *   |       |              |           |       |
 *   |       |              |           |      detect CRLF translations
 *   |       |              |          DOS end of file
 *   |       |             detect CRLF translations
 *   |      BDS marker (human readable ASCII)
 *  detect 8-bit mangling, non-text content
 *
 * Reuse the clever PNG signature base, ...
 * http://www.libpng.org/pub/png/spec/1.2/PNG-Rationale.html
 */
    '0', '0', '0', '2'
/*  ------------------
 *   |
 *  ABI version (human readable ASCII)
 */
};

/** read and check the BDS signature */
static void _io_bds_read_sig(FILE * fp)
{
    char sig[_IO_BDS_SIG_LEN];
    size_t i;

    /* read with getc() to accomodate previous ungetc() */
    for (i = 0; i < _IO_BDS_SIG_LEN; i++)
        if (EOF == (sig[i] = getc(fp)))
            _IO_BDS_ABORT_ERR("read error");

    /* check the signature */
    for (i = 0; i < _IO_BDS_SIG_BASE_LEN; i++)
        if (_io_bds_sig[i] != sig[i])
            _IO_BDS_ABORT("data is not a correctly formatted BDS");
    for (i = _IO_BDS_SIG_BASE_LEN; i < _IO_BDS_SIG_LEN; i++)
        if (_io_bds_sig[i] != sig[i])
            _IO_BDS_ABORT("wrong BDS ABI version in this data stream");
    return;
}

/** write the BDS signature */
static void _io_bds_write_sig(FILE * fp)
{
    if (_IO_BDS_SIG_LEN != fwrite(_io_bds_sig, sizeof(char),
                                  _IO_BDS_SIG_LEN, fp))
        _IO_BDS_ABORT_ERR("write error");
    return;
}

/** BDS header length */
#define _IO_BDS_HDR_LEN 52
/**
 * @brief build the BDS header
 *
 * The header is a 52 byte string with the data type info and 3
 * dimensions. The string must be correcly terminated by the null
 * character (in the 52 bytes) and its content readable with a
 * C scanf() function using the formatting string "%s%lu%lu%lu".
 *
 * Currently, the data type info can only be "flt" for floats. There
 * is no point using the full C type names since this string won't be
 * processed as a C code fragment and unsigned types would require two
 * words, so we use compact abbreviations. "flt" is the standard C
 * abbreviation for floats (see float.h macros).
 *
 * The header length is sufficient for 3x 2**32 dimensions; the
 * dimensions are checked and this function will abort() if the array
 * size can't fit into the header length.
 *
 * @param nx ny nc array dimensions
 * @return freshly allocated string pointer
 */
static char *_io_bds_hdr(size_t nx, size_t ny, size_t nc)
{
    char *hdr;
    int len;

    /* empty header fille with spaces */
    hdr = _IO_BDS_SAFE_MALLOC(_IO_BDS_HDR_LEN, char);
    memset(hdr, ' ', _IO_BDS_HDR_LEN);

    /* check length */
    if (1                       /* leading space */
        + strlen("flt") + 1     /* "flt" data type info + space */
        + ndigits(nx) + 1 + ndigits(ny) + 1 + ndigits(nc)
        + 1                     /* ending '\0' */
        > _IO_BDS_HDR_LEN)
        _IO_BDS_ABORT("array dimensions too large for the header format");

    /* inhibit splint, already checked for overflow */
    /*@-bufferoverflowhigh@ */
    len = sprintf(hdr, " %s %lu %lu %lu", "flt", nx, ny, nc);
    /*@=bufferoverflowhigh@ */

    /* terminate at the end of the string */
    hdr[len] = ' ';
    hdr[_IO_BDS_HDR_LEN - 1] = '\0';
    return hdr;
}

/** read and check the BDS header */
static void _io_bds_read_hdr(size_t * nxp, size_t * nyp, size_t * ncp,
                             FILE * fp)
{
    char hdr[_IO_BDS_HDR_LEN];
    char dtype[_IO_BDS_HDR_LEN];
    unsigned long nx, ny, nc;

    if (_IO_BDS_HDR_LEN != fread(&hdr, sizeof(char), _IO_BDS_HDR_LEN, fp))
        _IO_BDS_ABORT_ERR("read error");

    /* ensure correct termination */
    hdr[_IO_BDS_HDR_LEN - 1] = '\0';

    if (4 != sscanf(hdr, "%s%lu%lu%lu", dtype, &nx, &ny, &nc))
        _IO_BDS_ABORT("wrong header in this data stream");
    if (0 != strcmp("flt", dtype))
        _IO_BDS_ABORT("unsupported data type in this data stream");

    *nxp = (size_t) nx;
    *nyp = (size_t) ny;
    *ncp = (size_t) nc;

    return;
}

/** write the BDS header */
static void _io_bds_write_hdr(size_t nx, size_t ny, size_t nc, FILE * fp)
{
    char *hdr;

    hdr = _io_bds_hdr(nx, ny, nc);
    if (_IO_BDS_HDR_LEN != fwrite(hdr, sizeof(char), _IO_BDS_HDR_LEN, fp))
        _IO_BDS_ABORT_ERR("write error");
    free(hdr);
    return;
}

/*
 * A 7x5x3 float array stream starts with these 64 bytes:
 *
 * 00000  89 42 44 53 0d 0a 1a 0a  30 30 30 32 20 66 6c 74  |.BDS....0002 flt|
 * 00010  20 37 20 35 20 33 20 20  20 20 20 20 20 20 20 20  | 7 5 3          |
 * 00020  20 20 20 20 20 20 20 20  20 20 20 20 20 20 20 20  |                |
 * 00030  20 20 20 20 20 20 20 20  20 20 20 20 20 20 20 00  |               .|
 *
 * Note that the header part is flexible and only contrained by scanf()
 * readability. The following variants with more or less spaces are
 * valid too:
 *
 * |.BDS....0002 flt 7 5 3                                         .|
 * |.BDS....0002 flt 7 5 3..........................................|
 * |.BDS....0002flt 7 5 3                                          .|
 * |.BDS....0002    flt    7    5    3                             .|
 */

/*
 * READ
 */

/**
 * @brief read binary data stream into a float array
 *
 * The array size (3 x size_t) is read first, then the array data.
 *
 * @param fname file name, "-" for stdin
 * @param nxp, nyp, ncp pointers to variables to be filled with the number of
 *        columns, lines and channels of the image
 * @return pointer to an array of pixels, abort() on error
 */
float *io_bds_read_flt(const char *fname,
                       size_t * nxp, size_t * nyp, size_t * ncp)
{
    float *data;
    FILE *fp;
    size_t nx, ny, nc;
    size_t size;

    if (0 == strcmp(fname, "-")) {
        fp = stdin;
#ifdef WIN32                    /* set the stream to binary mode */
        fflush(fp);
        setmode(fileno(fp), O_BINARY);
#endif
    }
    else {
        if (NULL == (fp = fopen(fname, "rb")))
            _IO_BDS_ABORT_ERR("failed to open file");
        fputs(_IO_BDS_FILE_WARNING, stderr);
    }

    /* read and check the signature and header */
    _io_bds_read_sig(fp);
    _io_bds_read_hdr(&nx, &ny, &nc, fp);

    /* read the data */
    size = nx * ny * nc;
    data = _IO_BDS_SAFE_MALLOC(size, float);
    if (size != fread(data, sizeof(float), size, fp))
        _IO_BDS_ABORT_ERR("read error");

    if (stdin != fp)
        (void) fclose(fp);
    *nxp = nx;
    *nyp = ny;
    *ncp = nc;
    return data;
}

/*
 * WRITE
 */

/**
 * @brief write a float array into a binary data stream
 *
 * @param fname file name, "-" for stdout
 * @param data array to write
 * @param nx, ny, nc dimensions of the array
 * @return void, abort() on error
 */
void io_bds_write_flt(const char *fname, const float *data,
                      size_t nx, size_t ny, size_t nc)
{
    FILE *fp;
    size_t size;

    if (0 == strcmp(fname, "-")) {
        fp = stdout;
#ifdef WIN32                    /* set the stream to binary mode */
        fflush(fp);
        setmode(fileno(fp), O_BINARY);
#endif
    }
    else {
        if (NULL == (fp = fopen(fname, "wb")))
            _IO_BDS_ABORT_ERR("failed to open file");
        fputs(_IO_BDS_FILE_WARNING, stderr);
    }
    /* write the signature and header */
    _io_bds_write_sig(fp);
    _io_bds_write_hdr(nx, ny, nc, fp);

    /* write the data */
    size = nx * ny * nc;
    if (size != fwrite(data, sizeof(float), size, fp))
        _IO_BDS_ABORT_ERR("write error");

    if (stdout != fp)
        (void) fclose(fp);

    return;
}
