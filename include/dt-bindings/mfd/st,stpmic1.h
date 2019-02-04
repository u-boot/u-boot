/*
 * This file is part of stpmu1 pmic driver
 *
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author: Pascal Paillet <p.paillet@st.com> for STMicroelectronics.
 *
 * License type: GPLv2
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DT_BINDINGS_STPMU1_H__
#define __DT_BINDINGS_STPMU1_H__

/* IRQ definitions */
#define IT_PONKEY_F 0
#define IT_PONKEY_R 1
#define IT_WAKEUP_F 2
#define IT_WAKEUP_R 3
#define IT_VBUS_OTG_F 4
#define IT_VBUS_OTG_R 5
#define IT_SWOUT_F 6
#define IT_SWOUT_R 7

#define IT_CURLIM_BUCK1 8
#define IT_CURLIM_BUCK2 9
#define IT_CURLIM_BUCK3 10
#define IT_CURLIM_BUCK4 11
#define IT_OCP_OTG 12
#define IT_OCP_SWOUT 13
#define IT_OCP_BOOST 14
#define IT_OVP_BOOST 15

#define IT_CURLIM_LDO1 16
#define IT_CURLIM_LDO2 17
#define IT_CURLIM_LDO3 18
#define IT_CURLIM_LDO4 19
#define IT_CURLIM_LDO5 20
#define IT_CURLIM_LDO6 21
#define IT_SHORT_SWOTG 22
#define IT_SHORT_SWOUT 23

#define IT_TWARN_F 24
#define IT_TWARN_R 25
#define IT_VINLOW_F 26
#define IT_VINLOW_R 27
#define IT_SWIN_F 30
#define IT_SWIN_R 31

#endif /* __DT_BINDINGS_STPMU1_H__ */
