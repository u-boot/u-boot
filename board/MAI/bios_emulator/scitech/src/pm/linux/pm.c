;/****************************************************************************
*
*                   SciTech OS Portability Manager Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*                   Portions copyright (C) Josh Vanderhoof
*
* Language:     ANSI C
* Environment:  Linux
*
* Description:  Implementation for the OS Portability Manager Library, which
*               contains functions to implement OS specific services in a
*               generic, cross platform API. Porting the OS Portability
*               Manager library is the first step to porting any SciTech
*               products to a new platform.
*
****************************************************************************/

#include "pmapi.h"
#include "drvlib/os/os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/kd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <syscall.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/types.h>
#ifdef ENABLE_MTRR
#include <asm/mtrr.h>
#endif
#include <asm/vm86.h>
#ifdef __GLIBC__
#include <sys/perm.h>
#endif

/*--------------------------- Global variables ----------------------------*/

#define REAL_MEM_BASE       ((void *)0x10000)
#define REAL_MEM_SIZE       0x10000
#define REAL_MEM_BLOCKS     0x100
#define DEFAULT_VM86_FLAGS  (IF_MASK | IOPL_MASK)
#define DEFAULT_STACK_SIZE  0x1000
#define RETURN_TO_32_INT    255

/* Quick and dirty fix for vm86() syscall from lrmi 0.6 */
static int
vm86(struct vm86_struct *vm)
    {
    int r;
#ifdef __PIC__
    asm volatile (
     "pushl %%ebx\n\t"
     "movl %2, %%ebx\n\t"
     "int $0x80\n\t"
     "popl %%ebx"
     : "=a" (r)
     : "0" (113), "r" (vm));
#else
    asm volatile (
     "int $0x80"
     : "=a" (r)
     : "0" (113), "b" (vm));
#endif
    return r;
    }


static struct {
    int                 ready;
    unsigned short      ret_seg, ret_off;
    unsigned short      stack_seg, stack_off;
    struct vm86_struct  vm;
    } context = {0};

struct mem_block {
    unsigned int size : 20;
    unsigned int free : 1;
    };

static struct {
    int ready;
    int count;
    struct mem_block blocks[REAL_MEM_BLOCKS];
    } mem_info = {0};

int                     _PM_console_fd = -1;
int                     _PM_leds = 0,_PM_modifiers = 0;
static ibool            inited = false;
static int              tty_vc = 0;
static int              console_count = 0;
static int              startup_vc;
static int              fd_mem = 0;
static ibool            in_raw_mode = false;
#ifdef ENABLE_MTRR
static int              mtrr_fd;
#endif
static uint VESABuf_len = 1024;     /* Length of the VESABuf buffer     */
static void *VESABuf_ptr = NULL;    /* Near pointer to VESABuf          */
static uint VESABuf_rseg;           /* Real mode segment of VESABuf     */
static uint VESABuf_roff;           /* Real mode offset of VESABuf      */
#ifdef TRACE_IO
static ulong            traceAddr;
#endif

static void (PMAPIP fatalErrorCleanup)(void) = NULL;

/*----------------------------- Implementation ----------------------------*/

#ifdef  TRACE_IO
extern void printk(char *msg,...);
#endif

static inline void port_out(int value, int port)
{
#ifdef TRACE_IO
    printk("%04X:%04X: outb.%04X <- %02X\n", traceAddr >> 16, traceAddr & 0xFFFF, (ushort)port, (uchar)value);
#endif
    asm volatile ("outb %0,%1"
	  ::"a" ((unsigned char) value), "d"((unsigned short) port));
}

static inline void port_outw(int value, int port)
{
#ifdef TRACE_IO
    printk("%04X:%04X: outw.%04X <- %04X\n", traceAddr >> 16,traceAddr & 0xFFFF, (ushort)port, (ushort)value);
#endif
    asm volatile ("outw %0,%1"
	 ::"a" ((unsigned short) value), "d"((unsigned short) port));
}

static inline void port_outl(int value, int port)
{
#ifdef TRACE_IO
    printk("%04X:%04X: outl.%04X <- %08X\n", traceAddr >> 16,traceAddr & 0xFFFF, (ushort)port, (ulong)value);
#endif
    asm volatile ("outl %0,%1"
	 ::"a" ((unsigned long) value), "d"((unsigned short) port));
}

static inline unsigned int port_in(int port)
{
    unsigned char value;
    asm volatile ("inb %1,%0"
	      :"=a" ((unsigned char)value)
	      :"d"((unsigned short) port));
#ifdef TRACE_IO
    printk("%04X:%04X:  inb.%04X -> %02X\n", traceAddr >> 16,traceAddr & 0xFFFF, (ushort)port, (uchar)value);
#endif
    return value;
}

static inline unsigned int port_inw(int port)
{
    unsigned short value;
    asm volatile ("inw %1,%0"
	      :"=a" ((unsigned short)value)
	      :"d"((unsigned short) port));
#ifdef TRACE_IO
    printk("%04X:%04X:  inw.%04X -> %04X\n", traceAddr >> 16,traceAddr & 0xFFFF, (ushort)port, (ushort)value);
#endif
    return value;
}

static inline unsigned int port_inl(int port)
{
    unsigned long value;
    asm volatile ("inl %1,%0"
	      :"=a" ((unsigned long)value)
	      :"d"((unsigned short) port));
#ifdef TRACE_IO
    printk("%04X:%04X:  inl.%04X -> %08X\n", traceAddr >> 16,traceAddr & 0xFFFF, (ushort)port, (ulong)value);
#endif
    return value;
}

static int real_mem_init(void)
{
    void    *m;
    int     fd_zero;

    if (mem_info.ready)
	return 1;

    if ((fd_zero = open("/dev/zero", O_RDONLY)) == -1)
	PM_fatalError("You must have root privledges to run this program!");
    if ((m = mmap((void *)REAL_MEM_BASE, REAL_MEM_SIZE,
	    PROT_READ | PROT_WRITE | PROT_EXEC,
	    MAP_FIXED | MAP_PRIVATE, fd_zero, 0)) == (void *)-1) {
	close(fd_zero);
	PM_fatalError("You must have root privledges to run this program!");
	}
    mem_info.ready = 1;
    mem_info.count = 1;
    mem_info.blocks[0].size = REAL_MEM_SIZE;
    mem_info.blocks[0].free = 1;
    return 1;
}

static void insert_block(int i)
{
    memmove(
	mem_info.blocks + i + 1,
	mem_info.blocks + i,
	(mem_info.count - i) * sizeof(struct mem_block));
    mem_info.count++;
}

static void delete_block(int i)
{
    mem_info.count--;

    memmove(
	mem_info.blocks + i,
	mem_info.blocks + i + 1,
	(mem_info.count - i) * sizeof(struct mem_block));
}

static inline void set_bit(unsigned int bit, void *array)
{
    unsigned char *a = array;
    a[bit / 8] |= (1 << (bit % 8));
}

static inline unsigned int get_int_seg(int i)
{
    return *(unsigned short *)(i * 4 + 2);
}

static inline unsigned int get_int_off(int i)
{
    return *(unsigned short *)(i * 4);
}

static inline void pushw(unsigned short i)
{
    struct vm86_regs *r = &context.vm.regs;
    r->esp -= 2;
    *(unsigned short *)(((unsigned int)r->ss << 4) + r->esp) = i;
}

ibool PMAPI PM_haveBIOSAccess(void)
{ return true; }

void PMAPI PM_init(void)
{
    void    *m;
    uint    r_seg,r_off;

    if (inited)
	return;

    /* Map the Interrupt Vectors (0x0 - 0x400) + BIOS data (0x400 - 0x502)
     * and the physical framebuffer and ROM images from (0xa0000 - 0x100000)
     */
    real_mem_init();
    if (!fd_mem && (fd_mem = open("/dev/mem", O_RDWR)) == -1) {
	PM_fatalError("You must have root privileges to run this program!");
	}
    if ((m = mmap((void *)0, 0x502,
	    PROT_READ | PROT_WRITE | PROT_EXEC,
	    MAP_FIXED | MAP_PRIVATE, fd_mem, 0)) == (void *)-1) {
	PM_fatalError("You must have root privileges to run this program!");
	}
    if ((m = mmap((void *)0xA0000, 0xC0000 - 0xA0000,
	    PROT_READ | PROT_WRITE,
	    MAP_FIXED | MAP_SHARED, fd_mem, 0xA0000)) == (void *)-1) {
	PM_fatalError("You must have root privileges to run this program!");
	}
    if ((m = mmap((void *)0xC0000, 0xD0000 - 0xC0000,
	    PROT_READ | PROT_WRITE | PROT_EXEC,
	    MAP_FIXED | MAP_PRIVATE, fd_mem, 0xC0000)) == (void *)-1) {
	PM_fatalError("You must have root privileges to run this program!");
	}
    if ((m = mmap((void *)0xD0000, 0x100000 - 0xD0000,
	    PROT_READ | PROT_WRITE,
	    MAP_FIXED | MAP_SHARED, fd_mem, 0xD0000)) == (void *)-1) {
	PM_fatalError("You must have root privileges to run this program!");
	}
    inited = 1;

    /* Allocate a stack */
    m = PM_allocRealSeg(DEFAULT_STACK_SIZE,&r_seg,&r_off);
    context.stack_seg = r_seg;
    context.stack_off = r_off+DEFAULT_STACK_SIZE;

    /* Allocate the return to 32 bit routine */
    m = PM_allocRealSeg(2,&r_seg,&r_off);
    context.ret_seg = r_seg;
    context.ret_off = r_off;
    ((uchar*)m)[0] = 0xCD;         /* int opcode */
    ((uchar*)m)[1] = RETURN_TO_32_INT;
    memset(&context.vm, 0, sizeof(context.vm));

    /* Enable kernel emulation of all ints except RETURN_TO_32_INT */
    memset(&context.vm.int_revectored, 0, sizeof(context.vm.int_revectored));
    set_bit(RETURN_TO_32_INT, &context.vm.int_revectored);
    context.ready = 1;
#ifdef ENABLE_MTRR
    mtrr_fd =  open("/dev/cpu/mtrr", O_RDWR, 0);
    if (mtrr_fd < 0)
       mtrr_fd =  open("/proc/mtrr", O_RDWR, 0);
#endif
    /* Enable I/O permissions to directly access I/O ports. We break the
     * allocation into two parts, one for the ports from 0-0x3FF and
     * another for the remaining ports up to 0xFFFF. Standard Linux kernels
     * only allow the first 0x400 ports to be enabled, so to enable all
     * 65536 ports you need a patched kernel that will enable the full
     * 8Kb I/O permissions bitmap.
     */
#ifndef TRACE_IO
    ioperm(0x0,0x400,1);
    ioperm(0x400,0x10000-0x400,1);
#endif
    iopl(3);
}

long PMAPI PM_getOSType(void)
{ return _OS_LINUX; }

int PMAPI PM_getModeType(void)
{ return PM_386; }

void PMAPI PM_backslash(char *s)
{
    uint pos = strlen(s);
    if (s[pos-1] != '/') {
	s[pos] = '/';
	s[pos+1] = '\0';
	}
}

void PMAPI PM_setFatalErrorCleanup(
    void (PMAPIP cleanup)(void))
{
    fatalErrorCleanup = cleanup;
}

void PMAPI PM_fatalError(const char *msg)
{
    if (fatalErrorCleanup)
	fatalErrorCleanup();
    fprintf(stderr,"%s\n", msg);
    fflush(stderr);
    exit(1);
}

static void ExitVBEBuf(void)
{
    if (VESABuf_ptr)
	PM_freeRealSeg(VESABuf_ptr);
    VESABuf_ptr = 0;
}

void * PMAPI PM_getVESABuf(uint *len,uint *rseg,uint *roff)
{
    if (!VESABuf_ptr) {
	/* Allocate a global buffer for communicating with the VESA VBE */
	if ((VESABuf_ptr = PM_allocRealSeg(VESABuf_len, &VESABuf_rseg, &VESABuf_roff)) == NULL)
	    return NULL;
	atexit(ExitVBEBuf);
	}
    *len = VESABuf_len;
    *rseg = VESABuf_rseg;
    *roff = VESABuf_roff;
    return VESABuf_ptr;
}

/* New raw console based getch and kbhit functions */

#define KB_CAPS     LED_CAP /* 4 */
#define KB_NUMLOCK  LED_NUM /* 2 */
#define KB_SCROLL   LED_SCR /* 1 */
#define KB_SHIFT    8
#define KB_CONTROL  16
#define KB_ALT      32

/* Structure used to save the keyboard mode to disk. We save it to disk
 * so that we can properly restore the mode later if the program crashed.
 */

typedef struct {
    struct termios  termios;
    int             kb_mode;
    int             leds;
    int             flags;
    int             startup_vc;
    } keyboard_mode;

/* Name of the file used to save keyboard mode information */

#define KBMODE_DAT  "kbmode.dat"

/****************************************************************************
REMARKS:
Open the keyboard mode file on disk.
****************************************************************************/
static FILE *open_kb_mode(
    char *mode,
    char *path)
{
    if (!PM_findBPD("graphics.bpd",path))
	return NULL;
    PM_backslash(path);
    strcat(path,KBMODE_DAT);
    return fopen(path,mode);
}

/****************************************************************************
REMARKS:
Restore the keyboard to normal mode
****************************************************************************/
void _PM_restore_kb_mode(void)
{
    FILE            *kbmode;
    keyboard_mode   mode;
    char            path[PM_MAX_PATH];

    if (_PM_console_fd != -1 && (kbmode = open_kb_mode("rb",path)) != NULL) {
	if (fread(&mode,1,sizeof(mode),kbmode) == sizeof(mode)) {
	    if (mode.startup_vc > 0)
		ioctl(_PM_console_fd, VT_ACTIVATE, mode.startup_vc);
	    ioctl(_PM_console_fd, KDSKBMODE, mode.kb_mode);
	    ioctl(_PM_console_fd, KDSETLED, mode.leds);
	    tcsetattr(_PM_console_fd, TCSAFLUSH, &mode.termios);
	    fcntl(_PM_console_fd,F_SETFL,mode.flags);
	    }
	fclose(kbmode);
	unlink(path);
	in_raw_mode = false;
	}
}

/****************************************************************************
REMARKS:
Safely abort the event module upon catching a fatal error.
****************************************************************************/
void _PM_abort(
    int signo)
{
    char    buf[80];

    sprintf(buf,"Terminating on signal %d",signo);
    _PM_restore_kb_mode();
    PM_fatalError(buf);
}

/****************************************************************************
REMARKS:
Put the keyboard into raw mode
****************************************************************************/
void _PM_keyboard_rawmode(void)
{
    struct termios conf;
    FILE            *kbmode;
    keyboard_mode   mode;
    char            path[PM_MAX_PATH];
    int             i;
    static int sig_list[] = {
	SIGHUP,
	SIGINT,
	SIGQUIT,
	SIGILL,
	SIGTRAP,
	SIGABRT,
	SIGIOT,
	SIGBUS,
	SIGFPE,
	SIGKILL,
	SIGSEGV,
	SIGTERM,
	};

    if ((kbmode = open_kb_mode("rb",path)) == NULL) {
	if ((kbmode = open_kb_mode("wb",path)) == NULL)
	    PM_fatalError("Unable to open kbmode.dat file for writing!");
	if (ioctl(_PM_console_fd, KDGKBMODE, &mode.kb_mode))
	    perror("KDGKBMODE");
	ioctl(_PM_console_fd, KDGETLED, &mode.leds);
	_PM_leds = mode.leds & 0xF;
	_PM_modifiers = 0;
	tcgetattr(_PM_console_fd, &mode.termios);
	conf = mode.termios;
	conf.c_lflag &= ~(ICANON | ECHO | ISIG);
	conf.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | BRKINT | PARMRK | INPCK | IUCLC | IXON | IXOFF);
	conf.c_iflag  |= (IGNBRK | IGNPAR);
	conf.c_cc[VMIN] = 1;
	conf.c_cc[VTIME] = 0;
	conf.c_cc[VSUSP] = 0;
	tcsetattr(_PM_console_fd, TCSAFLUSH, &conf);
	mode.flags = fcntl(_PM_console_fd,F_GETFL);
	if (ioctl(_PM_console_fd, KDSKBMODE, K_MEDIUMRAW))
	    perror("KDSKBMODE");
	atexit(_PM_restore_kb_mode);
	for (i = 0; i < sizeof(sig_list)/sizeof(sig_list[0]); i++)
	    signal(sig_list[i], _PM_abort);
	mode.startup_vc = startup_vc;
	if (fwrite(&mode,1,sizeof(mode),kbmode) != sizeof(mode))
	    PM_fatalError("Error writing kbmode.dat!");
	fclose(kbmode);
	in_raw_mode = true;
	}
}

int PMAPI PM_kbhit(void)
{
    fd_set s;
    struct timeval tv = { 0, 0 };

    if (console_count == 0)
	PM_fatalError("You *must* open a console before using PM_kbhit!");
    if (!in_raw_mode)
	_PM_keyboard_rawmode();
    FD_ZERO(&s);
    FD_SET(_PM_console_fd, &s);
    return select(_PM_console_fd+1, &s, NULL, NULL, &tv) > 0;
}

int PMAPI PM_getch(void)
{
    static uchar    c;
    int release;
    static struct kbentry ke;

    if (console_count == 0)
	PM_fatalError("You *must* open a console before using PM_getch!");
    if (!in_raw_mode)
	_PM_keyboard_rawmode();
    while (read(_PM_console_fd, &c, 1) > 0) {
	release = c & 0x80;
	c &= 0x7F;
	if (release) {
	    switch(c){
		case 42: case 54: /* Shift */
		    _PM_modifiers &= ~KB_SHIFT;
		    break;
		case 29: case 97: /* Control */
		    _PM_modifiers &= ~KB_CONTROL;
		    break;
		case 56: case 100: /* Alt / AltGr */
		    _PM_modifiers &= ~KB_ALT;
		    break;
		}
	    continue;
	    }
	switch (c) {
	    case 42: case 54: /* Shift */
		_PM_modifiers |= KB_SHIFT;
		 break;
	    case 29: case 97: /* Control */
		_PM_modifiers |= KB_CONTROL;
		break;
	    case 56: case 100: /* Alt / AltGr */
		_PM_modifiers |= KB_ALT;
		break;
	    case 58: /* Caps Lock */
		_PM_modifiers ^= KB_CAPS;
		ioctl(_PM_console_fd, KDSETLED, _PM_modifiers & 7);
		break;
	    case 69: /* Num Lock */
		_PM_modifiers ^= KB_NUMLOCK;
		ioctl(_PM_console_fd, KDSETLED, _PM_modifiers & 7);
		break;
	    case 70: /* Scroll Lock */
		_PM_modifiers ^= KB_SCROLL;
		ioctl(_PM_console_fd, KDSETLED, _PM_modifiers & 7);
		break;
	    case 28:
		return 0x1C;
	    default:
		ke.kb_index = c;
		ke.kb_table = 0;
		if ((_PM_modifiers & KB_SHIFT) || (_PM_modifiers & KB_CAPS))
		    ke.kb_table |= K_SHIFTTAB;
		if (_PM_modifiers & KB_ALT)
		    ke.kb_table |= K_ALTTAB;
		ioctl(_PM_console_fd, KDGKBENT, (ulong)&ke);
		c = ke.kb_value & 0xFF;
		return c;
	    }
	}
    return 0;
}

/****************************************************************************
REMARKS:
Sleep until the virtual terminal is active
****************************************************************************/
static void wait_vt_active(
    int _PM_console_fd)
{
    while (ioctl(_PM_console_fd, VT_WAITACTIVE, tty_vc) < 0) {
	if ((errno != EAGAIN) && (errno != EINTR)) {
	    perror("ioctl(VT_WAITACTIVE)");
	    exit(1);
	    }
	usleep(150000);
	}
}

/****************************************************************************
REMARKS:
Checks the owner of the specified virtual console.
****************************************************************************/
static int check_owner(
    int vc)
{
    struct stat sbuf;
    char fname[30];

    sprintf(fname, "/dev/tty%d", vc);
    if ((stat(fname, &sbuf) >= 0) && (getuid() == sbuf.st_uid))
	return 1;
    printf("You must be the owner of the current console to use this program.\n");
    return 0;
}

/****************************************************************************
REMARKS:
Checks if the console is currently in graphics mode, and if so we forcibly
restore it back to text mode again. This handles the case when a Nucleus or
MGL program crashes and leaves the console in graphics mode. Running the
textmode utility (or any other Nucleus/MGL program) via a telnet session
into the machine will restore it back to normal.
****************************************************************************/
static void restore_text_console(
    int console_id)
{
    if (ioctl(console_id, KDSETMODE, KD_TEXT) < 0)
	LOGWARN("ioctl(KDSETMODE) failed");
    _PM_restore_kb_mode();
}

/****************************************************************************
REMARKS:
Opens up the console device for output by finding an appropriate virutal
console that we can run on.
****************************************************************************/
PM_HWND PMAPI PM_openConsole(
    PM_HWND hwndUser,
    int device,
    int xRes,
    int yRes,
    int bpp,
    ibool fullScreen)
{
    struct vt_mode  vtm;
    struct vt_stat  vts;
    struct stat     sbuf;
    char            fname[30];

    /* Check if we have already opened the console */
    if (console_count++)
	return _PM_console_fd;

    /* Now, it would be great if we could use /dev/tty and see what it is
     * connected to. Alas, we cannot find out reliably what VC /dev/tty is
     * bound to. Thus we parse stdin through stderr for a reliable VC.
     */
    startup_vc = 0;
    for (_PM_console_fd = 0; _PM_console_fd < 3; _PM_console_fd++) {
	if (fstat(_PM_console_fd, &sbuf) < 0)
	    continue;
	if (ioctl(_PM_console_fd, VT_GETMODE, &vtm) < 0)
	    continue;
	if ((sbuf.st_rdev & 0xFF00) != 0x400)
	    continue;
	if (!(sbuf.st_rdev & 0xFF))
	    continue;
	tty_vc = sbuf.st_rdev & 0xFF;
	restore_text_console(_PM_console_fd);
	return _PM_console_fd;
	}
    if ((_PM_console_fd = open("/dev/console", O_RDWR)) < 0) {
	printf("open_dev_console: can't open /dev/console \n");
	exit(1);
	}
    if (ioctl(_PM_console_fd, VT_OPENQRY, &tty_vc) < 0)
	goto Error;
    if (tty_vc <= 0)
	goto Error;
    sprintf(fname, "/dev/tty%d", tty_vc);
    close(_PM_console_fd);

    /* Change our control terminal */
    setsid();

    /* We must use RDWR to allow for output... */
    if (((_PM_console_fd = open(fname, O_RDWR)) >= 0) &&
	    (ioctl(_PM_console_fd, VT_GETSTATE, &vts) >= 0)) {
	if (!check_owner(vts.v_active))
	    goto Error;
	restore_text_console(_PM_console_fd);

	/* Success, redirect all stdios */
	fflush(stdin);
	fflush(stdout);
	fflush(stderr);
	close(0);
	close(1);
	close(2);
	dup(_PM_console_fd);
	dup(_PM_console_fd);
	dup(_PM_console_fd);

	/* clear screen and switch to it */
	fwrite("\e[H\e[J", 6, 1, stderr);
	fflush(stderr);
	if (tty_vc != vts.v_active) {
	    startup_vc = vts.v_active;
	    ioctl(_PM_console_fd, VT_ACTIVATE, tty_vc);
	    wait_vt_active(_PM_console_fd);
	    }
	}
    return _PM_console_fd;

Error:
    if (_PM_console_fd > 2)
	close(_PM_console_fd);
    console_count = 0;
    PM_fatalError(
	"Not running in a graphics capable console,\n"
	"and unable to find one.\n");
    return -1;
}

#define FONT_C  0x10000     /* 64KB for font data                       */

/****************************************************************************
REMARKS:
Returns the size of the console state buffer.
****************************************************************************/
int PMAPI PM_getConsoleStateSize(void)
{
    if (!inited)
	PM_init();
    return PM_getVGAStateSize() + FONT_C*2;
}

/****************************************************************************
REMARKS:
Save the state of the Linux console.
****************************************************************************/
void PMAPI PM_saveConsoleState(void *stateBuf,int console_id)
{
    uchar   *regs = stateBuf;

    /* Save the current console font */
    if (ioctl(console_id,GIO_FONT,&regs[PM_getVGAStateSize()]) < 0)
	perror("ioctl(GIO_FONT)");

    /* Inform the Linux console that we are going into graphics mode */
    if (ioctl(console_id, KDSETMODE, KD_GRAPHICS) < 0)
	perror("ioctl(KDSETMODE)");

    /* Save state of VGA registers */
    PM_saveVGAState(stateBuf);
}

void PMAPI PM_setSuspendAppCallback(int (_ASMAPIP saveState)(int flags))
{
    /* TODO: Implement support for allowing console switching! */
}

/****************************************************************************
REMARKS:
Restore the state of the Linux console.
****************************************************************************/
void PMAPI PM_restoreConsoleState(const void *stateBuf,PM_HWND console_id)
{
    const uchar *regs = stateBuf;

    /* Restore the state of the VGA compatible registers */
    PM_restoreVGAState(stateBuf);

    /* Inform the Linux console that we are back from graphics modes */
    if (ioctl(console_id, KDSETMODE, KD_TEXT) < 0)
	LOGWARN("ioctl(KDSETMODE) failed");

    /* Restore the old console font */
    if (ioctl(console_id,PIO_FONT,&regs[PM_getVGAStateSize()]) < 0)
	LOGWARN("ioctl(KDSETMODE) failed");

    /* Coming back from graphics mode on Linux also restored the previous
     * text mode console contents, so we need to clear the screen to get
     * around this since the cursor does not get homed by our code.
     */
    fflush(stdout);
    fflush(stderr);
    printf("\033[H\033[J");
    fflush(stdout);
}

/****************************************************************************
REMARKS:
Close the Linux console and put it back to normal.
****************************************************************************/
void PMAPI PM_closeConsole(PM_HWND _PM_console_fd)
{
    /* Restore console to normal operation */
    if (--console_count == 0) {
	/* Re-activate the original virtual console */
	if (startup_vc > 0)
	    ioctl(_PM_console_fd, VT_ACTIVATE, startup_vc);

	/* Close the console file descriptor */
	if (_PM_console_fd > 2)
	    close(_PM_console_fd);
	_PM_console_fd = -1;
	}
}

void PM_setOSCursorLocation(int x,int y)
{
    /* Nothing to do in here */
}

/****************************************************************************
REMARKS:
Set the screen width and height for the Linux console.
****************************************************************************/
void PM_setOSScreenWidth(int width,int height)
{
    struct winsize  ws;
    struct vt_sizes vs;

    /* Resize the software terminal */
    ws.ws_col = width;
    ws.ws_row = height;
    ioctl(_PM_console_fd, TIOCSWINSZ, &ws);

    /* And the hardware */
    vs.v_rows = height;
    vs.v_cols = width;
    vs.v_scrollsize = 0;
    ioctl(_PM_console_fd, VT_RESIZE, &vs);
}

ibool PMAPI PM_setRealTimeClockHandler(PM_intHandler ih, int frequency)
{
    /* TODO: Implement this for Linux */
    return false;
}

void PMAPI PM_setRealTimeClockFrequency(int frequency)
{
    /* TODO: Implement this for Linux */
}

void PMAPI PM_restoreRealTimeClockHandler(void)
{
    /* TODO: Implement this for Linux */
}

char * PMAPI PM_getCurrentPath(
    char *path,
    int maxLen)
{
    return getcwd(path,maxLen);
}

char PMAPI PM_getBootDrive(void)
{ return '/'; }

const char * PMAPI PM_getVBEAFPath(void)
{ return PM_getNucleusConfigPath(); }

const char * PMAPI PM_getNucleusPath(void)
{
    char *env = getenv("NUCLEUS_PATH");
    return env ? env : "/usr/lib/nucleus";
}

const char * PMAPI PM_getNucleusConfigPath(void)
{
    static char path[256];
    strcpy(path,PM_getNucleusPath());
    PM_backslash(path);
    strcat(path,"config");
    return path;
}

const char * PMAPI PM_getUniqueID(void)
{
    static char buf[128];
    gethostname(buf, 128);
    return buf;
}

const char * PMAPI PM_getMachineName(void)
{
    static char buf[128];
    gethostname(buf, 128);
    return buf;
}

void * PMAPI PM_getBIOSPointer(void)
{
    static uchar *zeroPtr = NULL;
    if (!zeroPtr)
	zeroPtr = PM_mapPhysicalAddr(0,0xFFFFF,true);
    return (void*)(zeroPtr + 0x400);
}

void * PMAPI PM_getA0000Pointer(void)
{
    /* PM_init maps in the 0xA0000 framebuffer region 1:1 with our
     * address mapping, so we can return the address here.
     */
    if (!inited)
	PM_init();
    return (void*)(0xA0000);
}

void * PMAPI PM_mapPhysicalAddr(ulong base,ulong limit,ibool isCached)
{
    uchar   *p;
    ulong   baseAddr,baseOfs;

    if (!inited)
	PM_init();
    if (base >= 0xA0000 && base < 0x100000)
	return (void*)base;
    if (!fd_mem && (fd_mem = open("/dev/mem", O_RDWR)) == -1)
	return NULL;

    /* Round the physical address to a 4Kb boundary and the limit to a
     * 4Kb-1 boundary before passing the values to mmap. If we round the
     * physical address, then we also add an extra offset into the address
     * that we return.
     */
    baseOfs = base & 4095;
    baseAddr = base & ~4095;
    limit = ((limit+baseOfs+1+4095) & ~4095)-1;
    if ((p = mmap(0, limit+1,
	    PROT_READ | PROT_WRITE, MAP_SHARED,
	    fd_mem, baseAddr)) == (void *)-1)
	return NULL;
    return (void*)(p+baseOfs);
}

void PMAPI PM_freePhysicalAddr(void *ptr,ulong limit)
{
    if ((ulong)ptr >= 0x100000)
	munmap(ptr,limit+1);
}

ulong PMAPI PM_getPhysicalAddr(void *p)
{
    /* TODO: This function should find the physical address of a linear */
    /*       address. */
    return 0xFFFFFFFFUL;
}

ibool PMAPI PM_getPhysicalAddrRange(void *p,ulong length,ulong *physAddress)
{
    /* TODO: This function should find a range of physical addresses */
    /*       for a linear address. */
    return false;
}

void PMAPI PM_sleep(ulong milliseconds)
{
    /* TODO: Put the process to sleep for milliseconds */
}

int PMAPI PM_getCOMPort(int port)
{
    /* TODO: Re-code this to determine real values using the Plug and Play */
    /*       manager for the OS. */
    switch (port) {
	case 0: return 0x3F8;
	case 1: return 0x2F8;
	}
    return 0;
}

int PMAPI PM_getLPTPort(int port)
{
    /* TODO: Re-code this to determine real values using the Plug and Play */
    /*       manager for the OS. */
    switch (port) {
	case 0: return 0x3BC;
	case 1: return 0x378;
	case 2: return 0x278;
	}
    return 0;
}

void * PMAPI PM_mallocShared(long size)
{
    return PM_malloc(size);
}

void PMAPI PM_freeShared(void *ptr)
{
    PM_free(ptr);
}

void * PMAPI PM_mapToProcess(void *base,ulong limit)
{ return (void*)base; }

void * PMAPI PM_mapRealPointer(uint r_seg,uint r_off)
{
    /* PM_init maps in the 0xA0000-0x100000 region 1:1 with our
     * address mapping, as well as all memory blocks in a 1:1 address
     * mapping so we can simply return the physical address in here.
     */
    if (!inited)
	PM_init();
    return (void*)MK_PHYS(r_seg,r_off);
}

void * PMAPI PM_allocRealSeg(uint size,uint *r_seg,uint *r_off)
{
    int     i;
    char    *r = (char *)REAL_MEM_BASE;

    if (!inited)
	PM_init();
    if (!mem_info.ready)
	return NULL;
    if (mem_info.count == REAL_MEM_BLOCKS)
	return NULL;
    size = (size + 15) & ~15;
    for (i = 0; i < mem_info.count; i++) {
	if (mem_info.blocks[i].free && size < mem_info.blocks[i].size) {
	    insert_block(i);
	    mem_info.blocks[i].size = size;
	    mem_info.blocks[i].free = 0;
	    mem_info.blocks[i + 1].size -= size;
	    *r_seg = (uint)(r) >> 4;
	    *r_off = (uint)(r) & 0xF;
	    return (void *)r;
	    }
	r += mem_info.blocks[i].size;
	}
    return NULL;
}

void PMAPI PM_freeRealSeg(void *mem)
{
    int     i;
    char    *r = (char *)REAL_MEM_BASE;

    if (!mem_info.ready)
	return;
    i = 0;
    while (mem != (void *)r) {
	r += mem_info.blocks[i].size;
	i++;
	if (i == mem_info.count)
	    return;
	}
    mem_info.blocks[i].free = 1;
    if (i + 1 < mem_info.count && mem_info.blocks[i + 1].free) {
	mem_info.blocks[i].size += mem_info.blocks[i + 1].size;
	delete_block(i + 1);
	}
    if (i - 1 >= 0 && mem_info.blocks[i - 1].free) {
	mem_info.blocks[i - 1].size += mem_info.blocks[i].size;
	delete_block(i);
	}
}

#define DIRECTION_FLAG  (1 << 10)

static void em_ins(int size)
{
    unsigned int edx, edi;

    edx = context.vm.regs.edx & 0xffff;
    edi = context.vm.regs.edi & 0xffff;
    edi += (unsigned int)context.vm.regs.ds << 4;
    if (context.vm.regs.eflags & DIRECTION_FLAG) {
	if (size == 4)
	    asm volatile ("std; insl; cld"
	     : "=D" (edi) : "d" (edx), "0" (edi));
	else if (size == 2)
	    asm volatile ("std; insw; cld"
	     : "=D" (edi) : "d" (edx), "0" (edi));
	else
	    asm volatile ("std; insb; cld"
	     : "=D" (edi) : "d" (edx), "0" (edi));
	}
    else {
	if (size == 4)
	    asm volatile ("cld; insl"
	     : "=D" (edi) : "d" (edx), "0" (edi));
	else if (size == 2)
	    asm volatile ("cld; insw"
	     : "=D" (edi) : "d" (edx), "0" (edi));
	else
	    asm volatile ("cld; insb"
	     : "=D" (edi) : "d" (edx), "0" (edi));
	}
    edi -= (unsigned int)context.vm.regs.ds << 4;
    context.vm.regs.edi &= 0xffff0000;
    context.vm.regs.edi |= edi & 0xffff;
}

static void em_rep_ins(int size)
{
    unsigned int ecx, edx, edi;

    ecx = context.vm.regs.ecx & 0xffff;
    edx = context.vm.regs.edx & 0xffff;
    edi = context.vm.regs.edi & 0xffff;
    edi += (unsigned int)context.vm.regs.ds << 4;
    if (context.vm.regs.eflags & DIRECTION_FLAG) {
	if (size == 4)
	    asm volatile ("std; rep; insl; cld"
	     : "=D" (edi), "=c" (ecx)
	     : "d" (edx), "0" (edi), "1" (ecx));
	else if (size == 2)
	    asm volatile ("std; rep; insw; cld"
	     : "=D" (edi), "=c" (ecx)
	     : "d" (edx), "0" (edi), "1" (ecx));
	else
	    asm volatile ("std; rep; insb; cld"
	     : "=D" (edi), "=c" (ecx)
	     : "d" (edx), "0" (edi), "1" (ecx));
	}
    else {
	if (size == 4)
	    asm volatile ("cld; rep; insl"
	     : "=D" (edi), "=c" (ecx)
	     : "d" (edx), "0" (edi), "1" (ecx));
	else if (size == 2)
	    asm volatile ("cld; rep; insw"
	     : "=D" (edi), "=c" (ecx)
	     : "d" (edx), "0" (edi), "1" (ecx));
	else
	    asm volatile ("cld; rep; insb"
	     : "=D" (edi), "=c" (ecx)
	     : "d" (edx), "0" (edi), "1" (ecx));
	}

    edi -= (unsigned int)context.vm.regs.ds << 4;
    context.vm.regs.edi &= 0xffff0000;
    context.vm.regs.edi |= edi & 0xffff;
    context.vm.regs.ecx &= 0xffff0000;
    context.vm.regs.ecx |= ecx & 0xffff;
}

static void em_outs(int size)
{
    unsigned int edx, esi;

    edx = context.vm.regs.edx & 0xffff;
    esi = context.vm.regs.esi & 0xffff;
    esi += (unsigned int)context.vm.regs.ds << 4;
    if (context.vm.regs.eflags & DIRECTION_FLAG) {
	if (size == 4)
	    asm volatile ("std; outsl; cld"
	     : "=S" (esi) : "d" (edx), "0" (esi));
	else if (size == 2)
	    asm volatile ("std; outsw; cld"
	     : "=S" (esi) : "d" (edx), "0" (esi));
	else
	    asm volatile ("std; outsb; cld"
	     : "=S" (esi) : "d" (edx), "0" (esi));
	}
    else {
	if (size == 4)
	    asm volatile ("cld; outsl"
	     : "=S" (esi) : "d" (edx), "0" (esi));
	else if (size == 2)
	    asm volatile ("cld; outsw"
	     : "=S" (esi) : "d" (edx), "0" (esi));
	else
	    asm volatile ("cld; outsb"
	     : "=S" (esi) : "d" (edx), "0" (esi));
	}

    esi -= (unsigned int)context.vm.regs.ds << 4;
    context.vm.regs.esi &= 0xffff0000;
    context.vm.regs.esi |= esi & 0xffff;
}

static void em_rep_outs(int size)
{
    unsigned int ecx, edx, esi;

    ecx = context.vm.regs.ecx & 0xffff;
    edx = context.vm.regs.edx & 0xffff;
    esi = context.vm.regs.esi & 0xffff;
    esi += (unsigned int)context.vm.regs.ds << 4;
    if (context.vm.regs.eflags & DIRECTION_FLAG) {
	if (size == 4)
	    asm volatile ("std; rep; outsl; cld"
	     : "=S" (esi), "=c" (ecx)
	     : "d" (edx), "0" (esi), "1" (ecx));
	else if (size == 2)
	    asm volatile ("std; rep; outsw; cld"
	     : "=S" (esi), "=c" (ecx)
	     : "d" (edx), "0" (esi), "1" (ecx));
	else
	    asm volatile ("std; rep; outsb; cld"
	     : "=S" (esi), "=c" (ecx)
	     : "d" (edx), "0" (esi), "1" (ecx));
	}
    else {
	if (size == 4)
	    asm volatile ("cld; rep; outsl"
	     : "=S" (esi), "=c" (ecx)
	     : "d" (edx), "0" (esi), "1" (ecx));
	else if (size == 2)
	    asm volatile ("cld; rep; outsw"
	     : "=S" (esi), "=c" (ecx)
	     : "d" (edx), "0" (esi), "1" (ecx));
	else
	    asm volatile ("cld; rep; outsb"
	     : "=S" (esi), "=c" (ecx)
	     : "d" (edx), "0" (esi), "1" (ecx));
	}

    esi -= (unsigned int)context.vm.regs.ds << 4;
    context.vm.regs.esi &= 0xffff0000;
    context.vm.regs.esi |= esi & 0xffff;
    context.vm.regs.ecx &= 0xffff0000;
    context.vm.regs.ecx |= ecx & 0xffff;
}

static int emulate(void)
{
    unsigned char *insn;
    struct {
	unsigned int size : 1;
	unsigned int rep : 1;
	} prefix = { 0, 0 };
    int i = 0;

    insn = (unsigned char *)((unsigned int)context.vm.regs.cs << 4);
    insn += context.vm.regs.eip;

    while (1) {
#ifdef TRACE_IO
	traceAddr = ((ulong)context.vm.regs.cs << 16) + context.vm.regs.eip + i;
#endif
	if (insn[i] == 0x66) {
	    prefix.size = 1 - prefix.size;
	    i++;
	    }
	else if (insn[i] == 0xf3) {
	    prefix.rep = 1;
	    i++;
	    }
	else if (insn[i] == 0xf0 || insn[i] == 0xf2
	     || insn[i] == 0x26 || insn[i] == 0x2e
	     || insn[i] == 0x36 || insn[i] == 0x3e
	     || insn[i] == 0x64 || insn[i] == 0x65
	     || insn[i] == 0x67) {
	    /* these prefixes are just ignored */
	    i++;
	    }
	else if (insn[i] == 0x6c) {
	    if (prefix.rep)
		em_rep_ins(1);
	    else
		em_ins(1);
	    i++;
	    break;
	    }
	else if (insn[i] == 0x6d) {
	    if (prefix.rep) {
		if (prefix.size)
		    em_rep_ins(4);
		else
		    em_rep_ins(2);
		}
	    else {
		if (prefix.size)
		    em_ins(4);
		else
		    em_ins(2);
		}
	    i++;
	    break;
	    }
	else if (insn[i] == 0x6e) {
	    if (prefix.rep)
		em_rep_outs(1);
	    else
		em_outs(1);
	    i++;
	    break;
	    }
	else if (insn[i] == 0x6f) {
	    if (prefix.rep) {
		if (prefix.size)
		    em_rep_outs(4);
		else
		    em_rep_outs(2);
		}
	    else {
		if (prefix.size)
		    em_outs(4);
		else
		    em_outs(2);
		}
	    i++;
	    break;
	    }
	else if (insn[i] == 0xec) {
	    *((uchar*)&context.vm.regs.eax) = port_in(context.vm.regs.edx);
	    i++;
	    break;
	    }
	else if (insn[i] == 0xed) {
	    if (prefix.size)
		*((ulong*)&context.vm.regs.eax) = port_inl(context.vm.regs.edx);
	    else
		*((ushort*)&context.vm.regs.eax) = port_inw(context.vm.regs.edx);
	    i++;
	    break;
	    }
	else if (insn[i] == 0xee) {
	    port_out(context.vm.regs.eax,context.vm.regs.edx);
	    i++;
	    break;
	    }
	else if (insn[i] == 0xef) {
	    if (prefix.size)
		port_outl(context.vm.regs.eax,context.vm.regs.edx);
	    else
		port_outw(context.vm.regs.eax,context.vm.regs.edx);
	    i++;
	    break;
	    }
	else
	    return 0;
	}

    context.vm.regs.eip += i;
    return 1;
}

static void debug_info(int vret)
{
    int i;
    unsigned char *p;

    fputs("vm86() failed\n", stderr);
    fprintf(stderr, "return = 0x%x\n", vret);
    fprintf(stderr, "eax = 0x%08lx\n", context.vm.regs.eax);
    fprintf(stderr, "ebx = 0x%08lx\n", context.vm.regs.ebx);
    fprintf(stderr, "ecx = 0x%08lx\n", context.vm.regs.ecx);
    fprintf(stderr, "edx = 0x%08lx\n", context.vm.regs.edx);
    fprintf(stderr, "esi = 0x%08lx\n", context.vm.regs.esi);
    fprintf(stderr, "edi = 0x%08lx\n", context.vm.regs.edi);
    fprintf(stderr, "ebp = 0x%08lx\n", context.vm.regs.ebp);
    fprintf(stderr, "eip = 0x%08lx\n", context.vm.regs.eip);
    fprintf(stderr, "cs  = 0x%04x\n", context.vm.regs.cs);
    fprintf(stderr, "esp = 0x%08lx\n", context.vm.regs.esp);
    fprintf(stderr, "ss  = 0x%04x\n", context.vm.regs.ss);
    fprintf(stderr, "ds  = 0x%04x\n", context.vm.regs.ds);
    fprintf(stderr, "es  = 0x%04x\n", context.vm.regs.es);
    fprintf(stderr, "fs  = 0x%04x\n", context.vm.regs.fs);
    fprintf(stderr, "gs  = 0x%04x\n", context.vm.regs.gs);
    fprintf(stderr, "eflags  = 0x%08lx\n", context.vm.regs.eflags);
    fputs("cs:ip = [ ", stderr);
    p = (unsigned char *)((context.vm.regs.cs << 4) + (context.vm.regs.eip & 0xffff));
    for (i = 0; i < 16; ++i)
	    fprintf(stderr, "%02x ", (unsigned int)p[i]);
    fputs("]\n", stderr);
    fflush(stderr);
}

static int run_vm86(void)
{
    unsigned int vret;

    for (;;) {
	vret = vm86(&context.vm);
	if (VM86_TYPE(vret) == VM86_INTx) {
	    unsigned int v = VM86_ARG(vret);
	    if (v == RETURN_TO_32_INT)
		return 1;
	    pushw(context.vm.regs.eflags);
	    pushw(context.vm.regs.cs);
	    pushw(context.vm.regs.eip);
	    context.vm.regs.cs = get_int_seg(v);
	    context.vm.regs.eip = get_int_off(v);
	    context.vm.regs.eflags &= ~(VIF_MASK | TF_MASK);
	    continue;
	    }
	if (VM86_TYPE(vret) != VM86_UNKNOWN)
	    break;
	if (!emulate())
	    break;
	}
    debug_info(vret);
    return 0;
}

#define IND(ereg) context.vm.regs.ereg = regs->ereg
#define OUTD(ereg) regs->ereg = context.vm.regs.ereg

void PMAPI DPMI_int86(int intno, DPMI_regs *regs)
{
    if (!inited)
	PM_init();
    memset(&context.vm.regs, 0, sizeof(context.vm.regs));
    IND(eax); IND(ebx); IND(ecx); IND(edx); IND(esi); IND(edi);
    context.vm.regs.eflags = DEFAULT_VM86_FLAGS;
    context.vm.regs.cs = get_int_seg(intno);
    context.vm.regs.eip = get_int_off(intno);
    context.vm.regs.ss = context.stack_seg;
    context.vm.regs.esp = context.stack_off;
    pushw(DEFAULT_VM86_FLAGS);
    pushw(context.ret_seg);
    pushw(context.ret_off);
    run_vm86();
    OUTD(eax); OUTD(ebx); OUTD(ecx); OUTD(edx); OUTD(esi); OUTD(edi);
    regs->flags = context.vm.regs.eflags;
}

#define IN(ereg) context.vm.regs.ereg = in->e.ereg
#define OUT(ereg) out->e.ereg = context.vm.regs.ereg

int PMAPI PM_int86(int intno, RMREGS *in, RMREGS *out)
{
    if (!inited)
	PM_init();
    memset(&context.vm.regs, 0, sizeof(context.vm.regs));
    IN(eax); IN(ebx); IN(ecx); IN(edx); IN(esi); IN(edi);
    context.vm.regs.eflags = DEFAULT_VM86_FLAGS;
    context.vm.regs.cs = get_int_seg(intno);
    context.vm.regs.eip = get_int_off(intno);
    context.vm.regs.ss = context.stack_seg;
    context.vm.regs.esp = context.stack_off;
    pushw(DEFAULT_VM86_FLAGS);
    pushw(context.ret_seg);
    pushw(context.ret_off);
    run_vm86();
    OUT(eax); OUT(ebx); OUT(ecx); OUT(edx); OUT(esi); OUT(edi);
    out->x.cflag = context.vm.regs.eflags & 1;
    return out->x.ax;
}

int PMAPI PM_int86x(int intno, RMREGS *in, RMREGS *out,
    RMSREGS *sregs)
{
    if (!inited)
	PM_init();
    if (intno == 0x21) {
	time_t today = time(NULL);
	struct tm *t;
	t = localtime(&today);
	out->x.cx = t->tm_year + 1900;
	out->h.dh = t->tm_mon + 1;
	out->h.dl = t->tm_mday;
	}
    else {
	unsigned int seg, off;
	seg = get_int_seg(intno);
	off = get_int_off(intno);
	memset(&context.vm.regs, 0, sizeof(context.vm.regs));
	IN(eax); IN(ebx); IN(ecx); IN(edx); IN(esi); IN(edi);
	context.vm.regs.eflags = DEFAULT_VM86_FLAGS;
	context.vm.regs.cs = seg;
	context.vm.regs.eip = off;
	context.vm.regs.es = sregs->es;
	context.vm.regs.ds = sregs->ds;
	context.vm.regs.fs = sregs->fs;
	context.vm.regs.gs = sregs->gs;
	context.vm.regs.ss = context.stack_seg;
	context.vm.regs.esp = context.stack_off;
	pushw(DEFAULT_VM86_FLAGS);
	pushw(context.ret_seg);
	pushw(context.ret_off);
	run_vm86();
	OUT(eax); OUT(ebx); OUT(ecx); OUT(edx); OUT(esi); OUT(edi);
	sregs->es = context.vm.regs.es;
	sregs->ds = context.vm.regs.ds;
	sregs->fs = context.vm.regs.fs;
	sregs->gs = context.vm.regs.gs;
	out->x.cflag = context.vm.regs.eflags & 1;
	}
    return out->e.eax;
}

#define OUTR(ereg) in->e.ereg = context.vm.regs.ereg

void PMAPI PM_callRealMode(uint seg,uint off, RMREGS *in,
    RMSREGS *sregs)
{
    if (!inited)
	PM_init();
    memset(&context.vm.regs, 0, sizeof(context.vm.regs));
    IN(eax); IN(ebx); IN(ecx); IN(edx); IN(esi); IN(edi);
    context.vm.regs.eflags = DEFAULT_VM86_FLAGS;
    context.vm.regs.cs = seg;
    context.vm.regs.eip = off;
    context.vm.regs.ss = context.stack_seg;
    context.vm.regs.esp = context.stack_off;
    context.vm.regs.es = sregs->es;
    context.vm.regs.ds = sregs->ds;
    context.vm.regs.fs = sregs->fs;
    context.vm.regs.gs = sregs->gs;
    pushw(DEFAULT_VM86_FLAGS);
    pushw(context.ret_seg);
    pushw(context.ret_off);
    run_vm86();
    OUTR(eax); OUTR(ebx); OUTR(ecx); OUTR(edx); OUTR(esi); OUTR(edi);
    sregs->es = context.vm.regs.es;
    sregs->ds = context.vm.regs.ds;
    sregs->fs = context.vm.regs.fs;
    sregs->gs = context.vm.regs.gs;
    in->x.cflag = context.vm.regs.eflags & 1;
}

void PMAPI PM_availableMemory(ulong *physical,ulong *total)
{
    FILE    *mem = fopen("/proc/meminfo","r");
    char    buf[1024];

    fgets(buf,1024,mem);
    fgets(buf,1024,mem);
    sscanf(buf,"Mem: %*d %*d %ld", physical);
    fgets(buf,1024,mem);
    sscanf(buf,"Swap: %*d %*d %ld", total);
    fclose(mem);
    *total += *physical;
}

void * PMAPI PM_allocLockedMem(uint size,ulong *physAddr,ibool contiguous,ibool below16M)
{
    /* TODO: Implement this for Linux */
    return NULL;
}

void PMAPI PM_freeLockedMem(void *p,uint size,ibool contiguous)
{
    /* TODO: Implement this for Linux */
}

void * PMAPI PM_allocPage(
    ibool locked)
{
    /* TODO: Implement this for Linux */
    return NULL;
}

void PMAPI PM_freePage(
    void *p)
{
    /* TODO: Implement this for Linux */
}

void PMAPI PM_setBankA(int bank)
{
    if (!inited)
	PM_init();
    memset(&context.vm.regs, 0, sizeof(context.vm.regs));
    context.vm.regs.eax = 0x4F05;
    context.vm.regs.ebx = 0x0000;
    context.vm.regs.edx = bank;
    context.vm.regs.eflags = DEFAULT_VM86_FLAGS;
    context.vm.regs.cs = get_int_seg(0x10);
    context.vm.regs.eip = get_int_off(0x10);
    context.vm.regs.ss = context.stack_seg;
    context.vm.regs.esp = context.stack_off;
    pushw(DEFAULT_VM86_FLAGS);
    pushw(context.ret_seg);
    pushw(context.ret_off);
    run_vm86();
}

void PMAPI PM_setBankAB(int bank)
{
    if (!inited)
	PM_init();
    memset(&context.vm.regs, 0, sizeof(context.vm.regs));
    context.vm.regs.eax = 0x4F05;
    context.vm.regs.ebx = 0x0000;
    context.vm.regs.edx = bank;
    context.vm.regs.eflags = DEFAULT_VM86_FLAGS;
    context.vm.regs.cs = get_int_seg(0x10);
    context.vm.regs.eip = get_int_off(0x10);
    context.vm.regs.ss = context.stack_seg;
    context.vm.regs.esp = context.stack_off;
    pushw(DEFAULT_VM86_FLAGS);
    pushw(context.ret_seg);
    pushw(context.ret_off);
    run_vm86();
    context.vm.regs.eax = 0x4F05;
    context.vm.regs.ebx = 0x0001;
    context.vm.regs.edx = bank;
    context.vm.regs.eflags = DEFAULT_VM86_FLAGS;
    context.vm.regs.cs = get_int_seg(0x10);
    context.vm.regs.eip = get_int_off(0x10);
    context.vm.regs.ss = context.stack_seg;
    context.vm.regs.esp = context.stack_off;
    pushw(DEFAULT_VM86_FLAGS);
    pushw(context.ret_seg);
    pushw(context.ret_off);
    run_vm86();
}

void PMAPI PM_setCRTStart(int x,int y,int waitVRT)
{
    if (!inited)
	PM_init();
    memset(&context.vm.regs, 0, sizeof(context.vm.regs));
    context.vm.regs.eax = 0x4F07;
    context.vm.regs.ebx = waitVRT;
    context.vm.regs.ecx = x;
    context.vm.regs.edx = y;
    context.vm.regs.eflags = DEFAULT_VM86_FLAGS;
    context.vm.regs.cs = get_int_seg(0x10);
    context.vm.regs.eip = get_int_off(0x10);
    context.vm.regs.ss = context.stack_seg;
    context.vm.regs.esp = context.stack_off;
    pushw(DEFAULT_VM86_FLAGS);
    pushw(context.ret_seg);
    pushw(context.ret_off);
    run_vm86();
}

int PMAPI PM_enableWriteCombine(ulong base,ulong length,uint type)
{
#ifdef ENABLE_MTRR
    struct mtrr_sentry sentry;

    if (mtrr_fd < 0)
	return PM_MTRR_ERR_NO_OS_SUPPORT;
    sentry.base = base;
    sentry.size = length;
    sentry.type = type;
    if (ioctl(mtrr_fd, MTRRIOC_ADD_ENTRY, &sentry) == -1) {
	/* TODO: Need to decode MTRR error codes!! */
	return PM_MTRR_NOT_SUPPORTED;
	}
    return PM_MTRR_ERR_OK;
#else
    return PM_MTRR_ERR_NO_OS_SUPPORT;
#endif
}

/****************************************************************************
PARAMETERS:
callback    - Function to callback with write combine information

REMARKS:
Function to enumerate all write combine regions currently enabled for the
processor.
****************************************************************************/
int PMAPI PM_enumWriteCombine(
    PM_enumWriteCombine_t callback)
{
#ifdef ENABLE_MTRR
    struct mtrr_gentry gentry;

    if (mtrr_fd < 0)
	return PM_MTRR_ERR_NO_OS_SUPPORT;

    for (gentry.regnum = 0; ioctl (mtrr_fd, MTRRIOC_GET_ENTRY, &gentry) == 0;
	 ++gentry.regnum) {
	if (gentry.size > 0) {
	    /* WARNING: This code assumes that the types in pmapi.h match the ones */
	    /* in the Linux kernel (mtrr.h) */
	    callback(gentry.base, gentry.size, gentry.type);
	}
    }

    return PM_MTRR_ERR_OK;
#else
    return PM_MTRR_ERR_NO_OS_SUPPORT;
#endif
}

ibool PMAPI PM_doBIOSPOST(
    ushort axVal,
    ulong BIOSPhysAddr,
    void *copyOfBIOS,
    ulong BIOSLen)
{
    char        *bios_ptr = (char*)0xC0000;
    char        *old_bios;
    ulong       Current10, Current6D, *rvec = 0;
    RMREGS      regs;
    RMSREGS     sregs;

    /* The BIOS is mapped to 0xC0000 with a private memory mapping enabled
     * which means we have a copy on write scheme. Hence we simply copy
     * the secondary BIOS image over the top of the old one.
     */
    if (!inited)
	PM_init();
    if ((old_bios = PM_malloc(BIOSLen)) == NULL)
	return false;
    if (BIOSPhysAddr != 0xC0000) {
	memcpy(old_bios,bios_ptr,BIOSLen);
	memcpy(bios_ptr,copyOfBIOS,BIOSLen);
	}

    /* The interrupt vectors should already be mmap()'ed from 0-0x400 in PM_init */
    Current10 = rvec[0x10];
    Current6D = rvec[0x6D];

    /* POST the secondary BIOS */
    rvec[0x10] = rvec[0x42]; /* Restore int 10h to STD-BIOS */
    regs.x.ax = axVal;
    PM_callRealMode(0xC000,0x0003,&regs,&sregs);

    /* Restore interrupt vectors */
    rvec[0x10] = Current10;
    rvec[0x6D] = Current6D;

    /* Restore original BIOS image */
    if (BIOSPhysAddr != 0xC0000)
	memcpy(bios_ptr,old_bios,BIOSLen);
    PM_free(old_bios);
    return true;
}

int PMAPI PM_lockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    p = p;  len = len;
    return 1;
}

int PMAPI PM_unlockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    p = p;  len = len;
    return 1;
}

int PMAPI PM_lockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    p = p;  len = len;
    return 1;
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    p = p;  len = len;
    return 1;
}

PM_MODULE PMAPI PM_loadLibrary(
    const char *szDLLName)
{
    /* TODO: Implement this to load shared libraries! */
    (void)szDLLName;
    return NULL;
}

void * PMAPI PM_getProcAddress(
    PM_MODULE hModule,
    const char *szProcName)
{
    /* TODO: Implement this! */
    (void)hModule;
    (void)szProcName;
    return NULL;
}

void PMAPI PM_freeLibrary(
    PM_MODULE hModule)
{
    /* TODO: Implement this! */
    (void)hModule;
}

int PMAPI PM_setIOPL(
    int level)
{
    /* TODO: Move the IOPL switching into this function!! */
    return level;
}

void PMAPI PM_flushTLB(void)
{
    /* Do nothing on Linux. */
}
