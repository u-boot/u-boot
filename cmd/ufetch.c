// SPDX-License-Identifier: GPL-2.0

/* Small "fetch" utility for U-Boot */

#ifdef CONFIG_ARM64
#include <asm/system.h>
#endif
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <display_options.h>
#include <mmc.h>
#include <time.h>
#include <asm/global_data.h>
#include <cli.h>
#include <command.h>
#include <dm/ofnode.h>
#include <env.h>
#include <rand.h>
#include <vsprintf.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <version.h>

DECLARE_GLOBAL_DATA_PTR;

#define LINE_WIDTH 40
#define BLUE "\033[34m"
#define YELLOW "\033[33m"
#define BOLD "\033[1m"
#define RESET "\033[0m"
static const char * const logo_lines[] = {
	BLUE BOLD "                  ......::......                   ",
	BLUE BOLD "             ...::::::::::::::::::...              ",
	BLUE BOLD "          ..::::::::::::::::::::::::::..           ",
	BLUE BOLD "        .::::.:::::::::::::::...::::.::::.         ",
	BLUE BOLD "      .::::::::::::::::::::..::::::::::::::.       ",
	BLUE BOLD "    .::.:::::::::::::::::::" YELLOW "=*%#*" BLUE "::::::::::.::.     ",
	BLUE BOLD "   .:::::::::::::::::....." YELLOW "*%%*-" BLUE ":....::::::::::.    ",
	BLUE BOLD "  .:.:::...:::::::::.:-" YELLOW "===##*---==-" BLUE "::::::::::.:.   ",
	BLUE BOLD " .::::..::::........" YELLOW "-***#****###****-" BLUE "...::::::.:.  ",
	BLUE BOLD " ::.:.-" YELLOW "+***+=" BLUE "::-" YELLOW "=+**#%%%%%%%%%%%%###*= " BLUE "-::...::::. ",
	BLUE BOLD ".:.::-" YELLOW "*****###%%%%%%%%%%%%%%%%%%%%%%%%%%#*=" BLUE ":..:::: ",
	BLUE BOLD ".::" YELLOW "##" BLUE ":" YELLOW "***#%%%%%%#####%%%%%%%####%%%%%####%%%*" BLUE "-.::. ",
	BLUE BOLD ":.:" YELLOW "#%" BLUE "::" YELLOW "*%%%%%%%#*****##%%%#*****##%%##*****#%%+" BLUE ".::.",
	BLUE BOLD ".::" YELLOW "**==#%%%%%%%##****#%%%%##****#%%%%#****###%%" BLUE ":.. ",
	BLUE BOLD "..:" YELLOW "#%" BLUE "::" YELLOW "*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%#%%%%%+ " BLUE ".:.",
	BLUE BOLD " ::" YELLOW "##" BLUE ":" YELLOW "+**#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%* " BLUE "-.:: ",
	BLUE BOLD " ..::-" YELLOW "#****#%#%%%%%%%%%%%%%%%%%%%%%%%%%%#*=" BLUE "-..::.  ",
	BLUE BOLD "  ...:=" YELLOW "*****=" BLUE "::-" YELLOW "=+**###%%%%%%%%###**+=  " BLUE "--:...:::  ",
	BLUE BOLD "   .::.::--:........::::::--::::::......::::::.    ",
	BLUE BOLD "    .::.....::::::::::...........:::::::::.::.     ",
	BLUE BOLD "      .::::::::::::::::::::::::::::::::::::.       ",
	BLUE BOLD "        .::::.::::::::::::::::::::::.::::.         ",
	BLUE BOLD "          ..::::::::::::::::::::::::::..           ",
	BLUE BOLD "             ...::::::::::::::::::...              ",
	BLUE BOLD "                  ......::......                   ",
};

enum output_lines {
	FIRST,
	SECOND,
	KERNEL,
	SYSINFO,
	HOST,
	UPTIME,
	IP,
	CMDS,
	CONSOLES,
	FEATURES,
	RELOCATION,
	CORES,
	MEMORY,
	STORAGE,

	/* Up to 10 storage devices... Should be enough for anyone right? */
	_LAST_LINE = (STORAGE + 10),
#define LAST_LINE (_LAST_LINE - 1UL)
};

/*
 * TODO/ideas:
 * - Refactor to not use a for loop
 * - Handle multiple network interfaces
 * - Include stats about number of bound/probed devices
 * - Show U-Boot's size and malloc usage, fdt size, etc.
 */


static int do_ufetch(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	int num_lines = max((size_t)LAST_LINE + 1, ARRAY_SIZE(logo_lines));
	const char *model, *compatible;
	char *ipaddr;
	int n_cmds, n_cpus = 0, compatlen;
	size_t size = 0;
	ofnode np;
	bool skip_ascii = false;

	if (argc > 1 && strcmp(argv[1], "-n") == 0) {
		skip_ascii = true;
		num_lines = LAST_LINE;
	}

	for (int line = 0; line < num_lines; line++) {
		if (!skip_ascii) {
			if (line < ARRAY_SIZE(logo_lines))
				printf("%s  ", logo_lines[line]);
			else
				printf("%*c  ", LINE_WIDTH, ' ');
		}
		switch (line) {
		case FIRST:
			compatible = ofnode_read_string(ofnode_root(), "compatible");
			if (!compatible)
				compatible = "unknown";
			printf(RESET "%s\n", compatible);
			compatlen = strlen(compatible);
			break;
		case SECOND:
			for (int j = 0; j < compatlen; j++)
				putc('-');
			putc('\n');
			break;
		case KERNEL:
			printf("Kernel:" RESET " %s\n", U_BOOT_VERSION);
			break;
		case SYSINFO:
			printf("Config:" RESET " %s_defconfig\n", CONFIG_SYS_CONFIG_NAME);
			break;
		case HOST:
			model = ofnode_read_string(ofnode_root(), "model");
			if (model)
				printf("Host:" RESET " %s\n", model);
			break;
		case UPTIME:
			printf("Uptime:" RESET " %ld seconds\n", get_timer(0) / 1000);
			break;
		case IP:
			ipaddr = env_get("ipaddr");
			if (!ipaddr)
				ipaddr = "none";
			printf("IP Address:" RESET " %s", ipaddr);
			ipaddr = env_get("ipv6addr");
			if (ipaddr)
				printf(", %s\n", ipaddr);
			else
				putc('\n');
			break;
		case CMDS:
			n_cmds = ll_entry_count(struct cmd_tbl, cmd);
			printf("Commands:" RESET " %d (help)\n", n_cmds);
			break;
		case CONSOLES:
			printf("Consoles:" RESET " %s", env_get("stdout"));
			if (gd->baudrate)
				printf(" (%d baud)", gd->baudrate);
			putc('\n');
			break;
		case FEATURES:
			printf("Features:" RESET " ");
			if (IS_ENABLED(CONFIG_NET))
				printf("Net");
			if (IS_ENABLED(CONFIG_EFI_LOADER))
				printf(", EFI");
			if (IS_ENABLED(CONFIG_CMD_CAT))
				printf(", cat :3");
#ifdef CONFIG_ARM64
			switch (current_el()) {
			case 2:
				printf(", VMs");
				break;
			case 3:
				printf(", full control!");
				break;
			}
#endif
			printf("\n");
			break;
		case RELOCATION:
			if (gd->flags & GD_FLG_SKIP_RELOC)
				printf("Relocated:" RESET " no\n");
			else
				printf("Relocated:" RESET " to %#011lx\n", gd->relocaddr);
			break;
		case CORES:
			ofnode_for_each_subnode(np, ofnode_path("/cpus")) {
				if (ofnode_name_eq(np, "cpu"))
					n_cpus++;
			}
			printf("CPU: " RESET CONFIG_SYS_ARCH " (%d cores, 1 in use)\n", n_cpus);
			break;
		case MEMORY:
			for (int j = 0; j < CONFIG_NR_DRAM_BANKS && gd->bd->bi_dram[j].size; j++)
				size += gd->bd->bi_dram[j].size;
			printf("Memory:" RESET " ");
			print_size(size, "\n");
			break;
		case STORAGE:
		default: {
#ifdef CONFIG_BLK
			struct udevice *dev;
			struct blk_desc *desc;
			int ret;

			ret = uclass_find_device_by_seq(UCLASS_BLK, line - STORAGE, &dev);
			if (!ret && dev) {
				desc = dev_get_uclass_plat(dev);
				size = desc->lba * desc->blksz;
				printf("%4s %d: " RESET, blk_get_uclass_name(desc->uclass_id),
					desc->lun);
				if (size)
					print_size(size, "");
				else
					printf("No media");
			} else if (ret == -ENODEV && (skip_ascii || line > ARRAY_SIZE(logo_lines))) {
				break;
			}
#endif
			printf("\n");
		}
		}
	}

	printf(RESET "\n\n");

	return 0;
}

U_BOOT_CMD(ufetch, 2, 1, do_ufetch,
	   "U-Boot fetch utility",
	   "Print information about your device.\n"
	   "    -n    Don't print the ASCII logo"
);
