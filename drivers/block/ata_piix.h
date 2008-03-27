#ifndef __ATA_PIIX_H__
#define __ATA_PIIX_H__

#if (DEBUG_SATA)
#define PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

struct sata_ioports {
	unsigned long cmd_addr;
	unsigned long data_addr;
	unsigned long error_addr;
	unsigned long feature_addr;
	unsigned long nsect_addr;
	unsigned long lbal_addr;
	unsigned long lbam_addr;
	unsigned long lbah_addr;
	unsigned long device_addr;
	unsigned long status_addr;
	unsigned long command_addr;
	unsigned long altstatus_addr;
	unsigned long ctl_addr;
	unsigned long bmdma_addr;
	unsigned long scr_addr;
};

struct sata_port {
	unsigned char port_no;		/* primary=0, secondary=1	*/
	struct sata_ioports ioaddr;	/* ATA cmd/ctl/dma reg blks	*/
	unsigned char ctl_reg;
	unsigned char last_ctl;
	unsigned char port_state;	/* 1-port is available and	*/
					/* 0-port is not available	*/
	unsigned char dev_mask;
};

/***********SATA LIBRARY SPECIFIC DEFINITIONS AND DECLARATIONS**************/
#ifdef SATA_DECL		/*SATA library specific declarations */
#define ata_id_has_lba48(id)    ((id)[83] & (1 << 10))
#define ata_id_has_lba(id)      ((id)[49] & (1 << 9))
#define ata_id_has_dma(id)      ((id)[49] & (1 << 8))
#define ata_id_u32(id,n)        \
	(((u32) (id)[(n) + 1] << 16) | ((u32) (id)[(n)]))
#define ata_id_u64(id,n)        \
	(((u64) (id)[(n) + 3] << 48) | \
	((u64) (id)[(n) + 2] << 32) | \
	((u64) (id)[(n) + 1] << 16) | \
	((u64) (id)[(n) + 0]) )
#endif

#ifdef SATA_DECL		/*SATA library specific declarations */
static inline void
ata_dump_id (u16 * id)
{
	PRINTF ("49 = 0x%04x  "
		"53 = 0x%04x  "
		"63 = 0x%04x  "
		"64 = 0x%04x  "
		"75 = 0x%04x  \n", id[49], id[53], id[63], id[64], id[75]);
	PRINTF ("80 = 0x%04x  "
		"81 = 0x%04x  "
		"82 = 0x%04x  "
		"83 = 0x%04x  "
		"84 = 0x%04x  \n", id[80], id[81], id[82], id[83], id[84]);
	PRINTF ("88 = 0x%04x  " "93 = 0x%04x\n", id[88], id[93]);
}
#endif

#ifdef SATA_DECL		/*SATA library specific declarations */
int sata_bus_softreset (int num);
void sata_identify (int num, int dev);
void sata_port (struct sata_ioports *ioport);
void set_Feature_cmd (int num, int dev);
int sata_devchk (struct sata_ioports *ioaddr, int dev);
void dev_select (struct sata_ioports *ioaddr, int dev);
u8 sata_busy_wait (struct sata_ioports *ioaddr, int bits, unsigned int max);
u8 sata_chk_status (struct sata_ioports *ioaddr);
ulong sata_read (int device, ulong blknr,lbaint_t blkcnt, void * buffer);
ulong sata_write (int device,ulong blknr, lbaint_t blkcnt, void * buffer);
void msleep (int count);
#endif

/************DRIVER SPECIFIC DEFINITIONS AND DECLARATIONS**************/

#ifdef DRV_DECL			/*Driver specific declaration */
int init_sata (int dev);
#endif

#ifdef DRV_DECL			/*Defines Driver Specific variables */
struct sata_port port[CFG_SATA_MAXBUS];
#endif

#endif /* __ATA_PIIX_H__ */
