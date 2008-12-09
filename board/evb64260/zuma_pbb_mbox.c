#include <common.h>
#include <galileo/pci.h>
#include <net.h>
#include <pci.h>

#include "zuma_pbb.h"
#include "zuma_pbb_mbox.h"


struct _zuma_mbox_dev zuma_mbox_dev;


static int zuma_mbox_write(struct _zuma_mbox_dev *dev, unsigned int data)
{
  unsigned int status, count = 0, i;

  status = (volatile int)le32_to_cpu(dev->sip->mbox_status);

  while((status & OUT_PENDING) && count < 1000) {
    count++;
    for(i=0;i<1000;i++);
    status = (volatile int)le32_to_cpu(dev->sip->mbox_status);
  }
  if(count < 1000) {
    /* if SET it means msg pending */
    /* printf("mbox real write %08x\n",data); */
    dev->sip->mbox_out = cpu_to_le32(data);
    return 4;
  }

  printf("mbox tx timeout\n");
  return 0;
}

static int zuma_mbox_read(struct _zuma_mbox_dev *dev, unsigned int *data)
{
  unsigned int status, count = 0, i;

  status = (volatile int)le32_to_cpu(dev->sip->mbox_status);

  while(!(status & IN_VALID) && count < 1000) {
    count++;
    for(i=0;i<1000;i++);
    status = (volatile int)le32_to_cpu(dev->sip->mbox_status);
  }
  if(count < 1000) {
    /* if SET it means msg pending */
    *data=le32_to_cpu(dev->sip->mbox_in);
    /*printf("mbox real read %08x\n", *data); */
    return 4;
  }
  printf("mbox rx timeout\n");
  return 0;
}

static int zuma_mbox_do_one_mailbox(unsigned int out, unsigned int *in)
{
  int ret;
  ret=zuma_mbox_write(&zuma_mbox_dev,out);
  /*printf("write 0x%08x (%d bytes)\n", out, ret); */
  if(ret!=4) return -1;
  ret=zuma_mbox_read(&zuma_mbox_dev,in);
  /*printf("read 0x%08x (%d bytes)\n", *in, ret); */
  if(ret!=4) return -1;
  return 0;
}


#define RET_IF_FAILED(x)	if ((x) == -1) return -1

static int zuma_mbox_do_all_mailbox(void)
{
  unsigned int data_in;
  unsigned short sdata_in;

  RET_IF_FAILED(zuma_mbox_do_one_mailbox(ZUMA_MBOXMSG_START, &data_in));

  RET_IF_FAILED(zuma_mbox_do_one_mailbox(ZUMA_MBOXMSG_MACL, &data_in));
  memcpy(zuma_acc_mac+2,&data_in,4);
  RET_IF_FAILED(zuma_mbox_do_one_mailbox(ZUMA_MBOXMSG_MACH, &data_in));
  sdata_in=data_in&0xffff;
  memcpy(zuma_acc_mac,&sdata_in,2);

  RET_IF_FAILED(zuma_mbox_do_one_mailbox(ZUMA_MBOXMSG_IP, &data_in));
  zuma_ip=data_in;

  RET_IF_FAILED(zuma_mbox_do_one_mailbox(ZUMA_MBOXMSG_SLOT, &data_in));
  zuma_slot_bac=data_in>>3;

  RET_IF_FAILED(zuma_mbox_do_one_mailbox(ZUMA_MBOXMSG_BAUD, &data_in));
  zuma_console_baud = data_in & 0xffff;
  zuma_debug_baud   = data_in >> 16;

  RET_IF_FAILED(zuma_mbox_do_one_mailbox(ZUMA_MBOXMSG_ENG_PRV_MACL, &data_in));
  memcpy(zuma_prv_mac+2,&data_in,4);
  RET_IF_FAILED(zuma_mbox_do_one_mailbox(ZUMA_MBOXMSG_ENG_PRV_MACH, &data_in));
  sdata_in=data_in&0xffff;
  memcpy(zuma_prv_mac,&sdata_in,2);

  RET_IF_FAILED(zuma_mbox_do_one_mailbox(ZUMA_MBOXMSG_DONE, &data_in));

  return 0;
}


static void
zuma_mbox_dump(void)
{
  printf("ACC MAC=%04x%08x\n",*(unsigned short *)(&zuma_acc_mac),*(unsigned int *)((char *)&zuma_acc_mac+2));
  printf("PRV MAC=%04x%08x\n",*(unsigned short *)(&zuma_prv_mac),*(unsigned int *)((char *)&zuma_prv_mac+2));
  printf("slot:bac=%d:%d\n",(zuma_slot_bac>>2)&0xf, zuma_slot_bac & 0x3);
  printf("BAUD1=%d BAUD2=%d\n",zuma_console_baud,zuma_debug_baud);
}


static void
zuma_mbox_setenv(void)
{
  char *data, buf[32];
  unsigned char save = 0;

  data = getenv("baudrate");

  if(!data || (zuma_console_baud != simple_strtoul(data, NULL, 10))) {
    sprintf(buf, "%6d", zuma_console_baud);
    setenv("baudrate", buf);
    save=1;
    printf("baudrate doesn't match from mbox\n");
  }

  ip_to_string(zuma_ip, buf);
  setenv("ipaddr", buf);

  sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",
	  zuma_prv_mac[0],
	  zuma_prv_mac[1],
	  zuma_prv_mac[2],
	  zuma_prv_mac[3],
	  zuma_prv_mac[4],
	  zuma_prv_mac[5]);
  setenv("ethaddr", buf);

  sprintf(buf,"%02x",zuma_slot_bac);
  setenv("bacslot", buf);

  if(save)
    saveenv();
}

/**
 *	zuma_mbox_init:
 */

int zuma_mbox_init(void)
{
  unsigned int iobase;
  memset(&zuma_mbox_dev, 0, sizeof(struct _zuma_mbox_dev));

  zuma_mbox_dev.dev = pci_find_device(VENDOR_ID_ZUMA, DEVICE_ID_ZUMA_PBB, 0);

  if(zuma_mbox_dev.dev == -1) {
    printf("no zuma pbb\n");
    return -1;
  }

  pci_read_config_dword(zuma_mbox_dev.dev, PCI_BASE_ADDRESS_0, &iobase);

  iobase &= PCI_BASE_ADDRESS_MEM_MASK;

  zuma_mbox_dev.sip = (PBB_DMA_REG_MAP *)iobase;

  zuma_mbox_dev.sip->int_mask.word=0;

  printf("pbb @ %p v%d.%d, timestamp %08x\n", zuma_mbox_dev.sip,
	 zuma_mbox_dev.sip->version.pci_bits.rev_major,
	 zuma_mbox_dev.sip->version.pci_bits.rev_minor,
	 zuma_mbox_dev.sip->timestamp);

  if (zuma_mbox_do_all_mailbox() == -1) {
	  printf("mailbox failed.. no ACC?\n");
	  return -1;
  }

  zuma_mbox_dump();

  zuma_mbox_setenv();

  return 0;
}
