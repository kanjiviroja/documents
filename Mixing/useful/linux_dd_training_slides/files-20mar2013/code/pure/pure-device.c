#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/errno.h>


extern struct bus_type pure_bus;
struct device *d;

static void pure_device_release (struct device *dev)
{
	dev_err(dev, "%s\n", __func__);
}

static int __init pure_device_init (void)
{
	int ret;

	d = kzalloc(sizeof(*d), GFP_USER);
	if (!d)
		return -ENOMEM;

	dev_set_name(d, "%s.%d", "pure", 0);
	d->bus = &pure_bus;
	d->release = pure_device_release;

	ret = device_register(d);
	if (ret)
		goto device_register_failed;

	return 0;

 device_register_failed:
	put_device(d);
	return ret;
}

static void __exit pure_device_exit (void)
{
	device_unregister(d);
	put_device(d);
}

module_init(pure_device_init);
module_exit(pure_device_exit);
MODULE_LICENSE("GPL");

