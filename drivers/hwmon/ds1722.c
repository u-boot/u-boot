
#include <common.h>
#include <ssi.h>

static void ds1722_select(int dev)
{
	ssi_set_interface(4096, 0, 0, 0);
	ssi_chip_select(0);
	udelay(1);
	ssi_chip_select(dev);
	udelay(1);
}


u8 ds1722_read(int dev, int addr)
{
	u8 res;

	ds1722_select(dev);

	ssi_tx_byte(addr);
	res = ssi_rx_byte();

	ssi_chip_select(0);

	return res;
}

void ds1722_write(int dev, int addr, u8 data)
{
	ds1722_select(dev);

	ssi_tx_byte(0x80|addr);
	ssi_tx_byte(data);

	ssi_chip_select(0);
}


u16 ds1722_temp(int dev, int resolution)
{
	static int useconds[] = {
		75000, 150000, 300000, 600000, 1200000
	};
	char temp;
	u16 res;


	/* set up the desired resulotion ... */
	ds1722_write(dev, 0, 0xe0 | (resolution << 1));

	/* wait while the chip measures the tremperature */
	udelay(useconds[resolution]);

	res = (temp = ds1722_read(dev, 2)) << 8;

	if (temp < 0) {
		temp = (16 - (ds1722_read(dev, 1) >> 4)) & 0x0f;
	} else {
		temp = (ds1722_read(dev, 1) >> 4);
	}

	switch (temp) {
	case 0:
		/* .0000 */
		break;
	case 1:
		/* .0625 */
		res |=1;
		break;
	case 2:
		/* .1250 */
		res |=1;
		break;
	case 3:
		/* .1875 */
		res |=2;
		break;
	case 4:
		/* .2500 */
		res |=3;
		break;
	case 5:
		/* .3125 */
		res |=3;
		break;
	case 6:
		/* .3750 */
		res |=4;
		break;
	case 7:
		/* .4375 */
		res |=4;
		break;
	case 8:
		/* .5000 */
		res |=5;
		break;
	case 9:
		/* .5625 */
		res |=6;
		break;
	case 10:
		/* .6250 */
		res |=6;
		break;
	case 11:
		/* .6875 */
		res |=7;
		break;
	case 12:
		/* .7500 */
		res |=8;
		break;
	case 13:
		/* .8125 */
		res |=8;
		break;
	case 14:
		/* .8750 */
		res |=9;
		break;
	case 15:
		/* .9375 */
		res |=9;
		break;
	}
	return res;

}

int ds1722_probe(int dev)
{
	u16 temp = ds1722_temp(dev, DS1722_RESOLUTION_12BIT);
	printf("%d.%d deg C\n\n", (char)(temp >> 8), temp & 0xff);
	return 0;
}
