/**
 * Simple linux kernel module that uses a table-driven state machine to implement
 * a traffic light using LEDs on a rasberry pi 3b+.
 *
 * The module assumes that the red, yellow and green led are connected to gpio
 * 14, 15 and 18 respectively.
 *
 * Green and red are on for 3 seconds. Yellow for 1.
 *
 * turn on traffic light: sudo sh -c 'echo 1 > /sys/kernel/traffic_light_control/power'
 * turn off traffic light: sudo sh -c 'echo 0 > /sys/kernel/traffic_light_control/power'
 */
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/gpio/driver.h>
#include <linux/timer.h>

static ssize_t power_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t power_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

#define RED_LED 14
#define YELLOW_LED 15
#define GREEN_LED 18

static struct kobject *traffic_light_kobj;
static struct kobj_attribute power_status_attr = __ATTR(power, 0664, power_show, (void *)power_store);
static struct gpio_chip *chip;

static bool g_power_state = false;

typedef enum {
    RED,
    YELLOW,
    GREEN,
    NUM_STATES
} light_t;

typedef struct {
    light_t light;
    struct timer_list timer;
} traffic_light_state_t;

static traffic_light_state_t traffic_light_state;

typedef struct {
    light_t next;
    u64 duration;
} transition_t;

transition_t state_table[NUM_STATES] = {
    [RED] = {GREEN, 3},
    [YELLOW] = {RED, 1},
    [GREEN] = {YELLOW, 3},
};

static int light_to_gpio_idx(light_t light) {
    switch (light) {
        case RED:
            return RED_LED;
            break;
        case YELLOW:
            return YELLOW_LED;
            break;
        case GREEN:
            return GREEN_LED;
            break;
        default:
            break;
    }

    return RED_LED;
}

static void turn_off_leds(void) {
    chip->set(chip, RED_LED, 0);
    chip->set(chip, YELLOW_LED, 0);
    chip->set(chip, GREEN_LED, 0);
}

static void turn_on_light(light_t light) {
    chip->set(chip, light_to_gpio_idx(light), 1);
}

static void turn_off_light(light_t light) {
    chip->set(chip, light_to_gpio_idx(light), 0);
}

static void set_timer(unsigned);
static void traffic_light_state_change(struct timer_list* timer) {
    traffic_light_state_t* traffic_light_state = from_timer(traffic_light_state, timer, timer);

    light_t cur = traffic_light_state->light;
    light_t next = state_table[cur].next;
    u64 next_duration = state_table[next].duration;

    turn_off_light(cur);
    turn_on_light(next);

    traffic_light_state->light = next;
    set_timer(next_duration);

    return;
}

static void set_timer(unsigned seconds) {
    struct timer_list* timer = &traffic_light_state.timer;
    timer_setup(timer, traffic_light_state_change, 0);
    timer->expires = jiffies + seconds * HZ;

    add_timer(timer);
}

static int chip_match_name(struct gpio_chip *chip, void *data)
{
	pr_info("chip label: %s", chip->label);
	return !strcmp(chip->label, data);
}

static ssize_t power_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "Power state: %d\n", g_power_state);
}

static ssize_t power_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    unsigned val;
    sscanf(buf, "%du", &val);

    // already on
    if ((bool)val && g_power_state) {
        return count;
    }
    // already off
    else if (!(bool)val && !g_power_state) {
        return count;
    }

    g_power_state = (bool)val;

    if (g_power_state) {
        pr_info("Starting traffic_light\n");

        traffic_light_state.light = RED;
        traffic_light_state_change(&traffic_light_state.timer);
    }
    else {
        pr_info("Turning off LEDs\n");

        del_timer(&traffic_light_state.timer);
        turn_off_leds();
    }
    return count;
}

static int __init mymodule_init(void)
{
    int err;

    chip = gpiochip_find("pinctrl-bcm2835", chip_match_name);
	if (!chip) {
        pr_err("Unable to find GPIO chip");
		return -ENODEV;
	}

    err = chip->direction_output(chip, RED_LED, 0);
    if (err) {
        pr_err("Error initializing led1");
        return err;
    }

    err = chip->direction_output(chip, YELLOW_LED, 0);
    if (err) {
        pr_err("Error initializing led1");
        return err;
    }

    err = chip->direction_output(chip, GREEN_LED, 0);
    if (err) {
        pr_err("Error initializing led1");
        return err;
    }

    // Create sysfs entry
    traffic_light_kobj = kobject_create_and_add("traffic_light_control", kernel_kobj);
    if (!traffic_light_kobj) {
        return -ENOMEM;
    }

    err = sysfs_create_file(traffic_light_kobj, &power_status_attr.attr);
    if (err) {
        pr_err("failed to create the myvariable file "
                "in /sys/kernel/mymodule\n");

        kobject_put(traffic_light_kobj);
        return err;
    }

    pr_info("Traffic light module initialized \n");
    return 0;
}

static void __exit mymodule_exit(void)
{
    sysfs_remove_file(traffic_light_kobj, &power_status_attr.attr);
    kobject_put(traffic_light_kobj);

    del_timer(&traffic_light_state.timer);
    turn_off_leds();

    pr_info("Traffic light module exit\n");

}

module_init(mymodule_init);
module_exit(mymodule_exit);


MODULE_LICENSE("GPL");
