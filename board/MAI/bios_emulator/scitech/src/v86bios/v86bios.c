/*
 * Copyright 1999 Egbert Eich
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#define DELETE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>
#if defined(__alpha__) || defined (__ia64__)
#include <sys/io.h>
#elif defined(HAVE_SYS_PERM)
#include <sys/perm.h>
#endif
#include "debug.h"
#include "v86bios.h"
#include "pci.h"
#include "AsmMacros.h"

#define SIZE 0x100000
#define VRAM_START 0xA0000
#define VRAM_SIZE 0x1FFFF
#define V_BIOS_SIZE 0x1FFFF
#define BIOS_START 0x7C00            /* default BIOS entry */
#define BIOS_MEM 0x600

/*CARD8 code[] = { 0xb8 , 0xf0 , 0xf0, 0xf4 }; */
#define VB_X(x) (V_BIOS >> x) & 0xFF
CARD8 code[] = { 6, 0x9a, 0x03, 0x00, 0x00, VB_X(12), 0xf4 };
/*CARD8 code[] = { 0x9a, 0x03, 0x00, 0x00, VB_X(12), 0xb8, 0x03, 0x00, */
/*0xcd, 0x10, 0xf4 }; */
/*CARD8 code[] = {  0xb8 , 0xf0 , 0xf0 ,0xf4 }; */

int ioperm_list[IOPERM_BITS] = {0,};

static void sig_handler(int);
static int map(void);
static void unmap(void);
static void bootBIOS(CARD16 ax);
static int map_vram(void);
static void unmap_vram(void);
static int copy_vbios(memType v_base);
static int copy_sys_bios(void);
static void save_bios_to_file(void);
static int setup_system_bios(void);
static CARD32 setup_int_vect(void);
#ifdef __ia32__
static CARD32 setup_primary_int_vect(void);
#endif
static int chksum(CARD8 *start);
static void setup_bios_regs(i86biosRegsPtr regs, CARD32 ax);
static void print_regs(i86biosRegsPtr regs);
static void print_usage(void);
static void set_hlt(Bool set);
static void set_ioperm(void);

extern void yyparse();

void loadCodeToMem(unsigned char *ptr, CARD8 *code);
void dprint(unsigned long start, unsigned long size);

static int vram_mapped = 0;
static char* bios_var = NULL;
static CARD8 save_msr;
static CARD8 save_pos102;
static CARD8 save_vse;
static CARD8 save_46e8;
static haltpoints hltp[20] = { {0, 0}, };

console Console = {-1,-1};
struct config Config;

int main(int argc,char **argv)
{
    int c;

    Config.PrintPort = PRINT_PORT;
    Config.IoStatistics = IO_STATISTICS;
    Config.PrintIrq = PRINT_IRQ;
    Config.PrintPci = PRINT_PCI;
    Config.ShowAllDev = SHOW_ALL_DEV;
    Config.PrintIp = PRINT_IP;
    Config.SaveBios = SAVE_BIOS;
    Config.Trace = TRACE;
    Config.ConfigActiveOnly = CONFIG_ACTIVE_ONLY;  /* boot */
    Config.ConfigActiveDevice = CONFIG_ACTIVE_DEVICE; /* boot */
    Config.MapSysBios = MAP_SYS_BIOS;
    Config.Resort = RESORT;  /* boot */
    Config.FixRom = FIX_ROM;
    Config.NoConsole = NO_CONSOLE;
    Config.BootOnly = FALSE;
    Config.Verbose = VERBOSE;

    opterr = 0;
    while ((c = getopt(argc,argv,"psicaPStAdbrfnv:?")) != EOF) {
    switch(c) {
    case 'p':
	Config.PrintPort = TRUE;
	break;
    case 's':
	Config.IoStatistics = TRUE;
	break;
    case 'i':
	Config.PrintIrq = TRUE;
	break;
    case 'c':
	Config.PrintPci = TRUE;
	break;
    case 'a':
	Config.ShowAllDev = TRUE;
	break;
    case 'P':
	Config.PrintIp = TRUE;
	break;
    case 'S':
	Config.SaveBios = TRUE;
	break;
    case 't':
	Config.Trace = TRUE;
	break;
    case 'A':
	Config.ConfigActiveOnly = TRUE;
	break;
    case 'd':
	Config.ConfigActiveDevice = TRUE;
	break;
    case 'b':
	Config.MapSysBios = TRUE;
	break;
    case 'r':
	Config.Resort = TRUE;
	break;
    case 'f':
	Config.FixRom = TRUE;
	break;
    case 'n':
	Config.NoConsole = TRUE;
	break;
    case 'v':
	Config.Verbose = strtol(optarg,NULL,0);
	break;
    case '?':
	print_usage();
	break;
    default:
	break;
    }
    }


    if (!map())
    exit(1);

    if (!setup_system_bios())
    exit(1);

    iopl(3);

    scan_pci();

    save_msr = inb(0x3CC);
    save_vse = inb(0x3C3);
    save_46e8 = inb(0x46e8);
    save_pos102 = inb(0x102);

    if (Config.BootOnly) {

    if (!CurrentPci && !Config.ConfigActiveDevice
	&& !Config.ConfigActiveOnly) {
	iopl(0);
	unmap();
	exit (1);
    }
    call_boot(NULL);
    } else {
    using_history();
    yyparse();
    }

    unmap();

    pciVideoRestore();

    outb(0x102, save_pos102);
    outb(0x46e8, save_46e8);
    outb(0x3C3, save_vse);
    outb(0x3C2, save_msr);

    iopl(0);

    close_console(Console);

    exit(0);
}


void
call_boot(struct device *dev)
{
    int Active_is_Pci = 0;
    CARD32 vbios_base;

    CurrentPci = PciList;
    Console = open_console();

    set_ioperm();


    signal(2,sig_handler);
    signal(11,sig_handler);

    /* disable primary card */
    pciVideoRestore(); /* reset PCI state to see primary card */
    outb(0x3C2,~(CARD8)0x03 & save_msr);
    outb(0x3C3,~(CARD8)0x01 & save_vse);
    outb(0x46e8, ~(CARD8)0x08 & save_46e8);
    outb(0x102, ~(CARD8)0x01 & save_pos102);

    pciVideoDisable();

    while (CurrentPci) {
    CARD16 ax;

    if (CurrentPci->active) {
	Active_is_Pci = 1;
	if (!Config.ConfigActiveDevice && !dev) {
	CurrentPci = CurrentPci->next;
	continue;
	}
    } else if (Config.ConfigActiveOnly && !dev) {
	CurrentPci = CurrentPci->next;
	continue;
    }
    if (dev && ((dev->type != PCI)
	    || (dev->type == PCI
	    && (dev->loc.pci.dev != CurrentPci->dev
		|| dev->loc.pci.bus != CurrentPci->bus
		|| dev->loc.pci.func != CurrentPci->func)))) {
	CurrentPci = CurrentPci->next;
	continue;
    }

    EnableCurrent();

    if (CurrentPci->active) {
	outb(0x102, save_pos102);
	outb(0x46e8, save_46e8);
	outb(0x3C3, save_vse);
	outb(0x3C2, save_msr);
    }

    /* clear interrupt vectors */
#ifdef __ia32__
    vbios_base = CurrentPci->active ? setup_primary_int_vect()
	: setup_int_vect();
#else
    vbios_base = setup_int_vect();
#endif
    ax = ((CARD16)(CurrentPci->bus) << 8)
	| (CurrentPci->dev << 3) | (CurrentPci->func & 0x7);
    if (Config.Verbose > 1) P_printf("ax: 0x%x\n",ax);

    BootBios = findPciByIDs(CurrentPci->bus,CurrentPci->dev,
		   CurrentPci->func);
    if (!((mapPciRom(BootBios) && chksum((CARD8*)V_BIOS))
	  || (CurrentPci->active && copy_vbios(vbios_base)))) {
	CurrentPci = CurrentPci->next;
	continue;
    }
    if (!map_vram()) {
	CurrentPci = CurrentPci->next;
	continue;
    }
    if (Config.SaveBios) save_bios_to_file();
    printf("initializing PCI bus: %i dev: %i func: %i\n",CurrentPci->bus,
	   CurrentPci->dev,CurrentPci->func);
    bootBIOS(ax);
    unmap_vram();

    if (CurrentPci->active)
	close_console(Console);

    if (dev) return;

    CurrentPci = CurrentPci->next;
    }

    /* We have an ISA device - configure if requested */
    if (!Active_is_Pci /* no isa card in system! */
    && ((!dev && (Config.ConfigActiveDevice || Config.ConfigActiveOnly))
	|| (dev && dev->type == ISA))) {

    pciVideoDisable();

    if (!dev || dev->type == ISA) {
	outb(0x102, save_pos102);
	outb(0x46e8, save_46e8);
	outb(0x3C3, save_vse);
	outb(0x3C2, save_msr);

#ifdef __ia32__
	vbios_base = setup_primary_int_vect();
#else
	vbios_base = setup_int_vect();
#endif
	if (copy_vbios(vbios_base)) {

	if (Config.SaveBios) save_bios_to_file();
	if  (map_vram()) {
	    printf("initializing ISA bus\n");
	    bootBIOS(0);
	}
	}

	unmap_vram();
	sleep(1);
	close_console(Console);
    }
    }


}

int
map(void)
{
    void* mem;
    mem = mmap(0, (size_t)SIZE,
	       PROT_EXEC | PROT_READ | PROT_WRITE,
	       MAP_FIXED | MAP_PRIVATE | MAP_ANON,
	       -1, 0 );
    if (mem != 0) {
	perror("anonymous map");
	return (0);
    }
    memset(mem,0,SIZE);

    return (1);
}

static void
unmap(void)
{
    munmap(0,SIZE);
}

static void
bootBIOS(CARD16 ax)
{
    i86biosRegs bRegs;
#ifdef V86BIOS_DEBUG
    printf("starting BIOS\n");
#endif
    setup_io();
    setup_bios_regs(&bRegs, ax);
    loadCodeToMem((unsigned char *) BIOS_START, code);
    do_x86(BIOS_START,&bRegs);
#ifdef V86BIOS_DEBUG
    printf("done\n");
#endif
}

static int
map_vram(void)
{
    int mem_fd;

#ifdef __ia64__
    if ((mem_fd = open(MEM_FILE,O_RDWR | O_SYNC))<0)
#else
    if ((mem_fd = open(MEM_FILE,O_RDWR))<0)
#endif
	{
	perror("opening memory");
	return 0;
    }

#ifdef __alpha__
       if (!_bus_base()) sparse_shift = 7; /* Uh, oh, JENSEN... */
	 if (!_bus_base_sparse()) sparse_shift = 0;
	 if ((vram_map = mmap(0,(size_t) (VRAM_SIZE << sparse_shift),
						 PROT_READ | PROT_WRITE,
						 MAP_SHARED,
						 mem_fd, (VRAM_START << sparse_shift)
						 | _bus_base_sparse())) == (void *) -1)
#else
      if (mmap((void *) VRAM_START, (size_t) VRAM_SIZE,
			 PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
			 mem_fd, VRAM_START) == (void *) -1)
#endif
      {
	perror("mmap error in map_hardware_ram (1)");
	    close(mem_fd);
	    return (0);
	}
    vram_mapped = 1;
    close(mem_fd);
    return (1);
}

static void
unmap_vram(void)
{
    if (!vram_mapped) return;

    munmap((void*)VRAM_START,VRAM_SIZE);
    vram_mapped = 0;
}

static int
copy_vbios(memType v_base)
{
    int mem_fd;
    unsigned char *tmp;
    int size;

    if ((mem_fd = open(MEM_FILE,O_RDONLY))<0) {
	perror("opening memory");
	return (0);
    }

    if (lseek(mem_fd,(off_t) v_base, SEEK_SET) != (off_t) v_base) {
	  fprintf(stderr,"Cannot lseek\n");
	  goto Error;
      }
    tmp = (unsigned char *)malloc(3);
    if (read(mem_fd, (char *)tmp, (size_t) 3) != (size_t) 3) {
	    fprintf(stderr,"Cannot read\n");
	goto Error;
    }
    if (lseek(mem_fd,(off_t) v_base,SEEK_SET) != (off_t) v_base)
	goto Error;

    if (*tmp != 0x55 || *(tmp+1) != 0xAA ) {
	fprintf(stderr,"No bios found at: 0x%lx\n",v_base);
	goto Error;
    }
#ifdef DEBUG
	dprint((unsigned long)tmp,0x100);
#endif
    size = *(tmp+2) * 512;

    if (read(mem_fd, (char *)v_base, (size_t) size) != (size_t) size) {
	    fprintf(stderr,"Cannot read\n");
	goto Error;
    }
    free(tmp);
    close(mem_fd);
    if (!chksum((CARD8*)v_base))
	return (0);

    return (1);

Error:
    perror("v_bios");
    close(mem_fd);
    return (0);
}

static int
copy_sys_bios(void)
{
#define SYS_BIOS 0xF0000
    int mem_fd;

    if ((mem_fd = open(MEM_FILE,O_RDONLY))<0) {
	perror("opening memory");
	return (0);
    }

    if (lseek(mem_fd,(off_t) SYS_BIOS,SEEK_SET) != (off_t) SYS_BIOS)
	goto Error;
    if (read(mem_fd, (char *)SYS_BIOS, (size_t) 0xFFFF) != (size_t) 0xFFFF)
	goto Error;

    close(mem_fd);
    return (1);

Error:
    perror("sys_bios");
    close(mem_fd);
    return (0);
}

void
loadCodeToMem(unsigned char *ptr, CARD8 code[])
{
    int i;
    CARD8 val;
    int size = code[0];

    for ( i=1;i<=size;i++) {
	val = code[i];
	*ptr++ = val;
    }
    return;
}

void
dprint(unsigned long start, unsigned long size)
{
    int i,j;
    char *c = (char *)start;

    for (j = 0; j < (size >> 4); j++) {
    char *d = c;
    printf("\n0x%lx:  ",(unsigned long)c);
    for (i = 0; i<16; i++)
	printf("%2.2x ",(unsigned char) (*(c++)));
    c = d;
    for (i = 0; i<16; i++) {
	printf("%c",((((CARD8)(*c)) > 32) && (((CARD8)(*c)) < 128)) ?
	   (unsigned char) (*(c)): '.');
	c++;
    }
    }
    printf("\n");
}

static void
save_bios_to_file(void)
{
    static int num = 0;
    int size, count;
    char file_name[256];
    int fd;

    sprintf(file_name,"bios_%i.fil",num);
    if ((fd =  open(file_name,O_WRONLY | O_CREAT | O_TRUNC,00644)) == -1)
	return;
    size = (*(unsigned char*)(V_BIOS + 2)) * 512;
#ifdef V86BIOS_DEBUG
    dprint(V_BIOS,20);
#endif
    if ((count = write(fd,(void *)(V_BIOS),size)) != size)
	fprintf(stderr,"only saved %i of %i bytes\n",size,count);
    num++;
}

static void
sig_handler(int unused)
{
    fflush(stdout);
    fflush(stderr);

    /* put system back in a save state */
    unmap_vram();
    pciVideoRestore();
    outb(0x102, save_pos102);
    outb(0x46e8, save_46e8);
    outb(0x3C3, save_vse);
    outb(0x3C2, save_msr);

    close_console(Console);
    iopl(0);
    unmap();

    exit(1);
}

/*
 * For initialization we just pass ax to the BIOS.
 * PCI BIOSes need this. All other register are set 0.
 */
static void setup_bios_regs(i86biosRegsPtr regs, CARD32 ax)
{
    regs->ax = ax;
    regs->bx = 0;
    regs->cx = 0;
    regs->dx = 0;
    regs->es = 0;
    regs->ds = 0x40;               /* standard pc ds */
    regs->si = 0;
    regs->di = 0;
}

/*
 * here we are really paranoid about faking a "real"
 * BIOS. Most of this information was pulled from
 * dosem.
 */

#ifdef __ia32__
static CARD32
setup_primary_int_vect(void)
{
    int mem_fd;
    CARD32 vbase;
    void *map;

    if ((mem_fd = open(MEM_FILE,O_RDWR))<0)
	    {
    perror("opening memory");
    return (0);
    }

    if ((map = mmap((void *) 0, (size_t) 0x2000,
	 PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED,
	 mem_fd, 0)) == (void *)-1)   {
    perror("mmap error in map_hardware_ram (2)");
    close(mem_fd);
    return (0);
    }

    close(mem_fd);
    memcpy(0,map,BIOS_MEM);
    munmap(map,0x2000);
    /*
     * create a backup copy of the bios variables to write back the
     * modified values
     */
    if (!bios_var)
    bios_var = (char *)malloc(BIOS_MEM);
    memcpy(bios_var,0,BIOS_MEM);

    vbase = (*((CARD16*)(0x10 << 2) + 1)) << 4;
    if (Config.Verbose > 0) printf("vbase: 0x%x\n",vbase);
    return vbase;
}
#endif

static CARD32
setup_int_vect(void)
{
    const CARD16 cs = 0x0;
    const CARD16 ip = 0x0;
    int i;

    /* let the int vects point to the SYS_BIOS seg */
    for (i=0; i<0x80; i++) {
	((CARD16*)0)[i<<1] = ip;
	((CARD16*)0)[(i<<1)+1] = cs;
    }
    /* video interrupts default location */
    ((CARD16*)0)[(0x42<<1)+1] = 0xf000;
    ((CARD16*)0)[0x42<<1] = 0xf065;
    ((CARD16*)0)[(0x10<<1)+1] = 0xf000;
    ((CARD16*)0)[0x10<<1] = 0xf065;
    /* video param table default location (int 1d) */
    ((CARD16*)0)[(0x1d<<1)+1] = 0xf000;
    ((CARD16*)0)[0x1d<<1] = 0xf0A4;
    /* font tables default location (int 1F) */
    ((CARD16*)0)[(0x1f<<1)+1] = 0xf000;
    ((CARD16*)0)[0x1f<<1] = 0xfa6e;

    /* int 11 default location */
    ((CARD16*)0)[(0x11<<1)+1] = 0xf000;
    ((CARD16*)0)[0x11<<1] = 0xf84d;
    /* int 12 default location */
    ((CARD16*)0)[(0x12<<1)+1] = 0xf000;
    ((CARD16*)0)[0x12<<1] = 0xf841;
    /* int 15 default location */
    ((CARD16*)0)[(0x15<<1)+1] = 0xf000;
    ((CARD16*)0)[0x15<<1] = 0xf859;
    /* int 1A default location */
    ((CARD16*)0)[(0x1a<<1)+1] = 0xf000;
    ((CARD16*)0)[0x1a<<1] = 0xff6e;
    /* int 05 default location */
    ((CARD16*)0)[(0x05<<1)+1] = 0xf000;
    ((CARD16*)0)[0x05<<1] = 0xff54;
    /* int 08 default location */
    ((CARD16*)0)[(0x8<<1)+1] = 0xf000;
    ((CARD16*)0)[0x8<<1] = 0xfea5;
    /* int 13 default location (fdd) */
    ((CARD16*)0)[(0x13<<1)+1] = 0xf000;
    ((CARD16*)0)[0x13<<1] = 0xec59;
    /* int 0E default location */
    ((CARD16*)0)[(0xe<<1)+1] = 0xf000;
    ((CARD16*)0)[0xe<<1] = 0xef57;
    /* int 17 default location */
    ((CARD16*)0)[(0x17<<1)+1] = 0xf000;
    ((CARD16*)0)[0x17<<1] = 0xefd2;
    /* fdd table default location (int 1e) */
    ((CARD16*)0)[(0x1e<<1)+1] = 0xf000;
    ((CARD16*)0)[0x1e<<1] = 0xefc7;
    return V_BIOS;
}

static int
setup_system_bios(void)
{
    char *date = "06/01/99";
    char *eisa_ident = "PCI/ISA";

    if (Config.MapSysBios) {

	if (!copy_sys_bios()) return 0;
	return 1;

    } else {

/*    memset((void *)0xF0000,0xf4,0xfff7); */

	/*
	 * we trap the "industry standard entry points" to the BIOS
	 * and all other locations by filling them with "hlt"
	 * TODO: implement hlt-handler for these
	 */
	memset((void *)0xF0000,0xf4,0x10000);

	/*
	 * TODO: we should copy the fdd table (0xfec59-0xfec5b)
	 * the video parameter table (0xf0ac-0xf0fb)
	 * and the font tables (0xfa6e-0xfe6d)
	 * from the original bios here
	 */

	/* set bios date */
	strcpy((char *)0xFFFF5,date);
	/* set up eisa ident string */
	strcpy((char *)0xFFFD9,eisa_ident);
	/* write system model id for IBM-AT */
	((char *)0)[0xFFFFE] = 0xfc;

	return 1;
    }

}

static void
update_bios_vars(void)
{
    int mem_fd;
    void *map;
    memType i;

#ifdef __ia64__
    if ((mem_fd = open(MEM_FILE,O_RDWR | O_SYNC))<0)
#else
    if ((mem_fd = open(MEM_FILE,O_RDWR))<0)
#endif
	    {
    perror("opening memory");
    return;
    }

    if ((map = mmap((void *) 0, (size_t) 0x2000,
	 PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED,
	 mem_fd, 0)) == (void *)-1)   {
    perror("mmap error in map_hardware_ram (3)");
    close(mem_fd);
    return;
    }

    for (i = 0; i < BIOS_MEM; i++) {
    if (bios_var[i] != *(CARD8*)i)
	*((CARD8*)map + i) = *(CARD8*)i;
    }

    munmap(map,0x2000);
    close(mem_fd);
}

static int
chksum(CARD8 *start)
{
  CARD16 size;
  CARD8 val = 0;
  int i;

  size = *(start+2) * 512;
  for (i = 0; i<size; i++)
    val += *(start + i);

  if (!val)
    return 1;

    fprintf(stderr,"BIOS cksum wrong!\n");
  return 0;
}

void
runINT(int num, i86biosRegsPtr Regs)
{
    Bool isVideo = FALSE;
    CARD8 code_int[] = { 3, 0xcd, 0x00, 0xf4 };

    code_int[2] = (CARD8) num;

    if (num == 0x10)
    isVideo = TRUE;

    if (!setup_system_bios())
    return;

    if ((isVideo && (!CurrentPci || CurrentPci->active)) || !isVideo) {
    CARD32 vbios_base;

#ifdef __ia32__
    if (!(vbios_base = setup_primary_int_vect()))
#else
    if (!(vbios_base = setup_int_vect()))
#endif
	return;
    if (!copy_vbios(vbios_base))
	return;
    }

    if (!map_vram())
    return;

#ifdef V86BIOS_DEBUG
	printf("starting BIOS\n");
#endif
    loadCodeToMem((unsigned char *) BIOS_START, code_int);
    setup_io();
    print_regs(Regs);
    set_ioperm();
    set_hlt(TRUE);
    do_x86(BIOS_START,Regs);
    set_hlt(FALSE);
    print_regs(Regs);

#ifdef V86BIOS_DEBUG
    printf("done\n");
#endif

    if ((isVideo && (!CurrentPci || CurrentPci->active)) || !isVideo)
    update_bios_vars();
}

static void
print_regs(i86biosRegsPtr regs)
{
    printf("ax=%x bx=%x cx=%x dx=%x ds=%x es=%x di=%x si=%x\n",
       (CARD16)regs->ax,(CARD16)regs->bx,(CARD16)regs->cx,(CARD16)regs->dx,
       (CARD16)regs->ds,(CARD16)regs->es,(CARD16)regs->di,
       (CARD16)regs->si);
}

static void
print_usage(void)
{
}

void
add_hlt(unsigned long val)
{
    int i;

    if (val < BIOS_MEM || (val > VRAM_START && val < (VRAM_START + VRAM_SIZE))
    || val >= SIZE) {
    printf("address out of range\n");
    return;
    }

    for (i=0; i<20; i++) {
    if (hltp[i].address == 0) {
	hltp[i].address = (void*)val;
	break;
    }
    }
    if (i == 20) printf("no more hltpoints available\n");
}

void
del_hlt(int val)
{
    if (val == 21) { /* delete all */
    int i;
    printf("clearing all hltpoints\n");
    for (i=0; i <20; i++)
	hltp[i].address = NULL;
    } else if (val >= 0 &&  val <20)
    hltp[val].address = NULL;
    else printf("hltpoint %i out of range: valid range 0-19\n",val);
}

void
list_hlt()
{
    int i;
    for (i=0; i<20; i++)
    if (hltp[i].address)
	printf("hltpoint[%i]: 0x%lx\n",i,(unsigned long)hltp[i].address);
}

static void
set_hlt(Bool set)
{
    int i;
    for (i=0; i<20; i++)
    if (hltp[i].address) {
	if (set) {
	hltp[i].orgval = *(CARD8*)hltp[i].address;
	*(CARD8*)hltp[i].address = 0xf4;
	} else
	*(CARD8*)hltp[i].address = hltp[i].orgval;
    }
}

static void
set_ioperm(void)
{
    int i, start;

    ioperm(0,IOPERM_BITS,0);

    for (i = 0; i < IOPERM_BITS;i++)
    if (ioperm_list[i]) {
	start = i;
	for (;i < IOPERM_BITS; i++) {
	if (!ioperm_list[i]) {
	    ioperm(start,i - start, 1);
	    break;
	}
	}
    }
}
