#ifndef __EXPORTS_H__
#define __EXPORTS_H__

#ifndef __ASSEMBLY__

struct spi_slave;

/* These are declarations of exported functions available in C code */
unsigned long get_version(void);
int  getc(void);
int  tstc(void);
void putc(const char);
void puts(const char*);
int printf(const char* fmt, ...);
void install_hdlr(int, interrupt_handler_t, void*);
void free_hdlr(int);
void *malloc(size_t);
#ifndef CONFIG_SYS_MALLOC_SIMPLE
void free(void*);
#endif
void __udelay(unsigned long);
unsigned long get_timer(unsigned long);
int vprintf(const char *, va_list);
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);
int strict_strtoul(const char *cp, unsigned int base, unsigned long *res);
char *getenv (const char *name);
int setenv (const char *varname, const char *varvalue);
long simple_strtol(const char *cp, char **endp, unsigned int base);
int strcmp(const char *cs, const char *ct);
unsigned long ustrtoul(const char *cp, char **endp, unsigned int base);
unsigned long long ustrtoull(const char *cp, char **endp, unsigned int base);
#if defined(CONFIG_CMD_I2C) && \
		(!defined(CONFIG_DM_I2C) || defined(CONFIG_DM_I2C_COMPAT))
int i2c_write (uchar, uint, int , uchar* , int);
int i2c_read (uchar, uint, int , uchar* , int);
#endif

void app_startup(char * const *);

#endif    /* ifndef __ASSEMBLY__ */

struct jt_funcs {
#define EXPORT_FUNC(impl, res, func, ...) res(*func)(__VA_ARGS__);
#include <_exports.h>
#undef EXPORT_FUNC
};


#define XF_VERSION	7

#if defined(CONFIG_X86)
extern gd_t *global_data;
#endif

#endif	/* __EXPORTS_H__ */
