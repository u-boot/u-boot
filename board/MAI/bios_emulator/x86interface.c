#include "x86emu.h"
#include "glue.h"


/*
 * This isn't nice, but there are a lot of incompatibilities in the U-Boot and scitech include
 * files that this is the only really workable solution.
 * Might be cleaned out later.
 */

#ifdef DEBUG
#undef DEBUG
#endif

#undef IO_LOGGING
#undef MEM_LOGGING

#ifdef IO_LOGGING
#define LOGIO(port, format, args...) if (dolog(port)) _printf(format , ## args)
#else
#define LOGIO(port, format, args...)
#endif

#ifdef MEM_LOGGIN
#define LOGMEM(format, args...) _printf(format , ## args)
#else
#define LOGMEM(format, args...)
#endif

#ifdef DEBUG
#define PRINTF(format, args...) _printf(format , ## args)
#else
#define PRINTF(format, argc...)
#endif

typedef unsigned char UBYTE;
typedef unsigned short UWORD;
typedef unsigned long ULONG;

typedef char BYTE;
typedef short WORT;
typedef long LONG;

#define EMULATOR_MEM_SIZE       (1024*1024)
#define EMULATOR_BIOS_OFFSET    0xC0000
#define EMULATOR_STRAP_OFFSET   0x30000
#define EMULATOR_STACK_OFFSET   0x20000
#define EMULATOR_LOGO_OFFSET    0x40000 /* If you change this, change the strap code, too */
#define VIDEO_BASE (void *)0xFD0B8000

extern char *getenv(char *);
extern int tstc(void);
extern int getc(void);
extern unsigned char video_get_attr(void);

int atoi(char *string)
{
    int res = 0;
    while (*string>='0' && *string <='9')
    {
	res *= 10;
	res += *string-'0';
	string++;
    }

    return res;
}

void cons_gets(char *buffer)
{
    int i = 0;
    char c = 0;

    buffer[0] = 0;
    if (getenv("x86_runthru")) return; /*FIXME: */
    while (c != 0x0D && c != 0x0A)
    {
	while (!tstc());
	c = getc();
	if (c>=32 && c < 127)
	{
	    buffer[i] = c;
	    i++;
	    buffer[i] = 0;
	    putc(c);
	}
	else
	{
	    if (c == 0x08)
	    {
		if (i>0) i--;
		buffer[i] = 0;
	    }
	}
    }
    buffer[i] = '\n';
    buffer[i+1] = 0;
}

char *bios_date = "08/14/02";
UBYTE model = 0xFC;
UBYTE submodel = 0x00;

static inline UBYTE read_byte(volatile UBYTE* from)
{
    int x;
    asm volatile ("lbz %0,%1\n eieio" : "=r" (x) : "m" (*from));
    return (UBYTE)x;
}

static inline void write_byte(volatile UBYTE *to, int x)
{
    asm volatile ("stb %1,%0\n eieio" : "=m" (*to) : "r" (x));
}

static inline UWORD read_word_little(volatile UWORD *from)
{
    int x;
    asm volatile ("lhbrx %0,0,%1\n eieio" : "=r" (x) : "r" (from), "m" (*from));
    return (UWORD)x;
}

static inline UWORD read_word_big(volatile UWORD *from)
{
    int x;
    asm volatile ("lhz %0,%1\n eieio" : "=r" (x) : "m" (*from));
    return (UWORD)x;
}

static inline void write_word_little(volatile UWORD *to, int x)
{
    asm volatile ("sthbrx %1,0,%2\n eieio" : "=m" (*to) : "r" (x), "r" (to));
}

static inline void write_word_big(volatile UWORD *to, int x)
{
    asm volatile ("sth %1,%0\n eieio" : "=m" (*to) : "r" (x));
}

static inline ULONG read_long_little(volatile ULONG *from)
{
    unsigned long x;
    asm volatile ("lwbrx %0,0,%1\n eieio" : "=r" (x) : "r" (from), "m"(*from));
    return (ULONG)x;
}

static inline ULONG read_long_big(volatile ULONG *from)
{
    unsigned long x;
    asm volatile ("lwz %0,%1\n eieio" : "=r" (x) : "m" (*from));
    return (ULONG)x;
}

static inline void write_long_little(volatile ULONG *to, ULONG x)
{
    asm volatile ("stwbrx %1,0,%2\n eieio" : "=m" (*to) : "r" (x), "r" (to));
}

static inline void write_long_big(volatile ULONG *to, ULONG x)
{
    asm volatile ("stw %1,%0\n eieio" : "=m" (*to) : "r" (x));
}

static int log_init = 0;
static int log_do = 0;
static int log_low = 0;

int dolog(int port)
{
    if (log_init && log_do)
    {
	if (log_low && port > 0x400) return 0;
	return 1;
    }

    if (!log_init)
    {
	log_init = 1;
	log_do = (getenv("x86_logio") != (char *)0);
	log_low = (getenv("x86_loglow") != (char *)0);
	if (log_do)
	{
	    if (log_low && port > 0x400) return 0;
	    return 1;
	}
    }
    return 0;
}

/* Converts an emulator address to a physical address. */
/* Handles all special cases (bios date, model etc), and might need work */
u32 memaddr(u32 addr)
{
/*    if (addr >= 0xF0000 && addr < 0xFFFFF) printf("WARNING: Segment F access (0x%x)\n", addr); */
/*    printf("MemAddr=%p\n", addr); */
    if (addr >= 0xA0000 && addr < 0xC0000)
	return 0xFD000000 + addr;
    else if (addr >= 0xFFFF5 && addr < 0xFFFFE)
    {
	return (u32)bios_date+addr-0xFFFF5;
    }
    else if (addr == 0xFFFFE)
	return (u32)&model;
    else if (addr == 0xFFFFF)
	return (u32)&submodel;
    else if (addr >= 0x80000000)
    {
	/*printf("Warning: High memory access at 0x%x\n", addr); */
	return addr;
    }
    else
	return (u32)M.mem_base+addr;
}

u8 A1_rdb(u32 addr)
{
    u8 a = read_byte((UBYTE *)memaddr(addr));
    LOGMEM("rdb: %x -> %x\n", addr, a);
    return a;
}

u16 A1_rdw(u32 addr)
{
    u16 a = read_word_little((UWORD *)memaddr(addr));
    LOGMEM("rdw: %x -> %x\n", addr, a);
    return a;
}

u32 A1_rdl(u32 addr)
{
    u32 a = read_long_little((ULONG *)memaddr(addr));
    LOGMEM("rdl: %x -> %x\n", addr, a);
    return a;
}

void A1_wrb(u32 addr, u8 val)
{
    LOGMEM("wrb: %x <- %x\n", addr, val);
    write_byte((UBYTE *)memaddr(addr), val);
}

void A1_wrw(u32 addr, u16 val)
{
    LOGMEM("wrw: %x <- %x\n", addr, val);
    write_word_little((UWORD *)memaddr(addr), val);
}

void A1_wrl(u32 addr, u32 val)
{
    LOGMEM("wrl: %x <- %x\n", addr, val);
    write_long_little((ULONG *)memaddr(addr), val);
}

X86EMU_memFuncs _A1_mem =
{
    A1_rdb,
    A1_rdw,
    A1_rdl,
    A1_wrb,
    A1_wrw,
    A1_wrl,
};

#define ARTICIAS_PCI_CFGADDR  0xfec00cf8
#define ARTICIAS_PCI_CFGDATA  0xfee00cfc
#define IOBASE                0xFE000000

#define in_byte(from) read_byte( (UBYTE *)port_to_mem(from))
#define in_word(from) read_word_little((UWORD *)port_to_mem(from))
#define in_long(from) read_long_little((ULONG *)port_to_mem(from))
#define out_byte(to, val) write_byte((UBYTE *)port_to_mem(to), val)
#define out_word(to, val) write_word_little((UWORD *)port_to_mem(to), val)
#define out_long(to, val) write_long_little((ULONG *)port_to_mem(to), val)

u32 port_to_mem(int port)
{
    if (port >= 0xCFC && port <= 0xCFF) return 0xFEE00000+port;
    else if (port >= 0xCF8 && port <= 0xCFB) return 0xFEC00000+port;
    else return IOBASE + port;
}

u8 A1_inb(int port)
{
    u8 a;
    /*if (port == 0x3BA) return 0; */
    a = in_byte(port);
    LOGIO(port, "inb: %Xh -> %d (%Xh)\n", port, a, a);
    return a;
}

u16 A1_inw(int port)
{
    u16 a = in_word(port);
    LOGIO(port, "inw: %Xh -> %d (%Xh)\n", port, a, a);
    return a;
}

u32 A1_inl(int port)
{
    u32 a = in_long(port);
    LOGIO(port, "inl: %Xh -> %d (%Xh)\n", port, a, a);
    return a;
}

void A1_outb(int port, u8 val)
{
    LOGIO(port, "outb: %Xh <- %d (%Xh)\n", port, val, val);
/*    if (port == 0xCF8) port = 0xCFB;
    else if (port == 0xCF9) port = 0xCFA;
    else if (port == 0xCFA) port = 0xCF9;
    else if (port == 0xCFB) port = 0xCF8;*/
    out_byte(port, val);
}

void A1_outw(int port, u16 val)
{
    LOGIO(port, "outw: %Xh <- %d (%Xh)\n", port, val, val);
    out_word(port, val);
}

void A1_outl(int port, u32 val)
{
    LOGIO(port, "outl: %Xh <- %d (%Xh)\n", port, val, val);
    out_long(port, val);
}

X86EMU_pioFuncs _A1_pio =
{
    A1_inb,
    A1_inw,
    A1_inl,
    A1_outb,
    A1_outw,
    A1_outl,
};

static int reloced_ops = 0;

void reloc_ops(void *reloc_addr)
{
    extern void (*x86emu_optab[256])(u8);
    extern void (*x86emu_optab2[256])(u8);
    extern void tables_relocate(unsigned int offset);
    int i;
    unsigned long delta;
    if (reloced_ops == 1) return;
    reloced_ops = 1;

    delta = TEXT_BASE - (unsigned long)reloc_addr;

    for (i=0; i<256; i++)
    {
	x86emu_optab[i] -= delta;
	x86emu_optab2[i] -= delta;
    }

    _A1_mem.rdb = A1_rdb;
    _A1_mem.rdw = A1_rdw;
    _A1_mem.rdl = A1_rdl;
    _A1_mem.wrb = A1_wrb;
    _A1_mem.wrw = A1_wrw;
    _A1_mem.wrl = A1_wrl;

    _A1_pio.inb = A1_inb;
    _A1_pio.inw = A1_inw;
    _A1_pio.inl = A1_inl;
    _A1_pio.outb = A1_outb;
    _A1_pio.outw = A1_outw;
    _A1_pio.outl = A1_outl;

    tables_relocate(delta);

}


#define ANY_KEY(text)				\
    printf(text);				\
    while (!tstc());


unsigned char more_strap[] = {
	0xb4, 0x0, 0xb0, 0x2, 0xcd, 0x10,
};
#define MORE_STRAP_BYTES 6 /* Additional bytes of strap code */


unsigned char *done_msg="VGA Initialized\0";

int execute_bios(pci_dev_t gr_dev, void *reloc_addr)
{
    extern void bios_init(void);
    extern void remove_init_data(void);
    extern int video_rows(void);
    extern int video_cols(void);
    extern int video_size(int, int);
    u8 *strap;
    unsigned char *logo;
    u8 cfg;
    int i;
    char c;
    char *s;
#ifdef EASTEREGG
    int easteregg_active = 0;
#endif
    char *pal_reset;
    u8 *fb;
    unsigned char *msg;
    unsigned char current_attr;

    PRINTF("Trying to remove init data\n");
    remove_init_data();
    PRINTF("Removed init data from cache, now in RAM\n");

    reloc_ops(reloc_addr);
    PRINTF("Attempting to run emulator on %02x:%02x:%02x\n",
	   PCI_BUS(gr_dev), PCI_DEV(gr_dev), PCI_FUNC(gr_dev));

    /* Enable compatibility hole for emulator access to frame buffer */
    PRINTF("Enabling compatibility hole\n");
    enable_compatibility_hole();

    /* Allocate memory */
    /* FIXME: We shouldn't use this much memory really. */
    memset(&M, 0, sizeof(X86EMU_sysEnv));
    M.mem_base = malloc(EMULATOR_MEM_SIZE);
    M.mem_size = EMULATOR_MEM_SIZE;

    if (!M.mem_base)
    {
	PRINTF("Unable to allocate one megabyte for emulator\n");
	return 0;
    }

    if (attempt_map_rom(gr_dev, M.mem_base + EMULATOR_BIOS_OFFSET) == 0)
    {
	PRINTF("Error mapping rom. Emulation terminated\n");
	return 0;
    }

#if 1 /*def DEBUG*/
    s = getenv("x86_ask_start");
    if (s)
    {
	printf("Press 'q' to skip initialization, 'd' for dry init\n'i' for i/o session");
	while (!tstc());
	c = getc();
	if (c == 'q') return 0;
	if (c == 'd')
	{
	    extern void bios_set_mode(int mode);
	    bios_set_mode(0x03);
	    return 0;
	}
	if (c == 'i') do_inout();
    }


#endif

#ifdef EASTEREGG
/*    if (tstc())
    {
	if (getc() == 'c')
	{
	    easteregg_active = 1;
	}
    }
*/
    if (getenv("easteregg"))
    {
	easteregg_active = 1;
    }

    if (easteregg_active)
    {
	/* Yay! */
	setenv("x86_mode", "1");
	setenv("vga_fg_color", "11");
	setenv("vga_bg_color", "1");
	easteregg_active = 1;
    }
#endif

    strap = (u8*)M.mem_base + EMULATOR_STRAP_OFFSET;

    {
	char *m = getenv("x86_mode");
	if (m)
	{
	    more_strap[3] = atoi(m);
	    if (more_strap[3] == 1) video_size(40, 25);
	    else                    video_size(80, 25);
	}
    }

    /*
     * Poke the strap routine. This might need a bit of extending
     * if there is a mode switch involved, i.e. we want to int10
     * afterwards to set a different graphics mode, or alternatively
     * there might be a different start address requirement if the
     * ROM doesn't have an x86 image in its first image.
     */

    PRINTF("Poking strap...\n");

    /* FAR CALL c000:0003 */
    *strap++ = 0x9A; *strap++ = 0x03; *strap++ = 0x00;
    *strap++ = 0x00; *strap++ = 0xC0;

#if 1
    /* insert additional strap code */
    for (i=0; i < MORE_STRAP_BYTES; i++)
    {
	*strap++ = more_strap[i];
    }
#endif
    /* HALT */
    *strap++ = 0xF4;

    PRINTF("Setting up logo data\n");
    logo = (unsigned char *)M.mem_base + EMULATOR_LOGO_OFFSET;
    for (i=0; i<16; i++)
    {
	*logo++ = 0xFF;
    }

    /*
     * Setup the init parameters.
     * Per PCI specs, AH must contain the bus and AL
     * must contain the devfn, encoded as (dev<<3)|fn
     */

    /* Execution starts here */
    M.x86.R_CS = SEG(EMULATOR_STRAP_OFFSET);
    M.x86.R_IP = OFF(EMULATOR_STRAP_OFFSET);

    /* Stack at top of ram */
    M.x86.R_SS = SEG(EMULATOR_STACK_OFFSET);
    M.x86.R_SP = OFF(EMULATOR_STACK_OFFSET);

    /* Input parameters */
    M.x86.R_AH = PCI_BUS(gr_dev);
    M.x86.R_AL = (PCI_DEV(gr_dev)<<3) | PCI_FUNC(gr_dev);

    /* Set the I/O and memory access functions */
    X86EMU_setupMemFuncs(&_A1_mem);
    X86EMU_setupPioFuncs(&_A1_pio);

    /* Enable timer 2 */
    cfg = in_byte(0x61); /* Get Misc control */
    cfg |= 0x01;         /* Enable timer 2 */
    out_byte(0x61, cfg); /* output again */

    /* Set up the timers */
    out_byte(0x43, 0x54);
    out_byte(0x41, 0x18);

    out_byte(0x43, 0x36);
    out_byte(0x40, 0x00);
    out_byte(0x40, 0x00);

    out_byte(0x43, 0xb6);
    out_byte(0x42, 0x31);
    out_byte(0x42, 0x13);

    /* Init the "BIOS". */
    bios_init();

    /* Video Card Reset */
    out_byte(0x3D8, 0);
    out_byte(0x3B8, 1);
    (void)in_byte(0x3BA);
    (void)in_byte(0x3DA);
    out_byte(0x3C0, 0);
    out_byte(0x61, 0xFC);

#ifdef DEBUG
    s = _getenv("x86_singlestep");
    if (s && strcmp(s, "on")==0)
    {
	PRINTF("Enabling single stepping for debug\n");
	X86EMU_trace_on();
    }
#endif

    /* Ready set go... */
    PRINTF("Running emulator\n");
    X86EMU_exec();
    PRINTF("Done running emulator\n");

/* FIXME: Remove me */
    pal_reset = getenv("x86_palette_reset");
    if (pal_reset && strcmp(pal_reset, "on") == 0)
    {
	PRINTF("Palette reset\n");
	/*(void)in_byte(0x3da); */
	/*out_byte(0x3c0, 0); */

	out_byte(0x3C8, 0);
	out_byte(0x3C9, 0);
	out_byte(0x3C9, 0);
	out_byte(0x3C9, 0);
	for (i=0; i<254; i++)
	{
	    out_byte(0x3C9, 63);
	    out_byte(0x3C9, 63);
	    out_byte(0x3C9, 63);
	}

	out_byte(0x3c0, 0x20);
    }
/* FIXME: remove me */
#ifdef EASTEREGG
    if (easteregg_active)
    {
	extern void video_easteregg(void);
	video_easteregg();
    }
#endif
/*
    current_attr = video_get_attr();
    fb = (u8 *)VIDEO_BASE;
    for (i=0; i<video_rows()*video_cols()*2; i+=2)
    {
	*(fb+i) = ' ';
	*(fb+i+1) = current_attr;
    }

    fb = (u8 *)VIDEO_BASE + (video_rows())-1*(video_cols()*2);
    for (i=0; i<video_cols(); i++)
    {
	*(fb + 2*i)     = 32;
	*(fb + 2*i + 1) = 0x17;
    }

    msg = done_msg;
    while (*msg)
    {
	*fb = *msg;
	fb  += 2;
	msg ++;
    }
*/
#ifdef DEBUG
    if (getenv("x86_do_inout")) do_inout();
#endif

/*FIXME:    dcache_disable(); */
    return 1;
}

/* Clean up the x86 mess */
void shutdown_bios(void)
{
/*    disable_compatibility_hole(); */
    /* Free the memory associated */
    free(M.mem_base);

}

int to_int(char *buffer)
{
    int base = 0;
    int res  = 0;

    if (*buffer == '$')
    {
	base = 16;
	buffer++;
    }
    else base = 10;

    for (;;)
    {
	switch(*buffer)
	{
	case '0' ... '9':
	    res *= base;
	    res += *buffer - '0';
	    break;
	case 'A':
	case 'a':
	    res *= base;
	    res += 10;
	    break;
	case 'B':
	case 'b':
	    res *= base;
	    res += 11;
	    break;
	case 'C':
	case 'c':
	    res *= base;
	    res += 12;
	    break;
	case 'D':
	case 'd':
	    res *= base;
	    res += 13;
	    break;
	case 'E':
	case 'e':
	    res *= base;
	    res += 14;
	    break;
	case 'F':
	case 'f':
	    res *= base;
	    res += 15;
	    break;
	default:
	    return res;
	}
	buffer++;
    }
    return res;
}

void one_arg(char *buffer, int *a)
{
    while (*buffer && *buffer != '\n')
    {
	if (*buffer == ' ') buffer++;
	else break;
    }

    *a = to_int(buffer);
}

void two_args(char *buffer, int *a, int *b)
{
    while (*buffer && *buffer != '\n')
    {
	if (*buffer == ' ') buffer++;
	else break;
    }

    *a = to_int(buffer);

    while (*buffer && *buffer != '\n')
    {
	if (*buffer != ' ') buffer++;
	else break;
    }

    while (*buffer && *buffer != '\n')
    {
	if (*buffer == ' ') buffer++;
	else break;
    }

    *b = to_int(buffer);
}

void do_inout(void)
{
    char buffer[256];
    char *arg1, *arg2;
    int a,b;

    printf("In/Out Session\nUse 'i[bwl]' for in, 'o[bwl]' for out and 'q' to quit\n");

    do
    {
	cons_gets(buffer);
	printf("\n");

	*arg1 = buffer;
	while (*arg1 != ' ' ) arg1++;
	while (*arg1 == ' ') arg1++;

	if (buffer[0] == 'i')
	{
	    one_arg(buffer+2, &a);
	    switch (buffer[1])
	    {
	    case 'b':
		printf("in_byte(%xh) = %xh\n", a, A1_inb(a));
		break;
	    case 'w':
		printf("in_word(%xh) = %xh\n", a, A1_inw(a));
		break;
	    case 'l':
		printf("in_dword(%xh) = %xh\n", a, A1_inl(a));
		break;
	    default:
		printf("Invalid length '%c'\n", buffer[1]);
		break;
	    }
	}
	else if (buffer[0] == 'o')
	{
	    two_args(buffer+2, &a, &b);
	    switch (buffer[1])
	    {
	    case 'b':
		printf("out_byte(%d, %d)\n", a, b);
		A1_outb(a,b);
		break;
	    case 'w':
		printf("out_word(%d, %d)\n", a, b);
		A1_outw(a, b);
		break;
	    case 'l':
		printf("out_long(%d, %d)\n", a, b);
		A1_outl(a, b);
		break;
	    default:
		printf("Invalid length '%c'\n", buffer[1]);
		break;
	    }
	} else if (buffer[0] == 'q') return;
    } while (1);
}
