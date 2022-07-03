// SPDX-License-Identifier: GPL-2.0+

/*
 * list.c - Linux kernel list API
 *
 * TODO 1/0: Fill in name / email
 * Author: FirstName LastName <user@email.com>
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define PROCFS_MAX_SIZE 512
#define LIST_ELEMENT_MAX_SIZE 10

#define procfs_dir_name "list"
#define procfs_file_read "preview"
#define procfs_file_write "management"

struct proc_dir_entry *proc_list;
struct proc_dir_entry *proc_list_read;
struct proc_dir_entry *proc_list_write;

/* TODO 2: define your list! */
struct char_list {
	char element[LIST_ELEMENT_MAX_SIZE];
	struct list_head list;
};
static struct list_head head;

static int list_proc_show(struct seq_file *m, void *v)
{
	/* TODO 3: print your list. One element / line. */
	struct list_head *p;
	struct char_list *clist;

	list_for_each (p, &head) {
		clist = list_entry(p, struct char_list, list);
		seq_puts(m, clist->element);
	}
	return 0;
}

static int list_read_open(struct inode *inode, struct file *file)
{
	return single_open(file, list_proc_show, NULL);
}

static int list_write_open(struct inode *inode, struct file *file)
{
	return single_open(file, list_proc_show, NULL);
}

static struct char_list *char_list_alloc(char *element)
{
	struct char_list *clist;
	int max_copy = LIST_ELEMENT_MAX_SIZE - 1;

	clist = kzalloc(sizeof(*clist), GFP_KERNEL);
	if (clist == NULL)
		return NULL;
	strncpy(clist->element, element, max_copy);

	return clist;
}

static void AddFront(char *char_element)
{
	struct char_list *clist;

	clist = char_list_alloc(char_element); //neglet check.. too lazy
	list_add(&clist->list, &head);
}

static void AddBack(char *char_element)
{
	struct char_list *clist;

	clist = char_list_alloc(char_element); //neglet check.. too lazy
	list_add_tail(&clist->list, &head);
}

static void RemoveFirstEntry(char *char_element)
{
	struct char_list *clist;
	struct list_head *pos, *next;

	list_for_each_safe (pos, next, &head) {
		clist = list_entry(pos, struct char_list, list);
		if (strcmp(&clist->element[0], char_element) == 0) {
			list_del(pos);
			kfree(clist);
			break;
		}
	}
}

static void RemoveAll(char *char_element)
{
	struct char_list *clist;
	struct list_head *pos, *next;

	list_for_each_safe (pos, next, &head) {
		clist = list_entry(pos, struct char_list, list);
		if (strcmp(&clist->element[0], char_element) == 0) {
			list_del(pos);
			kfree(clist);
		}
	}
}

static ssize_t list_write(struct file *file, const char __user *buffer,
			  size_t count, loff_t *offs)
{
	char local_buffer[PROCFS_MAX_SIZE];
	unsigned long local_buffer_size = 0;
	char *plocal_buffer = NULL;
	char *command = NULL;
	char *char_element = NULL;

	local_buffer_size = count;
	if (local_buffer_size > PROCFS_MAX_SIZE)
		local_buffer_size = PROCFS_MAX_SIZE;

	memset(local_buffer, 0, PROCFS_MAX_SIZE);
	if (copy_from_user(local_buffer, buffer, local_buffer_size))
		return -EFAULT;
	plocal_buffer = &local_buffer[0];

	/* local_buffer contains your command written in /proc/list/management
	 * TODO 4/0: parse the command and add/delete elements.
	 */
	command = strsep(&plocal_buffer, " ");
	char_element = plocal_buffer;
	if (char_element == NULL) {
		pr_err("Second param cannot be empty\n");
		return -EINVAL;
	}

	if (strcmp(command, "addf") == 0) {
		AddFront(char_element);
	} else if (strcmp(command, "adde") == 0) {
		AddBack(char_element);
	} else if (strcmp(command, "delf") == 0) {
		RemoveFirstEntry(char_element);
	} else if (strcmp(command, "dela") == 0) {
		RemoveAll(char_element);
	} else {
		pr_err("First param is not recognized\n");
		return -EINVAL;
	}

	return local_buffer_size;
}

static const struct proc_ops r_pops = {
	.proc_open = list_read_open,
	.proc_read = seq_read,
	.proc_release = single_release,
};

static const struct proc_ops w_pops = {
	.proc_open = list_write_open,
	.proc_write = list_write,
	.proc_release = single_release,
};

static int list_init(void)
{
	INIT_LIST_HEAD(&head); // TODO: Is this needed?

	proc_list = proc_mkdir(procfs_dir_name, NULL);
	if (!proc_list)
		return -ENOMEM;

	proc_list_read =
		proc_create(procfs_file_read, 0000, proc_list, &r_pops);
	if (!proc_list_read)
		goto proc_list_cleanup;

	proc_list_write =
		proc_create(procfs_file_write, 0000, proc_list, &w_pops);
	if (!proc_list_write)
		goto proc_list_read_cleanup;

	return 0;

proc_list_read_cleanup:
	proc_remove(proc_list_read);
proc_list_cleanup:
	proc_remove(proc_list);
	return -ENOMEM;
}

static void list_exit(void)
{
	proc_remove(proc_list);
}

module_init(list_init);
module_exit(list_exit);

MODULE_DESCRIPTION("Linux kernel list API");
/* TODO 5: Fill in your name / email address */
MODULE_AUTHOR("FirstName LastName <your@email.com>");
MODULE_LICENSE("GPL v2");
