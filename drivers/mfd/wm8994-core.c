/*
 * wm1811-core.c  --  Device access for Wolfson WM1811
 *
 * Copyright 2009 Wolfson Microelectronics PLC.
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mfd/core.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>

#include <linux/mfd/wm1811/core.h>
#include <linux/mfd/wm1811/pdata.h>
#include <linux/mfd/wm1811/registers.h>

#include <sound/soc.h>

static int wm1811_read(struct wm1811 *wm1811, unsigned short reg,
		       int bytes, void *dest)
{
	int ret, i;
	u16 *buf = dest;

	BUG_ON(bytes % 2);
	BUG_ON(bytes <= 0);

	ret = wm1811->read_dev(wm1811, reg, bytes, dest);
	if (ret < 0)
		return ret;

	for (i = 0; i < bytes / 2; i++) {
		buf[i] = be16_to_cpu(buf[i]);

		dev_vdbg(wm1811->dev, "Read %04x from R%d(0x%x)\n",
			 buf[i], reg + i, reg + i);
	}

	return 0;
}

/**
 * wm1811_reg_read: Read a single WM1811 register.
 *
 * @wm1811: Device to read from.
 * @reg: Register to read.
 */
int wm1811_reg_read(struct wm1811 *wm1811, unsigned short reg)
{
	unsigned short val;
	int ret;

	mutex_lock(&wm1811->io_lock);

	ret = wm1811_read(wm1811, reg, 2, &val);

	mutex_unlock(&wm1811->io_lock);

	if (ret < 0)
		return ret;
	else
		return val;
}
EXPORT_SYMBOL_GPL(wm1811_reg_read);

/**
 * wm1811_bulk_read: Read multiple WM1811 registers
 *
 * @wm1811: Device to read from
 * @reg: First register
 * @count: Number of registers
 * @buf: Buffer to fill.
 */
int wm1811_bulk_read(struct wm1811 *wm1811, unsigned short reg,
		     int count, u16 *buf)
{
	int ret;

	mutex_lock(&wm1811->io_lock);

	ret = wm1811_read(wm1811, reg, count * 2, buf);

	mutex_unlock(&wm1811->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(wm1811_bulk_read);

static int wm1811_write(struct wm1811 *wm1811, unsigned short reg,
			int bytes, void *src)
{
	u16 *buf = src;
	int i;

	BUG_ON(bytes % 2);
	BUG_ON(bytes <= 0);

	for (i = 0; i < bytes / 2; i++) {
		dev_vdbg(wm1811->dev, "Write %04x to R%d(0x%x)\n",
			 buf[i], reg + i, reg + i);

		buf[i] = cpu_to_be16(buf[i]);
	}

	return wm1811->write_dev(wm1811, reg, bytes, src);
}

/**
 * wm1811_reg_write: Write a single WM1811 register.
 *
 * @wm1811: Device to write to.
 * @reg: Register to write to.
 * @val: Value to write.
 */

int wm1811_reg_write(struct wm1811 *wm1811, unsigned short reg,
		     unsigned short val)
{
	int ret;

	mutex_lock(&wm1811->io_lock);

	ret = wm1811_write(wm1811, reg, 2, &val);

	mutex_unlock(&wm1811->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(wm1811_reg_write);

/**
 * wm1811_set_bits: Set the value of a bitfield in a WM1811 register
 *
 * @wm1811: Device to write to.
 * @reg: Register to write to.
 * @mask: Mask of bits to set.
 * @val: Value to set (unshifted)
 */
int wm1811_set_bits(struct wm1811 *wm1811, unsigned short reg,
		    unsigned short mask, unsigned short val)
{
	int ret;
	u16 r;

	mutex_lock(&wm1811->io_lock);

	ret = wm1811_read(wm1811, reg, 2, &r);
	if (ret < 0)
		goto out;

	r &= ~mask;
	r |= val;

	ret = wm1811_write(wm1811, reg, 2, &r);

out:
	mutex_unlock(&wm1811->io_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(wm1811_set_bits);

static struct mfd_cell wm1811_regulator_devs[] = {
	{ .name = "wm8994-ldo", .id = 1 },
	{ .name = "wm8994-ldo", .id = 2 },
};

static struct resource wm1811_codec_resources[] = {
	{
		.start = WM1811_IRQ_TEMP_SHUT,
		.end   = WM1811_IRQ_TEMP_WARN,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource wm1811_gpio_resources[] = {
	{
		.start = WM1811_IRQ_GPIO(1),
		.end   = WM1811_IRQ_GPIO(11),
		.flags = IORESOURCE_IRQ,
	},
};

static struct mfd_cell wm1811_devs[] = {
	{
		.name = "wm1811-codec",
		.num_resources = ARRAY_SIZE(wm1811_codec_resources),
		.resources = wm1811_codec_resources,
	},

	{
		.name = "wm8994-gpio",
		.num_resources = ARRAY_SIZE(wm1811_gpio_resources),
		.resources = wm1811_gpio_resources,
	},
};

/*
 * Supplies for the main bulk of CODEC; the LDO supplies are ignored
 * and should be handled via the standard regulator API supply
 * management.
 */
static const char *wm1811_main_supplies[] = {
	"DBVDD",
	"DCVDD",
	"AVDD1",
	"AVDD2",
	"CPVDD",
	"SPKVDD1",
	"SPKVDD2",
};

#ifdef CONFIG_PM
static int wm1811_device_suspend(struct device *dev)
{
	struct wm1811 *wm1811 = dev_get_drvdata(dev);
	int ret;

	/* GPIO configuration state is saved here since we may be configuring
	 * the GPIO alternate functions even if we're not using the gpiolib
	 * driver for them.
	 */

#if 0
	ret = wm1811_read(wm1811, WM1811_GPIO_1, WM1811_NUM_GPIO_REGS * 2,
			  &wm1811->gpio_regs);
	if (ret < 0)
		dev_err(dev, "Failed to save GPIO registers: %d\n", ret);

	/* For similar reasons we also stash the regulator states */
	ret = wm1811_read(wm1811, WM1811_LDO_1, WM1811_NUM_LDO_REGS * 2,
			  &wm1811->ldo_regs);
	if (ret < 0)
		dev_err(dev, "Failed to save LDO registers: %d\n", ret);
#endif

#if 0
	ret = regulator_bulk_disable(ARRAY_SIZE(wm1811_main_supplies),
				     wm1811->supplies);
	if (ret != 0) {
		dev_err(dev, "Failed to disable supplies: %d\n", ret);
		return ret;
	}
#endif

	return 0;
}

static int wm1811_device_resume(struct device *dev)
{
	struct wm1811 *wm1811 = dev_get_drvdata(dev);
	int ret;

#if 0
	ret = regulator_bulk_enable(ARRAY_SIZE(wm1811_main_supplies),
				    wm1811->supplies);
	if (ret != 0) {
		dev_err(dev, "Failed to enable supplies: %d\n", ret);
		return ret;
	}
#endif

#if 0
	ret = wm1811_write(wm1811, WM1811_INTERRUPT_STATUS_1_MASK,
			   WM1811_NUM_IRQ_REGS * 2, &wm1811->irq_masks_cur);
	if (ret < 0)
		dev_err(dev, "Failed to restore interrupt masks: %d\n", ret);

	ret = wm1811_write(wm1811, WM1811_LDO_1, WM1811_NUM_LDO_REGS * 2,
			   &wm1811->ldo_regs);
	if (ret < 0)
		dev_err(dev, "Failed to restore LDO registers: %d\n", ret);

	ret = wm1811_write(wm1811, WM1811_GPIO_1, WM1811_NUM_GPIO_REGS * 2,
			   &wm1811->gpio_regs);
	if (ret < 0)
		dev_err(dev, "Failed to restore GPIO registers: %d\n", ret);
#endif

	return 0;
}
#endif

#ifdef CONFIG_REGULATOR
static int wm1811_ldo_in_use(struct wm1811_pdata *pdata, int ldo)
{
	struct wm1811_ldo_pdata *ldo_pdata;

	if (!pdata)
		return 0;

	ldo_pdata = &pdata->ldo[ldo];

	if (!ldo_pdata->init_data)
		return 0;

	return ldo_pdata->init_data->num_consumer_supplies != 0;
}
#else
static int wm1811_ldo_in_use(struct wm1811_pdata *pdata, int ldo)
{
	return 0;
}
#endif

/*
 * Instantiate the generic non-control parts of the device.
 */
static int wm1811_device_init(struct wm1811 *wm1811, unsigned long id, int irq)
{
	struct wm1811_pdata *pdata = wm1811->dev->platform_data;
	int ret, i;

	printk ("wm1811_device_init\n");

	mutex_init(&wm1811->io_lock);
	// dev_set_drvdata(wm1811->dev, wm1811);

	/* Add the on-chip regulators first for bootstrapping */
	ret = mfd_add_devices(wm1811->dev, -1,
			      wm1811_regulator_devs,
			      ARRAY_SIZE(wm1811_regulator_devs),
			      NULL, 0);
	if (ret != 0) {
		dev_err(wm1811->dev, "Failed to add children: %d\n", ret);
		goto err;
	}

	wm1811->supplies = kzalloc(sizeof(struct regulator_bulk_data) *
				   ARRAY_SIZE(wm1811_main_supplies),
				   GFP_KERNEL);
	if (!wm1811->supplies)
		goto err;

	for (i = 0; i < ARRAY_SIZE(wm1811_main_supplies); i++)
		wm1811->supplies[i].supply = wm1811_main_supplies[i];

#if 0
	ret = regulator_bulk_get(wm1811->dev, ARRAY_SIZE(wm1811_main_supplies),
				 wm1811->supplies);
	if (ret != 0) {
		dev_err(wm1811->dev, "Failed to get supplies: %d\n", ret);
		goto err_supplies;
	}

	ret = regulator_bulk_enable(ARRAY_SIZE(wm1811_main_supplies),
				    wm1811->supplies);
	if (ret != 0) {
		dev_err(wm1811->dev, "Failed to enable supplies: %d\n", ret);
		goto err_get;
	}
#endif

	ret = wm1811_reg_read(wm1811, WM1811_SOFTWARE_RESET);
	printk ("WM1811_SOFTWARE_RESET=%x\n", ret);
	if (ret < 0) {
		dev_err(wm1811->dev, "Failed to read ID register\n");
		goto err_enable;
	}
	switch (ret) {
	case 0x1811:
	case 0x8958:
	case 0x8994:
		break;
	default:
		dev_err(wm1811->dev, "Device is not a WM1811, ID is %x\n",
			ret);
		ret = -EINVAL;
		goto err_enable;
	}

	ret = wm1811_reg_read(wm1811, WM1811_CHIP_REVISION);
	printk ("WM1811_CHIP_REVISION=%x\n", ret);
	if (ret < 0) {
		dev_err(wm1811->dev, "Failed to read revision register: %d\n",
			ret);
		goto err_enable;
	}

	switch (ret) {
	case 0:
	case 1:
		dev_warn(wm1811->dev, "revision %c not fully supported\n",
			'A' + ret);
		break;
	default:
		dev_info(wm1811->dev, "revision %c\n", 'A' + ret);
		break;
	}


	if (pdata) {
		wm1811->irq_base = pdata->irq_base;
		wm1811->gpio_base = pdata->gpio_base;

		/* GPIO configuration is only applied if it's non-zero */
		for (i = 0; i < ARRAY_SIZE(pdata->gpio_defaults); i++) {
			if (pdata->gpio_defaults[i]) {
				wm1811_set_bits(wm1811, WM1811_GPIO_1 + i,
						0xffff,
						pdata->gpio_defaults[i]);
			}
		}
	}

	/* In some system designs where the regulators are not in use,
	 * we can achieve a small reduction in leakage currents by
	 * floating LDO outputs.  This bit makes no difference if the
	 * LDOs are enabled, it only affects cases where the LDOs were
	 * in operation and are then disabled.
	 */
	for (i = 0; i < WM1811_NUM_LDO_REGS; i++) {
		if (wm1811_ldo_in_use(pdata, i))
			wm1811_set_bits(wm1811, WM1811_LDO_1 + i,
					WM1811_LDO1_DISCH, WM1811_LDO1_DISCH);
		else
			wm1811_set_bits(wm1811, WM1811_LDO_1 + i,
					WM1811_LDO1_DISCH, 0);
	}

	wm1811_irq_init(wm1811);

	ret = mfd_add_devices(wm1811->dev, -1,
			      wm1811_devs, ARRAY_SIZE(wm1811_devs),
			      NULL, 0);
	if (ret != 0) {
		dev_err(wm1811->dev, "Failed to add children: %d\n", ret);
		goto err_irq;
	}

	return 0;

err_irq:
	wm1811_irq_exit(wm1811);
err_enable:
#if 0
	regulator_bulk_disable(ARRAY_SIZE(wm1811_main_supplies),
			       wm1811->supplies);
err_get:
	regulator_bulk_free(ARRAY_SIZE(wm1811_main_supplies), wm1811->supplies);
err_supplies:
#endif
	kfree(wm1811->supplies);
err:
	mfd_remove_devices(wm1811->dev);
	kfree(wm1811);
	return ret;
}

static void wm1811_device_exit(struct wm1811 *wm1811)
{
	mfd_remove_devices(wm1811->dev);
	wm1811_irq_exit(wm1811);
	regulator_bulk_disable(ARRAY_SIZE(wm1811_main_supplies),
			       wm1811->supplies);
	regulator_bulk_free(ARRAY_SIZE(wm1811_main_supplies), wm1811->supplies);
	kfree(wm1811->supplies);
	kfree(wm1811);
}

static int wm1811_i2c_read_device(struct wm1811 *wm1811, unsigned short reg,
				  int bytes, void *dest)
{
	struct i2c_client *i2c = wm1811->control_data;
	int ret;
	u16 r = cpu_to_be16(reg);

	ret = i2c_master_send(i2c, (unsigned char *)&r, 2);
	if (ret < 0)
		return ret;
	if (ret != 2)
		return -EIO;

	ret = i2c_master_recv(i2c, dest, bytes);
	if (ret < 0)
		return ret;
	if (ret != bytes)
		return -EIO;
	return 0;
}

/* Currently we allocate the write buffer on the stack; this is OK for
 * small writes - if we need to do large writes this will need to be
 * revised.
 */
static int wm1811_i2c_write_device(struct wm1811 *wm1811, unsigned short reg,
				   int bytes, void *src)
{
	struct i2c_client *i2c = wm1811->control_data;
	unsigned char msg[bytes + 2];
	int ret;

	reg = cpu_to_be16(reg);
	memcpy(&msg[0], &reg, 2);
	memcpy(&msg[2], src, bytes);

	ret = i2c_master_send(i2c, msg, bytes + 2);
	if (ret < 0)
		return ret;
	if (ret < bytes + 2)
		return -EIO;

	return 0;
}

static int wm1811_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	printk ("wm1811_i2c_probe\n");
	
	struct wm1811 *wm1811;

	wm1811 = kzalloc(sizeof(struct wm1811), GFP_KERNEL);
	
	if (wm1811 == NULL) {
		kfree(i2c);
		return -ENOMEM;
	}

	i2c_set_clientdata(i2c, wm1811);
	wm1811->dev = &i2c->dev;
	wm1811->control_data = i2c;
	wm1811->read_dev = wm1811_i2c_read_device;
	wm1811->write_dev = wm1811_i2c_write_device;
	wm1811->irq = i2c->irq;

	return wm1811_device_init(wm1811, id->driver_data, i2c->irq);
}

static int wm1811_i2c_remove(struct i2c_client *i2c)
{
	struct wm1811 *wm1811 = i2c_get_clientdata(i2c);

	wm1811_device_exit(wm1811);

	return 0;
}

#ifdef CONFIG_PM
static int wm1811_i2c_suspend(struct i2c_client *i2c, pm_message_t state)
{
	return wm1811_device_suspend(&i2c->dev);
}

static int wm1811_i2c_resume(struct i2c_client *i2c)
{
	return wm1811_device_resume(&i2c->dev);
}
#else
#define wm1811_i2c_suspend NULL
#define wm1811_i2c_resume NULL
#endif

static const struct i2c_device_id wm1811_i2c_id[] = {
	{ "wm8994", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, wm1811_i2c_id);

static struct i2c_driver wm1811_i2c_driver = {
	.driver = {
		   .name = "wm8994",
		   .owner = THIS_MODULE,
	},
	.probe = wm1811_i2c_probe,
	.remove = wm1811_i2c_remove,
	.suspend = wm1811_i2c_suspend,
	.resume = wm1811_i2c_resume,
	.id_table = wm1811_i2c_id,
};

static int __init wm1811_i2c_init(void)
{
	int ret;
	
	printk ("wm1811_i2c_init");

	ret = i2c_add_driver(&wm1811_i2c_driver);

	if (ret != 0)
		pr_err("Failed to register wm1811 I2C driver: %d\n", ret);

	return ret;
}
module_init(wm1811_i2c_init);

static void __exit wm1811_i2c_exit(void)
{
	printk ("wm1811_i2c_exit");
	
	i2c_del_driver(&wm1811_i2c_driver);
}
module_exit(wm1811_i2c_exit);

MODULE_DESCRIPTION("Core support for the WM1811 audio CODEC");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Brown <broonie@opensource.wolfsonmicro.com>");
