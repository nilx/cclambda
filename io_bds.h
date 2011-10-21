#ifndef _IO_BDS_H
#define _IO_BDS_H

#ifdef __cplusplus
extern "C" {
#endif

#define IO_BDS_VERSION "0.20111010"

#include <stddef.h>

/* io_bin.c */
char *io_bds_info(void);
float *io_bds_read_flt(const char *fname, size_t *nxp, size_t *nyp, size_t *ncp);
void io_bds_write_flt(const char *fname, const float *data, size_t nx, size_t ny, size_t nc);

#ifdef __cplusplus
}
#endif

#endif /* !_IO_BDS_H */
