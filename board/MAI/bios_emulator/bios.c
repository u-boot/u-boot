/*
 * Mostly done after the Scitech Bios emulation
 * Written by Hans-Jörg Frieden
 * Hyperion Entertainment
 */
#include "x86emu.h"
#include "glue.h"

#undef DEBUG
#ifdef DEBUG
#define PRINTF(fmt, args...) printf(fmt, ## args)
#else
#define PRINTF(fmt, args...)
#endif

#define BIOS_SEG 0xFFF0
#define PCIBIOS_SUCCESSFUL 0
#define PCIBIOS_DEVICE_NOT_FOUND 0x86

typedef unsigned char UBYTE;
typedef unsigned short UWORD;
typedef unsigned long ULONG;

typedef char BYTE;
typedef short WORT;
typedef long LONG;

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

#define port_to_mem(from) (0xFE000000|(from))
#define in_byte(from) read_byte( (UBYTE *)port_to_mem(from))
#define in_word(from) read_word_little((UWORD *)port_to_mem(from))
#define in_long(from) read_long_little((ULONG *)port_to_mem(from))
#define out_byte(to, val) write_byte((UBYTE *)port_to_mem(to), val)
#define out_word(to, val) write_word_little((UWORD *)port_to_mem(to), val)
#define out_long(to, val) write_long_little((ULONG *)port_to_mem(to), val)

static void X86API undefined_intr(int intno)
{
    extern u16 A1_rdw(u32 addr);
    if (A1_rdw(intno * 4 + 2) == BIOS_SEG)
    {
	PRINTF("Undefined interrupt %xh called AX = %xh, BX = %xh, CX = %xh, DX = %xh\n",
	   intno, M.x86.R_AX, M.x86.R_BX, M.x86.R_CX, M.x86.R_DX);
	X86EMU_halt_sys();
    }
    else
    {
	PRINTF("Calling interrupt %xh, AL=%xh, AH=%xh\n", intno, M.x86.R_AL, M.x86.R_AH);
	X86EMU_prepareForInt(intno);
    }
}

static void X86API int42(int intno);
static void X86API int15(int intno);

static void X86API int10(int intno)
{
    if (A1_rdw(intno*4+2) == BIOS_SEG)
	int42(intno);
    else
    {
	PRINTF("int10: branching to %04X:%04X, AL=%xh, AH=%xh\n", A1_rdw(intno*4+2), A1_rdw(intno*4),
	       M.x86.R_AL, M.x86.R_AH);
	X86EMU_prepareForInt(intno);
    }
}

static void X86API int1A(int intno)
{
    int device;

    switch(M.x86.R_AX)
    {
    case 0xB101: /* PCI Bios Present? */
	M.x86.R_AL  = 0x00;
	M.x86.R_EDX = 0x20494350;
	M.x86.R_BX  = 0x0210;
	M.x86.R_CL  = 3;
	CLEAR_FLAG(F_CF);
	break;
    case 0xB102: /* Find device */
	device = mypci_find_device(M.x86.R_DX, M.x86.R_CX, M.x86.R_SI);
	if (device != -1)
	{
	    M.x86.R_AH = PCIBIOS_SUCCESSFUL;
	    M.x86.R_BH = mypci_bus(device);
	    M.x86.R_BL = mypci_devfn(device);
	}
	else
	{
	    M.x86.R_AH = PCIBIOS_DEVICE_NOT_FOUND;
	}
	CONDITIONAL_SET_FLAG((M.x86.R_AH != PCIBIOS_SUCCESSFUL), F_CF);
	break;
    case 0xB103: /* Find PCI class code */
	M.x86.R_AH = PCIBIOS_DEVICE_NOT_FOUND;
	/*printf("Find by class not yet implmented"); */
	CONDITIONAL_SET_FLAG((M.x86.R_AH != PCIBIOS_SUCCESSFUL), F_CF);
	break;
    case 0xB108: /* read config byte */
	M.x86.R_CL = mypci_read_cfg_byte(M.x86.R_BH, M.x86.R_BL, M.x86.R_DI);
	M.x86.R_AH = PCIBIOS_SUCCESSFUL;
	CONDITIONAL_SET_FLAG((M.x86.R_AH != PCIBIOS_SUCCESSFUL), F_CF);
	/*printf("read_config_byte %x,%x,%x -> %x\n", M.x86.R_BH, M.x86.R_BL, M.x86.R_DI, */
	/*	    M.x86.R_CL); */
	break;
    case 0xB109: /* read config word */
	M.x86.R_CX = mypci_read_cfg_word(M.x86.R_BH, M.x86.R_BL, M.x86.R_DI);
	M.x86.R_AH = PCIBIOS_SUCCESSFUL;
	CONDITIONAL_SET_FLAG((M.x86.R_AH != PCIBIOS_SUCCESSFUL), F_CF);
	/*printf("read_config_word %x,%x,%x -> %x\n", M.x86.R_BH, M.x86.R_BL, M.x86.R_DI, */
	/*	    M.x86.R_CX); */
	break;
    case 0xB10A: /* read config dword */
	M.x86.R_ECX = mypci_read_cfg_long(M.x86.R_BH, M.x86.R_BL, M.x86.R_DI);
	M.x86.R_AH = PCIBIOS_SUCCESSFUL;
	CONDITIONAL_SET_FLAG((M.x86.R_AH != PCIBIOS_SUCCESSFUL), F_CF);
	/*printf("read_config_long %x,%x,%x -> %x\n", M.x86.R_BH, M.x86.R_BL, M.x86.R_DI, */
	/*    M.x86.R_ECX); */
	break;
    case 0xB10B: /* write config byte */
	mypci_write_cfg_byte(M.x86.R_BH, M.x86.R_BL, M.x86.R_DI, M.x86.R_CL);
	M.x86.R_AH = PCIBIOS_SUCCESSFUL;
	CONDITIONAL_SET_FLAG((M.x86.R_AH != PCIBIOS_SUCCESSFUL), F_CF);
	/*printf("write_config_byte %x,%x,%x <- %x\n", M.x86.R_BH, M.x86.R_BL, M.x86.R_DI, */
	/*    M.x86.R_CL); */
	break;
    case 0xB10C: /* write config word */
	mypci_write_cfg_word(M.x86.R_BH, M.x86.R_BL, M.x86.R_DI, M.x86.R_CX);
	M.x86.R_AH = PCIBIOS_SUCCESSFUL;
	CONDITIONAL_SET_FLAG((M.x86.R_AH != PCIBIOS_SUCCESSFUL), F_CF);
	/*printf("write_config_word %x,%x,%x <- %x\n", M.x86.R_BH, M.x86.R_BL, M.x86.R_DI, */
	/*	    M.x86.R_CX); */
	break;
    case 0xB10D: /* write config dword */
	mypci_write_cfg_long(M.x86.R_BH, M.x86.R_BL, M.x86.R_DI, M.x86.R_ECX);
	M.x86.R_AH = PCIBIOS_SUCCESSFUL;
	CONDITIONAL_SET_FLAG((M.x86.R_AH != PCIBIOS_SUCCESSFUL), F_CF);
	/*printf("write_config_long %x,%x,%x <- %x\n", M.x86.R_BH, M.x86.R_BL, M.x86.R_DI, */
	/*	    M.x86.R_ECX); */
	break;
    default:
	PRINTF("BIOS int %xh: Unknown function AX=%04xh\n", intno, M.x86.R_AX);

    }
}

void bios_init(void)
{
    int i;
    X86EMU_intrFuncs bios_intr_tab[256];

    for (i=0; i<256; i++)
    {
	write_long_little(M.mem_base+i*4, BIOS_SEG<<16);
	bios_intr_tab[i] = undefined_intr;
    }

    bios_intr_tab[0x10] = int10;
    bios_intr_tab[0x1A] = int1A;
    bios_intr_tab[0x42] = int42;
    bios_intr_tab[0x15] = int15;

    bios_intr_tab[0x6D] = int42;

    X86EMU_setupIntrFuncs(bios_intr_tab);
    video_init();
}

unsigned char setup_40x25[] =
{
    0x38, 0x28, 0x2d, 0x0a, 0x1f, 6, 0x19,
    0x1c, 2, 7, 6, 7, 0, 0, 0, 0
};

unsigned char setup_80x25[] =
{
    0x71, 0x50, 0x5a, 0x0a, 0x1f, 6, 0x19,
    0x1c, 2, 7, 6, 7, 0, 0, 0, 0
};

unsigned char setup_graphics[] =
{
    0x38, 0x28, 0x20, 0x0a, 0x7f, 6, 0x64,
    0x70, 2, 1, 6, 7, 0, 0, 0, 0
};

unsigned char setup_bw[] =
{
    0x61, 0x50, 0x52, 0x0f, 0x19, 6, 0x19,
    0x19, 2, 0x0d, 0x0b, 0x0c, 0, 0, 0, 0
};

unsigned char * setup_modes[] =
{
    setup_40x25,     /* mode 0: 40x25 bw text */
    setup_40x25,     /* mode 1: 40x25 col text */
    setup_80x25,     /* mode 2: 80x25 bw text */
    setup_80x25,     /* mode 3: 80x25 col text */
    setup_graphics,  /* mode 4: 320x200 col graphics */
    setup_graphics,  /* mode 5: 320x200 bw graphics */
    setup_graphics,  /* mode 6: 640x200 bw graphics */
    setup_bw         /* mode 7: 80x25 mono text */
};

unsigned int setup_cols[] =
{
    40, 40, 80, 80, 40, 40, 80, 80
};

unsigned char setup_modesets[] =
{
     0x2C, 0x28, 0x2D, 0x29, 0x2A, 0x2E, 0x1E, 0x29
};

unsigned int setup_bufsize[] =
{
    2048, 2048, 4096, 2096, 16384, 16384, 16384, 4096
};

void bios_set_mode(int mode)
{
    int i;
    unsigned char mode_set = setup_modesets[mode]; /* Control register value */
    unsigned char *setup_regs = setup_modes[mode]; /* Register 3D4 Array */

    /* Switch video off */
    out_byte(0x3D8, mode_set & 0x37);

    /* Set up parameters at 3D4h */
    for (i=0; i<16; i++)
    {
	out_byte(0x3D4, (unsigned char)i);
	out_byte(0x3D5, *setup_regs);
	setup_regs++;
    }

    /* Enable video */
    out_byte(0x3D8, mode_set);

    /* Set overscan */
    if (mode == 6) out_byte(0x3D9, 0x3F);
    else           out_byte(0x3D9, 0x30);
}

static void bios_print_string(void)
{
    extern void video_bios_print_string(char *string, int x, int y, int attr, int count);
    char *s = (char *)(M.x86.R_ES<<4) + M.x86.R_BP;
    int attr;
    if (M.x86.R_AL & 0x02) attr = - 1;
    else                   attr = M.x86.R_BL;
    video_bios_print_string(s, M.x86.R_DH, M.x86.R_DL, attr, M.x86.R_CX);
}

static void X86API int42(int intno)
{
    switch (M.x86.R_AH)
    {
    case 0x00:
	bios_set_mode(M.x86.R_AL);
	break;
    case 0x13:
	bios_print_string();
	break;
    default:
	PRINTF("Warning: VIDEO BIOS interrupt %xh unimplemented function %xh, AL = %xh\n",
	       intno, M.x86.R_AH, M.x86.R_AL);
    }
}

static void X86API int15(int intno)
{
    PRINTF("Called interrupt 15h: AX = %xh, BX = %xh, CX = %xh, DX = %xh\n",
	   M.x86.R_AX, M.x86.R_BX, M.x86.R_CX, M.x86.R_DX);
}
