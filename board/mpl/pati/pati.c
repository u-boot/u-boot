/*
 * (C) Copyright 2003
 * Martin Winistoerfer, martinwinistoerfer@gmx.ch.
 * Atapted for PATI
 * Denis Peter, d.peter@mpl.ch
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/***********************************************************************************
 * Bits for the SDRAM controller
 * -----------------------------
 *
 * CAL:	CAS Latency. If cleared to 0 (default) the SDRAM controller asserts TA# on
 *	the 2nd Clock after ACTIVE command (CAS Latency = 2). If set to 1 the SDRAM
 *	controller asserts TA# on the 3rd Clock after ACTIVE command (CAS Latency = 3).
 * RCD:	RCD ACTIVE to READ or WRITE Delay (Ras to Cas Delay). If cleared 0 (default)
 *	tRCD of the SDRAM must equal or less 25ns. If set to 1 tRCD must be equal or less 50ns.
 * WREC:Write Recovery. If cleared 0 (default) tWR of the SDRAM must equal or less 25ns.
 *	If set to 1 tWR must be equal or less 50ns.
 * RP:	Precharge Command Time. If cleared 0 (default) tRP of the SDRAM must equal or less
 *	25ns. If set to 1 tRP must be equal or less 50ns.
 * RC:	Auto Refresh to Active Time. If cleared 0 (default) tRC of the SDRAM must equal
 *	or less 75ns. If set to 1 tRC must be equal or less 100ns.
 * LMR:	Bit to set the Mode Register of the SDRAM. If set, the next access to the SDRAM
 *	is the Load Mode Register Command.
 * IIP:	Init in progress. Set to 1 for starting the init sequence
 *	(Precharge All). As long this bit is set, the Precharge All is still in progress.
 *	After command has completed, wait at least for 8 refresh (200usec) before proceed.
 **********************************************************************************/

#include <common.h>
#include <mpc5xx.h>
#include <devices.h>
#include <pci_ids.h>
#define PLX9056_LOC
#include "plx9056.h"
#include "pati.h"

#if defined(__APPLE__)
/* Leading underscore on symbols */
#  define SYM_CHAR "_"
#else /* No leading character on symbols */
#  define SYM_CHAR
#endif

#undef SDRAM_DEBUG
/*
 * Macros to generate global absolutes.
 */
#define GEN_SYMNAME(str) SYM_CHAR #str
#define GEN_VALUE(str) #str
#define GEN_ABS(name, value) \
		asm (".globl " GEN_SYMNAME(name)); \
		asm (GEN_SYMNAME(name) " = " GEN_VALUE(value))


/************************************************************************
 * Early debug routines
 */
void write_hex (unsigned char i)
{
	char cc;

	cc = i >> 4;
	cc &= 0xf;
	if (cc > 9)
		serial_putc (cc + 55);
	else
		serial_putc (cc + 48);
	cc = i & 0xf;
	if (cc > 9)
		serial_putc (cc + 55);
	else
		serial_putc (cc + 48);
}

#if defined(SDRAM_DEBUG)

void write_4hex (unsigned long val)
{
	write_hex ((unsigned char) (val >> 24));
	write_hex ((unsigned char) (val >> 16));
	write_hex ((unsigned char) (val >> 8));
	write_hex ((unsigned char) val);
}

#endif

unsigned long in32(unsigned long addr)
{
	unsigned long *p=(unsigned long *)addr;
	return *p;
}

void out32(unsigned long addr,unsigned long data)
{
	unsigned long *p=(unsigned long *)addr;
	*p=data;
}

typedef struct {
	unsigned short boardtype; /* Board revision and Population Options */
	unsigned char cal;		/* cas Latency  0:CAL=2 1:CAL=3 */
	unsigned char rcd;		/* ras to cas delay  0:<25ns 1:<50ns*/
	unsigned char wrec;		/* write recovery 0:<25ns 1:<50ns */
	unsigned char pr;		/* Precharge Command Time 0:<25ns 1:<50ns */
	unsigned char rc;		/* Auto Refresh to Active Time 0:<75ns 1:<100ns */
	unsigned char sz;		/* log binary => Size = (4MByte<<sz) 5 = 128, 4 = 64, 3 = 32, 2 = 16, 1=8 */
} sdram_t;

const sdram_t sdram_table[] = {
	{ 0x0000,	/* PATI Rev A, 16MByte -1 Board */
		1,	/* Case Latenty = 3 */
		0,	/* ras to cas delay  0 (20ns) */
		0,	/* write recovery 0:<25ns 1:<50ns*/
		0,	/* Precharge Command Time 0 (20ns) */
		0,	/* Auto Refresh to Active Time 0 (68) */
		2	/* log binary => Size 2 = 16MByte, 1=8 */
	},
	{ 0xffff, /* terminator */
	  0xff,
	  0xff,
	  0xff,
	  0xff,
	  0xff,
	  0xff }
};


extern int mem_test (unsigned long start, unsigned long ramsize, int quiet);
extern void mem_test_reloc(void);

/*
 * Get RAM size.
 */
long int initdram(int board_type)
{
	unsigned char board_rev;
	unsigned long reg;
	unsigned long lmr;
	int i,timeout;

#if defined(SDRAM_DEBUG)
	reg=in32(PLD_CONFIG_BASE+PLD_PART_ID);
	puts("\n\nSYSTEM part 0x"); write_4hex(SYSCNTR_PART(reg));
	puts(" Vers 0x"); write_4hex(SYSCNTR_ID(reg));
	puts("\nSDRAM  part  0x"); write_4hex(SDRAM_PART(reg));
	puts(" Vers 0x"); write_4hex(SDRAM_ID(reg));
	reg=in32(PLD_CONFIG_BASE+PLD_BOARD_TIMING);
	puts("\nBoard rev.   0x"); write_4hex(SYSCNTR_BREV(reg));
   putc('\n');
#endif
	reg=in32(PLD_CONFIG_BASE+PLD_BOARD_TIMING);
	board_rev=(unsigned char)(SYSCNTR_BREV(reg));
	i=0;
	while(1) {
		if(sdram_table[i].boardtype==0xffff) {
			puts("ERROR, found no table for Board 0x");
			write_hex(board_rev);
			while(1);
		}
		if(sdram_table[i].boardtype==(unsigned char)board_rev)
			break;
		i++;
	}
	/* Set CAL, RCD, WREQ, PR and RC Bits */
#if defined(SDRAM_DEBUG)
	puts("Set CAL, RCD, WREQ, PR and RC Bits\n");
#endif
	/* mask bits */
	reg &= ~(SET_REG_BIT(1,SDRAM_CAL) | SET_REG_BIT(1,SDRAM_RCD) | SET_REG_BIT(1,SDRAM_WREQ) |
				SET_REG_BIT(1,SDRAM_PR)  |  SET_REG_BIT(1,SDRAM_RC) | SET_REG_BIT(1,SDRAM_LMR)  |
				SET_REG_BIT(1,SDRAM_IIP) | SET_REG_BIT(1,SDRAM_RES0));
	/* set bits */
	reg |= (SET_REG_BIT(sdram_table[i].cal,SDRAM_CAL) |
			  SET_REG_BIT(sdram_table[i].rcd,SDRAM_RCD) |
			  SET_REG_BIT(sdram_table[i].wrec,SDRAM_WREQ) |
			  SET_REG_BIT(sdram_table[i].pr,SDRAM_PR) |
			  SET_REG_BIT(sdram_table[i].rc,SDRAM_RC));

	out32(PLD_CONFIG_BASE+PLD_BOARD_TIMING,reg);
	/* step 2 set IIP */
#if defined(SDRAM_DEBUG)
	puts("step 2 set IIP\n");
#endif
	/* step 2 set IIP */
	reg |= SET_REG_BIT(1,SDRAM_IIP);
	timeout=0;
	while (timeout!=0xffff) {
		__asm__ volatile("eieio");
		reg=in32(PLD_CONFIG_BASE+PLD_BOARD_TIMING);
		if((reg & SET_REG_BIT(1,SDRAM_IIP))==0)
			break;
		timeout++;
		udelay(1);
	}
	/* wait for at least 8 refresh */
	udelay(1000);
	/* set LMR */
	reg |= SET_REG_BIT(1,SDRAM_LMR);
	out32(PLD_CONFIG_BASE+PLD_BOARD_TIMING,reg);
	__asm__ volatile("eieio");
	lmr=0x00000002; /* sequential burst 4 data */
	if(sdram_table[i].cal==1)
		lmr|=0x00000030; /* cal = 3 */
	else
		lmr|=0000000020; /* cal = 2 */
	/* rest standard operation programmed write burst length */
	/* we have a x32 bit bus to the SDRAM, so shift the addr with 2 */
	lmr<<=2;
	in32(CFG_SDRAM_BASE + lmr);
	/* ok, we're done, return SDRAM size */
	return ((0x400000 << sdram_table[i].sz));		/* log2 value of 4MByte  */
}


void set_flash_vpp(int ext_vpp, int ext_wp, int int_vpp)
{
	unsigned long reg;
	reg=in32(PLD_CONF_REG2+PLD_CONFIG_BASE);
	reg &= ~(SET_REG_BIT(1,SYSCNTR_CPU_VPP) |
			   SET_REG_BIT(1,SYSCNTR_FL_VPP) |
				SET_REG_BIT(1,SYSCNTR_FL_WP));

	reg |= (SET_REG_BIT(int_vpp,SYSCNTR_CPU_VPP) |
			   SET_REG_BIT(ext_vpp,SYSCNTR_FL_VPP) |
				SET_REG_BIT(ext_wp,SYSCNTR_FL_WP));
	out32(PLD_CONF_REG2+PLD_CONFIG_BASE,reg);
	udelay(100);
}


void show_pld_regs(void)
{
	unsigned long reg,reg1;
	reg=in32(PLD_CONFIG_BASE+PLD_PART_ID);
	printf("\nSYSTEM part %ld, Vers %ld\n",SYSCNTR_PART(reg),SYSCNTR_ID(reg));
	printf("SDRAM  part %ld, Vers %ld\n",SDRAM_PART(reg),SDRAM_ID(reg));
	reg=in32(PLD_CONFIG_BASE+PLD_BOARD_TIMING);
	printf("Board rev.  %c\n",(char) (SYSCNTR_BREV(reg)+'A'));
	printf("Waitstates  %ld\n",GET_SYSCNTR_FLWAIT(reg));
	printf("SDRAM:      CAL=%ld RCD=%ld WREQ=%ld PR=%ld\n            RC=%ld  LMR=%ld IIP=%ld\n",
		GET_REG_BIT(reg,SDRAM_CAL),GET_REG_BIT(reg,SDRAM_RCD),
		GET_REG_BIT(reg,SDRAM_WREQ),GET_REG_BIT(reg,SDRAM_PR),
		GET_REG_BIT(reg,SDRAM_RC),GET_REG_BIT(reg,SDRAM_LMR),
		GET_REG_BIT(reg,SDRAM_IIP));
	reg=in32(PLD_CONFIG_BASE+PLD_CONF_REG1);
	reg1=in32(PLD_CONFIG_BASE+PLD_CONF_REG2);
	printf("HW Config:  FLAG=%ld IP=%ld  index=%ld PRPM=%ld\n            ICW=%ld  ISB=%ld BDIS=%ld  PCIM=%ld\n",
		GET_REG_BIT(reg,SYSCNTR_FLAG),GET_REG_BIT(reg,SYSCNTR_IP),
		GET_SYSCNTR_BOOTIND(reg),GET_REG_BIT(reg,SYSCNTR_PRM),
		GET_REG_BIT(reg,SYSCNTR_ICW),GET_SYSCNTR_ISB(reg),
		GET_REG_BIT(reg1,SYSCNTR_BDIS),GET_REG_BIT(reg1,SYSCNTR_PCIM));
	printf("Switches:   MUX=%ld PCI_DIS=%ld Boot_EN=%ld  Config=%ld\n",GET_SDRAM_MUX(reg),
		GET_REG_BIT(reg,SDRAM_PDIS),GET_REG_BIT(reg1,SYSCNTR_BOOTEN),
		GET_SYSCNTR_CFG(reg1));
	printf("Misc:       RIP=%ld CPU_VPP=%ld FLSH_VPP=%ld FLSH_WP=%ld\n\n",
		GET_REG_BIT(reg,SDRAM_RIP),GET_REG_BIT(reg1,SYSCNTR_CPU_VPP),
		GET_REG_BIT(reg1,SYSCNTR_FL_VPP),GET_REG_BIT(reg1,SYSCNTR_FL_WP));
}


/****************************************************************
 * Setting IOs
 * -----------
 * GPIO6 is User LED1
 * GPIO7 is Interrupt PLX (Output)
 * GPIO5 is User LED0
 * GPIO2 is PLX USERi (Output)
 * GPIO1 is PLX Interrupt (Input)
 ****************************************************************/
 void init_ios(void)
 {
	volatile immap_t * immr = (immap_t *) CFG_IMMR;
	volatile sysconf5xx_t *sysconf = &immr->im_siu_conf;
	unsigned long reg;
	reg=sysconf->sc_sgpiocr; /* Data direction register */
	reg &= ~0x67000000;
	reg |= 0x27000000; /* set outpupts */
	sysconf->sc_sgpiocr=reg; /* Data direction register */
	reg=sysconf->sc_sgpiodt2; /* Data register */
	/* set output to 0 */
	reg &= ~0x27000000;
	/* set IRQ and USERi to 1 */
	reg |= 0x28000000;
	sysconf->sc_sgpiodt2=reg; /* Data register */
}

void user_led0(int led_on)
{
	volatile immap_t * immr = (immap_t *) CFG_IMMR;
	volatile sysconf5xx_t *sysconf = &immr->im_siu_conf;
	unsigned long reg;
	reg=sysconf->sc_sgpiodt2; /* Data register */
	if(led_on)	/* set output to 1 */
		reg |= 0x04000000;
	else
		reg &= ~0x04000000;
	sysconf->sc_sgpiodt2=reg; /* Data register */
}

void user_led1(int led_on)
{
	volatile immap_t * immr = (immap_t *) CFG_IMMR;
	volatile sysconf5xx_t *sysconf = &immr->im_siu_conf;
	unsigned long reg;
	reg=sysconf->sc_sgpiodt2; /* Data register */
	if(led_on)	/* set output to 1 */
		reg |= 0x02000000;
	else
		reg &= ~0x02000000;
	sysconf->sc_sgpiodt2=reg; /* Data register */
}


/****************************************************************
 * Last Stage Init
 ****************************************************************/
int last_stage_init (void)
{
	mem_test_reloc();
	init_ios();
	return 0;
}

/****************************************************************
 * Check the board
 ****************************************************************/

#define BOARD_NAME	"PATI"

int checkboard (void)
{
	unsigned char s[50];
	unsigned long reg;
	char rev;
	int i;

	puts ("\nBoard: ");
	reg=in32(PLD_CONFIG_BASE+PLD_BOARD_TIMING);
	rev=(char)(SYSCNTR_BREV(reg)+'A');
	i = getenv_r ("serial#", s, 32);
	if ((i == -1)) {
		puts ("### No HW ID - assuming " BOARD_NAME);
		printf(" Rev. %c\n",rev);
	}
	else {
		s[sizeof(BOARD_NAME)-1] = 0;
		printf ("%s-1 Rev %c SN: %s\n", s,rev,
				&s[sizeof(BOARD_NAME)]);
	}
	set_flash_vpp(1,0,0); /* set Flash VPP */
	return 0;
}


#ifdef CFG_PCI_CON_DEVICE
/************************************************************************
 * PCI Communication
 *
 * Alive (Pinging):
 * ----------------
 * PCI Host sends message ALIVE, Local acknowledges with ALIVE
 *
 * PCI_CON console over PCI:
 * -------------------------
 * Local side:
 *     - uses PCI9056_LOC_TO_PCI_DBELL register to signal that
 *       data is avaible (PCIMSG_CONN)
 *     - uses PCI9056_MAILBOX1 to send data
 *     - uses PCI9056_MAILBOX0 to receive data
 * PCI side:
 *     - uses PCI9056_PCI_TO_LOC_DBELL register to signal that
 *       data is avaible (PCIMSG_CONN)
 *     - uses PCI9056_MAILBOX0 to send data
 *     - uses PCI9056_MAILBOX1 to receive data
 *
 * How it works:
 *     Send:
 *     - check if PCICON_TRANSMIT_REG is empty
 *     - write data or'ed with 0x80000000 into the PCICON_TRANSMIT_REG
 *     - write PCIMSG_CONN into the PCICON_DBELL_REG to signal a data
 *       is waiting
 *     Receive:
 *     - get an interrupt via the PCICON_ACK_REG register message
 *       PCIMSG_CONN
 *     - write the data from the PCICON_RECEIVE_REG into the receive
 *       buffer and if the receive buffer is not full, clear the
 *       PCICON_RECEIVE_REG (this allows the counterpart to write more data)
 *     - Clear the interrupt by writing 0xFFFFFFFF to the PCICON_ACK_REG
 *
 *     The PCICON_RECEIVE_REG must be cleared by the routine which reads
 *     the receive buffer if the buffer is not full any more
 *
 */

#undef PCI_CON_DEBUG

#ifdef	PCI_CON_DEBUG
#define	PCI_CON_PRINTF(fmt,args...)	serial_printf (fmt ,##args)
#else
#define PCI_CON_PRINTF(fmt,args...)
#endif


/*********************************************************
 * we work only with a receive buffer on eiter side.
 * Transmit buffer is free, if mailbox is cleared.
 * Transmit character is or'ed with 0x80000000
 * PATI receive register MAILBOX0
 * PATI transmit register MAILBOX1
 *********************************************************/
#define PCICON_RECEIVE_REG	PCI9056_MAILBOX0
#define PCICON_TRANSMIT_REG	PCI9056_MAILBOX1
#define PCICON_DBELL_REG	PCI9056_LOC_TO_PCI_DBELL
#define PCICON_ACK_REG		PCI9056_PCI_TO_LOC_DBELL


#define PCIMSG_ALIVE		0x1
#define PCIMSG_CONN		0x2
#define PCIMSG_DISC		0x3
#define PCIMSG_CON_DATA	0x5


#define PCICON_GET_REG(x)	(in32(x + PCI_CONFIG_BASE))
#define PCICON_SET_REG(x,y)	(out32(x + PCI_CONFIG_BASE,y))
#define PCICON_TX_FLAG		0x80000000


#define REC_BUFFER_SIZE	0x100
int recbuf[REC_BUFFER_SIZE];
static int r_ptr = 0;
int w_ptr;
device_t pci_con_dev;
int conn=0;
int buff_full=0;

void pci_con_put_it(const char c)
{
	/* Test for completition */
	unsigned long reg;
	do {
		reg=PCICON_GET_REG(PCICON_TRANSMIT_REG);
	}while(reg);
	reg=PCICON_TX_FLAG + c;
	PCICON_SET_REG(PCICON_TRANSMIT_REG,reg);
	PCICON_SET_REG(PCICON_DBELL_REG,PCIMSG_CON_DATA);
}

void pci_con_putc(const char c)
{
	pci_con_put_it(c);
	if(c == '\n')
		pci_con_put_it('\r');
}


int pci_con_getc(void)
{
	int res;
	int diff;
	while(r_ptr==(volatile int)w_ptr);
	res=recbuf[r_ptr++];
	if(r_ptr==REC_BUFFER_SIZE)
		r_ptr=0;
	if(w_ptr<r_ptr)
		diff=r_ptr+REC_BUFFER_SIZE-w_ptr;
	else
		diff=r_ptr-w_ptr;
	if((diff<(REC_BUFFER_SIZE-4)) && buff_full) {
   		/* clear Mail box */
			buff_full=0;
			PCICON_SET_REG(PCICON_RECEIVE_REG,0L);
	}
	return res;
}

int pci_con_tstc(void)
{
	if(r_ptr==(volatile int)w_ptr)
		return 0;
	return 1;
}

void pci_con_puts (const char *s)
{
	while (*s) {
		pci_con_putc(*s);
		++s;
	}
}

void pci_con_init (void)
{
	w_ptr = 0;
	r_ptr = 0;
	PCICON_SET_REG(PCICON_RECEIVE_REG,0L);
	conn=1;
}

/*******************************************
 * IRQ routine
 ******************************************/
int pci_dorbell_irq(void)
{
	unsigned long reg,data;
	int diff;
	reg=PCICON_GET_REG(PCI9056_INT_CTRL_STAT);
	PCI_CON_PRINTF(" PCI9056_INT_CTRL_STAT = %08lX\n",reg);
	if(reg & (1<<20) ) {
		/* read doorbell */
		reg=PCICON_GET_REG(PCICON_ACK_REG);
		switch(reg) {
			case PCIMSG_ALIVE:
				PCI_CON_PRINTF(" Alive\n");
				PCICON_SET_REG(PCICON_DBELL_REG,PCIMSG_ALIVE);
				break;
			case PCIMSG_CONN:
				PCI_CON_PRINTF(" Conn %d",conn);
				w_ptr = 0;
				r_ptr = 0;
				buff_full=0;
				PCICON_SET_REG(PCICON_RECEIVE_REG,0L);
				conn=1;
				PCI_CON_PRINTF(" ... %d\n",conn);
				break;
			case PCIMSG_CON_DATA:
				data=PCICON_GET_REG(PCICON_RECEIVE_REG);
				recbuf[w_ptr++]=(int)(data&0xff);
				PCI_CON_PRINTF(" Data Console %lX, %X %d %d %X\n",data,((int)(data&0xFF)),
					r_ptr,w_ptr,recbuf[w_ptr-1]);
				if(w_ptr==REC_BUFFER_SIZE)
					w_ptr=0;
				if(w_ptr<r_ptr)
					diff=r_ptr+REC_BUFFER_SIZE-w_ptr;
				else
					diff=r_ptr-w_ptr;
				if(diff>(REC_BUFFER_SIZE-4))
					buff_full=1;
				else
					/* clear Mail box */
					PCICON_SET_REG(PCICON_RECEIVE_REG,0L);
				break;
			default:
				serial_printf(" PCI9056_PCI_TO_LOC_DBELL = %08lX\n",reg);
		}
		/* clear IRQ */
		PCICON_SET_REG(PCICON_ACK_REG,~0L);
	}
	return 0;
}

void pci_con_connect(void)
{
	unsigned long reg;
	conn=0;
	reg=PCICON_GET_REG(PCI9056_INT_CTRL_STAT);
	/* default 0x0f010180 */
	reg &= 0xff000000;
	reg |= 0x00030000; /* enable local dorbell */
	reg |= 0x00000300; /* enable PCI dorbell */
	PCICON_SET_REG(PCI9056_INT_CTRL_STAT , reg);
	irq_install_handler (0x2, (interrupt_handler_t *) pci_dorbell_irq,NULL);
	memset (&pci_con_dev, 0, sizeof (pci_con_dev));
	strcpy (pci_con_dev.name, "pci_con");
	pci_con_dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
	pci_con_dev.putc = pci_con_putc;
	pci_con_dev.puts = pci_con_puts;
	pci_con_dev.getc = pci_con_getc;
	pci_con_dev.tstc = pci_con_tstc;
	device_register (&pci_con_dev);
	printf("PATI ready for PCI connection, type ctrl-c for exit\n");
	do {
		udelay(10);
		if((volatile int)conn)
			break;
		if(ctrlc()) {
			irq_free_handler(0x2);
			return;
		}
	}while(1);
	console_assign(stdin,"pci_con");
	console_assign(stderr,"pci_con");
	console_assign(stdout,"pci_con");
}

void pci_con_disc(void)
{
	console_assign(stdin,"serial");
	console_assign(stderr,"serial");
	console_assign(stdout,"serial");
	PCICON_SET_REG(PCICON_DBELL_REG,PCIMSG_DISC);
	/* reconnection */
	irq_free_handler(0x02);
	pci_con_connect();
}
#endif /* #ifdef CFG_PCI_CON_DEVICE */

/*
 * Absolute environment address for linker file.
 */
GEN_ABS(env_start, CFG_ENV_OFFSET + CFG_FLASH_BASE);
