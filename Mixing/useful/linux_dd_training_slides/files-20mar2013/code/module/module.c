#include <linux/module.h>

#define MODULE_NAME "skeleton"

int __init example_init (void)
{
   printk(KERN_ERR "%s: %s()\n", MODULE_NAME, __FUNCTION__);
   return 0;
}

void __exit example_exit (void)
{
   printk(KERN_ERR "%s: %s()\n", MODULE_NAME, __FUNCTION__);
}

module_init(example_init);
module_exit(example_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("1.2-rc3");
MODULE_AUTHOR("Bill Gatliff <bgat@billgatliff.com>");
MODULE_DESCRIPTION("A do-nothing example");
