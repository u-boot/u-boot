#include	"macros.h"


#define GLOBALINFO0	      0x50
#define GLOBALINFO0_BO        (1<<7)
#define GLOBALINFO2_B1ARBITER (1<<6)
#define HBUSACR0              0x5c
#define HBUSACR2_BURST        (1<<0)
#define HBUSACR2_LAT          (1<<1)

#define RECEIVER_HOLDING 0
#define TRANSMITTER_HOLDING 0
#define INTERRUPT_ENABLE 1
#define INTERRUPT_STATUS 2
#define FIFO_CONTROL 2
#define LINE_CONTROL 3
#define MODEM_CONTROL 4
#define LINE_STATUS 5
#define MODEM_STATUS 6
#define SCRATCH_PAD 7

#define DIVISOR_LATCH_LSB 0
#define DIVISOR_LATCH_MSB 1
#define PRESCALER_DIVISION 5

#define UART(x) (0x3f8+(x))

#define GLOBALINFO0           0x50
#define GLOBALINFO0_BO        (1<<7)
#define GLOBALINFO2_B1ARBITER (1<<6)
#define HBUSACR0              0x5c
#define HBUSACR2_BURST        (1<<0)
#define HBUSACR2_LAT          (1<<1)

#define SUPERIO_1               ((7 << 3) | (0))
#define SUPERIO_2               ((7 << 3) | (1))

	.globl	board_asm_init

board_asm_init:
	mflr	r29
	/* Set 'Must-set' register */
	li	r3, 0
	li	r4, 0
	li	r5, 0x5e
	bl	pci_read_cfg_byte
	ori	r3, r3, (1<<1)
	xori	r6, r3, (1<<1)
	li	r3, 0
	bl	pci_write_cfg_byte

	li	r3, 0
	li	r5, 0x52
	bl	pci_read_cfg_byte
	ori	r6, r3, (1<<6)
	li	r3, 0
	bl	pci_write_cfg_byte

	li	r3, 0
	li	r4, 0x08
	li	r5, 0xd2
	bl	pci_read_cfg_byte
	ori	r6, r3, (1<<2)
	li	r3, 0
	bl	pci_write_cfg_byte


	/* Do PCI reset */
/*	li	r3, 0
	li	r4, 0x38
	li	r5, 0x47
	bl	pci_read_cfg_byte
	ori	r6, r3, 0x01
	li	r3, 0
	li	r4, 0x38
	li	r5, 0x47
	bl	pci_write_cfg_byte*/


	/* Enable NVRAM for environment */
	li	r3, 0
	li	r4, 0
	li	r5, 0x56
	li	r6, 0x0B
	bl	pci_write_cfg_byte


	/* Init Super-I/O chips */

	siowb	0x40, 0x08
	siowb	0x41, 0x01
	siowb	0x45, 0x80
	siowb	0x46, 0x60
	siowb	0x47, 0x20
	siowb	0x48, 0x01
	siowb	0x4a, 0xc4
	siowb	0x50, 0x0e
	siowb	0x51, 0x76
	siowb	0x52, 0x34
	siowb	0x54, 0x00
	siowb	0x55, 0x90
	siowb	0x56, 0x99
	siowb	0x57, 0x90
	siowb	0x85, 0x01

	/* Enable configuration mode for SuperIO */
	li	r3, 0
	li	r4, (7<<3)
	li	r5, 0x85
	bl	pci_read_cfg_byte
	ori	r6, r3, 0x02
	mr	r31, r6
	li	r3,0
	bl	pci_write_cfg_byte

	/* COM1 as 3f8 */
	outb	0x3f0, 0xe7
	outb	0x3f1, 0xfe

	/* COM2 as 2f8 */
	outb	0x3f0, 0xe8
	outb	0x3f1, 0xeb

	/* Enable */
	outb	0x3f0, 0xe2
	inb	r3, 0x3f1
	ori	r3, r3, 0x0c
	outb	0x3f0, 0xe2
	outbr	0x3f1, r3

	/* Disable configuration mode */
	li	r3, 0
	li	r4, (7<<3)
	li	r5, 0x85
	mr	r6, r31
	bl	pci_write_cfg_byte

	/* Set line control */
	outb	UART(LINE_CONTROL), 0x83
	outb	UART(DIVISOR_LATCH_LSB), 0x0c
	outb	UART(DIVISOR_LATCH_MSB), 0x00
	outb	UART(LINE_CONTROL), 0x3

	mtlr	r29
	blr


	.globl	new_reset
	.globl	new_reset_end
new_reset:
	li	r0, 0x100
	oris	r0, r0, 0xFFF0
	mtlr	r0
	blr

new_reset_end:
