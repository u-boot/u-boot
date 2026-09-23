/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Paul Gerber
 */

#ifndef __TQMA6_H
#define __TQMA6_H

const char *tqma6_get_fdt_configuration(void);
int tqma6_has_enet_workaround(void);

#endif /* __TQMA6_H */
