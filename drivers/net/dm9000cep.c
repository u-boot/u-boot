#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/io.h>
#include <dm9000.h>

#include "dm9000x.h"

/* Board/System/Debug information/definition ---------------- */

/* #define CONFIG_DM9000_DEBUG */

#ifdef CONFIG_DM9000_DEBUG
#define DM9000_DBG(fmt,args...) printf(fmt, ##args)
#define DM9000_DMP_PACKET(func,packet,length)  \
	do { \
		int i; 							\
		printf("%s: length: %d\n", func, length);		\
		for (i = 0; i < length; i++) {				\
			if (i % 8 == 0)					\
				printf("\n%s: %02x: ", func, i);	\
			printf("%02x ", ((unsigned char *) packet)[i]);	\
		} printf("\n");						\
	} while(0)
#else
#define DM9000_DBG(fmt,args...)
#define DM9000_DMP_PACKET(func,packet,length)
#endif

/* Structure/enum declaration ------------------------------- */
typedef struct board_info {
    u8  io_mode;    /* 0: word, 2: byte */
	u32 runt_length_counter;	/* counter: RX length < 64byte */
	u32 long_length_counter;	/* counter: RX length > 1514byte */
	u32 reset_counter;	/* counter: RESET */
	u32 reset_tx_timeout;	/* RESET caused by TX Timeout */
	u32 reset_rx_status;	/* RESET caused by RX Statsus wrong */
	u16 tx_pkt_cnt;
	u16 queue_start_addr;
	u16 dbug_cnt;
	u8 phy_addr;
	u8 device_wait_reset;	/* device state */
	unsigned char srom[128];
	void (*outblk)(volatile void *data_ptr, int count);
	void (*inblk)(void *data_ptr, int count);
	void (*rx_status)(u16 *RxStatus, u16 *RxLen);
	struct eth_device netdev;
} board_info_t;
static board_info_t dm9000_info;


/* function declaration ------------------------------------- */
static int dm9000_probe(void);
static u16 dm9000_phy_read(int);
static void dm9000_phy_write(int, u16);
static u8 DM9000_ior(int);
static void DM9000_iow(int reg, u8 value);

/* DM9000 network board routine ---------------------------- */
#define DM9000_outb(d,r) writeb(d, (volatile u8 *)(r))
#define DM9000_outw(d,r) writew(d, (volatile u16 *)(r))
#define DM9000_outl(d,r) writel(d, (volatile u32 *)(r))
#define DM9000_inb(r) readb((volatile u8 *)(r))
#define DM9000_inw(r) readw((volatile u16 *)(r))
#define DM9000_inl(r) readl((volatile u32 *)(r))

static void DM9000_iow(int reg, u8 value)
{
    DM9000_outb(reg, DM9000_IO);
    udelay(20);
    DM9000_outb(value, DM9000_DATA);
    udelay(20);
}

static u8 DM9000_ior(int reg)
{
    DM9000_outb(reg, DM9000_IO);
    udelay(20);
    return DM9000_inb(DM9000_DATA);
}

static void dm9000_outblk_16bit(volatile void *buf, int count)
{
    int i;
    u32 len = (count + 1) >> 1;

    for (i = 0; i < len; i++)
    {
        DM9000_outw(((u16 *)buf)[i], DM9000_DATA);
        udelay(20);
    }
}

static void dm9000_inblk_16bit(void *buf, int count)
{
    int i;
    u32 len = (count + 1) >> 1;

    for (i = 0; i < len; i++)
        ((u16 *)buf)[i] = DM9000_inw(DM9000_DATA);
}

static void dm9000_rx_status_16bit(u16 *status, u16 *len)
{
    DM9000_outb(DM9000_MRCMD, DM9000_IO);

    *status = __le16_to_cpu(DM9000_inw(DM9000_DATA));
    *len = __le16_to_cpu(DM9000_inw(DM9000_DATA));
}

static void dm9000_phy_write(int reg, u16 value)
{
    DM9000_DBG("%s reg: 0x%02x, value: 0x%04x\n", __func__, reg, value);

    /* Fill the phyxcer register into REG_0C */
    DM9000_iow(DM9000_EPAR, DM9000_PHY | reg);

    /* Fill the written data into REG_0D & REG_0E */
    DM9000_iow(DM9000_EPDRL, (value & 0xff));
    DM9000_iow(DM9000_EPDRH, ((value >> 8) & 0xff));

    /* Issue phyxcer write command */
    DM9000_iow(DM9000_EPCR, 0xa);

    do {
        udelay(1000);
    }while(0xa != DM9000_ior(DM9000_EPCR));

    /* Clear phyxcer write command */
    DM9000_iow(DM9000_EPCR, 0x00);
}

static u16 dm9000_phy_read(int reg)
{
    u16 val;

    DM9000_DBG("%s: reg: 0x%02x, ", __func__, reg);

    /* Fill the phyxcer register into REG_0C */
    DM9000_iow(DM9000_EPAR, DM9000_PHY | reg);

    /* Issue phyxcer read command */
    DM9000_iow(DM9000_EPCR, 0xc);
    do {
        udelay(1000);
    }while(0xc != DM9000_ior(DM9000_EPCR));

    /* Clear phyxcer read command */
    DM9000_iow(DM9000_EPCR, 0x00);

    val = (DM9000_ior(DM9000_EPDRH) << 8) | DM9000_ior(DM9000_EPDRL);

    DM9000_DBG("value: 0x%04x\n", val);

    return val;
}

static void dm9000_reset(void)
{
    DM9000_DBG("%s: Resetting device\n", __func__);

    DM9000_outb(DM9000_NCR, DM9000_IO);
    udelay(200);
    DM9000_outb(NCR_RST, DM9000_DATA);
    udelay(200);
}

static void dm9000_halt(struct eth_device *netdev)
{
    DM9000_DBG("Entering %s\n", __func__);
}

static int dm9000_init(struct eth_device *dev, bd_t *bd)
{
    int i, oft, lnk;

    DM9000_DBG("Entering %s\n", __func__);

    /*power up PHY */
    DM9000_iow(DM9000_GPR, 0);
    mdelay(20);

    DM9000_iow(DM9000_NCR, 3);
    udelay(500);
    DM9000_iow(DM9000_NCR, 0);
    udelay(500);

    DM9000_iow(DM9000_NCR, 3);
    udelay(500);
    DM9000_iow(DM9000_NCR, 0);
    udelay(500);

    if (dm9000_probe() < 0)
        return -1;

    /* Program operating register */
    DM9000_iow(DM9000_NSR, 0x2c);
    udelay(300);
    DM9000_iow(DM9000_ISR, 0x3f);
    DM9000_iow(DM9000_RCR, 0x39);
    DM9000_iow(DM9000_TCR, 0x00);
    DM9000_iow(DM9000_BPTR, 0x3F);
    DM9000_iow(DM9000_FCTR, 0x3a);
    DM9000_iow(DM9000_FCR, 0xff);
    DM9000_iow(DM9000_SMCR, 0x00);
    DM9000_iow(DM9000_TCR2, 0x80);
    DM9000_iow(DM9000_PBCR, 0x20);
    udelay(300);


    printf("MAC: %pM\n", dev->enetaddr);
    if (!is_valid_ethaddr(dev->enetaddr)) {
        printf("WARNING: Bad MAC address (uninitialized EEPROM?)\n");
    }

    /* fill device MAC address register */
    for (i = 0, oft = DM9000_PAR; i < 6; i++, oft++)
        DM9000_iow(oft, dev->enetaddr[i]);

#ifdef CONFIG_DM9000_DEBUG
    /* read back mac, just to bu sure */
    for (i = 0, oft = 0x10; i < 6; i++, oft++)
        DM9000_DBG("%02x:", DM9000_ior(oft));
    DM9000_DBG("\n");
#endif
    DM9000_iow(DM9000_NSR, 0x2c);
    DM9000_iow(DM9000_ISR, 0x3f);
    udelay(300);
    DM9000_iow(DM9000_IMR, 0x81);

    i = 0;
    while(!(dm9000_phy_read(0x01) & 0x0004)) {
        udelay(1000);
        i++;
        if (i == 10000) {
            printf("could not establish link\n");
            return 0;
        }
    }
    printf("Link Up\n");

    /* see what we've got */
    lnk = dm9000_phy_read(0x11) >> 12;
    printf("operating at ");
    switch (lnk) {
    case 1:
        printf("10M half duplex ");
        break;
    case 2:
        printf("10M full duplex ");
        break;
    case 4:
        printf("100M half duplex ");
        break;
    case 8:
        printf("100M full duplex ");
        break;
    }
    printf("mode\n");

    DM9000_DBG("Leaving %s\n", __func__);
    return 0;
}

static int dm9000_probe(void)
{
    u32 id_val;
    struct board_info *db = &dm9000_info;

    DM9000_DBG("Entering %s\n", __func__);

    id_val = DM9000_ior(DM9000_VIDL);
    id_val |= DM9000_ior(DM9000_VIDH) << 8;
    id_val |= DM9000_ior(DM9000_PIDL) << 16;
    id_val |= DM9000_ior(DM9000_PIDH) << 24;
    if (id_val == DM9000_ID) {
        printf("dm9000cep: id :0x%x\n", id_val);
    } else {
        printf("dm9000cep not found at 0x%08x id: 0x%08x\n",
                CONFIG_DM9000_BASE, id_val);
        return -1;
    }

    db->io_mode = DM9000_ior(DM9000_ISR) >> 7;
    switch (db->io_mode) {
    case 0:
        db->outblk      = dm9000_outblk_16bit;
        db->inblk       = dm9000_inblk_16bit;
        db->rx_status   = dm9000_rx_status_16bit;
        break;
    default:
        printf("dm9000cep: undefined IO mode: 0x%x\n", db->io_mode);
        return -1;
    }
    id_val = DM9000_ior(DM9000_CHIPR);
    printf("dm9000cep revision: 0x%02x, io_mode: 0x%x\n", id_val, db->io_mode);

    return 0;
}

static int dm9000_send(struct eth_device *netdev, void *packet, int length)
{
    int tmo;
    struct board_info *db = &dm9000_info;

    DM9000_DMP_PACKET(__func__, packet, length);

    /* clear Tx interrupt flags bit in ISR */
    DM9000_iow(DM9000_IMR, 0x80);

    /* set Tx length to DM9000CEP */
    DM9000_iow(DM9000_TXPLL, length & 0xff);
    DM9000_iow(DM9000_TXPLH, (length >> 8) & 0xff);

    /* Move data to DM9000CEP TX RAM */
    DM9000_outb(DM9000_MWCMD, DM9000_IO);
    (db->outblk)(packet, length);

    /* Issue TX polling command */
    DM9000_iow(DM9000_TCR, TCR_TXREQ);  /* cleared after TX complete */

    while(!(DM9000_ior(DM9000_NSR) & 0x0c))
        ;

    /* wait for end of transmission */
/*    tmo = get_timer(0) + 5 * CONFIG_SYS_HZ;
    while (!(DM9000_ior(DM9000_ISR) & ISR_PTS)) {
        if (get_timer(0) >= tmo) {
            printf("transmission timeout\n");
            break;
        }
    }
*/
    DM9000_iow(DM9000_NSR, 0x2c);
    udelay(100);
    DM9000_iow(DM9000_IMR, 0x81);

    DM9000_DBG("transmit done\n\n");
    return 0;
}

static int dm9000_recv(struct eth_device *netdev)
{
    return 0;
}

int dm9000_initialize(bd_t *bis)
{
    struct eth_device *dev = &dm9000_info.netdev;

    dev->init = dm9000_init;
    dev->halt = dm9000_halt;
    dev->send = dm9000_send;
    dev->recv = dm9000_recv;
    strcpy(dev->name, "dm9000cep");

    eth_register(dev);

    return 0;
}
