/*
 * Freescale Management Complex (MC) restool driver
 *
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 * Author: Lijun Pan <Lijun.Pan@freescale.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include "../include/mc-private.h"
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include "mc-ioctl.h"
#include "../include/mc-sys.h"
#include "../include/mc-cmd.h"
#include "../include/dpmng.h"

/**
 * Maximum number of DPRCs that can be opened at the same time
 */
#define MAX_DPRC_HANDLES	    64

/**
 * restool_misc - information associated with the newly added miscdevice
 * @misc: newly created miscdevice associated with root dprc
 * @miscdevt: device id of this miscdevice
 * @list: a linked list node representing this miscdevcie
 * @static_mc_io: pointer to the static MC I/O object used by the restool
 * @dynamic_instance_count: number of dynamically created instances
 * @static_instance_in_use: static instance is in use or not
 * @mutex: mutex lock to serialze the open/release operations
 * @dev: root dprc associated with this miscdevice
 */
struct restool_misc {
	struct miscdevice misc;
	dev_t miscdevt;
	struct list_head list;
	struct fsl_mc_io *static_mc_io;
	u32 dynamic_instance_count;
	bool static_instance_in_use;
	struct mutex mutex; /* serialze the open/release operations */
	struct device *dev;
};

/*
 * initialize a global list to link all
 * the miscdevice nodes (struct restool_misc)
 */
static LIST_HEAD(misc_list);
static DEFINE_MUTEX(misc_list_mutex);

static int fsl_mc_restool_dev_open(struct inode *inode, struct file *filep)
{
	struct fsl_mc_device *root_mc_dev;
	int error;
	struct fsl_mc_io *dynamic_mc_io = NULL;
	struct restool_misc *restool_misc = NULL;
	struct restool_misc *restool_misc_cursor;

	mutex_lock(&misc_list_mutex);

	list_for_each_entry(restool_misc_cursor, &misc_list, list) {
		if (restool_misc_cursor->miscdevt == inode->i_rdev) {
			restool_misc = restool_misc_cursor;
			break;
		}
	}

	mutex_unlock(&misc_list_mutex);

	if (!restool_misc)
		return -EINVAL;

	if (WARN_ON(!restool_misc->dev))
		return -EINVAL;

	mutex_lock(&restool_misc->mutex);

	if (!restool_misc->static_instance_in_use) {
		restool_misc->static_instance_in_use = true;
		filep->private_data = restool_misc->static_mc_io;
	} else {
		dynamic_mc_io = kzalloc(sizeof(*dynamic_mc_io), GFP_KERNEL);
		if (!dynamic_mc_io) {
			error = -ENOMEM;
			goto err_unlock;
		}

		root_mc_dev = to_fsl_mc_device(restool_misc->dev);
		error = fsl_mc_portal_allocate(root_mc_dev, 0, &dynamic_mc_io);
		if (error < 0) {
			pr_err("Not able to allocate MC portal\n");
			goto free_dynamic_mc_io;
		}
		++restool_misc->dynamic_instance_count;
		filep->private_data = dynamic_mc_io;
	}

	mutex_unlock(&restool_misc->mutex);

	return 0;

free_dynamic_mc_io:
	kfree(dynamic_mc_io);
err_unlock:
	mutex_unlock(&restool_misc->mutex);

	return error;
}

static int fsl_mc_restool_dev_release(struct inode *inode, struct file *filep)
{
	struct fsl_mc_io *local_mc_io = filep->private_data;
	struct restool_misc *restool_misc = NULL;
	struct restool_misc *restool_misc_cursor;

	if (WARN_ON(!filep->private_data))
		return -EINVAL;

	mutex_lock(&misc_list_mutex);

	list_for_each_entry(restool_misc_cursor, &misc_list, list) {
		if (restool_misc_cursor->miscdevt == inode->i_rdev) {
			restool_misc = restool_misc_cursor;
			break;
		}
	}

	mutex_unlock(&misc_list_mutex);

	if (!restool_misc)
		return -EINVAL;

	mutex_lock(&restool_misc->mutex);

	if (WARN_ON(restool_misc->dynamic_instance_count == 0 &&
		    !restool_misc->static_instance_in_use)) {
		mutex_unlock(&restool_misc->mutex);
		return -EINVAL;
	}

	/* Globally clean up opened/untracked handles */
	fsl_mc_portal_reset(local_mc_io);

	/*
	 * must check
	 * whether local_mc_io is dynamic or static instance
	 * Otherwise it will free up the reserved portal by accident
	 * or even not free up the dynamic allocated portal
	 * if 2 or more instances running concurrently
	 */
	if (local_mc_io == restool_misc->static_mc_io) {
		restool_misc->static_instance_in_use = false;
	} else {
		fsl_mc_portal_free(local_mc_io);
		kfree(filep->private_data);
		--restool_misc->dynamic_instance_count;
	}

	filep->private_data = NULL;
	mutex_unlock(&restool_misc->mutex);

	return 0;
}

static int restool_send_mc_command(unsigned long arg,
				   struct fsl_mc_io *local_mc_io)
{
	int error;
	struct mc_command mc_cmd;

	if (copy_from_user(&mc_cmd, (void __user *)arg, sizeof(mc_cmd)))
		return -EFAULT;

	/*
	 * Send MC command to the MC:
	 */
	error = mc_send_command(local_mc_io, &mc_cmd);
	if (error < 0)
		return error;

	if (copy_to_user((void __user *)arg, &mc_cmd, sizeof(mc_cmd)))
		return -EFAULT;

	return 0;
}

static long
fsl_mc_restool_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int error;

	switch (cmd) {
	case RESTOOL_SEND_MC_COMMAND:
		error = restool_send_mc_command(arg, file->private_data);
		break;
	default:
		pr_err("%s: unexpected ioctl call number\n", __func__);
		error = -EINVAL;
	}

	return error;
}

static const struct file_operations fsl_mc_restool_dev_fops = {
	.owner = THIS_MODULE,
	.open = fsl_mc_restool_dev_open,
	.release = fsl_mc_restool_dev_release,
	.unlocked_ioctl = fsl_mc_restool_dev_ioctl,
};

static int restool_add_device_file(struct device *dev)
{
	u32 name1 = 0;
	char name2[20] = {0};
	int error;
	struct fsl_mc_device *root_mc_dev;
	struct restool_misc *restool_misc;

	if (dev->bus == &platform_bus_type && dev->driver_data) {
		if (sscanf(dev_name(dev), "%x.%s", &name1, name2) != 2)
			return -EINVAL;

		if (strcmp(name2, "fsl-mc") == 0)
			pr_debug("platform's root dprc name is: %s\n",
				 dev_name(&(((struct fsl_mc *)
				 (dev->driver_data))->root_mc_bus_dev->dev)));
	}

	if (!fsl_mc_is_root_dprc(dev))
		return 0;

	restool_misc = kzalloc(sizeof(*restool_misc), GFP_KERNEL);
	if (!restool_misc)
		return -ENOMEM;

	restool_misc->dev = dev;
	root_mc_dev = to_fsl_mc_device(dev);
	error = fsl_mc_portal_allocate(root_mc_dev, 0,
				       &restool_misc->static_mc_io);
	if (error < 0) {
		pr_err("Not able to allocate MC portal\n");
		goto free_restool_misc;
	}

	restool_misc->misc.minor = MISC_DYNAMIC_MINOR;
	restool_misc->misc.name = dev_name(dev);
	restool_misc->misc.fops = &fsl_mc_restool_dev_fops;

	error = misc_register(&restool_misc->misc);
	if (error < 0) {
		pr_err("misc_register() failed: %d\n", error);
		goto free_portal;
	}

	restool_misc->miscdevt = restool_misc->misc.this_device->devt;
	mutex_init(&restool_misc->mutex);
	mutex_lock(&misc_list_mutex);
	list_add(&restool_misc->list, &misc_list);
	mutex_unlock(&misc_list_mutex);

	pr_info("/dev/%s driver registered\n", dev_name(dev));

	return 0;

free_portal:
	fsl_mc_portal_free(restool_misc->static_mc_io);
free_restool_misc:
	kfree(restool_misc);

	return error;
}

static int restool_bus_notifier(struct notifier_block *nb,
				unsigned long action, void *data)
{
	int error;
	struct device *dev = data;

	switch (action) {
	case BUS_NOTIFY_ADD_DEVICE:
		error = restool_add_device_file(dev);
		if (error)
			return error;
		break;
	case BUS_NOTIFY_DEL_DEVICE:
	case BUS_NOTIFY_REMOVED_DEVICE:
	case BUS_NOTIFY_BIND_DRIVER:
	case BUS_NOTIFY_BOUND_DRIVER:
	case BUS_NOTIFY_UNBIND_DRIVER:
	case BUS_NOTIFY_UNBOUND_DRIVER:
		break;
	default:
		pr_err("%s: unrecognized device action from %s\n", __func__,
		       dev_name(dev));
		return -EINVAL;
	}

	return 0;
}

static int add_to_restool(struct device *dev, void *data)
{
	return restool_add_device_file(dev);
}

static int __init fsl_mc_restool_driver_init(void)
{
	int error;
	struct notifier_block *nb;

	nb = kzalloc(sizeof(*nb), GFP_KERNEL);
	if (!nb)
		return -ENOMEM;

	nb->notifier_call = restool_bus_notifier;
	error = bus_register_notifier(&fsl_mc_bus_type, nb);
	if (error)
		goto free_nb;

	/*
	 * This driver runs after fsl-mc bus driver runs.
	 * Hence, many of the root dprcs are already attached to fsl-mc bus
	 * In order to make sure we find all the root dprcs,
	 * we need to scan the fsl_mc_bus_type.
	 */
	error  = bus_for_each_dev(&fsl_mc_bus_type, NULL, NULL, add_to_restool);
	if (error) {
		bus_unregister_notifier(&fsl_mc_bus_type, nb);
		kfree(nb);
		pr_err("restool driver registration failure\n");
		return error;
	}

	return 0;

free_nb:
	kfree(nb);
	return error;
}

module_init(fsl_mc_restool_driver_init);

static void __exit fsl_mc_restool_driver_exit(void)
{
	struct restool_misc *restool_misc;
	struct restool_misc *restool_misc_tmp;
	char name1[20] = {0};
	u32 name2 = 0;

	list_for_each_entry_safe(restool_misc, restool_misc_tmp,
				 &misc_list, list) {
		if (sscanf(restool_misc->misc.name, "%4s.%u", name1, &name2)
		    != 2)
			continue;

		pr_debug("name1=%s,name2=%u\n", name1, name2);
		pr_debug("misc-device: %s\n", restool_misc->misc.name);
		if (strcmp(name1, "dprc") != 0)
			continue;

		if (WARN_ON(!restool_misc->static_mc_io))
			return;

		if (WARN_ON(restool_misc->dynamic_instance_count != 0))
			return;

		if (WARN_ON(restool_misc->static_instance_in_use))
			return;

		misc_deregister(&restool_misc->misc);
		pr_info("/dev/%s driver unregistered\n",
			restool_misc->misc.name);
		fsl_mc_portal_free(restool_misc->static_mc_io);
		list_del(&restool_misc->list);
		kfree(restool_misc);
	}
}

module_exit(fsl_mc_restool_driver_exit);

MODULE_AUTHOR("Freescale Semiconductor Inc.");
MODULE_DESCRIPTION("Freescale's MC restool driver");
MODULE_LICENSE("GPL");
