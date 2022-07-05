/*
 * Character device drivers lab
 *
 * All tasks
 */

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#include "../include/so2_cdev.h"

MODULE_DESCRIPTION("SO2 character device");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

#define LOG_LEVEL KERN_INFO

#define MY_MAJOR 42
#define MY_MINOR 0
#define NUM_MINORS 1
#define MODULE_NAME "so2_cdev"
#define MESSAGE "hello\n"
#define IOCTL_MESSAGE "Hello ioctl"

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

dev_t so2_dev_t = MKDEV(MY_MAJOR, MY_MINOR);

struct so2_device_data {
	/* TODO 2: add cdev member */
	struct cdev cdev;
	/* TODO 4: add buffer with BUFSIZ elements */
	char buffer[BUFSIZ];
	size_t buffer_size; // real buffer size
	/* TODO 7: extra members for home */
	/* TODO 3: add atomic_t access variable to keep track if file is opened */
	atomic_t access;
};

struct so2_device_data devs[NUM_MINORS];

static int so2_cdev_open(struct inode *inode, struct file *file)
{
	struct so2_device_data *data;

	/* TODO 2: print message when the device file is open. */
	pr_info("opening...\n");

	/* TODO 3: inode->i_cdev contains our cdev struct, use container_of to obtain a pointer to so2_device_data */
	data = container_of(inode->i_cdev, struct so2_device_data, cdev);
	file->private_data = data;

	/* TODO 3: return immediately if access is != 0, use atomic_cmpxchg */
	if (atomic_cmpxchg(&(data->access), 0, 1) != 0)
		return -EBUSY;

	//set_current_state(TASK_INTERRUPTIBLE);
	//schedule_timeout(10 * HZ);

	return 0;
}

static int so2_cdev_release(struct inode *inode, struct file *file)
{
	struct so2_device_data *data =
		(struct so2_device_data *)file->private_data;

	/* TODO 2: print message when the device file is closed. */
	pr_info("releasing...\n");

#ifndef EXTRA

	/* TODO 3: reset access variable to 0, use atomic_set */
	atomic_set(&data->access, 0);
#endif
	return 0;
}

static ssize_t so2_cdev_read(struct file *file, char __user *user_buffer,
			     size_t size, loff_t *offset)
{
	struct so2_device_data *data =
		(struct so2_device_data *)file->private_data;
	size_t to_read = 0;
	pr_info("read request...\n");

#ifdef EXTRA
	/* TODO 7: extra tasks for home */
#endif

	/* TODO 4: Copy data->buffer to user_buffer, use copy_to_user */
	if ((*offset + size) > data->buffer_size)
		to_read = data->buffer_size - *offset;
	else
		to_read = size;

	if (copy_to_user(user_buffer, data->buffer + *offset,
			 to_read) != 0) // or &data->buffer[*offset] as arg 2
		return -EFAULT;

	*offset += to_read; // update current file position

	return to_read;
}

static ssize_t so2_cdev_write(struct file *file, const char __user *user_buffer,
			      size_t size, loff_t *offset)
{
	struct so2_device_data *data =
		(struct so2_device_data *)file->private_data;

	/* TODO 5: copy user_buffer to data->buffer, use copy_from_user */
	if ((*offset + size) > BUFSIZ)
		size = BUFSIZ - *offset;
	else
		size = size;

	if (copy_from_user(data->buffer + *offset, user_buffer, size) != 0)
		return -EFAULT;
	*offset += size;
	data->buffer_size = *offset;

	/* TODO 7: extra tasks for home */

	return size;
}

static long so2_cdev_ioctl(struct file *file, unsigned int cmd,
			   unsigned long arg)
{
	struct so2_device_data *data =
		(struct so2_device_data *)file->private_data;
	int ret = 0;
	int remains;

	switch (cmd) {
	/* TODO 6: if cmd = MY_IOCTL_PRINT, display IOCTL_MESSAGE */
	case MY_IOCTL_PRINT:
		pr_info("%s\n", IOCTL_MESSAGE);
		break;
	/* TODO 7: extra tasks, for home */
	case MY_IOCTL_SET_BUFFER:
		if (copy_from_user(&data->buffer, (typeof(data->buffer) *)arg,
				   BUFFER_SIZE))
			return -EFAULT;
		break;
	case MY_IOCTL_GET_BUFFER:
		if (copy_to_user((typeof(data->buffer) *)arg, &data->buffer,
				 BUFFER_SIZE))
			return -EFAULT;
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static const struct file_operations so2_fops = {
	.owner = THIS_MODULE,
	/* TODO 2: add open and release functions */
	.open = so2_cdev_open,
	.release = so2_cdev_release,
	/* TODO 4: add read function */
	.read = so2_cdev_read,
	/* TODO 5: add write function */
	.write = so2_cdev_write,
	/* TODO 6: add ioctl function */
	.unlocked_ioctl = so2_cdev_ioctl,
};

static int so2_cdev_init(void)
{
	int err;
	int i;

	/* TODO 1: register char device region for MY_MAJOR and NUM_MINORS starting at MY_MINOR */
	err = register_chrdev_region(so2_dev_t, NUM_MINORS, MODULE_NAME);

	if (err != 0)
		return err;

	for (i = 0; i < NUM_MINORS; i++) {
#ifdef EXTRA
		/* TODO 7: extra tasks, for home */
#else
		/*TODO 4: initialize buffer with MESSAGE string */
		strncpy((devs[i].buffer), MESSAGE, BUFSIZ);
		devs[i].buffer_size = sizeof(
			MESSAGE); // maybe strlen(devs[i].buffer) is also ok
#endif
		/* TODO 7: extra tasks for home */
		/* TODO 3: set access variable to 0, use atomic_set */
		atomic_set(&(devs[i].access), 0);
		/* TODO 2: init and add cdev to kernel core */
		cdev_init(&(devs[i].cdev), &so2_fops);
		// +i because of minor number
		cdev_add(&(devs[i].cdev), so2_dev_t + i, 1);
	}
	pr_info("module loaded\n");
	return 0;
}

static void so2_cdev_exit(void)
{
	int i;

	for (i = 0; i < NUM_MINORS; i++) {
		/* TODO 2: delete cdev from kernel core */
		cdev_del(&(devs[i].cdev));
	}

	/* TODO 1: unregister char device region, for MY_MAJOR and NUM_MINORS starting at MY_MINOR */
	unregister_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), NUM_MINORS);
	pr_info("module unloaded\n");
}

module_init(so2_cdev_init);
module_exit(so2_cdev_exit);
