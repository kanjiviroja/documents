#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/completion.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#define LED_GPIO AT91_PIN_PB27
#define LED_IRQ gpio_to_irq(LED_GPIO)

#define CLIENT_ADDR 0x40

extern struct bus_type pure_bus;

static struct device_driver *d;

struct pure {
	struct completion ca, cb;
	int a, b;
	
	int irq;
	struct i2c_client client;
	struct task_struct *kthrd;
};

static irqreturn_t led_irq(int irq, void *pv)
{
	struct pure *p = pv;

	wake_up_process(p->kthrd);
	return IRQ_HANDLED;
}

static int pure_sensord(void *arg)
{
	struct pure *p = arg;
	int s, slow, shigh, sxor;

	/* TODO: finish me! */
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();

		pr_err("%s: running!\n", __func__);

		s = i2c_smbus_read_byte(&p->client);
		slow = (s & 0xf);
		shigh = (s & 0xf0) >> 4;
		sxor = slow ^ shigh;
		
		/* TODO: this isn't entirely correct! */
		if (sxor & 1) {
			p->a = (slow & 1) ? 1 : 0;
			complete(&p->ca);
		}
		if (sxor & 2) {
			p->b = (slow & 2) ? 1 : 0;
			complete(&p->cb);
		}
	}
	return 0;
}

static ssize_t pure_sensor_a_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct pure *p = dev_get_drvdata(dev);

	init_completion(&p->ca);
	wait_for_completion_interruptible(&p->ca);

	return sprintf(buf, "%d\n", p->a);
}
static DEVICE_ATTR(sensor_a, S_IRUGO, pure_sensor_a_show, NULL);

static ssize_t pure_sensor_b_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct pure *p = dev_get_drvdata(dev);

	init_completion(&p->cb);
	wait_for_completion_interruptible(&p->cb);

	return sprintf(buf, "%d\n", p->b);
}
static DEVICE_ATTR(sensor_b, S_IRUGO, pure_sensor_b_show, NULL);

static struct attribute *pure_attrs[] = {
	&dev_attr_sensor_a.attr,
	&dev_attr_sensor_b.attr,
	NULL,
};
static struct attribute_group pure_attr_group = {
	.attrs = pure_attrs,
};

static int pure_probe(struct device *dev)
{
	int ret = 0;
	struct pure *p;

	p = kzalloc(sizeof *p, GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	p->irq = LED_IRQ;

	dev_set_drvdata(dev, p);

	init_completion(&p->ca);
	init_completion(&p->cb);

	p->client.adapter = i2c_get_adapter(0);
	p->client.addr = CLIENT_ADDR;

	p->kthrd = kthread_run(pure_sensord, p,
			       "sensord%02x", p->client.addr);
	if (!p->kthrd) {
		ret = -ENOMEM;
		goto err_kthread_run;
	}

	ret = request_irq(p->irq, led_irq, 0, "led", p);
	if (ret)
		goto err_request_irq;

	ret = sysfs_create_group(&dev->kobj, &pure_attr_group);
	if (ret)
		goto err_sysfs_create_group;

	return 0;

err_sysfs_create_group:
	free_irq(p->irq, p);
err_request_irq:
	kthread_stop(p->kthrd);
err_kthread_run:
	i2c_put_adapter(p->client.adapter);
	kfree(p);
	return ret;
}

static int pure_remove(struct device *dev)
{
	struct pure *p = dev_get_drvdata(dev);

	free_irq(p->irq, p);
	kthread_stop(p->kthrd);
	sysfs_remove_group(&dev->kobj, &pure_attr_group);
	i2c_put_adapter(p->client.adapter);

	return 0;
}


static int __init pure_driver_init (void)
{
	int ret;

	d = kzalloc(sizeof(*d), GFP_USER);
	if (!d)
		return -ENOMEM;

	d->name = "pure";
	d->bus = &pure_bus;

	d->probe = pure_probe;
	d->remove = pure_remove;

	ret = driver_register(d);
	if (ret)
		goto driver_register_failed;

	return 0;

driver_register_failed:
	kfree(d);
	return ret;
}

static void __exit pure_driver_exit (void)
{
	driver_unregister(d);
	kfree(d);
}

module_init(pure_driver_init);
module_exit(pure_driver_exit);
MODULE_LICENSE("GPL");

