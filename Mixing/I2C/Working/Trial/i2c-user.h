/* ------------------------------------------------------------------------ *
 *  i2c-user.h - user-space read/write to register-based i2c devices        *
 * ------------------------------------------------------------------------ *
   Copyright (C) 2006 Chuck Harrison  <cfharr@erols.com>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ------------------------------------------------------------------------ */

struct i2c_targ_dev {
	int	fd;	/* file descriptor for /dev/i2c-xx */
	__u16	device_addr;	/* 7- or 10-bit address */
	int	reg_addr_size;	/* 1 or 2 byte register addresses */
};

int write_regs(struct i2c_targ_dev *target, int reg, unsigned char *buf, int len);
int read_regs(struct i2c_targ_dev *target, int reg, unsigned char *buf, int len);
