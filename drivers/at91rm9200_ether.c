
#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <net.h>

/* ----- Ethernet Buffer definitions ----- */

typedef struct {
  unsigned long addr,size;
} rbf_t;

#define RBF_ADDR      0xfffffffc
#define RBF_OWNER     (1<<0)
#define RBF_WRAP      (1<<1)
#define RBF_BROADCAST (1<<31)
#define RBF_MULTICAST (1<<30)
#define RBF_UNICAST   (1<<29)
#define RBF_EXTERNAL  (1<<28)
#define RBF_UNKOWN    (1<<27)
#define RBF_SIZE      0x07ff
#define RBF_LOCAL4    (1<<26)
#define RBF_LOCAL3    (1<<25)
#define RBF_LOCAL2    (1<<24)
#define RBF_LOCAL1    (1<<23)

#define RBF_FRAMEMAX 10
#define RBF_FRAMEMEM 0x200000
#define RBF_FRAMELEN 0x600

#define RBF_FRAMEBTD RBF_FRAMEMEM
#define RBF_FRAMEBUF (RBF_FRAMEMEM + RBF_FRAMEMAX*sizeof(rbf_t))

/* stolen from mii.h */
/* Generic MII registers. */

#define MII_BMCR            0x00        /* Basic mode control register */
#define MII_BMSR            0x01        /* Basic mode status register  */
#define BMSR_JCD                0x0002  /* Jabber detected             */
#define BMSR_LSTATUS            0x0004  /* Link status                 */
#define BMSR_10HALF             0x0800  /* Can do 10mbps, half-duplex  */
#define BMSR_100FULL            0x4000  /* Can do 100mbps, full-duplex */
#define BMSR_10FULL             0x1000  /* Can do 10mbps, full-duplex  */
#define BMSR_100HALF            0x2000  /* Can do 100mbps, half-duplex */

#define MII_STS2_REG	17  /* Davicom specific */
#define MII_MDINTR_REG	21  /* Davicom specific */

#ifdef CONFIG_DRIVER_ETHER

#if (CONFIG_COMMANDS & CFG_CMD_NET)

AT91PS_EMAC p_mac;

int MII_ReadPhy(unsigned char addr, unsigned short *ret)
  {

  p_mac->EMAC_MAN = 0x60020000 | (addr << 18);
  udelay(10000);
  *ret = (unsigned short)p_mac->EMAC_MAN;
  return 1;
  }


int MII_GetLinkSpeed(void)
  {
  unsigned short stat1, stat2;
  int ret;

  if (!(ret = MII_ReadPhy(MII_BMSR, &stat1)))
    return 0;

  if (stat1 & BMSR_JCD)
    {
#ifdef DEBUG
    printf("MII: jabber condition detected\n");
#endif      /*jabber detected re-read the register*/
    }
  if (!(ret = MII_ReadPhy(MII_BMSR, &stat1)))
    return 0;
  if (!(stat1 & BMSR_LSTATUS))  /* link status up? */
    {
    printf("MII: no Link\n");
    return 0;
    }

  if (!(ret = MII_ReadPhy(MII_STS2_REG, &stat2)))
    return 0;

  if ((stat1 & BMSR_100FULL) && (stat2 & 0x8000) )
    {
    /* set MII for 100BaseTX and Full Duplex */
    p_mac->EMAC_CFG |= AT91C_EMAC_SPD | AT91C_EMAC_FD;
#ifdef DEBUG
    printf("MII: 100BaseTX and Full Duplex detected\n");
#endif
    return 1;
    }

  else
    if ((stat1 & BMSR_10FULL) && (stat2 & 0x2000))
      {
      /* set MII for 10BaseT and Full Duplex */
      p_mac->EMAC_CFG = (p_mac->EMAC_CFG & ~(AT91C_EMAC_SPD | AT91C_EMAC_FD));
#ifdef DEBUG
        printf("MII: 10BaseT and Full Duplex detected\n");
#endif
      return 1;
      }
    else
      if ((stat1 & BMSR_100HALF) && (stat2 & 0x4000))
        {
        /* set MII for 100BaseTX and Half Duplex */
        p_mac->EMAC_CFG = (p_mac->EMAC_CFG & ~(AT91C_EMAC_SPD | AT91C_EMAC_FD));
#ifdef DEBUG
          printf("MII: 100BaseTX and Hall Duplex detected\n");
#endif
        return 1;
        }
      else
        if ((stat1 & BMSR_10HALF) && (stat2 & 0x1000))
          {
          /*set MII for 10BaseT and Half Duplex */
          p_mac->EMAC_CFG &= ~(AT91C_EMAC_SPD | AT91C_EMAC_FD);
#ifdef DEBUG
          printf("MII: 10BaseT and Hall Duplex detected\n");
#endif
          return 1;
          }

  return 0;
  }


int MDIO_StartupPhy(void)
  {
  int ret;

  if(p_mac->EMAC_SR & AT91C_EMAC_LINK)
    {
    printf("MDIO_StartupPhy: no link\n");
    return 0;
    };

  p_mac->EMAC_CTL |= AT91C_EMAC_MPE;

  ret = MII_GetLinkSpeed();
  if (ret == 0)
    {
    printf("MDIO_StartupPhy: MII_GetLinkSpeed failed\n");
    ret = 0;
    }
  else
    {
    ret = 1;
    }

  p_mac->EMAC_CTL &= ~AT91C_EMAC_MPE;
  return ret;

  }


rbf_t* rbfdt;
rbf_t* rbfp;

int eth_init( bd_t *bd )
  {
  int ret;
  int i;
  p_mac = AT91C_BASE_EMAC;

  *AT91C_PIOA_PDR = AT91C_PA16_EMDIO |
    AT91C_PA15_EMDC | AT91C_PA14_ERXER | AT91C_PA13_ERX1 | AT91C_PA12_ERX0 |
    AT91C_PA11_ECRS_ECRSDV | AT91C_PA10_ETX1 | AT91C_PA9_ETX0 | AT91C_PA8_ETXEN |
    AT91C_PA7_ETXCK_EREFCK; 	/* PIO Disable Register */

  *AT91C_PIOB_PDR = AT91C_PB25_EF100 |
    AT91C_PB19_ERXCK | AT91C_PB18_ECOL | AT91C_PB17_ERXDV | AT91C_PB16_ERX3 |
    AT91C_PB15_ERX2 | AT91C_PB14_ETXER | AT91C_PB13_ETX3 | AT91C_PB12_ETX2;

  *AT91C_PIOB_BSR = AT91C_PB25_EF100 |
    AT91C_PB19_ERXCK | AT91C_PB18_ECOL | AT91C_PB17_ERXDV | AT91C_PB16_ERX3 |
    AT91C_PB15_ERX2 | AT91C_PB14_ETXER | AT91C_PB13_ETX3 | AT91C_PB12_ETX2;  /* Select B Register */
  *AT91C_PMC_PCER = 1 << AT91C_ID_EMAC;  /* Peripheral Clock Enable Register */
  p_mac->EMAC_CFG |= AT91C_EMAC_CSR;  /* Clear statistics */

  rbfdt=(rbf_t *)RBF_FRAMEBTD;
  for(i = 0; i < RBF_FRAMEMAX; i++)
    {
    rbfdt[i].addr=RBF_FRAMEBUF+RBF_FRAMELEN*i;
    rbfdt[i].size=0;
    }
  rbfdt[RBF_FRAMEMAX-1].addr|=RBF_WRAP;
  rbfp=&rbfdt[0];

  if (!(ret = MDIO_StartupPhy()))
    {
    printf("MAC: error during MII initialization\n");
    return 0;
    }

  p_mac->EMAC_SA2L = (bd->bi_enetaddr[3] << 24) | (bd->bi_enetaddr[2] << 16)
    | (bd->bi_enetaddr[1] <<  8) | (bd->bi_enetaddr[0]);
  p_mac->EMAC_SA2H = (bd->bi_enetaddr[5] <<  8) | (bd->bi_enetaddr[4]);

  p_mac->EMAC_RBQP = (long)(&rbfdt[0]);
  p_mac->EMAC_RSR &= ~(AT91C_EMAC_RSR_OVR | AT91C_EMAC_REC | AT91C_EMAC_BNA);
  p_mac->EMAC_CFG = (p_mac->EMAC_CFG | AT91C_EMAC_CAF | AT91C_EMAC_NBC | AT91C_EMAC_RMII) & ~AT91C_EMAC_CLK;
  p_mac->EMAC_CTL |= AT91C_EMAC_TE | AT91C_EMAC_RE ;

  return 0;
  }

int eth_send(volatile void *packet, int length)
  {
  while(!(p_mac->EMAC_TSR & AT91C_EMAC_BNQ))
    ;
  p_mac->EMAC_TAR = (long)packet;
  p_mac->EMAC_TCR = length;
  while(p_mac->EMAC_TCR & 0x7ff)
    ;
  p_mac->EMAC_TSR |= AT91C_EMAC_COMP;
  return 0;
  }

int eth_rx(void)
  {
  int size;

  if(!(rbfp->addr & RBF_OWNER))
    return 0;

  size=rbfp->size & RBF_SIZE;
  NetReceive((volatile uchar *) (rbfp->addr & RBF_ADDR), size);

  rbfp->addr &= ~RBF_OWNER;
  if(rbfp->addr & RBF_WRAP)
    rbfp = &rbfdt[0];
  else
    rbfp++;

  p_mac->EMAC_RSR |= AT91C_EMAC_REC;

  return size;
  }

void eth_halt( void )
 {};
#endif
#endif
