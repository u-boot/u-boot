#define IN_VALID 1
#define OUT_PENDING 2

enum {
	ZUMA_MBOXMSG_DONE,
	ZUMA_MBOXMSG_MACL,
	ZUMA_MBOXMSG_MACH,
	ZUMA_MBOXMSG_IP,
	ZUMA_MBOXMSG_SLOT,
	ZUMA_MBOXMSG_RESET,
	ZUMA_MBOXMSG_BAUD,
	ZUMA_MBOXMSG_START,
	ZUMA_MBOXMSG_ENG_PRV_MACL,
	ZUMA_MBOXMSG_ENG_PRV_MACH,

	MBOXMSG_LAST
};

struct zuma_mailbox_info {
	unsigned char acc_mac[6];
	unsigned char prv_mac[6];
	unsigned int ip;
	unsigned int slot_bac;
	unsigned int console_baud;
	unsigned int debug_baud;
};

struct _zuma_mbox_dev {
	pci_dev_t dev;
	PBB_DMA_REG_MAP *sip;
	struct zuma_mailbox_info mailbox;
};

#define zuma_prv_mac		zuma_mbox_dev.mailbox.prv_mac
#define zuma_acc_mac		zuma_mbox_dev.mailbox.acc_mac
#define zuma_ip                 zuma_mbox_dev.mailbox.ip
#define zuma_slot_bac		zuma_mbox_dev.mailbox.slot_bac
#define zuma_console_baud	zuma_mbox_dev.mailbox.console_baud
#define zuma_debug_baud		zuma_mbox_dev.mailbox.debug_baud


extern struct _zuma_mbox_dev zuma_mbox_dev;
extern int zuma_mbox_init (void);
