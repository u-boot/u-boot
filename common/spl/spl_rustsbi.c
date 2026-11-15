// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Institute of Software Chinese Academy of Sciences,
 * Zhouqi Jiang <jiangzhouqi2025@iscas.ac.cn>
 */

#include <spl.h>

void __noreturn spl_invoke_rustsbi(struct spl_image_info *spl_image)
{
	/* RustSBI and OpenSBI use the same dynamic handoff protocol. */
	spl_invoke_opensbi(spl_image);
}
