#include <common.h>
#include <flash.h>

flash_info_t    flash_info[CFG_MAX_FLASH_BANKS];


unsigned long flash_init(void)
{
    int i;

    for (i = 0; i < CFG_MAX_FLASH_BANKS; i++)
    {
	flash_info[i].flash_id = FLASH_UNKNOWN;
	flash_info[i].sector_count = 0;
	flash_info[i].size = 0;
    }


    return 1;
}

int flash_erase(flash_info_t *info, int s_first, int s_last)
{
    return 1;
}

void flash_print_info(flash_info_t *info)
{
    printf("No flashrom installed\n");
}

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
    return 0;
}
