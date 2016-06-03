#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/completion.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define LED_GPIO AT91_PIN_PB27
#define LED_IRQ gpio_to_irq(LED_GPIO)

extern struct bus_type pure_bus;

static struct device_driver *d;

struct pure {
	struct completion c;
	int irq;
};

static irqreturn_t led_irq(int irq, void *pv)
{
	struct pure *p = pv;

	complete(&p->c);
	return IRQ_HANDLED;
}

static ssize_t pure_led_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct pure *p = dev_get_drvdata(dev);

	init_completion(&p->c);
	wait_for_completion_interruptible(&p->c);

	return sprintf(buf, "sensor interrupt!\n");
}

static DEVICE_ATTR(led, S_IRUGO, pure_led_show, NULL);

static int pure_probe(struct device *dev)
{
	int ret;
	struct pure *p;

	dev_err(dev, "%s\n", __func__);

	p = kzalloc(sizeof *p, GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	p->irq = LED_IRQ;

	dev_set_drvdata(dev, p);

	init_completion(&p->c);

	ret = request_irq(p->irq, led_irq, 0, "led", p);
	if (ret)
		return ret;

	return device_create_file(dev, &dev_attr_led);
}

static int pure_remove(struct device *dev)
{
	struct pure *p = dev_get_drvdata(dev);

	dev_err(dev, "%s\n", __func__);

	free_irq(p->irq, NULL);

	device_remove_file(dev, &dev_attr_led);
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

