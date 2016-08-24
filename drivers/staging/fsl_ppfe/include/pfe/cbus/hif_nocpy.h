/*
 *  Copyright (c) 2011, 2014 Freescale Semiconductor, Inc.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
*/
#ifndef _HIF_NOCPY_H_
#define _HIF_NOCPY_H_

#define HIF_NOCPY_VERSION		(HIF_NOCPY_BASE_ADDR + 0x00)
#define HIF_NOCPY_TX_CTRL		(HIF_NOCPY_BASE_ADDR + 0x04)
#define HIF_NOCPY_TX_CURR_BD_ADDR	(HIF_NOCPY_BASE_ADDR + 0x08)
#define HIF_NOCPY_TX_ALLOC		(HIF_NOCPY_BASE_ADDR + 0x0c)
#define HIF_NOCPY_TX_BDP_ADDR		(HIF_NOCPY_BASE_ADDR + 0x10)
#define HIF_NOCPY_TX_STATUS		(HIF_NOCPY_BASE_ADDR + 0x14)
#define HIF_NOCPY_RX_CTRL		(HIF_NOCPY_BASE_ADDR + 0x20)
#define HIF_NOCPY_RX_BDP_ADDR		(HIF_NOCPY_BASE_ADDR + 0x24)
#define HIF_NOCPY_RX_STATUS		(HIF_NOCPY_BASE_ADDR + 0x30)
#define HIF_NOCPY_INT_SRC		(HIF_NOCPY_BASE_ADDR + 0x34)
#define HIF_NOCPY_INT_ENABLE		(HIF_NOCPY_BASE_ADDR + 0x38)
#define HIF_NOCPY_POLL_CTRL		(HIF_NOCPY_BASE_ADDR + 0x3c)
#define HIF_NOCPY_RX_CURR_BD_ADDR	(HIF_NOCPY_BASE_ADDR + 0x40)
#define HIF_NOCPY_RX_ALLOC		(HIF_NOCPY_BASE_ADDR + 0x44)
#define HIF_NOCPY_TX_DMA_STATUS		(HIF_NOCPY_BASE_ADDR + 0x48)
#define HIF_NOCPY_RX_DMA_STATUS		(HIF_NOCPY_BASE_ADDR + 0x4c)
#define HIF_NOCPY_RX_INQ0_PKTPTR	(HIF_NOCPY_BASE_ADDR + 0x50)
#define HIF_NOCPY_RX_INQ1_PKTPTR	(HIF_NOCPY_BASE_ADDR + 0x54)
#define HIF_NOCPY_TX_PORT_NO		(HIF_NOCPY_BASE_ADDR + 0x60)
#define HIF_NOCPY_LMEM_ALLOC_ADDR	(HIF_NOCPY_BASE_ADDR + 0x64)
#define HIF_NOCPY_CLASS_ADDR		(HIF_NOCPY_BASE_ADDR + 0x68)
#define HIF_NOCPY_TMU_PORT0_ADDR	(HIF_NOCPY_BASE_ADDR + 0x70)
#define HIF_NOCPY_TMU_PORT1_ADDR	(HIF_NOCPY_BASE_ADDR + 0x74)
#define HIF_NOCPY_TMU_PORT2_ADDR	(HIF_NOCPY_BASE_ADDR + 0x7c)
#define HIF_NOCPY_TMU_PORT3_ADDR	(HIF_NOCPY_BASE_ADDR + 0x80)
#define HIF_NOCPY_TMU_PORT4_ADDR	(HIF_NOCPY_BASE_ADDR + 0x84)
#define HIF_NOCPY_INT_COAL		(HIF_NOCPY_BASE_ADDR + 0x90)


#endif /* _HIF_NOCPY_H_ */
