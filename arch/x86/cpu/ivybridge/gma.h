/*
 * From Coreboot file of the same name
 *
 * Copyright (C) 2012 Chromium OS Authors
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

/* mailbox 0: header */
__packed struct opregion_header {
	u8	signature[16];
	u32	size;
	u32	version;
	u8	sbios_version[32];
	u8	vbios_version[16];
	u8	driver_version[16];
	u32	mailboxes;
	u8	reserved[164];
};

#define IGD_OPREGION_SIGNATURE "IntelGraphicsMem"
#define IGD_OPREGION_VERSION  2

#define IGD_MBOX1	(1 << 0)
#define IGD_MBOX2	(1 << 1)
#define IGD_MBOX3	(1 << 2)
#define IGD_MBOX4	(1 << 3)
#define IGD_MBOX5	(1 << 4)

#define MAILBOXES_MOBILE  (IGD_MBOX1 | IGD_MBOX2 | IGD_MBOX3 | \
			   IGD_MBOX4 | IGD_MBOX5)
#define MAILBOXES_DESKTOP (IGD_MBOX2 | IGD_MBOX4)

#define SBIOS_VERSION_SIZE 32

/* mailbox 1: public acpi methods */
__packed struct opregion_mailbox1 {
	u32	drdy;
	u32	csts;
	u32	cevt;
	u8	reserved1[20];
	u32	didl[8];
	u32	cpdl[8];
	u32	cadl[8];
	u32	nadl[8];
	u32	aslp;
	u32	tidx;
	u32	chpd;
	u32	clid;
	u32	cdck;
	u32	sxsw;
	u32	evts;
	u32	cnot;
	u32	nrdy;
	u8	reserved2[60];
};

/* mailbox 2: software sci interface */
__packed struct opregion_mailbox2 {
	u32	scic;
	u32	parm;
	u32	dslp;
	u8	reserved[244];
};

/* mailbox 3: power conservation */
__packed struct opregion_mailbox3 {
	u32	ardy;
	u32	aslc;
	u32	tche;
	u32	alsi;
	u32	bclp;
	u32	pfit;
	u32	cblv;
	u16	bclm[20];
	u32	cpfm;
	u32	epfm;
	u8	plut[74];
	u32	pfmb;
	u32	ccdv;
	u32	pcft;
	u8	reserved[94];
};

#define IGD_BACKLIGHT_BRIGHTNESS 0xff
#define IGD_INITIAL_BRIGHTNESS 0x64

#define IGD_FIELD_VALID	(1 << 31)
#define IGD_WORD_FIELD_VALID (1 << 15)
#define IGD_PFIT_STRETCH 6

/* mailbox 4: vbt */
__packed struct {
	u8 gvd1[7168];
} opregion_vbt_t;

/* IGD OpRegion */
__packed struct igd_opregion {
	opregion_header_t header;
	opregion_mailbox1_t mailbox1;
	opregion_mailbox2_t mailbox2;
	opregion_mailbox3_t mailbox3;
	opregion_vbt_t vbt;
};

/* Intel Video BIOS (Option ROM) */
__packed struct optionrom_header {
	u16	signature;
	u8	size;
	u8	reserved[21];
	u16	pcir_offset;
	u16	vbt_offset;
};

#define OPROM_SIGNATURE 0xaa55

__packed struct optionrom_pcir {
	u32 signature;
	u16 vendor;
	u16 device;
	u16 reserved1;
	u16 length;
	u8  revision;
	u8  classcode[3];
	u16 imagelength;
	u16 coderevision;
	u8  codetype;
	u8  indicator;
	u16 reserved2;
};

__packed struct optionrom_vbt {
	u8  hdr_signature[20];
	u16 hdr_version;
	u16 hdr_size;
	u16 hdr_vbt_size;
	u8  hdr_vbt_checksum;
	u8  hdr_reserved;
	u32 hdr_vbt_datablock;
	u32 hdr_aim[4];
	u8  datahdr_signature[16];
	u16 datahdr_version;
	u16 datahdr_size;
	u16 datahdr_datablocksize;
	u8  coreblock_id;
	u16 coreblock_size;
	u16 coreblock_biossize;
	u8  coreblock_biostype;
	u8  coreblock_releasestatus;
	u8  coreblock_hwsupported;
	u8  coreblock_integratedhw;
	u8  coreblock_biosbuild[4];
	u8  coreblock_biossignon[155];
};

#define VBT_SIGNATURE 0x54425624
