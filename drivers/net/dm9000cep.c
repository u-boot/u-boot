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

static void dm9000_phy_write(int reg, u16 value)
{
    DM9000_DBG("%s reg: 0x%02x, value: 0x%04x\n", __func__, reg, value);


}

int dm9000_initialize(bd_t *bis)
{
    struct eth_device *dev = &dm9000_info.netdev;

    dev->init = dm9000_init;
    dev->halt = dm9000_halt;
    dev->send = dm9000_send;
    dev->recv = dm9000_recv;
    dev->write_hwaddr = dm9000_write_hwaddr;
    strcpy(dev->name, "dm9000cep");

    eth_register(dev);

    return 0;
}
