/***************************************************************************
 *   Copyright (C) 2005 by Dominic Rath                                    *
 *   Dominic.Rath@gmx.de                                                   *
 *                                                                         *
 *   Copyright (C) 2007,2008 Øyvind Harboe                                 *
 *   oyvind.harboe@zylin.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "arm_jtag.h"


#if 0
#define _ARM_JTAG_SCAN_N_CHECK_
#endif

int arm_jtag_set_instr_inner(struct arm_jtag *jtag_info, uint32_t new_instr,  void *no_verify_capture, tap_state_t end_state)
{
	struct jtag_tap *tap;
	tap = jtag_info->tap;
	struct scan_field field;
	uint8_t t[4];

	field.num_bits = tap->ir_length;
	field.out_value = t;
	buf_set_u32(field.out_value, 0, field.num_bits, new_instr);
	field.in_value = NULL;

	if (no_verify_capture == NULL)
	{
		jtag_add_ir_scan(tap, &field, end_state);
	} else
	{
		/* FIX!!!! this is a kludge!!! arm926ejs.c should reimplement this arm_jtag_set_instr to
		 * have special verification code.
		 */
		jtag_add_ir_scan_noverify(tap, &field, end_state);
	}

	return ERROR_OK;
}

int arm_jtag_scann_inner(struct arm_jtag *jtag_info, uint32_t new_scan_chain, tap_state_t end_state)
{
	int retval = ERROR_OK;
	uint32_t values[1];
	int num_bits[1];

	values[0]=new_scan_chain;
	num_bits[0]=jtag_info->scann_size;

	if ((retval = arm_jtag_set_instr(jtag_info, jtag_info->scann_instr, NULL, end_state)) != ERROR_OK)
	{
		return retval;
	}

	jtag_add_dr_out(jtag_info->tap,
			1,
			num_bits,
			values,
			end_state);

	jtag_info->cur_scan_chain = new_scan_chain;

	return retval;
}

int arm_jtag_reset_callback(enum jtag_event event, void *priv)
{
	struct arm_jtag *jtag_info = priv;

	if (event == JTAG_TRST_ASSERTED)
	{
		jtag_info->cur_scan_chain = 0;
	}

	return ERROR_OK;
}

int arm_jtag_setup_connection(struct arm_jtag *jtag_info)
{
	jtag_info->scann_instr = 0x2;
	jtag_info->cur_scan_chain = 0;
	jtag_info->intest_instr = 0xc;

	return jtag_register_event_callback(arm_jtag_reset_callback, jtag_info);
}
