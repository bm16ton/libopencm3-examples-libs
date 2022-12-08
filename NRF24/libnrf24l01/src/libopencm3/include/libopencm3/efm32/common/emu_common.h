/** @addtogroup emu_defines
 */
/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2015 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <libopencm3/efm32/memorymap.h>
#include <libopencm3/cm3/common.h>

/**@{*/

#define EMU_CTRL			MMIO32(EMU_BASE + 0x000)
#define EMU_LOCK			MMIO32(EMU_BASE + 0x008)
#define EMU_AUXCTRL			MMIO32(EMU_BASE + 0x024)
#define EMU_EM4CONF			MMIO32(EMU_BASE + 0x02C)
#define EMU_BUCTRL			MMIO32(EMU_BASE + 0x030)
#define EMU_PWRCONF			MMIO32(EMU_BASE + 0x034)
#define EMU_BUINACT			MMIO32(EMU_BASE + 0x038)
#define EMU_BUACT			MMIO32(EMU_BASE + 0x03C)
#define EMU_STATUS			MMIO32(EMU_BASE + 0x040)
#define EMU_ROUTE			MMIO32(EMU_BASE + 0x044)
#define EMU_IF				MMIO32(EMU_BASE + 0x048)
#define EMU_IFS				MMIO32(EMU_BASE + 0x04C)
#define EMU_IFC				MMIO32(EMU_BASE + 0x050)
#define EMU_IEN				MMIO32(EMU_BASE + 0x054)
#define EMU_BUBODBUVINCAL		MMIO32(EMU_BASE + 0x058)
#define EMU_BUBODUNREGCAL		MMIO32(EMU_BASE + 0x05C)

/* EMU_CTRL */
#define EMU_CTRL_EM4CTRL_SHIFT		(2)
#define EMU_CTRL_EM4CTRL_MASK		(0x3 << EMU_CTRL_EM4CTRL_SHIFT)
#define EMU_CTLR_EM4CTRL(v)		\
	(((v) << EMU_CTRL_EM4CTRL_SHIFT) & EMU_CTRL_EM4CTRL_MASK)

#define EMU_CTRL_EM2BLOCK		(1 << 1)
#define EMU_CTRL_EMVREG			(1 << 0)

/* EMU_LOCK */
#define EMU_LOCK_LOCKKEY_MASK		(0xFFFF)
#define EMU_LOCK_LOCKKEY_LOCK		(0)
#define EMU_LOCK_LOCKKEY_UNLOCK		(0xADE8)

/* EMU_AUXCTRL */
#define EMU_AUXCTRL_HRCCLR		(1 << 0)

/* EMU_EM4CONF */
#define EMU_EM4CONF_LOCKCONF		(1 << 16)
#define EMU_EM4CONF_BUBODRSTDIS		(1 << 4)

#define EMU_EM4CONF_OSC_SHIFT		(2)
#define EMU_EM4CONF_OSC_MASK		(0x3 << EMU_EM4CONF_OSC_SHIFT)
#define EMU_EM4CONF_OSC(v)		\
	(((v) << EMU_EM4CONF_OSC_SHIFT) & EMU_EM4CONF_OSC_MASK)
#define EMU_EM4CONF_OSC_ULFRCO	0
#define EMU_EM4CONF_OSC_LFRCO	1
#define EMU_EM4CONF_OSC_LFXO	2

#define EMU_EM4CONF_BURTCWU		(1 << 1)
#define EMU_EM4CONF_VREGEN		(1 << 0)

/* EMU_BUCTRL */
#define EMU_BUCTRL_PROBE_SHIFT		(5)
#define EMU_BUCTRL_PROBE_MASK		(0x3 << EMU_BUCTRL_PROBE_SHIFT)
#define EMU_BUCTRL_PROBE(v)		\
	(((v) << EMU_BUCTRL_PROBE_SHIFT) & EMU_BUCTRL_PROBE_MASK)
#define EMU_BUCTRL_PROBE_DISABLE	0
#define EMU_BUCTRL_PROBE_VDDDREG	1
#define EMU_BUCTRL_PROBE_BUIN		2
#define EMU_BUCTRL_PROBE_BUOUT		3

#define EMU_BUCTRL_BUMODEBODEN		(1 << 3)
#define EMU_BUCTRL_BODCAL		(1 << 2)
#define EMU_BUCTRL_STATEN		(1 << 1)
#define EMU_BUCTRL_EN			(1 << 0)

/* EMU_PWRCONF */
#define EMU_PWRCONF_PWRRES_SHIFT	(3)
#define EMU_PWRCONF_PWRRES_MASK		(0x3 << EMU_PWRCONF_PWRRES_SHIFT)
#define EMU_PWRCONF_PWRRES(v)		\
	(((v) << EMU_PWRCONF_PWRRES_SHIFT) & EMU_PWRCONF_PWRRES_MASK)
#define EMU_PWRCONF_PWRRES_DISABLE	0
#define EMU_PWRCONF_PWRRES_VDDDREG	1
#define EMU_PWRCONF_PWRRES_BUIN		2
#define EMU_PWRCONF_PWRRES_BUOUT	3

#define EMU_PWRCONF_VOUTSTRONG		(1 << 2)
#define EMU_PWRCONF_VOUTMED		(1 << 1)
#define EMU_PWRCONF_VOUTWEAK		(1 << 0)

/* EMU_BUINACT */
#define EMU_BUINACT_PWRCON_SHIFT	(5)
#define EMU_BUINACT_PWRCON_MASK		(0x3 << EMU_BUINACT_PWRCON_SHIFT)
#define EMU_BUINACT_PWRCON(v)		\
	(((v) << EMU_BUINACT_PWRCON_SHIFT) & EMU_BUINACT_PWRCON_MASK)
#define EMU_BUINACT_PWRCON_NONE		0
#define EMU_BUINACT_PWRCON_BUMAIN	1
#define EMU_BUINACT_PWRCON_MAINBU	2
#define EMU_BUINACT_PWRCON_NODIODE	3

#define EMU_BUINACT_BUENRANGE_SHIFT	(3)
#define EMU_BUINACT_BUENRANGE_MASK	(0x3 << EMU_BUINACT_BUENRANGE_SHIFT)
#define EMU_BUINACT_BUENRANGE(v)	\
	(((v) << EMU_BUINACT_BUENRANGE_SHIFT) & EMU_BUINACT_BUENRANGE_MASK)

#define EMU_BUINACT_BUENTHRES_SHIFT	(0)
#define EMU_BUINACT_BUENTHRES_MASK	(0x7 << EMU_BUINACT_BUENTHRES_SHIFT)
#define EMU_BUINACT_BUENTHRES(v)	\
	(((v) << EMU_BUINACT_BUENTHRES_SHIFT) & EMU_BUINACT_BUENTHRES_MASK)

/* EMU_BUACT */
#define EMU_BUACT_PWRCON_SHIFT		(5)
#define EMU_BUACT_PWRCON_MASK		(0x3 << EMU_BUACT_PWRCON_SHIFT)
#define EMU_BUACT_PWRCON(v)			\
	(((v) << EMU_BUACT_PWRCON_SHIFT) & EMU_BUACT_PWRCON_MASK)
#define EMU_BUACT_PWRCON_NONE		0
#define EMU_BUACT_PWRCON_BUMAIN		1
#define EMU_BUACT_PWRCON_MAINBU		2
#define EMU_BUACT_PWRCON_NODIODE	3

#define EMU_BUACT_BUEXRANGE_SHIFT	(3)
#define EMU_BUACT_BUEXRANGE_MASK	(0x3 << EMU_BUACT_BUEXRANGE_SHIFT)
#define EMU_BUACT_BUEXRANGE(v)		\
	(((v) << EMU_BUACT_BUEXRANGE_SHIFT) & EMU_BUACT_BUEXRANGE_MASK)

#define EMU_BUACT_BUEXTHRES_SHIFT	(0)
#define EMU_BUACT_BUEXTHRES_MASK	(0x7 << EMU_BUACT_BUEXTHRES_SHIFT)
#define EMU_BUACT_BUEXTHRES(v)		\
	(((v) << EMU_BUACT_BUEXTHRES_SHIFT) & EMU_BUACT_BUEXTHRES_MASK)

/* EMU_STATUS */
#define EMU_STATUS_BURDY		(1 << 0)

/* EMU_ROUTE */
#define EMU_ROUTE_BUVINPEN		(1 << 0)

/* EMU_IF */
#define EMU_IF_BURDY			(1 << 0)

/* EMU_IFS */
#define EMU_IFS_BURDY			(1 << 0)

/* EMU_IFC */
#define EMU_IFC_BURDY			(1 << 0)

/* EMU_IEN */
#define EMU_IEN_BURDY			(1 << 0)

/* EMU_BUBODBUVINCAL */
#define EMU_BUBODBUVINCAL_RANGE_SHIFT	(3)
#define EMU_BUBODBUVINCAL_RANGE_MASK	(0x3 << EMU_BUBODBUVINCAL_RANGE_SHIFT)
#define EMU_BUBODBUVINCAL_RANGE(v)	\
	(((v) << EMU_BUBODBUVINCAL_RANGE_SHIFT) & \
	 EMU_BUBODBUVINCAL_RANGE_MASK)

#define EMU_BUBODBUVINCAL_THRES_SHIFT	(0)
#define EMU_BUBODBUVINCAL_THRES_MASK	(0x7 << EMU_BUBODBUVINCAL_THRES_SHIFT)
#define EMU_BUBODBUVINCAL_THRES(v)	\
	(((v) << EMU_BUBODBUVINCAL_THRES_SHIFT) & \
	 EMU_BUBODBUVINCAL_THRES_MASK)

/* EMU_BUBODUNREGCAL */
#define EMU_BUBODUNREGCAL_RANGE_SHIFT	(3)
#define EMU_BUBODUNREGCAL_RANGE_MASK	(0x3 << EMU_BUBODUNREGCAL_RANGE_SHIFT)
#define EMU_BUBODUNREGCAL_RANGE(v)	\
	(((v) << EMU_BUBODUNREGCAL_RANGE_SHIFT) & \
	 EMU_BUBODUNREGCAL_RANGE_MASK)

#define EMU_BUBODUNREGCAL_THRES_SHIFT	(0)
#define EMU_BUBODUNREGCAL_THRES_MASK	(0x7 << EMU_BUBODUNREGCAL_THRES_SHIFT)
#define EMU_BUBODUNREGCAL_THRES(v)	\
	(((v) << EMU_BUBODUNREGCAL_THRES_SHIFT) & \
	 EMU_BUBODUNREGCAL_THRES_MASK)

/**@}*/