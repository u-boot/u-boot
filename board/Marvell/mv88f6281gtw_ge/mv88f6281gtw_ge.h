/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MV88F6281GTW_GE_H
#define __MV88F6281GTW_GE_H

#define MV88F6281GTW_GE_OE_LOW		(~((1 << 7) | (1 << 12) \
					  |(1 << 20) | (1 << 21)))	/*enable GLED,RLED */
#define MV88F6281GTW_GE_OE_HIGH		(~((1 << 4)|(1 << 6)|(1 << 7)|(1 << 12) \
					  |(1 << 13)|(1 << 16)|(1 << 17)))
#define MV88F6281GTW_GE_OE_VAL_LOW	(1 << 20)	/*make GLED on */
#define MV88F6281GTW_GE_OE_VAL_HIGH	((1 << 6)|(1 << 13)|(1 << 16)|(1 << 17))


#endif /* __MV88F6281GTW_GE_H */
