#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/errno.h>

extern struct bus_type pure_bus;

static struct device_driver *d;

static int pure_probe(struct device *dev)
{
	dev_err(dev, "%s\n", __func__);
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

