#include <common.h>
#include <syscall.h>

extern unsigned long __dummy;
void do_reset (void);
void do_updater(void);

void _main(void)
{
    int i;
    mon_printf("U-Boot Firmware Updater\n\n\n");
    mon_printf("****************************************************\n"
	       "*  ATTENTION!! PLEASE READ THIS NOTICE CAREFULLY!  *\n"
	       "****************************************************\n\n"
	       "This program  will update your computer's  firmware.\n"
	       "Do NOT  remove the disk,  reset the  machine,  or do\n"
	       "anything that  might disrupt functionality.  If this\n");
    mon_printf("Program fails, your computer  might be unusable, and\n"
	       "you will  need to return your  board for reflashing.\n"
	       "If you find this too risky,  remove the diskette and\n"
	       "switch off your  machine now.  Otherwise  press the \n"
	       "SPACE key now to start the process\n\n");
    do
    {
	char x;
	while (!mon_tstc());
	x = mon_getc();
	if (x == ' ') break;
    } while (1);

    do_updater();

    i = 5;

    mon_printf("\nUpdate done. Please remove diskette.\n");
    mon_printf("The machine will automatically reset in %d seconds\n", i);
    mon_printf("You can switch off/reset now when the floppy is removed\n\n");
    
    while (i)
    {
	mon_printf("Resetting in %d\r", i);
	mon_udelay(1000000);
	i--;
    }
    do_reset();
    while (1);
}

int flash_sect_protect (int p, ulong addr_first, ulong addr_last);
int flash_sect_erase (ulong addr_first, ulong addr_last);
int flash_write (uchar *src, ulong addr, ulong cnt);

void do_updater(void)
{
    unsigned long *addr = &__dummy + 65;
    unsigned long flash_size = flash_init();
    int rc;

    flash_sect_protect(0, 0xFFF00000, 0xFFF7FFFF);
    mon_printf("Erasing ");
    flash_sect_erase(0xFFF00000, 0xFFF7FFFF);
    mon_printf("Writing ");
    rc = flash_write((uchar *)addr, 0xFFF00000, 0x7FFFF);
    if (rc != 0) mon_printf("\nFlashing failed due to error %d\n", rc);
    else mon_printf("\ndone\n");
    flash_sect_protect(1, 0xFFF00000, 0xFFF7FFFF);
}
