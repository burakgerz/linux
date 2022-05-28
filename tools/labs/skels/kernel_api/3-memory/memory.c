/*
 * SO2 lab3 - task 3
 */

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("Memory processing");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

struct task_info {
	pid_t pid;
	unsigned long timestamp;
};

static struct task_info *ti1, *ti2, *ti3, *ti4;

static struct task_info *task_info_alloc(int pid)
{
	struct task_info *ti;

	ti = kzalloc(sizeof(*ti), GFP_KERNEL);
	ti->pid = pid;
	ti->timestamp = jiffies;

	return ti;
}

static int memory_init(void)
{
	ti1 = task_info_alloc(current->pid);

	ti2 = task_info_alloc(current->parent->pid);

	ti3 = task_info_alloc(next_task(current)->pid);

	ti4 = task_info_alloc(next_task(next_task(current))->pid);

	return 0;
}

static void memory_exit(void)
{
	pr_notice("Current pid: %d, timestamp: %ld\n", ti1->pid,
		  ti1->timestamp);
	pr_notice("Parent pid: %d, timestamp: %ld\n", ti2->pid, ti2->timestamp);
	pr_notice("Next pid: %d, timestamp: %ld\n", ti3->pid, ti3->timestamp);
	pr_notice("Next of next pid: %d, timestamp: %ld\n", ti4->pid,
		  ti4->timestamp);

	kfree(ti1);
	kfree(ti2);
	kfree(ti3);
	kfree(ti4);
}

module_init(memory_init);
module_exit(memory_exit);
