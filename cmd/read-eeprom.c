#include <stdio.h>
// #include <common.h>
#include <command.h>
#include <linux/string.h>


uint8_t *find_char(uint8_t *str, uint8_t *character)
{
	while (*str) {
		if (*str == *character)
		{
			printf("char_pos*:       %p\n", (void *)str);
			return str;
		}
		str++;
	}
	return NULL;
}

static int do_read_eeprom(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 3)
		return CMD_RET_USAGE;

	uint8_t *character = argv[1];
	uint8_t *str = argv[2];

	uint8_t *char_pos = find_char(str, character);

	printf("char_pos*:  %p\n", (void *)char_pos);
	printf("character*: %p\n", (void *)character);
	printf("str*:       %p\n", (void *)str);

	if (char_pos) {
		printf("First occurrence of '%c' is at position: %ld\n", character[0], char_pos - str);
	} else {
		printf("No null byte found in the string.\n");
	}

	return 0;

}

U_BOOT_CMD(
	read_eeprom, 3, 1, do_read_eeprom,
	"find the first occurrence of the byte 0x00 in a string",
	"string\n"
	"    - find the first occurrence of the byte 0x00 in the given string"
);
