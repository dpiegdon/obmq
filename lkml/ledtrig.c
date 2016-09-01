/*
 * OBMQ LED Trigger
 *
 * Copyright (C) 2016 David R. Piegdon
 *
 * Based on Richard Purdie's ledtrig-timer.c
 * and Atsushi Nemoto's ledtrig-heartbeat.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/leds.h>
#include <linux/reboot.h>
#include <linux/suspend.h>

#include "obmq.h"

// from linux/drivers/leds.h which is NOT exported to external modules...
extern void led_set_brightness_nosleep(struct led_classdev *led_cdev,
				enum led_brightness value);

static int panic_obmqs;

struct obmq_trig_data {
	struct timer_list timer;
	unsigned int invert;
	OneBitMessageQueue mqueue;
	int idle_state;
};

static void led_obmq_function(unsigned long data)
{
	struct led_classdev *led_cdev = (struct led_classdev *) data;
	struct obmq_trig_data *obmq_data = led_cdev->trigger_data;
	unsigned long delay;

	if (unlikely(panic_obmqs)) {
		led_set_brightness_nosleep(led_cdev, LED_OFF);
		return;
	}

	if(0 != obmq_messages_queued(&obmq_data->mqueue))
	{
		delay = msecs_to_jiffies(40);
		obmq_data->idle_state = 0;
	} else {
		delay = msecs_to_jiffies(1000);
		obmq_data->idle_state += 1;
		if(obmq_data->idle_state == 2) {
			led_set_brightness_nosleep(led_cdev, LED_OFF);
		} else {
			led_set_brightness_nosleep(led_cdev, led_cdev->max_brightness);
			obmq_data->idle_state = 1;
		}
	}

	obmq_trigger(&obmq_data->mqueue);

	mod_timer(&obmq_data->timer, jiffies + delay);
}

static ssize_t led_invert_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct obmq_trig_data *obmq_data = led_cdev->trigger_data;

	return sprintf(buf, "%u\n", obmq_data->invert);
}

static ssize_t led_invert_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct obmq_trig_data *obmq_data = led_cdev->trigger_data;
	unsigned long state;
	int ret;

	ret = kstrtoul(buf, 0, &state);
	if (ret)
		return ret;

	obmq_data->invert = !!state;

	return size;
}

static ssize_t led_message_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct obmq_trig_data *obmq_data = led_cdev->trigger_data;
	int i;

	for(i = 0; i < size; ++i)
		obmq_queuemessage(&obmq_data->mqueue, buf[i]);
	return i;
}

static DEVICE_ATTR(invert, 0644, led_invert_show, led_invert_store);
static DEVICE_ATTR(queue, 0200, NULL, led_message_store);

static void obmq_trigger_set_led(void * data, char value)
{
	struct led_classdev *led_cdev = (struct led_classdev *) data;
	struct obmq_trig_data *obmq_data = led_cdev->trigger_data;

	bool set = value;
	if(obmq_data->invert)
		set = !set;

	led_set_brightness_nosleep(led_cdev, set ? led_cdev->max_brightness : LED_OFF);
}

static void obmq_trig_activate(struct led_classdev *led_cdev)
{
	struct obmq_trig_data *obmq_data;
	int rc;

	obmq_data = kzalloc(sizeof(*obmq_data), GFP_KERNEL);
	if (!obmq_data)
		return;

	led_cdev->trigger_data = obmq_data;
	rc = device_create_file(led_cdev->dev, &dev_attr_invert);
	if (rc) {
		kfree(led_cdev->trigger_data);
		return;
	}
	rc = device_create_file(led_cdev->dev, &dev_attr_queue);
	if (rc) {
		device_remove_file(led_cdev->dev, &dev_attr_queue);
		kfree(led_cdev->trigger_data);
		return;
	}

	setup_timer(&obmq_data->timer,
		    led_obmq_function, (unsigned long) led_cdev);
	obmq_init(&obmq_data->mqueue, obmq_trigger_set_led, led_cdev, 0, 0, 4, 8, 0);
	led_obmq_function(obmq_data->timer.data);
	led_cdev->activated = true;
}

static void obmq_trig_deactivate(struct led_classdev *led_cdev)
{
	struct obmq_trig_data *obmq_data = led_cdev->trigger_data;

	if (led_cdev->activated) {
		del_timer_sync(&obmq_data->timer);
		device_remove_file(led_cdev->dev, &dev_attr_queue);
		device_remove_file(led_cdev->dev, &dev_attr_invert);
		kfree(obmq_data);
		led_cdev->activated = false;
	}
}

static struct led_trigger obmq_led_trigger = {
	.name     = "obmq",
	.activate = obmq_trig_activate,
	.deactivate = obmq_trig_deactivate,
};

static int obmq_pm_notifier(struct notifier_block *nb,
				 unsigned long pm_event, void *unused)
{
	int rc;

	switch (pm_event) {
	case PM_SUSPEND_PREPARE:
	case PM_HIBERNATION_PREPARE:
	case PM_RESTORE_PREPARE:
		led_trigger_unregister(&obmq_led_trigger);
		break;
	case PM_POST_SUSPEND:
	case PM_POST_HIBERNATION:
	case PM_POST_RESTORE:
		rc = led_trigger_register(&obmq_led_trigger);
		if (rc)
			pr_err("could not re-register obmq trigger\n");
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}

static int obmq_reboot_notifier(struct notifier_block *nb,
				     unsigned long code, void *unused)
{
	led_trigger_unregister(&obmq_led_trigger);
	return NOTIFY_DONE;
}

static int obmq_panic_notifier(struct notifier_block *nb,
				     unsigned long code, void *unused)
{
	panic_obmqs = 1;
	return NOTIFY_DONE;
}

static struct notifier_block obmq_pm_nb = {
	.notifier_call = obmq_pm_notifier,
};

static struct notifier_block obmq_reboot_nb = {
	.notifier_call = obmq_reboot_notifier,
};

static struct notifier_block obmq_panic_nb = {
	.notifier_call = obmq_panic_notifier,
};

static int __init obmq_trig_init(void)
{
	int rc = led_trigger_register(&obmq_led_trigger);

	if (!rc) {
		atomic_notifier_chain_register(&panic_notifier_list,
					       &obmq_panic_nb);
		register_reboot_notifier(&obmq_reboot_nb);
		register_pm_notifier(&obmq_pm_nb);
	}
	return rc;
}

static void __exit obmq_trig_exit(void)
{
	unregister_pm_notifier(&obmq_pm_nb);
	unregister_reboot_notifier(&obmq_reboot_nb);
	atomic_notifier_chain_unregister(&panic_notifier_list,
					 &obmq_panic_nb);
	led_trigger_unregister(&obmq_led_trigger);
}

module_init(obmq_trig_init);
module_exit(obmq_trig_exit);

MODULE_DESCRIPTION("OneBitMessageQueue as LED trigger");
MODULE_AUTHOR("David R. Piegdon");
MODULE_LICENSE("GPL");

