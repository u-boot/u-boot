/* SPDX-License-Identifier: GPL-2.0
 *
 * (C) 2020, EPAM Systems Inc.
 */
#ifndef __XEN_H__
#define __XEN_H__

/**
 * xen_init() - Xen initialization
 *
 * Map Xen memory pages, initialize event handler and xenbus.
 */
void xen_init(void);

#endif /* __XEN_H__ */
