#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/errno.h>

static int
pure_bus_match(struct device *dev, struct device_driver *drv)
{
	/* fake it */
	return 1;
}

static struct bus_type pure_bus = {
	.name = "pure_bus",
	.match = pure_bus_match,
};

static int __init pure_bus_init (void)
{
	return bus_register(&pure_bus);
}

static void __exit pure_bus_exit (void)
{
	bus_unregister(&pure_bus);
}

EXPORT_SYMBOL(pure_bus);

module_init(pure_bus_init);
module_exit(pure_bus_exit);
MODULE_LICENSE("GPL");

