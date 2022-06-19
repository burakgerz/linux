/*
 * Deferred Work
 *
 * Exercise #1, #2: simple timer
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

MODULE_DESCRIPTION("Simple kernel timer");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

#define TIMER_TIMEOUT_MS 5000

static struct timer_list timer;

static void timer_handler(struct timer_list *tl)
{
	/* TODO 1: print a message */
	pr_info("[timer_handler] printing sth...\n");

	/* TODO 2: rechedule timer */
	mod_timer(tl, jiffies + msecs_to_jiffies(TIMER_TIMEOUT_MS));
}

static int __init timer_init(void)
{
	pr_info("[timer_init] Init module\n");

	/* TODO 1: initialize timer */
	timer_setup(&timer, timer_handler, 0);

	/* TODO 1: schedule timer for the first time */
	mod_timer(&timer, jiffies + msecs_to_jiffies(TIMER_TIMEOUT_MS));

	return 0;
}

static void __exit timer_exit(void)
{
	pr_info("[timer_exit] Exit module\n");

	/* TODO 1: cleanup; make sure the timer is not running after we exit */
	del_timer_sync(&timer);
}

module_init(timer_init);
module_exit(timer_exit);
