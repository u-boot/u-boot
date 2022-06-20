// SPDX-License-Identifier: GPL-2.0+

#include <asm/global_data.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

u32 rom_api_download_image(u8 *dest, u32 offset, u32 size)
{
	u32 xor = (uintptr_t)dest ^ offset ^ size;
	volatile gd_t *sgd = gd;
	u32 ret;

	ret = g_rom_api->download_image(dest, offset, size, xor);
	set_gd(sgd);

	return ret;
}

u32 rom_api_query_boot_infor(u32 info_type, u32 *info)
{
	u32 xor = info_type ^ (uintptr_t)info;
	volatile gd_t *sgd = gd;
	u32 ret;

	ret = g_rom_api->query_boot_infor(info_type, info, xor);
	set_gd(sgd);

	return ret;
}
