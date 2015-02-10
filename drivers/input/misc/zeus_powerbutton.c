/* This file is subject to the terms and conditions of the GNU General
 * Public License. See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <plat/gpio.h>
#include <plat/hardware.h>
#include <plat/mux.h>

#include <linux/i2c/twl.h>

//#define USE_DEBOUNCE	/* to prevent chattering effect for pwr and home button */

#ifdef USE_DEBOUNCE
#define DEBOUNCE_INTERVAL_MS	10
#endif

static int __devinit power_key_driver_probe(struct platform_device *plat_dev);
static irqreturn_t powerkey_press_handler(int irq_num, void * dev);
#ifdef CONFIG_INPUT_HARD_RESET_KEY
static irqreturn_t homekey_press_handler(int irq_num, void * dev);
#endif

struct work_struct  lcd_work;
static struct workqueue_struct *lcd_wq=NULL;
void lcd_work_func(struct work_struct *work)
{
//	printk("[LCD] lcd_work_func\n");
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x5, 0x1E);	/* VAUX2 dedicated 1.8v */
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0xe0, 0x1B);	/* VAUX2 devgrp */

	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0xb, 0x26);	/* VAUX4 dedicated 3.0v */
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0xe0, 0x23);	/* VAUX4 devgrp */
}

static struct input_dev *g_input_dev = NULL;
struct work_struct pwr_button_work;
static void pwr_button_work_func(struct work_struct* work)
{
	int key_press_status = gpio_get_value(OMAP_GPIO_KEY_PWRON);

	if (key_press_status >= 0){
		printk("[PWR-KEY] KEY_POWER event = %d\n",key_press_status);
		input_report_key(g_input_dev, KEY_POWER, key_press_status);
		input_sync(g_input_dev);
		if (lcd_wq && key_press_status)  
			queue_work(lcd_wq, &lcd_work);
	}
}

#ifdef CONFIG_INPUT_HARD_RESET_KEY
struct work_struct home_button_work;
static void home_button_work_func(struct work_struct* work)
{
	int key_press_status = !gpio_get_value(OMAP_GPIO_KEY_HOME);

	if (key_press_status >= 0) {
		printk("[HOME-KEY] KEY_HOME event = %d\n",key_press_status);
		input_report_key(g_input_dev, KEY_HOME, key_press_status);
		input_sync(g_input_dev);
	}
}
#endif

#ifdef USE_DEBOUNCE

static struct timer_list debounce_pwrbtn_timer;

static void debounce_pwrbtn_timer_handler(unsigned long data)
{
	schedule_work(&pwr_button_work);
}

#ifdef CONFIG_INPUT_HARD_RESET_KEY
static struct timer_list debounce_homebtn_timer;

static void debounce_homebtn_timer_handler(unsigned long data)
{
	schedule_work(&home_button_work);
}
#endif

#endif	/*USE_DEBOUNCE */


ssize_t gpiokey_pressed_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int key_press_status=0;
	key_press_status |= gpio_get_value(OMAP_GPIO_KEY_PWRON);
#ifdef CONFIG_INPUT_HARD_RESET_KEY
	key_press_status |= !gpio_get_value(OMAP_GPIO_KEY_HOME);
#endif
	key_press_status |= !gpio_get_value(OMAP_GPIO_KEY_AALTO_VOLUP);
	key_press_status |= !gpio_get_value(OMAP_GPIO_KEY_AALTO_VOLDN);

	return sprintf(buf, "%u\n", key_press_status);
}

static DEVICE_ATTR(gpiokey_pressed, S_IRUGO, gpiokey_pressed_show, NULL);
  
static irqreturn_t powerkey_press_handler(int irq_num, void * dev)
{
	struct input_dev *ip_dev = (struct input_dev *)dev;

	if (!ip_dev) {
		dev_err(ip_dev->dev.parent,"Input Device not allocated\n");
		return IRQ_HANDLED;
	}

	if (g_input_dev == NULL)
		g_input_dev = (struct input_dev*)dev;

#ifdef USE_DEBOUNCE
	mod_timer(&debounce_pwrbtn_timer, jiffies + msecs_to_jiffies(DEBOUNCE_INTERVAL_MS));
#else
	schedule_work(&pwr_button_work);
#endif

	return IRQ_HANDLED;
	}

#ifdef CONFIG_INPUT_HARD_RESET_KEY
static irqreturn_t homekey_press_handler(int irq_num, void * dev)
{
	struct input_dev *ip_dev = (struct input_dev *)dev;
  
	if (!ip_dev) {
		dev_err(ip_dev->dev.parent,"Input Device not allocated\n");
		return IRQ_HANDLED;
	}

	if (g_input_dev == NULL)
		g_input_dev = (struct input_dev*)dev;

#ifdef USE_DEBOUNCE
	mod_timer(&debounce_homebtn_timer, jiffies + msecs_to_jiffies(DEBOUNCE_INTERVAL_MS));
#else
	schedule_work(&home_button_work);
#endif

	return IRQ_HANDLED;
}
#endif

static int __devinit power_key_driver_probe(struct platform_device *plat_dev)
{
	struct input_dev *power_key = NULL;
	struct kobject *gpiokey = NULL;
	int pwr_key_irq = -1;
	int err = 0;

	gpiokey = kobject_create_and_add("gpiokey", NULL);

	if (!gpiokey) {
		printk("Failed to create sysfs(gpiokey)!\n");
		return -ENOMEM;
	}

	if (sysfs_create_file(gpiokey, &dev_attr_gpiokey_pressed.attr) < 0)
		printk("Failed to create device file(%s)!\n", dev_attr_gpiokey_pressed.attr.name);

	INIT_WORK(&lcd_work, lcd_work_func);
	lcd_wq = create_singlethread_workqueue("lcd_vaux_wq");

	INIT_WORK(&pwr_button_work, pwr_button_work_func);
	INIT_WORK(&home_button_work, home_button_work_func);

#ifdef USE_DEBOUNCE
	setup_timer(&debounce_pwrbtn_timer, debounce_pwrbtn_timer_handler, 0);
	setup_timer(&debounce_homebtn_timer, debounce_homebtn_timer_handler, 0);
#endif

	pwr_key_irq = platform_get_irq(plat_dev, 0);
#ifdef CONFIG_INPUT_HARD_RESET_KEY
	int home_key_irq = -1;
	home_key_irq = platform_get_irq(plat_dev, 1);
	if(pwr_key_irq <= 0 || home_key_irq <= 0) {
		dev_err(&plat_dev->dev,"failed to map the power key to an IRQ %d & %d\n",pwr_key_irq, home_key_irq);
		err = -ENXIO;
		return err;
	}
#else
	if (pwr_key_irq <= 0 ) {
		dev_err(&plat_dev->dev,"failed to map the power key to an IRQ %d\n",pwr_key_irq);
		err = -ENXIO;
		return err;
	}
#endif
	power_key = input_allocate_device();
	if (!power_key) {
		dev_err(&plat_dev->dev,"failed to allocate an input devd %d \n",pwr_key_irq);
		err = -ENOMEM;
		return err;
	}
	err = request_irq(pwr_key_irq, &powerkey_press_handler ,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
					"power_key_driver",power_key);
#ifdef CONFIG_INPUT_HARD_RESET_KEY
	err |= request_irq(home_key_irq, &homekey_press_handler ,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,
                    "power_key_driver",power_key);
#endif
	if (err) {
#ifdef CONFIG_INPUT_HARD_RESET_KEY
		dev_err(&plat_dev->dev,"failed to request an IRQ handler for num %d & %d\n",pwr_key_irq, home_key_irq);
#else
		dev_err(&plat_dev->dev,"failed to request an IRQ handler for num %d\n",pwr_key_irq);
#endif
		goto free_input_dev;
	}

	dev_dbg(&plat_dev->dev,"\n Power Key Drive:Assigned IRQ num %d SUCCESS \n",pwr_key_irq);
#ifdef CONFIG_INPUT_HARD_RESET_KEY
	dev_dbg(&plat_dev->dev,"\n HOME Key Drive:Assigned IRQ num %d SUCCESS \n",home_key_irq);
#endif
	/* register the input device now */
	input_set_capability(power_key, EV_KEY, KEY_POWER);
#ifdef CONFIG_INPUT_HARD_RESET_KEY
	input_set_capability(power_key, EV_KEY, KEY_HOME);
#endif
	power_key->name = "sec_power_key";
	power_key->phys = "sec_power_key/input0";
	power_key->dev.parent = &plat_dev->dev;
	platform_set_drvdata(plat_dev, power_key);

	err = input_register_device(power_key);
	if (err) {
		dev_err(&plat_dev->dev, "power key couldn't be registered: %d\n", err);
		goto release_irq_num;
	}
 
	return 0;

release_irq_num:
	free_irq(pwr_key_irq, NULL); //pass devID as NULL as device registration failed 

free_input_dev:
	input_free_device(power_key);

	return err;
}

static int __devexit power_key_driver_remove(struct platform_device *plat_dev)
{
	struct input_dev *ip_dev= platform_get_drvdata(plat_dev);
	int pwr_key_irq=0;
#ifdef CONFIG_INPUT_HARD_RESET_KEY
	int home_key_irq=0;
#endif

#ifdef USE_DEBOUNCE
	del_timer_sync(&debounce_homebtn_timer);
	del_timer_sync(&debounce_pwrbtn_timer);
#endif

	pwr_key_irq = platform_get_irq(plat_dev, 0);
#ifdef CONFIG_INPUT_HARD_RESET_KEY
	home_key_irq = platform_get_irq(plat_dev, 1);
#endif
	free_irq(pwr_key_irq, ip_dev);
#ifdef CONFIG_INPUT_HARD_RESET_KEY
	free_irq(home_key_irq, ip_dev);
#endif

	input_unregister_device(ip_dev);

	if (lcd_wq)
		destroy_workqueue(lcd_wq);

	return 0;
}

struct platform_driver power_key_driver_t = {
	.probe		= &power_key_driver_probe,
	.remove		= __devexit_p(power_key_driver_remove),
	.driver		= {
		.name   = "power_key_device", 
		.owner  = THIS_MODULE,
	},
};

static int __init power_key_driver_init(void)
{
	return platform_driver_register(&power_key_driver_t);
}
module_init(power_key_driver_init);

static void __exit power_key_driver_exit(void)
{
	platform_driver_unregister(&power_key_driver_t);
}
module_exit(power_key_driver_exit);

MODULE_ALIAS("platform:power and home key driver");
MODULE_DESCRIPTION("board aalto power and home key");
MODULE_LICENSE("GPL");
