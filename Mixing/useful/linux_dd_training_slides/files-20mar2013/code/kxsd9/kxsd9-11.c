/*
 * Kionix KXSD9 ± 2g Tri-Axis Digital Accelerometer support
 *
 * Copyright (c) 2012 Bill Gatliff and Associates, Inc.
 * Copyright (c) 2008-2009 Jonathan Cameron <jic23@cam.ac.uk>
 *
 * Author: Bill Gatliff <bgat@billgatliff.com>
 * Author: Jonathan Cameron <jic23@cam.ac.uk>
 *
 * This program is free software; you may redistribute and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* #define DEBUG */
#include <linux/module.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mutex.h>

enum {
	KXSD9_REG_XOUT_H	= 0x0,
	KXSD9_REG_XOUT_H__reserved	= 0,
	KXSD9_REG_XOUT_L	= 0x1,
	KXSD9_REG_XOUT_L__reserved	= 0xf,
	KXSD9_REG_YOUT_H	= 0x2,
	KXSD9_REG_YOUT_H__reserved	= 0,
	KXSD9_REG_YOUT_L	= 0x3,
	KXSD9_REG_YOUT_L__reserved	= 0xf,
	KXSD9_REG_ZOUT_H	= 0x4,
	KXSD9_REG_ZOUT_H__reserved	= 0,
	KXSD9_REG_ZOUT_L	= 0x5,
	KXSD9_REG_ZOUT_L__reserved	= 0xf,
	KXSD9_REG_AUXOUT_H	= 0x6,
	KXSD9_REG_AUXOUT_H__reserved	= 0,
	KXSD9_REG_AUXOUT_L	= 0x7,
	KXSD9_REG_AUXOUT_L__reserved	= 0xf,

	KXSD9_REG_RESET_WRITE	= 0xa,
	KXSD9_REG_RESET_WRITE_KEY	= 0xca,

	KXSD9_REG_CTRL_REGC	= 0xc,
	KXSD9_REG_CTRL_REGB	= 0xd,
	KXSD9_REG_CTRL_REGA	= 0xe,
};

struct kxsd9 {
	struct i2c_client *i2c;
};

/* TODO: is "int" vs. "s32" always ok here et. al? */
static int __kxs_reg_read(struct kxsd9 *kxs, int reg)
{
	return i2c_smbus_read_byte_data(kxs->i2c, reg);
}

static int __kxs_reg_write(struct kxsd9 *kxs, int reg, int val)
{
	return i2c_smbus_write_byte_data(kxs->i2c, reg, val);
}

static int __kxs_reset(struct kxsd9 *kxs)
{
	return __kxs_reg_write(kxs, KXSD9_REG_RESET_WRITE,
			       KXSD9_REG_RESET_WRITE_KEY);
}

#define KXS_REG_READ(_name) \
static int __kxs_reg_read_##_name(struct kxsd9 *kxs, int *v) \
{ \
	int ret = __kxs_reg_read(kxs, KXSD9_REG_##_name); \
	if (ret < 0) return ret; \
	*v = ret & ~(KXSD9_REG_##_name ##__reserved); \
	return 0;\
}

#define KXS_REG_SHOW(_name) \
static ssize_t kxsd9_show_##_name(struct device *dev, \
				 struct device_attribute *attr, \
				 char *buf) \
{ \
	struct kxsd9 *kxs = dev_get_drvdata(dev); \
	int v, ret; \
	ret = __kxs_reg_read_##_name(kxs, &v); \
	if (ret) return ret; \
	return sprintf(buf, "%d\n", v); \
}

#define KXS_READABLE(_name) \
KXS_REG_READ(_name); \
KXS_REG_SHOW(_name); \
static DEVICE_ATTR(_name, S_IRUGO, kxsd9_show_##_name, NULL);

KXS_READABLE(XOUT_H);
KXS_READABLE(XOUT_L);
KXS_READABLE(YOUT_H);
KXS_READABLE(YOUT_L);
KXS_READABLE(ZOUT_H);
KXS_READABLE(ZOUT_L);
KXS_READABLE(AUXOUT_H);
KXS_READABLE(AUXOUT_L);

static struct attribute *kxs_sysattr[] = {
	&dev_attr_XOUT_H.attr,
	&dev_attr_XOUT_L.attr,
	&dev_attr_YOUT_H.attr,
	&dev_attr_YOUT_L.attr,
	&dev_attr_ZOUT_H.attr,
	&dev_attr_ZOUT_L.attr,
	&dev_attr_AUXOUT_H.attr,
	&dev_attr_AUXOUT_L.attr,
	NULL,
};
static struct attribute_group kxs_sysattrg = {
	.attrs = kxs_sysattr,
};

static int kxs_probe(struct i2c_client *client,
		     const struct i2c_device_id *id)
{
	struct kxsd9 *kxs;
	int ret;

	kxs = kzalloc(sizeof(*kxs), GFP_KERNEL);
	if (!kxs)
		return -ENOMEM;

	i2c_set_clientdata(client, kxs);
	kxs->i2c = client;

	ret = __kxs_reset(kxs);
	if (ret)
		goto err_reset;

	ret = sysfs_create_group(&client->dev.kobj, &kxs_sysattrg);
	if (ret)
		goto err_sysfs_create_group;

	return 0;

err_sysfs_create_group:
err_reset:
	kfree(kxs);
	return ret;
}

static int kxs_remove(struct i2c_client *client)
{
	struct kxsd9 *kxs = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &kxs_sysattrg);
	i2c_set_clientdata(kxs->i2c, NULL);
	kfree(kxs);
	return 0;
}

static const struct i2c_device_id kxs_i2c_id[] = {
	{"kxsd9", 0},
	{}
};

static struct i2c_driver kxs_driver = {
	.driver = {
		.name  = "kxsd9",
		.owner = THIS_MODULE,
	},
	.probe         = kxs_probe,
	.remove        = kxs_remove,
	.id_table      = kxs_i2c_id,
};


static int __init kxs_init(void)
{
	return i2c_add_driver(&kxs_driver);
}
module_init(kxs_init);

static void __exit kxs_exit(void)
{
	i2c_del_driver(&kxs_driver);
}
module_exit(kxs_exit);


MODULE_LICENSE("GPL");
MODULE_ALIAS("kxsd9");
MODULE_AUTHOR("Bill Gatliff <bgat@billgatliff.com>");
MODULE_DESCRIPTION("Kionix KXSD9 ± 2g Tri-Axis Digital Accelerometer support (i2c and spi)");
