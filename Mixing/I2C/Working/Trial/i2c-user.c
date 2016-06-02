/* ------------------------------------------------------------------------ *
 *  i2c-user.c - user-space read/write to register-based i2c devices        *
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


#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include </usr/include/linux/i2c-dev.h>
#include <errno.h>
#include "i2c-user.h"
#include <linux/i2c.h>
#include <linux/i2c-smbus.h>

/* TODO: include i2c.h effectively

  Some structures and constants defined in linux/i2c.h for kernel
  environment are needed for ioctl parameters in this user-space
  program. For now we just copy it over. */

/* this is for i2c-dev.c	*/
#define I2C_SLAVE	0x0703	/* Change slave address			*/
				/* Attn.: Slave address is 7 or 10 bits */
#define I2C_SLAVE_FORCE	0x0706	/* Change slave address			*/
				/* Attn.: Slave address is 7 or 10 bits */
				/* This changes the address, even if it */
				/* is already taken!			*/
#define I2C_TENBIT	0x0704	/* 0 for 7 bit addrs, != 0 for 10 bit	*/

#define I2C_FUNCS	0x0705	/* Get the adapter functionality */
#define I2C_RDWR	0x0707	/* Combined R/W transfer (one stop only)*/
#define I2C_PEC		0x0708	/* != 0 for SMBus PEC                   */
/*
 * I2C Message - used for pure i2c transaction, also from /dev interface
 */
//struct i2c_msg {
//	__u16 addr;	/* slave address			*/
//	__u16 flags;		
//#define I2C_M_TEN	0x10	/* we have a ten bit chip address	*/
//#define I2C_M_RD	0x01
//#define I2C_M_NOSTART	0x4000
//#define I2C_M_REV_DIR_ADDR	0x2000
//#define I2C_M_IGNORE_NAK	0x1000
//#define I2C_M_NO_RD_ACK		0x0800
//#define I2C_M_RECV_LEN		0x0400 /* length will be first received byte */
//#define I2C_M_RECV_PEC		0x0200 /* receive one more than the returned
//					  length byte for the PEC */
//	__u16 len;		/* msg length				*/
//	__u8 *buf;		/* pointer to msg data			*/
//	int err;
//	short done;
//};
/* end of <linux/i2c.h include */

int write_regs(struct i2c_targ_dev *target, int reg, unsigned char *buf, int len) {
	int rc;
	struct i2c_msg wmsg = {
		.addr	= target->device_addr,
		.flags	= 0,
	};
	struct i2c_rdwr_ioctl_data ioctl_data = {
		.msgs	= &wmsg,
		.nmsgs	= 1L
	};
	/* allocate a message buffer with room for register address */
	wmsg.len = len + target->reg_addr_size;
	if(NULL==(wmsg.buf = malloc(wmsg.len)))
		return -ENOMEM;
	if (target->reg_addr_size==1)
		wmsg.buf[0] = reg & 0xFF;
	else {
		wmsg.buf[0] = reg>>8 & 0xFF;	/* MSB of device register address */
		wmsg.buf[1] = reg & 0xFF;	/* LSB */
	}
	memcpy(wmsg.buf+target->reg_addr_size, buf, len);
	if (!target->fd)
		return(-1);	/* device has not been opened */
	if ((rc=ioctl(target->fd,I2C_SLAVE,target->device_addr)) < 0)
		return(rc);
	if ((rc=ioctl(target->fd,I2C_RDWR,&ioctl_data)) < 0)
		return(rc);
	free(wmsg.buf);
	return(0);
}

int read_regs(struct i2c_targ_dev *target, int reg, unsigned char *buf, int len) {
	int rc;
	unsigned char reg_addr[2] = {
		(reg>>8)&0xFF, reg&0xFF };
	struct i2c_msg wmsgs[2] = {
		{
		.addr	= target->device_addr,
		.flags	= 0,
		.len	= target->reg_addr_size,
		.buf	= reg_addr
		},
		{
		.addr	= target->device_addr,
		.flags	= I2C_M_RD,
		.len	= len,
		.buf	= buf
		}
	};
	struct i2c_rdwr_ioctl_data ioctl_data = {
		.msgs	= wmsgs,
		.nmsgs	= 2L
	};
	if (!target->fd)
		return(-1);	/* device has not been opened */
	if ((rc=ioctl(target->fd,I2C_SLAVE,target->device_addr)) < 0)
		return(rc);
	if ((rc=ioctl(target->fd,I2C_RDWR,&ioctl_data)) < 0) 
		return(rc);
	return(0);
}

