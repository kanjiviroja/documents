/*
 * Bosch Sensortec BMA250/BMA255 Triaxial Accelerometer support
 *
 * Copyright (C) 2012 Sony Ericsson Mobile Communications AB.
 *
 * Author: Bill Gatliff <bill.gatliff@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 and only version 2 as published by the Free Software Foundation.
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
/* TODO: support for the BMA255! */
#include <linux/module.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/bma25x-mib.h>

static int default_autosuspend_ms = 10;
module_param(default_autosuspend_ms, int, S_IRUGO);

static int default_evdev_delay_ms = 500;
module_param(default_evdev_delay_ms, int, S_IRUGO);

enum {
	BMA250_REG_CHIP_ID = 0,
	BMA250_REG_CHIP_ID__reserved = 0,

	/* TODO: marked as "reserved" in my datasheet! */
	BMA250_REG_VERSION = 1,
	BMA250_REG_VERSION__reserved = 0,

	BMA250_REG_X_AXIS_LSB = 2,
	BMA250_REG_X_AXIS_LSB__reserved = 0x3e,

	BMA250_REG_X_AXIS_MSB = 3,
	BMA250_REG_X_AXIS_MSB__reserved = 0,

	/* an alias for X_LSB and X_MSB */
	BMA250_REG_X_AXIS = BMA250_REG_X_AXIS_LSB,
	BMA250_REG_X_AXIS__ACCEL__shift = 6,
	BMA250_REG_X_AXIS__NEW_DATA = (1 << 0),

	BMA250_REG_Y_AXIS_LSB = 4,
	BMA250_REG_Y_AXIS_LSB__reserved = 0x3e,

	BMA250_REG_Y_AXIS_MSB = 5,
	BMA250_REG_Y_AXIS_MSB__reserved = 0,

	/* an alias for Y_LSB and Y_MSB */
	BMA250_REG_Y_AXIS = BMA250_REG_Y_AXIS_LSB,
	BMA250_REG_Y_AXIS__ACCEL__shift = 6,
	BMA250_REG_Y_AXIS__NEW_DATA = (1 << 0),

	BMA250_REG_Z_AXIS_LSB = 6,
	BMA250_REG_Z_AXIS_LSB__reserved = 0x3e,

	BMA250_REG_Z_AXIS_MSB = 7,
	BMA250_REG_Z_AXIS_MSB__reserved = 0,

	/* an alias for Z_LSB and Z_MSB */
	BMA250_REG_Z_AXIS = BMA250_REG_Z_AXIS_LSB,
	BMA250_REG_Z_AXIS__ACCEL__shift = 6,
	BMA250_REG_Z_AXIS__NEW_DATA = (1 << 0),

	BMA250_REG_TEMP = 8,
	BMA250_REG_TEMP__shift = 0,
	BMA250_REG_TEMP__reserved = 0,

	BMA250_REG_STATUS = 9,
	BMA250_REG_STATUS__FLAT_INT = (1 << 7),
	BMA250_REG_STATUS__ORIENT_INT = (1 << 6),
	BMA250_REG_STATUS__S_TAP_INT = (1 << 5),
	BMA250_REG_STATUS__D_TAP_INT = (1 << 4),
	BMA250_REG_STATUS__reserved = (1 << 3),
	BMA250_REG_STATUS__SLOPE_INT = (1 << 2),
	BMA250_REG_STATUS__HIGH_INT = (1 << 1),
	BMA250_REG_STATUS__LOW_INT = (1 << 0),

	BMA250_REG_DATA_INT = 0xa,
	BMA250_REG_DATA_INT__DATA_INT = (1 << 7),
	BMA250_REG_DATA_INT__reserved = 0x7f,

	BMA250_REG_TAP_SLOPE_INT_STATUS = 0xb,
	BMA250_REG_TAP_SLOPE_INT_STATUS__TAP_SIGN = (1 << 7),
	BMA250_REG_TAP_SLOPE_INT_STATUS__TAP_FIRST_Z = (1 << 6),
	BMA250_REG_TAP_SLOPE_INT_STATUS__TAP_FIRST_Y = (1 << 5),
	BMA250_REG_TAP_SLOPE_INT_STATUS__TAP_FIRST_X = (1 << 4),
	BMA250_REG_TAP_SLOPE_INT_STATUS__SLOPE_SIGN = (1 << 3),
	BMA250_REG_TAP_SLOPE_INT_STATUS__SLOPE_FIRST_Z = (1 << 2),
	BMA250_REG_TAP_SLOPE_INT_STATUS__SLOPE_FIRST_Y = (1 << 1),
	BMA250_REG_TAP_SLOPE_INT_STATUS__SLOPE_FIRST_X = (1 << 0),
	BMA250_REG_TAP_SLOPE_INT_STATUS__reserved = 0,

	BMA250_REG_FLAT_ORIENT_STATUS = 0xc,
	BMA250_REG_FLAT_ORIENT_STATUS__FLAT = (1 << 7),
	BMA250_REG_FLAT_ORIENT_STATUS__ORIENT_Z = (1 << 6),
	BMA250_REG_FLAT_ORIENT_STATUS__ORIENT_XY = ((1 << 5) | (1 << 4)),
	BMA250_REG_FLAT_ORIENT_STATUS__HIGH_SIGN = (1 << 3),
	BMA250_REG_FLAT_ORIENT_STATUS__HIGH_FIRST_Z = (1 << 2),
	BMA250_REG_FLAT_ORIENT_STATUS__HIGH_FIRST_Y = (1 << 1),
	BMA250_REG_FLAT_ORIENT_STATUS__HIGH_FIRST_X = (1 << 0),
	BMA250_REG_FLAT_ORIENT_STATUS__reserved = 0,

	BMA250_REG_RANGE = 0xf,
	BMA250_REG_RANGE__RANGE	= 0x0f,
	BMA250_REG_RANGE__RANGE__4G = 5,
	BMA250_REG_RANGE__RANGE__8G = 8,
	BMA250_REG_RANGE__RANGE__16G = 0xc,
	BMA250_REG_RANGE__RANGE__shift = 0,
	BMA250_REG_RANGE__reserved = 0xf0,

	BMA250_REG_BANDWIDTH = 0x10,
	BMA250_REG_BANDWIDTH__BANDWIDTH = 0x1f,
	BMA250_REG_BANDWIDTH__BANDWIDTH__64MS = 8,
	BMA250_REG_BANDWIDTH__BANDWIDTH__32MS = 9,
	BMA250_REG_BANDWIDTH__BANDWIDTH__16MS = 10,
	BMA250_REG_BANDWIDTH__BANDWIDTH__8MS = 11,
	BMA250_REG_BANDWIDTH__BANDWIDTH__4MS = 12,
	BMA250_REG_BANDWIDTH__BANDWIDTH__2MS = 13,
	BMA250_REG_BANDWIDTH__BANDWIDTH__1MS = 14,
	BMA250_REG_BANDWIDTH__BANDWIDTH__0_5MS = 15,
	BMA250_REG_BANDWIDTH__reserved = 0xe0,

	BMA250_REG_POWER = 0x11,
	BMA250_REG_POWER__SUSPEND = (1 << 7),
	BMA250_REG_POWER__LOWPOWER_EN = (1 << 6),
	BMA250_REG_POWER__SLEEP_DUR = ((1 << 4) | (1 << 3) | (1 << 2) | (1 << 1)),
	BMA250_REG_POWER__SLEEP_DUR_0_5MS = ((1 << 4) | (1 << 1)),
	BMA250_REG_POWER__SLEEP_DUR_1MS = ((1 << 3) | (1 << 2)),
	BMA250_REG_POWER__SLEEP_DUR_2MS = ((1 << 3) | (1 << 2) | (1 << 1)),
	BMA250_REG_POWER__SLEEP_DUR_4MS = (1 << 4),
	BMA250_REG_POWER__SLEEP_DUR_6MS = ((1 << 4) | (1 << 1)),
	BMA250_REG_POWER__SLEEP_DUR_10MS = ((1 << 4) | (1 << 2)),
	BMA250_REG_POWER__SLEEP_DUR_25MS = ((1 << 4) | (1 << 2) | (1 << 1)),
	BMA250_REG_POWER__SLEEP_DUR_50MS = ((1 << 4) | (1 << 3)),
	BMA250_REG_POWER__SLEEP_DUR_100MS = ((1 << 4) | (1 << 3) | (1 << 1)),
	BMA250_REG_POWER__SLEEP_DUR_500MS = ((1 << 4) | (1 << 3) | (1 << 2)),
	BMA250_REG_POWER__SLEEP_DUR_1000MS = ((1 << 4) | (1 << 3) | (1 << 2) | (1 << 1)),
	BMA250_REG_POWER__reserved = ((1 << 5) | (1 << 0)),

	BMA250_REG_DAQ = 0x13,
	BMA250_REG_DAQ__DATA_HIGH_BW = (1 << 7),
	BMA250_REG_DAQ__SHADOW_DIS = (1 << 6),
	BMA250_REG_DAQ__reserved = 0x3f,

	BMA250_REG_RESET = 0x14,
	BMA250_REG_RESET__RESET = 0xb6,
	BMA250_REG_RESET__reserved = 0,

	BMA250_REG_INT1 = 0x16,
	BMA250_REG_INT1__FLAT_EN = (1 << 7),
	BMA250_REG_INT1__ORIENT_EN = (1 << 6),
	BMA250_REG_INT1__S_TAP_EN = (1 << 5),
	BMA250_REG_INT1__D_TAP_EN = (1 << 4),
	BMA250_REG_INT1__SLOPE_EN_Z = (1 << 2),
	BMA250_REG_INT1__SLOPE_EN_Y = (1 << 1),
	BMA250_REG_INT1__SLOPE_EN_X = (1 << 0),
	BMA250_REG_INT1__reserved = (1 << 3),

	BMA250_REG_INT2 = 0x17,
	BMA250_REG_INT2__DATA_EN = (1 << 4),
	BMA250_REG_INT2__LOW_EN = (1 << 3),
	BMA250_REG_INT2__HIGH_EN_Z = (1 << 2),
	BMA250_REG_INT2__HIGH_EN_Y = (1 << 1),
	BMA250_REG_INT2__HIGH_EN_X = (1 << 0),
	BMA250_REG_INT2__reserved = 0xe0,

	/* TODO: lots more registers */
	/* TODO: EEPROM registers */
};

struct bma250 {
	struct i2c_client *client;

	struct regulator *vdd;
	struct regulator *vddio;

	struct mutex mutex;

	int irq;

	struct input_dev *input;
	struct delayed_work input_worker;
	int evdev_delay_ms;

	/* cached register values */
	int POWER;
	int RANGE;
	int BANDWIDTH;

	/* used when pushing/popping modes */
	int prev_POWER;
	int pushpop_count;
};

/* TODO: elsewhere needs to check the return result */
static int __bma_enable_regulators(struct bma250 *bma)
{
	int ret;

	if (!IS_ERR_OR_NULL(bma->vdd))
		ret = regulator_enable(bma->vdd);
	if (ret < 0)
		goto done;
	if (!IS_ERR_OR_NULL(bma->vddio))
		ret = regulator_enable(bma->vddio);
	if (ret < 0) {
		if (!IS_ERR_OR_NULL(bma->vdd))
			regulator_disable(bma->vdd);
		goto done;
	}

	/* from t.s_up parameter specification "Start-Up Time" in
	 * BMA250 03 March 2011 Datasheet */
	usleep(2000);

done:
	return ret;
}

static int __bma_disable_regulators(struct bma250 *bma)
{
	if (!IS_ERR_OR_NULL(bma->vddio))
		regulator_enable(bma->vddio);
	if (!IS_ERR_OR_NULL(bma->vdd))
		regulator_disable(bma->vdd);
	return 0;
}

static void bma_put_regulators(struct bma250 *bma)
{
	if (!IS_ERR_OR_NULL(bma->vddio))
		regulator_put(bma->vddio);
	if (!IS_ERR_OR_NULL(bma->vdd))
		regulator_put(bma->vdd);
}

static int bma_get_regulators(struct bma250 *bma)
{
	int ret;

	bma->vdd = regulator_get(&bma->client->dev, "VDD");
	if (!IS_ERR_OR_NULL(bma->vdd)) {
		ret = regulator_set_voltage(bma->vdd, 1620000UL, 3600000UL);
		if (ret) {
			dev_dbg(&bma->client->dev,
				"%s: regulator_set_voltage(VDD) "
				"returned %d (fatal)\n",
				__func__, ret);
			goto err_set_vdd;
		}
	}

	bma->vddio = regulator_get(&bma->client->dev, "VLOGIC");
	if (!IS_ERR_OR_NULL(bma->vddio)) {
		ret = regulator_set_voltage(bma->vddio, 1200000UL, 3600000UL);
		if (ret) {
#if 0
			dev_dbg(&bma->client->dev,
				"%s: regulator_set_voltage(VLOGIC) "
				"returned %d (fatal)\n",
				__func__, ret);
			goto err_set_vlogic;
#else
			/* TODO: fix up platform issues that trigger this error! */
			WARN(ret < 0, "%s: regulator_set_voltage(VLOGIC) "
			     "returned %d (fatal)\n", __func__, ret);
			ret = 0;
#endif
		}
	}

	return 0;

#if 0
err_set_vlogic:
#endif
	if (!IS_ERR_OR_NULL(bma->vddio))
		regulator_put(bma->vddio);
err_set_vdd:
	if (!IS_ERR_OR_NULL(bma->vdd))
		regulator_put(bma->vdd);
	return ret;
}

/* TODO: is "int" vs. "s32" always ok here et. al? */
static int __bma_reg_read(struct bma250 *bma, int reg)
{
	if (bma->client)
		return i2c_smbus_read_byte_data(bma->client, reg);
	return -ENODEV;
}
static int __bma_reg_write(struct bma250 *bma, int reg, int val)
{
	if (bma->client)
		return i2c_smbus_write_byte_data(bma->client, reg, val);
	return -ENODEV;
}
static int __bma_reg16_read(struct bma250 *bma, int reg)
{
	if (bma->client)
		return i2c_smbus_read_word_data(bma->client, reg);
	return -ENODEV;
}

/* NOTE: The BMA250 registers can be read anytime the chip is powered
 * up; the chip must be in ACTIVE mode for writes to any register
 * except POWER, however, so we put it there and then put it back to
 * the previous mode after the update when necessary.
 *
 * NOTE: Why a switch()?  Because we don't necessarily want to cache
 * every register value, the code is clearer, and the optimizer gets
 * rid of the lines that aren't relevant when the template function is
 * created.
 */
#define BMA_REG_READ(_name) \
	static int __bma_reg_read_##_name(struct bma250 *bma) \
{ \
	int ret = __bma_reg_read(bma, BMA250_REG_##_name); \
	if (ret < 0) \
		return ret; \
	ret &= ~BMA250_REG_##_name ## __reserved; \
	switch (BMA250_REG_##_name) { \
	case BMA250_REG_RANGE: bma->RANGE = ret; break; \
	case BMA250_REG_POWER: bma->POWER = ret; break; \
	case BMA250_REG_BANDWIDTH: bma->BANDWIDTH = ret; break; \
	} \
	dev_dbg(&bma->client->dev, "%s: %02x\n", __func__, ret); \
	return ret; \
}
#define BMA_REG_WRITE(_name) \
static int __bma_reg_write_##_name(struct bma250 *bma, int v) \
{ \
	int ret; \
	dev_dbg(&bma->client->dev, "%s: %02x\n", __func__, v); \
	v &= ~BMA250_REG_##_name ## __reserved; \
	ret = __bma_reg_write(bma, BMA250_REG_##_name, v); \
	if (ret < 0) \
		return ret; \
	switch (BMA250_REG_##_name) { \
	case BMA250_REG_RANGE: bma->RANGE = v; break; \
	case BMA250_REG_POWER: bma->POWER = v; break; \
	case BMA250_REG_BANDWIDTH: bma->BANDWIDTH = v; break; \
	case BMA250_REG_RESET: usleep(4000); break; \
	} \
	return 0; \
}
#define BMA_REG_ATTR_SHOW(_name) \
static ssize_t bma_show_##_name(struct device *dev, \
				   struct device_attribute *attr, \
				   char *buf) \
{ \
	struct bma250 *bma = dev_get_drvdata(dev); \
	int ret; \
	pm_runtime_get_sync(dev); \
	ret = mutex_lock_interruptible(&bma->mutex); \
	if (ret < 0) \
		return ret; \
	ret = __bma_reg_read_##_name(bma); \
	mutex_unlock(&bma->mutex); \
	pm_runtime_mark_last_busy(dev); \
	pm_runtime_put_autosuspend(dev); \
	return (ret < 0) ? ret : sprintf(buf, "%02x\n", ret);	\
}
#define BMA_REG_ATTR_STORE(_name) \
static ssize_t bma_store_##_name(struct device *dev, \
                                    struct device_attribute *attr, \
				    const char *buf, size_t len) \
{ \
	struct bma250 *bma = dev_get_drvdata(dev); \
	int ret; \
	unsigned long v; \
	ret = strict_strtoul(buf, 16, &v); \
	if (ret) \
		return ret; \
	pm_runtime_get_sync(dev); \
	ret = mutex_lock_interruptible(&bma->mutex); \
	if (ret < 0) \
		return ret; \
	switch (BMA250_REG_##_name) { \
	case BMA250_REG_POWER: \
		ret = __bma_reg_write_POWER(bma, v); \
		break; \
	default: \
		__bma_push_mode_active(bma); \
		ret = __bma_reg_write_##_name(bma, v);	\
		__bma_pop_mode(bma); \
		break; \
	} \
	mutex_unlock(&bma->mutex); \
	pm_runtime_mark_last_busy(dev); \
	pm_runtime_put_autosuspend(dev); \
	return (ret < 0) ? ret : len;	 \
}

#define BMA_REG(_name) \
	BMA_REG_READ(_name) \
	BMA_REG_ATTR_SHOW(_name) \
	static DEVICE_ATTR(_name, S_IRUGO, bma_show_##_name, NULL);
#define BMA_REG_READWRITE(_name) \
	BMA_REG_READ(_name) \
	BMA_REG_WRITE(_name) \
	BMA_REG_ATTR_SHOW(_name) \
	BMA_REG_ATTR_STORE(_name) \
	static DEVICE_ATTR(_name, S_IRUGO | S_IWUSR, \
			   bma_show_##_name, bma_store_##_name);
#define BMA_REG_WRITEONLY(_name) \
	BMA_REG_WRITE(_name) \
	BMA_REG_ATTR_STORE(_name) \
	static DEVICE_ATTR(_name, S_IWUSR, NULL, bma_store_##_name);

/* Reformat the X, Y, and Z registers into native integers.
 * NOTE: these work ONLY for the X, Y, and Z registers!
 *
 * TODO: this is not endian-neutral code, but needs to be! */
#define BMA_REG16_READ(_name) \
static int __bma_reg_read_##_name(struct bma250 *bma) \
{ \
	union { \
		unsigned char b[sizeof(int)];\
		int i; \
	} uret; \
	int ret = __bma_reg16_read(bma, BMA250_REG_##_name); \
	if (ret < 0) \
		return ret; \
	uret.i = ret; \
	if (!(uret.b[0] & BMA250_REG_##_name ## __NEW_DATA)) \
		return -EAGAIN; \
	uret.b[0] &= BMA250_REG_##_name ## _LSB__reserved; \
	uret.b[1] &= BMA250_REG_##_name ## _MSB__reserved; \
	ret = uret.b[1]; \
	ret <<= 8; \
	ret += uret.b[0]; \
	ret >>= BMA250_REG_##_name ## __ACCEL__shift; \
	dev_dbg(&bma->client->dev, "%s: %04x\n", __func__, ret); \
	return ret; \
} \
static ssize_t bma_show_##_name(struct device *dev, \
				   struct device_attribute *attr, \
				   char *buf) \
{ \
	struct bma250 *bma = dev_get_drvdata(dev); \
	int ret; \
	pm_runtime_get_sync(dev); \
	ret = mutex_lock_interruptible(&bma->mutex); \
	if (ret) \
		goto err; \
	ret = __bma_reg_read_##_name(bma); \
	mutex_unlock(&bma->mutex); \
err: \
	pm_runtime_mark_last_busy(dev); \
	pm_runtime_put_autosuspend(dev); \
	return (ret < 0) ? ret : sprintf(buf, "%04x\n", ret); \
}

#define BMA_REG16_ATTR(_name) \
static DEVICE_ATTR(_name, S_IRUGO, bma_show_##_name, NULL);
#define BMA_REG16(_name) \
	BMA_REG16_READ(_name) \
	BMA_REG16_ATTR(_name)

static int __bma_push_mode_active(struct bma250 *bma);
static int __bma_pop_mode(struct bma250 *bma);

BMA_REG(CHIP_ID);
BMA_REG(VERSION);

/* NOTE: DO NOT use these registers to read acceleration data in the
 * usual way, since there are hardware interactions between LSB/MSB
 * for each axis (see the datasheet); use them only for testing! */
BMA_REG(X_AXIS_LSB);
BMA_REG(X_AXIS_MSB);
BMA_REG(Y_AXIS_LSB);
BMA_REG(Y_AXIS_MSB);
BMA_REG(Z_AXIS_LSB);
BMA_REG(Z_AXIS_MSB);

BMA_REG(TEMP);
BMA_REG(STATUS);
BMA_REG(DATA_INT);
BMA_REG(TAP_SLOPE_INT_STATUS);
BMA_REG(FLAT_ORIENT_STATUS);


/* NOTE: all the read/write registers need to turn POWER on during
 * writes, so we declare POWER first */
/* TODO: should we be "smart" with POWER changes and an open evdev? */
BMA_REG_READWRITE(POWER);

BMA_REG_READWRITE(RANGE);
BMA_REG_READWRITE(BANDWIDTH);
BMA_REG_READWRITE(DAQ);
BMA_REG_WRITEONLY(RESET);
BMA_REG_READWRITE(INT1);
BMA_REG_READWRITE(INT2);

/* Note: if you aren't using our evdev interface, then these
 * attributes provide the "usual way" to read acceleration data */
BMA_REG16(X_AXIS);
BMA_REG16(Y_AXIS);
BMA_REG16(Z_AXIS);

static int __bma_reset(struct bma250 *bma)
{
	/* NOTE:  __reg_write_RESET() contains the required (?) 4ms delay */
	return __bma_reg_write_RESET(bma, BMA250_REG_RESET__RESET);
}

static int __bma_push_mode_active(struct bma250 *bma)
{
	BUG_ON(bma->pushpop_count != 0);

	bma->pushpop_count++;
	bma->prev_POWER = bma->POWER;

	if (bma->POWER & (BMA250_REG_POWER__SUSPEND
			  | BMA250_REG_POWER__LOWPOWER_EN))
		return __bma_reg_write_POWER(bma, 0);
	return 0;
}

static int __bma_pop_mode(struct bma250 *bma)
{
	BUG_ON(bma->pushpop_count != 1);

	bma->pushpop_count--;
	if (bma->prev_POWER != bma->POWER)
		return __bma_reg_write_POWER(bma, bma->prev_POWER);
	return 0;
}

/* Sets requested bits in the REG_POWER register 
 *
 * @suspend		- set the REG_POWER__SUSPEND bit
 * @lowpower_en		- set the REG_POWER__LOWPOWER_EN bit
 * @sleep_dur		- set REG_POWER__SLEEP_DUR
 *
 * Returns zero on success, an errno otherwise.
 *
 * Note: for @sleep_dur, use the REG_POWER__SLEEP_DUR_* enums
 */
static int __bma_set_power(struct bma250 *bma,
			   int suspend,
			   int lowpower_en,
			   int sleep_dur)
{
	int ret, power;

	/* be paranoid by re-reading POWER, and also skip push/pop;
	 * this is ok because the POWER register is read/write no
	 * matter what state the chip is in */
	ret = __bma_reg_read_POWER(bma);
	if (ret < 0)
		goto done;

	power = ret;
	if (suspend)
		power |= BMA250_REG_POWER__SUSPEND;
	if (lowpower_en)
		power |= BMA250_REG_POWER__LOWPOWER_EN;
	if (sleep_dur)
		power = (power & ~BMA250_REG_POWER__SLEEP_DUR)
			| (sleep_dur & BMA250_REG_POWER__SLEEP_DUR);

	ret = __bma_reg_write_POWER(bma, power);

done:
	return (ret < 0) ? ret : 0;
}

#if 0
/* clears requested bits in the REG_POWER register */
static int __bma_clear_power(struct bma250 *bma,
			   int suspend,
			   int lowpower_en,
			   int sleep_dur)
{
	int ret, power;

	ret = __bma_reg_read_POWER(bma);
	if (ret < 0)
		goto done;

	power = ret;
	if (suspend)
		power &= ~BMA250_REG_POWER__SUSPEND;
	if (lowpower_en)
		power &= ~BMA250_REG_POWER__LOWPOWER_EN;
	if (sleep_dur)
		power = (power & ~BMA250_REG_POWER__SLEEP_DUR)
			| (sleep_dur & BMA250_REG_POWER__SLEEP_DUR);

	ret = __bma_reg_write_POWER(bma, power);

done:
	return (ret < 0) ? ret : 0;
}
#endif

/* sets bits in the REG_INT2 register */
static int __bma_set_int2(struct bma250 *bma,
			  int data_en, int low_en,
			  int high_en_z, int high_en_y,
			  int high_en_x)
{
	int ret, int2;

	ret = __bma_reg_read_INT2(bma);
	if (ret < 0)
		return ret;

	int2 = ret;
	if (data_en)
		int2 |= BMA250_REG_INT2__DATA_EN;
	if (low_en)
		int2 |= BMA250_REG_INT2__LOW_EN;
	if (high_en_z)
		int2 |= BMA250_REG_INT2__HIGH_EN_Z;
	if (high_en_y)
		int2 |= BMA250_REG_INT2__HIGH_EN_Y;
	if (high_en_x)
		int2 |= BMA250_REG_INT2__HIGH_EN_X;

	ret = __bma_reg_write_INT2(bma, int2);
	return (ret < 0) ? ret : 0;
}

static int __bma_clear_int2(struct bma250 *bma,
			    int data_en, int low_en,
			    int high_en_z, int high_en_y,
			    int high_en_x)
{
	int ret, int2;

	ret = __bma_reg_read_INT2(bma);
	if (ret < 0)
		return ret;

	int2 = ret;
	if (data_en)
		int2 &= ~BMA250_REG_INT2__DATA_EN;
	if (low_en)
		int2 &= ~BMA250_REG_INT2__LOW_EN;
	if (high_en_z)
		int2 &= ~BMA250_REG_INT2__HIGH_EN_Z;
	if (high_en_y)
		int2 &= ~BMA250_REG_INT2__HIGH_EN_Y;
	if (high_en_x)
		int2 &= ~BMA250_REG_INT2__HIGH_EN_X;

	ret = __bma_reg_write_INT2(bma, int2);
	return (ret < 0) ? ret : 0;
}

static int bma_usecs_to_sleep_dur(int usecs)
{
	if (usecs == 500)
		return BMA250_REG_POWER__SLEEP_DUR_0_5MS;
	if (usecs == 1000)
		return BMA250_REG_POWER__SLEEP_DUR_1MS;
	if (usecs == 2000)
		return BMA250_REG_POWER__SLEEP_DUR_2MS;
	if (usecs == 4000)
		return BMA250_REG_POWER__SLEEP_DUR_4MS;
	if (usecs == 6000)
		return BMA250_REG_POWER__SLEEP_DUR_6MS;
	if (usecs == 10000)
		return BMA250_REG_POWER__SLEEP_DUR_10MS;
	if (usecs == 25000)
		return BMA250_REG_POWER__SLEEP_DUR_25MS;
	if (usecs == 50000)
		return BMA250_REG_POWER__SLEEP_DUR_50MS;
	if (usecs == 100000)
		return BMA250_REG_POWER__SLEEP_DUR_100MS;
	if (usecs == 500000)
		return BMA250_REG_POWER__SLEEP_DUR_500MS;
	if (usecs == 1000000)
		return BMA250_REG_POWER__SLEEP_DUR_1000MS;
	return -EINVAL;
}

/* sets the chip up to use sleep_dur for the evdev */
static int __bma_configure_evdev_sleep_dur(struct bma250 *bma, int sleep_dur)
{
	int ret;
	cancel_delayed_work_sync(&bma->input_worker);
	ret = __bma_reg_write_POWER(bma, 0);
	if (ret >= 0)
		ret = __bma_set_int2(bma, 1, 0, 0, 0, 0);
	if (ret >= 0)
		ret = __bma_set_power(bma, 0, 1, sleep_dur);
	return ret;
}

/* sets the chip up to use polling for the evdev */
static int __bma_configure_evdev_polled(struct bma250 *bma)
{
	int ret;
	ret = __bma_reg_write_POWER(bma, 0);
	if (ret >= 0)
		ret = __bma_clear_int2(bma, 1, 0, 0, 0, 0);
	if (ret >= 0)
		ret = schedule_delayed_work(&bma->input_worker,
					    msecs_to_jiffies(bma->evdev_delay_ms));
	return ret < 0 ? ret : 0;
}

static void __bma_unconfigure_evdev(struct bma250 *bma)
{
	cancel_delayed_work_sync(&bma->input_worker);
	__bma_reg_write_POWER(bma, 0);
	__bma_clear_int2(bma, 1, 0, 0, 0, 0);
	/* TODO: does this turn off non-evdev features too, like tap interrupts? */
	__bma_reg_write_POWER(bma, BMA250_REG_POWER__SUSPEND);
}

static int __bma_configure_evdev(struct bma250 *bma)
{
	int sleep_dur;

	sleep_dur = bma_usecs_to_sleep_dur(bma->evdev_delay_ms * 1000);

	if (bma->irq > 0 && sleep_dur != -EINVAL)
		return __bma_configure_evdev_sleep_dur(bma, sleep_dur);
	else
		return __bma_configure_evdev_polled(bma);
}

static ssize_t bma_show_evdev_delay_ms(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	struct bma250 *bma = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", bma->evdev_delay_ms);
}

static ssize_t bma_store_evdev_delay_ms(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t len)
{
	struct bma250 *bma = dev_get_drvdata(dev);
	int ret;
	unsigned long evdev_delay_ms;

	ret = strict_strtoul(buf, 10, &evdev_delay_ms);
	if (ret)
		return ret;
	if (!evdev_delay_ms)
		return -EINVAL;

	pm_runtime_get_sync(dev);

	ret = mutex_lock_interruptible(&bma->mutex);
	if (ret < 0)
		goto done;

	bma->evdev_delay_ms = evdev_delay_ms;

	ret = __bma_configure_evdev(bma);
	mutex_unlock(&bma->mutex);

done:
	pm_runtime_mark_last_busy(dev);
	pm_runtime_put_autosuspend(dev);
	return (ret < 0) ? ret : len;
}
static DEVICE_ATTR(delay_ms, S_IRUGO | S_IWUSR,
		   bma_show_evdev_delay_ms, bma_store_evdev_delay_ms);

static struct attribute *bma_attributes[] = {
	&dev_attr_CHIP_ID.attr,
	&dev_attr_VERSION.attr,
	&dev_attr_TEMP.attr,
	&dev_attr_STATUS.attr,
	&dev_attr_DATA_INT.attr,
	&dev_attr_TAP_SLOPE_INT_STATUS.attr,
	&dev_attr_FLAT_ORIENT_STATUS.attr,
	&dev_attr_RANGE.attr,
	&dev_attr_BANDWIDTH.attr,
	&dev_attr_POWER.attr,
	&dev_attr_DAQ.attr,
	&dev_attr_RESET.attr,
	&dev_attr_INT1.attr,
	&dev_attr_INT2.attr,
	&dev_attr_X_AXIS.attr,
	&dev_attr_X_AXIS_LSB.attr,
	&dev_attr_X_AXIS_MSB.attr,
	&dev_attr_Y_AXIS.attr,
	&dev_attr_Y_AXIS_LSB.attr,
	&dev_attr_Y_AXIS_MSB.attr,
	&dev_attr_Z_AXIS.attr,
	&dev_attr_Z_AXIS_LSB.attr,
	&dev_attr_Z_AXIS_MSB.attr,
	&dev_attr_delay_ms.attr,
	NULL,
};

static struct attribute_group bma_attribute_group = {
	.attrs = bma_attributes,
};

#if 0
/* certain values in the BANDWIDTH register fields are reserved beyond
 * what simple masking can implement */
static int __bma_set_bandwidth(struct bma250 *bma, int bw)
{
	/* from Table 8 */
	if (bw < BMA250_REG_BANDWIDTH__BANDWIDTH__64MS)
		bw = BMA250_REG_BANDWIDTH__BANDWIDTH__64MS;
	else if (bw > BMA250_REG_BANDWIDTH__BANDWIDTH__0_5MS)
		bw = BMA250_REG_BANDWIDTH__BANDWIDTH__0_5MS;
	return __bma_reg_write_BANDWIDTH(bma, bw);
}

/* TODO: certain values in the range selection register are reserved
 * beyond what simple masking can implement */
static int __bma_set_range(struct bma250 *bma, ...)
{
	return -EINVAL;
}
#endif

static int __bma_read_xyzt(struct bma250 *bma, int *x, int *y, int *z, int *t)
{
	int ret = 0;

	/* TODO: do we actually want to do a mass i2c-xfer of all six
	 * registers, to make them more likely (guaranteed?) to be
	 * synchronized? This would probably be faster, too.  Would it
	 * be implementable on SPI too? */

	if (x) {
		ret = __bma_reg_read_X_AXIS(bma);
		if (ret < 0)
			goto err_read_x;
		*x = ret;
	}

	if (y) {
		ret = __bma_reg_read_Y_AXIS(bma);
		if (ret < 0)
			goto err_read_y;
		*y = ret;
	}

	if (z) {
		ret = __bma_reg_read_Z_AXIS(bma);
		if (ret < 0)
			goto err_read_z;
		*z = ret;
	}

	if (t) {
		ret = __bma_reg_read_TEMP(bma);
		if (ret < 0)
			goto err_read_t;
		*t = ret;
	}

err_read_t:
err_read_z:
err_read_y:
err_read_x:
	return ret;
}

/* normalizes raw X, Y, Z accel readings to 256 lsb/g */
static void bma_normalize_xyz(struct bma250 *bma, int *x, int *y, int *z)
{
	int range = (bma->RANGE & BMA250_REG_RANGE__RANGE)
		>> BMA250_REG_RANGE__RANGE__shift;
	int shift;

	switch (range) {
	case BMA250_REG_RANGE__RANGE__4G:
		shift = 5; break;
	case BMA250_REG_RANGE__RANGE__8G:
		shift = 4; break;
	case BMA250_REG_RANGE__RANGE__16G:
		shift = 3; break;
	default:
		shift = 6; break;
	}

	if (x)
		*x >>= shift;
	if (y)
		*y >>= shift;
	if (z)
		*z >>= shift;
}

static int __bma_input_do_worker(struct bma250 *bma)
{
	int x, y, z, t;
	int ret;

	ret = __bma_read_xyzt(bma, &x, &y, &z, &t);
	if (ret < 0)
		return ret;

	bma_normalize_xyz(bma, &x, &y, &z);

	input_report_abs(bma->input, ABS_X, x);
	input_report_abs(bma->input, ABS_Y, y);
	input_report_abs(bma->input, ABS_Z, z);
	input_report_abs(bma->input, ABS_MISC, t);
	input_sync(bma->input);
	return 0;
}

static void bma_input_worker(struct work_struct *work)
{
	struct delayed_work *dw = container_of(work, struct delayed_work, work);
	struct bma250 *bma = container_of(dw, struct bma250, input_worker);

	if (mutex_lock_interruptible(&bma->mutex) < 0)
		return;
	__bma_input_do_worker(bma);
	mutex_unlock(&bma->mutex);
	pm_runtime_mark_last_busy(&bma->client->dev);
	schedule_delayed_work(dw, msecs_to_jiffies(bma->evdev_delay_ms));
}

static int bma_input_open(struct input_dev *dev)
{
	struct bma250 *bma = input_get_drvdata(dev);
	int ret;

	ret = mutex_lock_interruptible(&bma->mutex);
	if (ret < 0)
		return ret;
	ret = __bma_configure_evdev(bma);
	mutex_unlock(&bma->mutex);
	return (ret < 0) ? ret : 0;
}

static void bma_input_close(struct input_dev *dev)
{
	struct bma250 *bma = input_get_drvdata(dev);

	if (mutex_lock_interruptible(&bma->mutex) < 0)
		return;
	__bma_unconfigure_evdev(bma);
	mutex_unlock(&bma->mutex);
}

/* NOTE: this is a threaded handler, so sleeping is ok (has to be, for
 * i2c, etc. communications) */
static irqreturn_t bma_irq_handler(int irq, void *data)
{
	struct bma250 *bma = data;
	int ret, status, new_data;

	ret = mutex_lock_interruptible(&bma->mutex);
	if (ret < 0)
		return IRQ_HANDLED; /* TODO: what else can we do here? */

	/* NOTE: in the nominal case, the chip is in SUSPEND mode
	 * which prevents writing to registers; push the chip into
	 * ACTIVE mode if register updates are required! */

	/* TODO: do something useful with this value */
	status = __bma_reg_read_STATUS(bma);
	if (status < 0)
		goto done;
	dev_dbg(&bma->client->dev, "%s: STATUS %02x\n", __func__, status);

	new_data = __bma_reg_read_DATA_INT(bma);
	dev_dbg(&bma->client->dev, "%s: DATA_INT %02x\n", __func__, new_data);
	if (new_data >= 0 && (new_data & BMA250_REG_DATA_INT__DATA_INT))
		__bma_input_do_worker(bma);

done:
	mutex_unlock(&bma->mutex);
	return IRQ_HANDLED;
}

static int bma_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct bma250 *bma;
	struct bma25x_platform_data *pdata = client->dev.platform_data;
	int ret, chip_id, version;

	bma = kzalloc(sizeof(*bma), GFP_KERNEL);
	if (!bma)
		return -ENOMEM;

	i2c_set_clientdata(client, bma);
	bma->client = client;
	mutex_init(&bma->mutex);
	bma->evdev_delay_ms = (default_evdev_delay_ms > 0) ?
		default_evdev_delay_ms : 1000;

	/* TODO: this might not be the best choice for
	 * bma_input_worker(), since it will run on SCHED_OTHER; we
	 * probably want it to run on SCHED_RR or SCHED_FIFO */
	INIT_DELAYED_WORK(&bma->input_worker, bma_input_worker);

	ret = bma_get_regulators(bma);
	if (ret)
		goto err_get_regulators;

	/* awaken the chip to its SUSPEND mode */
	pm_runtime_enable(&bma->client->dev);
	ret = pm_runtime_resume(&bma->client->dev);
	if (ret)
		goto err_runtime_resume;

	ret = mutex_lock_interruptible(&bma->mutex);
	if (ret)
		goto err_lock_mutex;

	/* NOTE: we don't need to bring the chip out of its SUSPEND
	 * mode in order to merely READ register values; writes
	 * require us to push the chip into ACTIVE mode */

	/* read chip IDs, to make sure the chip is there */
	ret = __bma_reg_read_CHIP_ID(bma);
	if (ret < 0)
		goto err_read_chip_id;
	chip_id = ret;

	ret = __bma_reg_read_VERSION(bma);
	if (ret < 0)
		goto err_read_version;
	version = ret;

	mutex_unlock(&bma->mutex);

	dev_info(&client->dev, "chip_id %d version %d\n", chip_id, version);

	ret = sysfs_create_group(&bma->client->dev.kobj, &bma_attribute_group);
	if (ret)
		goto err_sysfs_create_group;

	bma->input = input_allocate_device();
	if (IS_ERR_OR_NULL(bma->input)) {
		ret = PTR_ERR(bma->input);
		goto err_input_allocate_device;
	}

	input_set_drvdata(bma->input, bma);

	bma->input->open = bma_input_open;
	bma->input->close = bma_input_close;
	bma->input->name = "bma250";

	/* TODO: proper values here */
	bma->input->id.vendor = 1;
	bma->input->id.product = 1;
	bma->input->id.version = 1;

	__set_bit(EV_ABS, bma->input->evbit);
	__set_bit(ABS_X, bma->input->absbit);
	__set_bit(ABS_Y, bma->input->absbit);
	__set_bit(ABS_Z, bma->input->absbit);
	__set_bit(ABS_MISC, bma->input->absbit);

	input_set_abs_params(bma->input, ABS_X, -4096, 4095, 0, 0);
	input_set_abs_params(bma->input, ABS_Y, -4096, 4095, 0, 0);
	input_set_abs_params(bma->input, ABS_Z, -4096, 4095, 0, 0);
	input_set_abs_params(bma->input, ABS_MISC, -80, 175, 0, 0);

	ret = input_register_device(bma->input);
	if (ret)
		goto err_input_register_device;

	if (pdata->irq > 0) {
		bma->irq = pdata->irq;
		
		/* TODO: IRQF flags should come from platform data,
		 * but we also have to make sure that our chip's
		 * interrupt configuration matches ! */
		ret = request_threaded_irq(bma->irq,
					   NULL, bma_irq_handler,
					   pdata->irq_flags ?
					   pdata->irq_flags : 0 /* TODO: */,
					   bma->client->name, bma);
	}

	pm_runtime_set_autosuspend_delay(&bma->client->dev,
					 default_autosuspend_ms);
	pm_runtime_mark_last_busy(&bma->client->dev);
	pm_runtime_use_autosuspend(&bma->client->dev);

	return 0;

err_input_register_device:
	input_free_device(bma->input);
err_input_allocate_device:
	sysfs_remove_group(&bma->client->dev.kobj, &bma_attribute_group);
err_sysfs_create_group:
err_read_version:
err_read_chip_id:
	/* NOTE: taken literally, we would do a mutex_unlock() here
	 * but we can't, because entry points further up the error
	 * recovery stack don't have the mutex locked. Leaving the
	 * mutex locked here doesn't cause any problems as long as
	 * code below this point doesn't depend on it being
	 * otherwise--- so be careful! */
err_lock_mutex:
err_runtime_resume:
	bma_put_regulators(bma);
err_get_regulators:
	i2c_set_clientdata(bma->client, NULL);
	kfree(bma);
	return ret;
}

static int bma_remove(struct i2c_client *client)
{
	struct bma250 *bma = i2c_get_clientdata(client);

	pm_runtime_set_suspended(&bma->client->dev);
	pm_runtime_disable(&bma->client->dev);

	sysfs_remove_group(&bma->client->dev.kobj, &bma_attribute_group);

	cancel_delayed_work_sync(&bma->input_worker);
	input_unregister_device(bma->input);
	input_free_device(bma->input);
	i2c_set_clientdata(bma->client, NULL);

	bma_put_regulators(bma);
	kfree(bma);
	return 0;
}

static int bma_runtime_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250 *bma = i2c_get_clientdata(client);
	int ret;

	dev_err(dev, "%s: TODO: incomplete!\n", __func__);

	ret = mutex_lock_interruptible(&bma->mutex);
	if (ret)
		goto err;

	/* drive the chip into its SUSPEND state, so that it is truly
	 * quiet even if its power sources don't go away (which might
	 * be the case if that regulator is powering other devices) */
	ret = __bma_reg_write_POWER(bma, BMA250_REG_POWER__SUSPEND);

	/* TODO: do we need a disable_irq() here? */

	/* tell Linux we don't need our regulators now; this doesn't
	 * guarantee that our regulators will ACTUALLY turn off! */
	__bma_disable_regulators(bma);

	mutex_unlock(&bma->mutex);
err:
	return (ret < 0) ? ret : 0;
}


static int bma_runtime_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250 *bma = i2c_get_clientdata(client);
	int ret;

	dev_err(dev, "%s: TODO: incomplete!\n", __func__);

	ret = mutex_lock_interruptible(&bma->mutex);
	if (ret < 0)
		return ret;

	__bma_enable_regulators(bma);

	ret = __bma_reset(bma);
	if (ret < 0)
		goto err_reset;

	/* TODO: self-test? */

	/* pre-populate caches with hardware defaults for RANGE and
	 * BANDWIDTH registers; probe() and userspace will update
	 * these later as they so choose, and we will subsequently
	 * preserve them across future suspend/resume operations */
	ret = __bma_reg_read_RANGE(bma);
	if (ret >= 0)
		ret = __bma_reg_read_BANDWIDTH(bma);
	if (ret < 0)
		goto err_init_caches;

err_init_caches:
err_reset:

	/* place chip into SUSPEND mode; other entry points will push
	 * the chip to higher functional modes as needed */
	ret = __bma_reg_write_POWER(bma, BMA250_REG_POWER__SUSPEND);
	mutex_unlock(&bma->mutex);

	pm_runtime_mark_last_busy(dev);

	return (ret < 0) ? ret : 0;
}

static const struct i2c_device_id bma_i2c_id[] = {
	{"bma250", 0},
	{}
};

static struct dev_pm_ops bma_pm_ops = {
	.runtime_suspend = bma_runtime_suspend,
	.runtime_resume = bma_runtime_resume,
};

static struct i2c_driver bma_driver = {
	.driver = {
		.name  = "bma250",
		.owner = THIS_MODULE,
		.pm = &bma_pm_ops,
	},
	.probe         = bma_probe,
	.remove        = bma_remove,
#if 0
	/* TODO: platform-initiated suspend/resume implementations */
	.suspend       = bma_suspend,
	.resume        = bma_resume,
#endif
	.id_table      = bma_i2c_id,
};


static int __init bma_init(void)
{
	return i2c_add_driver(&bma_driver);
}
module_init(bma_init);

static void __exit bma_exit(void)
{
	i2c_del_driver(&bma_driver);
}
module_exit(bma_exit);


MODULE_LICENSE("GPL");
MODULE_LICENSE("GPL");
MODULE_ALIAS("bma250");
MODULE_AUTHOR("Bill Gatliff <bill.gatliff@sonyericsson.com>");
MODULE_DESCRIPTION("Bosch Sensortec BMA250 Triaxial Acceleration Sensor support");
