#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>

static ssize_t show_test1(struct device *dev,
			  struct device_attribute *attr,
			  char *buf)
{
	int ret;

	dev_err(dev, "%s\n", __func__);

	ret = pm_runtime_get(dev);
	dev_err(dev, "%s(%d): pm_runtime_get() returns %d\n", __func__, __LINE__, ret);

	return sprintf(buf, "%s\n", __func__);
}

static ssize_t store_test1(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t len)
{
	int ret;

	dev_err(dev, "%s\n", __func__);

	ret = pm_runtime_put(dev);
	dev_err(dev, "%s(%d): pm_runtime_put() returns %d\n", __func__, __LINE__, ret);

	return len;
}
static DEVICE_ATTR(test1, S_IRUGO | S_IWUSR, show_test1, store_test1);

static ssize_t show_test2(struct device *dev,
			  struct device_attribute *attr,
			  char *buf)
{
	pm_runtime_get_sync(dev);
	pm_runtime_mark_last_busy(dev);
	pm_runtime_put_autosuspend(dev);
	return sprintf(buf, "%s\n", __func__);
}

static ssize_t store_test2(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t len)
{
	int ret;

	dev_err(dev, "%s\n", __func__);

	ret = pm_runtime_get_sync(dev);
	dev_err(dev, "%s: pm_runtime_get_sync() returns %d\n", __func__, ret);

	ret = pm_runtime_put_sync(dev);
	dev_err(dev, "%s: pm_runtime_put_sync() returns %d\n", __func__, ret);

	return len;
}
static DEVICE_ATTR(test2, S_IRUGO | S_IWUSR, show_test2, store_test2);

static struct attribute *attrs[] = {
	&dev_attr_test1.attr,
	&dev_attr_test2.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static int probe(struct platform_device *pdev)
{
	int ret;

	dev_err(&pdev->dev, "%s\n", __func__);

	ret = sysfs_create_group(&pdev->dev.kobj, &attr_group);
	if (ret)
		return ret;

	pm_runtime_set_suspended(&pdev->dev);
	pm_runtime_set_autosuspend_delay(&pdev->dev, 5000);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	ret = pm_runtime_resume(&pdev->dev);
	dev_err(&pdev->dev, "pm_runtime_resume() returned %d\n", ret);

	dev_err(&pdev->dev, "%s() returns %d\n", __func__, ret);

	return ret;
}

static int remove(struct platform_device *pdev)
{
	int ret;

	dev_err(&pdev->dev, "%s\n", __func__);

	ret = pm_runtime_resume(&pdev->dev);
	dev_err(&pdev->dev, "%s: pm_runtime_resume() returned %d\n", __func__, ret);

	sysfs_remove_group(&pdev->dev.kobj, &attr_group);

	pm_runtime_disable(&pdev->dev);

	dev_err(&pdev->dev, "%s() returns 0\n", __func__);

	return 0;
}

static int runtime_suspend(struct device *dev)
{
	dev_err(dev, "%s\n", __func__);
	return 0;
}

static int runtime_resume(struct device *dev)
{
	dev_err(dev, "%s\n", __func__);
	return 0;
}

static int runtime_idle(struct device *dev)
{
	dev_err(dev, "%s\n", __func__);
	return 0;
}

static int suspend(struct device *dev)
{
	dev_err(dev, "%s\n", __func__);
	return 0;
}

static int resume(struct device *dev)
{
	dev_err(dev, "%s\n", __func__);
	return 0;
}

static struct dev_pm_ops test_pm_ops = {
	.runtime_suspend = runtime_suspend,
	.runtime_resume = runtime_resume,
	.runtime_idle = runtime_idle,

	.suspend = suspend,
	.resume = resume,
};

static struct platform_driver test_driver = {
	.driver = {
		.name = "rpm_test",
		.owner = THIS_MODULE,
		.pm = &test_pm_ops,
	},
	.probe = probe,
	.remove = remove,
};

static struct platform_device *test_pdev;

static int test_init(void)
{
	int ret;

	ret = platform_driver_register(&test_driver);
	if (ret)
		return ret;

	test_pdev = platform_device_alloc("rpm_test", 0);
	if (!test_pdev) {
		ret = -EINVAL;
		goto err_alloc_null;
	}
	if (IS_ERR(test_pdev)) {
		ret = PTR_ERR(test_pdev);
		goto err_alloc_null;
	}

	ret = platform_device_add(test_pdev);
	if (ret) {
		platform_device_put(test_pdev);
		goto err_add;
	}

	return 0;

err_add:
	platform_device_del(test_pdev);
err_alloc_null:
	platform_driver_unregister(&test_driver);
	return ret;
}
module_init(test_init);

static void test_exit(void)
{
	if (!IS_ERR_OR_NULL(test_pdev))
		platform_device_del(test_pdev);
	platform_driver_unregister(&test_driver);
}
module_exit(test_exit);
MODULE_LICENSE("GPL");
