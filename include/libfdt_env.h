#ifndef _LIBFDT_ENV_H
#define _LIBFDT_ENV_H

#include <stddef.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/string.h>

struct fdt_header *fdt;         /* Pointer to the working fdt */

#define fdt32_to_cpu(x)		__be32_to_cpu(x)
#define cpu_to_fdt32(x)		__cpu_to_be32(x)
#define fdt64_to_cpu(x)		__be64_to_cpu(x)
#define cpu_to_fdt64(x)		__cpu_to_be64(x)

#endif /* _LIBFDT_ENV_H */
