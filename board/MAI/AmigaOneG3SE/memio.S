#include	"macros.h"

	

	.globl pci_read_cfg_byte
	
pci_read_cfg_byte:
	config_addr
	config_data	3
	eieio
	sync
	lbz	r3, 0(r9)
	blr

	

	.globl pci_write_cfg_byte
	
pci_write_cfg_byte:	
	config_addr
	config_data	3
	stb	r6, 0(r9)
	eieio
	sync
	blr



	.globl pci_read_cfg_word
	
pci_read_cfg_word:
	config_addr
	config_data	2
	lhbrx	r3, 0, r9
	eieio
	sync
	blr


	
	.globl pci_write_cfg_word
	
pci_write_cfg_word:
	config_addr
	config_data	2
	sthbrx	r6, 0, r9
	eieio
	sync
	blr

	

	.globl pci_read_cfg_long
	
pci_read_cfg_long:
	config_addr
	config_data	0
	lwbrx	r3, 0, r9
	eieio
	sync
	blr



	.globl pci_write_cfg_long
	
pci_write_cfg_long:
	config_addr
	config_data	0
	stwbrx	r6, 0, r9
	eieio
	sync
	blr

