#ifndef _LIBFDT_ENV_H
#define _LIBFDT_ENV_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define fdt32_to_cpu(x)		(x)
#define cpu_to_fdt32(x)		(x)
#define fdt64_to_cpu(x)		(x)
#define cpu_to_fdt64(x)		(x)
#else
#define fdt32_to_cpu(x)		(bswap_32((x)))
#define cpu_to_fdt32(x)		(bswap_32((x)))
#define fdt64_to_cpu(x)		(bswap_64((x)))
#define cpu_to_fdt64(x)		(bswap_64((x)))
#endif

#endif /* _LIBFDT_ENV_H */
