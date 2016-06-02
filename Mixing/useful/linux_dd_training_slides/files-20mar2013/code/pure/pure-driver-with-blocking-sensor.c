#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/completion.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>

#define LED_GPIO AT91_PIN_PB27
#define LED_IRQ gpio_to_irq(LED_GPIO)

#define CLIENT_ADDR 0x40

extern struct bus_type pure_bus;

static struct device_driver *d;

struct pure {
	struct completion c;
	int irq;
	struct i2c_client client;
};

static irqreturn_t led_irq(int irq, void *pv)
{
	struct pure *p = pv;

	complete(&p->c);
	return IRQ_HANDLED;
}

static ssize_t pure_client_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct pure *p = dev_get_drvdata(dev);
	int s;

	init_completion(&p->c);
	wait_for_completion_interruptible(&p->c);

	s = i2c_smbus_read_byte(&p->client);
	if (s >= 0)
		return sprintf(buf, "%02x\n", s);
	return s;
}

static DEVICE_ATTR(client, S_IRUGO, pure_client_show, NULL);

static int pure_probe(struct device *dev)
{
	int ret;
	struct pure *p;

	p = kzalloc(sizeof *p, GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	p->irq = LED_IRQ;

	dev_set_drvdata(dev, p);

	init_completion(&p->c);

	p->client.adapter = i2c_get_adapter(0);
	p->client.addr = CLIENT_ADDR;

	ret = request_irq(p->irq, led_irq, 0, "led", p);
	if (ret)
		goto err_request_irq;

	ret = device_create_file(dev, &dev_attr_client);
	if (ret)
		goto err_device_create_file;

	return 0;

err_device_create_file:
	free_irq(p->irq, p);
err_request_irq:
	i2c_put_adapter(p->client.adapter);
	kfree(p);
	return ret;
}

static int pure_remove(struct device *dev)
{
	struct pure *p = dev_get_drvdata(dev);

	free_irq(p->irq, p);
	device_remove_file(dev, &dev_attr_client);
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

