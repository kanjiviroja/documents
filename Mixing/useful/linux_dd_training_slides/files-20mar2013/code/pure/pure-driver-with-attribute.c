#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/errno.h>

extern struct bus_type pure_bus;

static struct device_driver *d;

static ssize_t pure_foo_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	return sprintf(buf, "%s: %s\n",
		       __func__, dev_name(dev));
}

static DEVICE_ATTR(foo, S_IRUGO, pure_foo_show, NULL);

static int pure_probe(struct device *dev)
{
	dev_err(dev, "%s\n", __func__);
	return device_create_file(dev, &dev_attr_foo);
}

static int pure_remove(struct device *dev)
{
	dev_err(dev, "%s\n", __func__);
	device_remove_file(dev, &dev_attr_foo);
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

