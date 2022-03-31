#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* Environment */
#define ENV_DEVICE_SETTINGS \
	"stdin=serial,usbkbd,spikbd\0" \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0"

#if CONFIG_IS_ENABLED(CMD_NVME)
	#define BOOT_TARGET_NVME(func) func(NVME, nvme, 0)
#else
	#define BOOT_TARGET_NVME(func)
#endif

#if CONFIG_IS_ENABLED(CMD_USB)
	#define BOOT_TARGET_USB(func) func(USB, usb, 0)
#else
	#define BOOT_TARGET_USB(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_NVME(func) \
	BOOT_TARGET_USB(func)

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_DEVICE_SETTINGS \
	BOOTENV

#endif
