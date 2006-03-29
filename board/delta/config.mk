#TEXT_BASE = 0x0
#TEXT_BASE = 0xa1700000
#TEXT_BASE = 0xa3080000
#TEXT_BASE = 0x9ffe0000
TEXT_BASE = 0x83008000

# Compile the new NAND code (needed iff #ifdef CONFIG_NEW_NAND_CODE)
BOARDLIBS = drivers/nand/libnand.a
