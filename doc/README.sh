
U-Boot for Renesas SuperH
	Last update 08/10/2007 by Nobuhiro Iwamatsu

================================================================================
0. What's this?
	This file contains status information for the port of U-Boot to the
	Renesas SuperH series of CPUs.

================================================================================
1. Overview
	SuperH has an original boot loader. However, source code is dirty, and
	maintenance is not done.
	To improve sharing and the maintenance of the code, Nobuhiro Iwamatsu
	started the porting to u-boot in 2007.

================================================================================
2. Supported CPUs

	2.1. Renesas SH7750/SH7750R
	2.2. Renesas SH7722

================================================================================
3. Supported Boards

	3.1. Hitachi UL MS7750SE01/MS7750RSE01
		Board specific code is in board/ms7750se
		To use this board, type "make ms7750se_config".

	3.2. Hitachi UL MS7722SE01
		Board specific code is in board/ms7722se
		To use this board, type "make ms7722se_config".

	** README **
		In SuperH, S-record and binary of made u-boot work on the memory.
		When u-boot is written in the flash, it is necessary to change the
		address by using 'objcopy'.
		ex) shX-linux-objcopy -Ibinary -Osrec u-boot.bin u-boot.flash.srec

================================================================================
4. Compiler
	You can use the following of u-boot to compile.
		- SuperH Linux Open site
			http://www.superh-linux.org/
		- KPIT GNU tools
			http://www.kpitgnutools.com/

================================================================================
5. Future
	I plan to support the following CPUs and boards.
		5.1. CPUs
			- SH7710/SH7712 (SH3)
			- SH7780(SH4)
			- SH7785(SH4)

		5.2. Boards
			- Many boards ;-)

================================================================================
Copyright (c) 2007
    Nobuhiro Iwamatsu <iwamatsu@nigaur.org>
