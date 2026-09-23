// SPDX-License-Identifier: GPL-2.0
/*
 * akd.c - Android Kernel Driver (character device)
 *
 * A small misc-device example for Android / Linux kernels.
 * Exposes /dev/akd with read, write, and ioctl.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/atomic.h>

#include "akd.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AKD");
MODULE_DESCRIPTION("Android kernel character device driver");
MODULE_VERSION("1.0");

static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable extra debug prints (0/1)");

#define akd_dbg(fmt, ...)						\
	do {								\
		if (debug)						\
			pr_info("akd: " fmt, ##__VA_ARGS__);		\
	} while (0)

struct akd_dev {
	char			*buf;
	size_t			len;
	u32			status;
	struct mutex		lock;
	atomic_t		open_count;
};

static struct akd_dev *akd;

static int akd_open(struct inode *inode, struct file *file)
{
	int n = atomic_inc_return(&akd->open_count);

	akd_dbg("open (count=%d)\n", n);
	return 0;
}

static int akd_release(struct inode *inode, struct file *file)
{
	int n = atomic_dec_return(&akd->open_count);

	akd_dbg("release (count=%d)\n", n);
	return 0;
}

static ssize_t akd_read(struct file *file, char __user *ubuf,
			size_t count, loff_t *ppos)
{
	ssize_t ret;

	if (!ubuf)
		return -EINVAL;

	if (mutex_lock_interruptible(&akd->lock))
		return -ERESTARTSYS;

	if (*ppos >= akd->len) {
		ret = 0;
		goto out;
	}

	if (count > akd->len - *ppos)
		count = akd->len - *ppos;

	if (copy_to_user(ubuf, akd->buf + *ppos, count)) {
		ret = -EFAULT;
		goto out;
	}

	*ppos += count;
	ret = count;
	akd_dbg("read %zu bytes (pos=%lld)\n", count, *ppos);

out:
	mutex_unlock(&akd->lock);
	return ret;
}

static ssize_t akd_write(struct file *file, const char __user *ubuf,
			 size_t count, loff_t *ppos)
{
	ssize_t ret;

	if (!ubuf)
		return -EINVAL;

	if (count > AKD_BUF_SIZE)
		count = AKD_BUF_SIZE;

	if (mutex_lock_interruptible(&akd->lock))
		return -ERESTARTSYS;

	if (copy_from_user(akd->buf, ubuf, count)) {
		ret = -EFAULT;
		goto out;
	}

	akd->len = count;
	*ppos = count;
	akd->status = AKD_STATUS_IDLE;
	ret = count;
	akd_dbg("write %zu bytes\n", count);

out:
	mutex_unlock(&akd->lock);
	return ret;
}

static long akd_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	u32 val;
	int ret = 0;

	switch (cmd) {
	case AKD_IOC_GET_VERSION:
		val = AKD_VERSION;
		if (copy_to_user((void __user *)arg, &val, sizeof(val)))
			return -EFAULT;
		break;

	case AKD_IOC_GET_STATUS:
		if (mutex_lock_interruptible(&akd->lock))
			return -ERESTARTSYS;
		val = akd->status;
		mutex_unlock(&akd->lock);
		if (copy_to_user((void __user *)arg, &val, sizeof(val)))
			return -EFAULT;
		break;

	case AKD_IOC_SET_STATUS:
		if (copy_from_user(&val, (void __user *)arg, sizeof(val)))
			return -EFAULT;
		if (val > AKD_STATUS_ERROR)
			return -EINVAL;
		if (mutex_lock_interruptible(&akd->lock))
			return -ERESTARTSYS;
		akd->status = val;
		mutex_unlock(&akd->lock);
		akd_dbg("status set to %u\n", val);
		break;

	case AKD_IOC_CLEAR:
		if (mutex_lock_interruptible(&akd->lock))
			return -ERESTARTSYS;
		memset(akd->buf, 0, AKD_BUF_SIZE);
		akd->len = 0;
		akd->status = AKD_STATUS_IDLE;
		mutex_unlock(&akd->lock);
		akd_dbg("buffer cleared\n");
		break;

	case AKD_IOC_GET_LEN:
		if (mutex_lock_interruptible(&akd->lock))
			return -ERESTARTSYS;
		val = (u32)akd->len;
		mutex_unlock(&akd->lock);
		if (copy_to_user((void __user *)arg, &val, sizeof(val)))
			return -EFAULT;
		break;

	default:
		ret = -ENOTTY;
		break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long akd_compat_ioctl(struct file *file, unsigned int cmd,
			     unsigned long arg)
{
	return akd_ioctl(file, cmd, arg);
}
#endif

static const struct file_operations akd_fops = {
	.owner		= THIS_MODULE,
	.open		= akd_open,
	.release	= akd_release,
	.read		= akd_read,
	.write		= akd_write,
	.unlocked_ioctl	= akd_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= akd_compat_ioctl,
#endif
	.llseek		= default_llseek,
};

static struct miscdevice akd_misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= AKD_DEVICE_NAME,
	.fops	= &akd_fops,
	.mode	= 0660,
};

static int __init akd_init(void)
{
	int ret;

	akd = kzalloc(sizeof(*akd), GFP_KERNEL);
	if (!akd)
		return -ENOMEM;

	akd->buf = kzalloc(AKD_BUF_SIZE, GFP_KERNEL);
	if (!akd->buf) {
		kfree(akd);
		akd = NULL;
		return -ENOMEM;
	}

	mutex_init(&akd->lock);
	atomic_set(&akd->open_count, 0);
	akd->status = AKD_STATUS_IDLE;

	ret = misc_register(&akd_misc);
	if (ret) {
		pr_err("akd: misc_register failed: %d\n", ret);
		kfree(akd->buf);
		kfree(akd);
		akd = NULL;
		return ret;
	}

	pr_info("akd: loaded, device /dev/%s (version 0x%08x)\n",
		AKD_DEVICE_NAME, AKD_VERSION);
	return 0;
}

static void __exit akd_exit(void)
{
	misc_deregister(&akd_misc);
	if (akd) {
		kfree(akd->buf);
		kfree(akd);
		akd = NULL;
	}
	pr_info("akd: unloaded\n");
}

module_init(akd_init);
module_exit(akd_exit);
