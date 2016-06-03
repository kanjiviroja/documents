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
#include <linux/regulator/consumer.h>
#include <linux/input.h>
#include <linux/interrupt.h>

enum {
	/* TODO: these are i2c-only register addresses! */

	KXSD9_REG_XOUT_H	= 0x0,
	KXSD9_REG_XOUT_H__reserved	= 0,

	KXSD9_REG_XOUT_L	= 0x1,
	KXSD9_REG_XOUT_L__shift		= 4,
	KXSD9_REG_XOUT_L__reserved	= 0xf,

	KXSD9_REG_ZOUT_H	= 0x4,
	KXSD9_REG_ZOUT_H__reserved	= 0,

	KXSD9_REG_ZOUT_L	= 0x5,
	KXSD9_REG_ZOUT_L__shift		= 4,
	KXSD9_REG_ZOUT_L__reserved	= 0xf,

	KXSD9_REG_AUXOUT_H	= 0x6,
	KXSD9_REG_AUXOUT_H__reserved	= 0,

	KXSD9_REG_AUXOUT_L	= 0x7,
	KXSD9_REG_AUXOUT_L__shift	= 4,
	KXSD9_REG_AUXOUT_L__reserved	= 0xf,

	KXSD9_REG_RESET_WRITE	= 0xa,
	KXSD9_REG_RESET_WRITE_KEY	= 0xca,
	KXSD9_REG_RESET_WRITE__reserved	= 0,

	KXSD9_REG_CTRL_REGC	= 0xc,
	KXSD9_REG_CTRL_REGC__reserved	= 0x4,
	KXSD9_REG_CTRL_REGC__FS0__shift		= 0,
	KXSD9_REG_CTRL_REGC__FS0__mask		= (1 << 0), /* TODO: BIT(0)? */
	KXSD9_REG_CTRL_REGC__FS1__shift		= 1,
	KXSD9_REG_CTRL_REGC__FS1__mask		= (1 << 1), /* TODO: ibid */
	KXSD9_REG_CTRL_REGC__MOTIAT__shift	= 3,
	KXSD9_REG_CTRL_REGC__MOTIAT__mask	= (1 << 3),
	KXSD9_REG_CTRL_REGC__MOTIEV__shift	= 4,
	KXSD9_REG_CTRL_REGC__MOTIEV__mask	= (1 << 4),
	KXSD9_REG_CTRL_REGC__LP0__shift		= 5,
	KXSD9_REG_CTRL_REGC__LP0__mask		= (1 << 5),
	KXSD9_REG_CTRL_REGC__LP1__shift		= 6,
	KXSD9_REG_CTRL_REGC__LP1__mask		= (1 << 6),
	KXSD9_REG_CTRL_REGC__LP2__shift		= 7,
	KXSD9_REG_CTRL_REGC__LP2__mask		= (1 << 7),

	KXSD9_REG_CTRL_REGB	= 0xd,
	KXSD9_REG_CTRL_REGB__reserved	= 0x1b,
	KXSD9_REG_CTRL_REGB__MOTIEN__shift	= 2,
	KXSD9_REG_CTRL_REGB__MOTIEN__mask	= (1 << 2),
	KXSD9_REG_CTRL_REGB__ENABLE__shift	= 6,
	KXSD9_REG_CTRL_REGB__ENABLE__mask	= (1 << 6),
	KXSD9_REG_CTRL_REGB__CLKHLD__shift	= 7,
	KXSD9_REG_CTRL_REGB__CLKHLD__mask	= (1 << 7),

	KXSD9_REG_CTRL_REGA	= 0xe,
	KXSD9_REG_CTRL_REGA__reserved	= 0xfd,
	KXSD9_REG_CTRL_REGA__MOTI__shift	= 1,
	KXSD9_REG_CTRL_REGA__MOTI__mask		= (1 << 1),
};

struct kxsd9 {
	struct i2c_client *i2c;

	struct regulator *vdd;
	struct regulator *vddio;

	struct mutex mutex;
};

static int kxs_mutex_lock(struct kxsd9 *kxs, const char *msg)
{
	dev_dbg(&kxs->i2c->dev, "%s (%s)\n", __func__, msg);
	return mutex_lock_interruptible(&kxs->mutex);
}

static void kxs_mutex_unlock(struct kxsd9 *kxs, const char *msg)
{
	mutex_unlock(&kxs->mutex);
	dev_dbg(&kxs->i2c->dev, "%s (%s)\n", __func__, msg);
}

static int __kxs_enable_regulators(struct kxsd9 *kxs)
{
	int err = 0;

	/* the fussy IS_ERR_OR_NULL() everywhere is because vdd and
	 * vddio might be unspecified, which someone should really
	 * care about---but not us */
	if (!IS_ERR_OR_NULL(kxs->vdd)) {
		err = regulator_enable(kxs->vdd);
		if (err < 0)
			goto err_vdd_enable;
	}

	if (!IS_ERR_OR_NULL(kxs->vddio)) {
		err = regulator_enable(kxs->vddio);
		if (err < 0)
			goto err_vddio_enable;
	}

	/* from "Table 2: Electrical" in KXSD9-2050 Rev. 3 Jul-2010,
	 * longest power-up time is 15.9 ms typical, lower if you
	 * consider LPF settings.
	 * TODO: tighten up this range later?
	 * TODO: if regulators weren't previously off, no delay needed?
	 */
	usleep_range(16000UL, 16000UL);
	return 0;

err_vddio_enable:
	if (!IS_ERR_OR_NULL(kxs->vdd))
		regulator_disable(kxs->vdd);
err_vdd_enable:
	return err;
}

static int __kxs_disable_regulators(struct kxsd9 *kxs)
{
	if (!IS_ERR_OR_NULL(kxs->vddio))
		regulator_enable(kxs->vddio);
	if (!IS_ERR_OR_NULL(kxs->vdd))
		regulator_disable(kxs->vdd);
	return 0;
}

static void kxs_put_regulators(struct kxsd9 *kxs)
{
	if (!IS_ERR_OR_NULL(kxs->vddio))
		regulator_put(kxs->vddio);
	if (!IS_ERR_OR_NULL(kxs->vdd))
		regulator_put(kxs->vdd);
}

static int kxs_get_regulators(struct kxsd9 *kxs)
{
	int err = 0;

	kxs->vdd = regulator_get(&kxs->i2c->dev, "VDD");
	if (!IS_ERR_OR_NULL(kxs->vdd)) {
		err = regulator_set_voltage(kxs->vdd, 1800000UL, 3300000UL);
		if (err) {
			dev_dbg(&kxs->i2c->dev,
				"%s: regulator_set_voltage(VDD) "
				"returned %d (fatal)\n",
				__func__, err);
			goto err_set_vdd;
		}
	}

#error "You need to fix up kxs_get_regulators()"
	kxs->vddio = regulator_get(&kxs->i2c->dev, "VDDIO");
	if (!IS_ERR_OR_NULL(kxs->vddio)) {
		/* TODO: this spec is really 1.2V-Vdd, but a
		 * regulator_get_voltage(VDD) might return a different
		 * value later that I will need a notifier to keep
		 * track of; hope for the best for now */
		err = regulator_set_voltage(kxs->vddio, 1200000UL, 3300000UL);
		if (err) {
#if 0
			dev_dbg(&kxs->i2c->dev,
				"%s: regulator_set_voltage(VDDIO) "
				"returned %d (fatal)\n",
				__func__, err);
			goto err_set_vddio;
#else
			/* TODO: fix up platform issues that trigger this error! */
			WARN(err < 0, "%s: regulator_set_voltage(VDDIO) "
			     "returned %d (fatal)\n", __func__, err);
			err = 0;
#endif
		}
	}

	return 0;

#if 0
err_set_vddio:
#endif
	if (!IS_ERR_OR_NULL(kxs->vddio))
		regulator_put(kxs->vddio);
err_set_vdd:
	if (!IS_ERR_OR_NULL(kxs->vdd))
		regulator_put(kxs->vdd);
	return err;
}

/* TODO: is "int" vs. "s32" always ok here et. al? */
static int __kxs_reg_read(struct kxsd9 *kxs, int reg)
{
	if (kxs->i2c)
		return i2c_smbus_read_byte_data(kxs->i2c, reg);
	return -ENODEV;
}
static int __kxs_reg_write(struct kxsd9 *kxs, int reg, int val)
{
	if (kxs->i2c)
		return i2c_smbus_write_byte_data(kxs->i2c, reg, val);
	return -ENODEV;
}

#if 0
/* TODO: test this */
static int __kxs_reg16_read(struct kxsd9 *kxs, int reg)
{
	if (kxs->i2c)
		return i2c_smbus_read_word_data(kxs->i2c, reg);
	return -ENODEV;
}
#endif


/* TODO: NOTE: The kxsd9 registers can be read anytime the chip is
 * powered up, but the X, Y, Z, and AUX register values (and probably
 * others) are undefined if CTRL_REGB.ENABLE == 0.
 *
 * TODO: figure out what, if anything, to do pre- and post-read
 * e.g. push and pop ENABLE state.
 */
#define KXS_REG_READ(_name) \
	static int __kxs_reg_read_##_name(struct kxsd9 *kxs) \
{ \
	int ret; \
	switch (KXSD9_REG_##_name) { \
	} \
	ret = __kxs_reg_read(kxs, KXSD9_REG_##_name);	\
	if (ret < 0) \
		return ret; \
	ret &= ~KXSD9_REG_##_name ## __reserved; \
	switch (KXSD9_REG_##_name) { \
	} \
	dev_dbg(&kxs->i2c->dev, "%s: %02x\n", __func__, ret); \
	return ret; \
}
#define KXS_REG_WRITE(_name) \
static int __kxs_reg_write_##_name(struct kxsd9 *kxs, int v) \
{ \
	int ret; \
	dev_dbg(&kxs->i2c->dev, "%s: %02x\n", __func__, v); \
	v &= ~KXSD9_REG_##_name ## __reserved; \
	switch (KXSD9_REG_##_name) { \
	} \
	ret = __kxs_reg_write(kxs, KXSD9_REG_##_name, v); \
	if (ret < 0) \
		return ret; \
	switch (KXSD9_REG_##_name) { \
	/* TODO: I don't really know the required wait time */ \
		/* case KXSD9_REG_RESET_WRITE: usleep_range(16000UL, 16000UL); break; */  \
	} \
	return 0; \
}
#define KXS_REG_ATTR_SHOW(_name) \
static ssize_t kxs_show_##_name(struct device *dev, \
				struct device_attribute *attr,	\
				char *buf)			\
{ \
	struct kxsd9 *kxs = dev_get_drvdata(dev); \
	int ret; \
	ret = kxs_mutex_lock(kxs, __func__); \
	if (ret < 0) \
		return ret; \
	ret = __kxs_reg_read_##_name(kxs); \
	kxs_mutex_unlock(kxs, __func__); \
	return (ret < 0) ? ret : sprintf(buf, "%02x\n", ret);	\
}
#define KXS_REG_ATTR_STORE(_name) \
static ssize_t kxs_store_##_name(struct device *dev, \
				 struct device_attribute *attr,	 \
				 const char *buf, size_t len)	 \
{ \
	struct kxsd9 *kxs = dev_get_drvdata(dev); \
	int ret; \
	unsigned long v; \
	ret = strict_strtoul(buf, 16, &v); \
	if (ret) \
		return ret; \
	ret = kxs_mutex_lock(kxs, __func__); \
	if (ret < 0) \
		return ret; \
	switch (KXSD9_REG_##_name) { \
	default: \
		ret = __kxs_reg_write_##_name(kxs, v);	\
		break; \
	} \
	kxs_mutex_unlock(kxs, __func__); \
	return (ret < 0) ? ret : len;	 \
}

#define KXS_REG(_name) \
	KXS_REG_READ(_name) \
	KXS_REG_ATTR_SHOW(_name) \
	static DEVICE_ATTR(_name, S_IRUGO, kxs_show_##_name, NULL);
#define KXS_REG_READWRITE(_name) \
	KXS_REG_READ(_name) \
	KXS_REG_WRITE(_name) \
	KXS_REG_ATTR_SHOW(_name) \
	KXS_REG_ATTR_STORE(_name) \
	static DEVICE_ATTR(_name, S_IRUGO | S_IWUSR, \
			   kxs_show_##_name, kxs_store_##_name);
#define KXS_REG_WRITEONLY(_name) \
	KXS_REG_WRITE(_name) \
	KXS_REG_ATTR_STORE(_name) \
	static DEVICE_ATTR(_name, S_IWUSR, NULL, kxs_store_##_name);

/* NOTE: don't use these to read acceleration data in the usual way,
 * since they don't provide any interlocks between the high and low
 * bytes; these are provided only for register-level testing */
KXS_REG(XOUT_H);
KXS_REG(XOUT_L);
/*
KXS_REG(YOUT_H);
KXS_REG(YOUT_L);
*/
KXS_REG(ZOUT_H);
KXS_REG(ZOUT_L);

KXS_REG(AUXOUT_H);
KXS_REG(AUXOUT_L);

KXS_REG_WRITEONLY(RESET_WRITE);
KXS_REG_READWRITE(CTRL_REGC);
KXS_REG_READWRITE(CTRL_REGB);
KXS_REG_READWRITE(CTRL_REGA);

static int __kxs_reset(struct kxsd9 *kxs)
{
	/* NOTE: required (?) post-reset wait is implemented in
	 * __kxs_reg_write_RESET_WRITE() */
	/* TODO: datasheet doesn't suggest that this command restores
	 * other registers to factory default values, so we probably should */
#error "You need to fix up __kxs_reset() and __kxs_reg_write_RESET_WRITE()"
	return __kxs_reg_write_RESET_WRITE(kxs, KXSD9_REG_RESET_WRITE_KEY);
}

#if 0
/* TODO: commented-out due to temporary lack of use */

/* Sets indicated bits in the CTRL_REGB register; leaves other bits alone
 *
 * @clkhld		- set the CTRL_REGB_CLKHLD bit
 * @enable		- set the CTRL_REGB_ENABLE bit
 * @st			- set the CTRL_REGB_ST bit
 * @motien		- set the CTRL_REGB_MOTIEN bit
 *
 * Returns zero on success, an errno otherwise.
 */
static int __kxs_set_CTRL_REGB(struct kxsd9 *kxs,
			       int clkhld, int enable,
			       int st, int motien)
{
	int ret, ctrl_regb;

	ret = __kxs_reg_read_CTRL_REGB(kxs);
	if (ret < 0)
		goto done;

	ctrl_regb = ret;
	if (clkhld)
		ctrl_regb |= KXSD9_REG_CTRL_REGB__CLKHLD__mask;
	if (enable)
		ctrl_regb |= KXSD9_REG_CTRL_REGB__ENABLE__mask;
	if (motien)
		ctrl_regb |= KXSD9_REG_CTRL_REGB__MOTIEN__mask;
	ret = __kxs_reg_write_CTRL_REGB(kxs, ctrl_regb);
done:
	return (ret < 0) ? ret : 0;
}

/* Clears indicated bits in the CTRL_REGB register; leaves other bits alone
 *
 * @clkhld		- clear the CTRL_REGB_CLKHLD bit
 * @enable		- clear the CTRL_REGB_ENABLE bit
 * @st			- clear the CTRL_REGB_ST bit
 * @motien		- clear the CTRL_REGB_MOTIEN bit
 *
 * Returns zero on success, an errno otherwise.
 */
static int __kxs_clear_CTRL_REGB(struct kxsd9 *kxs,
				 int clkhld, int enable,
				 int st, int motien)
{
	int ret, ctrl_regb;

	ret = __kxs_reg_read_CTRL_REGB(kxs);
	if (ret < 0)
		goto done;

	ctrl_regb = ret;
	if (clkhld)
		ctrl_regb &= ~KXSD9_REG_CTRL_REGB__CLKHLD__mask;
	if (enable)
		ctrl_regb &= ~KXSD9_REG_CTRL_REGB__ENABLE__mask;
	if (motien)
		ctrl_regb &= ~KXSD9_REG_CTRL_REGB__MOTIEN__mask;
	ret = __kxs_reg_write_CTRL_REGB(kxs, ctrl_regb);
done:
	return (ret < 0) ? ret : 0;
}
#endif

static struct attribute *kxs_attributes[] = {
	&dev_attr_XOUT_H.attr,
	&dev_attr_XOUT_L.attr,
/*
	&dev_attr_YOUT_H.attr,
	&dev_attr_YOUT_L.attr,
*/
	&dev_attr_ZOUT_H.attr,
	&dev_attr_ZOUT_L.attr,
	&dev_attr_AUXOUT_H.attr,
	&dev_attr_AUXOUT_L.attr,
	&dev_attr_RESET_WRITE.attr,
	&dev_attr_CTRL_REGC.attr,
	&dev_attr_CTRL_REGB.attr,
	&dev_attr_CTRL_REGA.attr,
	NULL,
};

static struct attribute_group kxs_attribute_group = {
	.attrs = kxs_attributes,
};

static int kxs_probe(struct i2c_client *client,
		     const struct i2c_device_id *id)
{
	struct kxsd9 *kxs;
	/* struct kxsd9_platform_data *pdata = client->dev.platform_data; */
	int ret;

	kxs = kzalloc(sizeof(*kxs), GFP_KERNEL);
	if (!kxs)
		return -ENOMEM;

	i2c_set_clientdata(client, kxs);
	kxs->i2c = client;
	mutex_init(&kxs->mutex);

	ret = kxs_get_regulators(kxs);
	if (ret)
		goto err_get_regulators;

	/* TODO: error handling! */
	ret = kxs_mutex_lock(kxs, __func__);
	ret = __kxs_enable_regulators(kxs);
	ret = __kxs_reset(kxs);

	/* TODO: self-test? */

	/* TODO: be smarter than just turning the chip fully-on */
	ret = __kxs_reg_write_CTRL_REGB(kxs, KXSD9_REG_CTRL_REGB__ENABLE__mask);

	kxs_mutex_unlock(kxs, __func__);

#if 0
	ret = sysfs_create_group(&kxs->i2c->dev.kobj, &kxs_attribute_group);
	if (ret)
		goto err_sysfs_create_group;
#endif

	return 0;

#if 0
	sysfs_remove_group(&kxs->i2c->dev.kobj, &kxs_attribute_group);
err_sysfs_create_group:
#endif
	kxs_put_regulators(kxs);
err_get_regulators:
	i2c_set_clientdata(kxs->i2c, NULL);
	kfree(kxs);
	return ret;
}

static int kxs_remove(struct i2c_client *client)
{
	struct kxsd9 *kxs = i2c_get_clientdata(client);

	sysfs_remove_group(&kxs->i2c->dev.kobj, &kxs_attribute_group);
	i2c_set_clientdata(kxs->i2c, NULL);
	__kxs_disable_regulators(kxs);
	kxs_put_regulators(kxs);
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
#if 0
	/* TODO: platform-initiated suspend/resume implementations */
	.suspend       = kxs_suspend,
	.resume        = kxs_resume,
#endif
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
