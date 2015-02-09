/*
 * Copyright (C) 2010 MEMSIC, Inc.
 *
 * Initial Code:
 *	Robbie Cao
 * 	Dale Hou
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <asm/uaccess.h>

#include <linux/mmc328x.h>
#include <plat/i2c-omap-gpio.h>
#include <linux/regulator/consumer.h>

#define DEBUG			0
#define MAX_FAILURE_COUNT	3
#define READMD			1

#define MMC328X_DELAY_TM	10	/* ms */
#define MMC328X_DELAY_RM	10	/* ms */
#define MMC328X_DELAY_STDN	1	/* ms */
#define MMC328X_DELAY_RRM       1       /* ms */

#define MMC328X_RETRY_COUNT	3
#define MMC328X_RRM_INTV	100

#define MMC328X_DEV_NAME	"mmc328x"
#define POWER_ON 1
#define POWER_OFF 0
#define SUSPEND 1
#define RESUME 0

extern int get_hw_revision(void);
static struct regulator *vreg_sensor = NULL;

static u32 read_idx = 0;
static atomic_t	suspend_flag;
#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
static struct early_suspend early_suspend;
static int mmc328x_magnetic_suspend(struct early_suspend *handler);
static int mmc328x_magnetic_resume(struct early_suspend *handler);
#endif

//gpio i2c emulation.
static OMAP_GPIO_I2C_CLIENT * g_client;

static int mmc328x_i2c_rx_data(char *buf, int len)
{
	u8 reg;
	OMAP_GPIO_I2C_RD_DATA i2c_rd_param;
	reg = *buf;
	i2c_rd_param.reg_addr = &reg;
	i2c_rd_param.reg_len = 1;
	i2c_rd_param.rdata_len = len;
	i2c_rd_param.rdata = buf;
	return omap_gpio_i2c_read(g_client, &i2c_rd_param);
}

static int mmc328x_i2c_tx_data(char *buf, int len)
{
	u8 reg;
	OMAP_GPIO_I2C_WR_DATA i2c_wr_param;
	reg = *buf;
	i2c_wr_param.reg_addr = &reg;
	i2c_wr_param.reg_len = 1;
	i2c_wr_param.wdata_len = len-1;
	i2c_wr_param.wdata = buf+1;
	return omap_gpio_i2c_write(g_client, &i2c_wr_param);
}

static int mmc328x_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static int mmc328x_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int mmc328x_ioctl(struct inode *inode, struct file *file, 
	unsigned int cmd, unsigned long arg)
{
	void __user *pa = (void __user *)arg;
	unsigned char data[16] = {0};
	int vec[3] = {0};
	int MD_times = 0;
	short flag;

	switch (cmd) {
		case MMC328X_IOC_TM:
			data[0] = MMC328X_REG_CTRL;
			data[1] = MMC328X_CTRL_TM;
			if (mmc328x_i2c_tx_data(data, 2) < 0) {
				return -EFAULT;
			}
			/* wait TM done for coming data read */
			msleep(MMC328X_DELAY_TM);
			break;
		case MMC328X_IOC_RM:
			data[0] = MMC328X_REG_CTRL;
			data[1] = MMC328X_CTRL_RM;
			if (mmc328x_i2c_tx_data(data, 2) < 0) {
				return -EFAULT;
			}
			/* wait external capacitor charging done for next SET*/
			msleep(MMC328X_DELAY_RM);
			break;
		case MMC328X_IOC_RRM:
			data[0] = MMC328X_REG_CTRL;
			data[1] = MMC328X_CTRL_RRM;
			if (mmc328x_i2c_tx_data(data, 2) < 0) {
				return -EFAULT;
			}
			/* wait external capacitor charging done for next RRM */
			msleep(MMC328X_DELAY_RM);
			break;
		case MMC328X_IOC_READ:
			data[0] = MMC328X_REG_DATA;
			if (mmc328x_i2c_rx_data(data, 6) < 0) {
				return -EFAULT;
			}
			vec[0] = data[1] << 8 | data[0];
			vec[1] = data[3] << 8 | data[2];
			vec[2] = 8192 - (data[5] << 8 | data[4]);
			//vec[2] = data[5] << 8 | data[4];
#ifdef MMC328X_SENSOR_DEBUG
			printk("[X - %04x] [Y - %04x] [Z - %04x]\n", 
					vec[0], vec[1], vec[2]);
#endif
			if (copy_to_user(pa, vec, sizeof(vec))) {
				return -EFAULT;
			}
			break;
		case MMC328X_IOC_READXYZ:
			/* do RM every MMC328X_RRM_INTV times read */
			if (!(read_idx % MMC328X_RRM_INTV)) {
#ifdef CONFIG_SENSORS_MMC328X 
				data[0] = MMC328X_REG_CTRL;
				data[1] = MMC328X_CTRL_RRM;
				mmc328x_i2c_tx_data(data, 2);
				msleep(MMC328X_DELAY_RRM);
#endif
				/* RM */
				data[0] = MMC328X_REG_CTRL;
				data[1] = MMC328X_CTRL_RM;
				/* not check return value here, assume it always OK */
				mmc328x_i2c_tx_data(data, 2);
				/* wait external capacitor charging done for next RM */
				msleep(MMC328X_DELAY_RM);
			}
			/* send TM cmd before read */
			data[0] = MMC328X_REG_CTRL;
			data[1] = MMC328X_CTRL_TM;
			/* not check return value here, assume it always OK */
			mmc328x_i2c_tx_data(data, 2);
			/* wait TM done for coming data read */
			msleep(MMC328X_DELAY_TM);
#if READMD
			/* Read MD */
			data[0] = MMC328X_REG_DS;
			if (mmc328x_i2c_rx_data(data, 1) < 0) {
				return -EFAULT;
			}
			while (!(data[0] & 0x01)) {
				msleep(1);
				/* Read MD again*/
				data[0] = MMC328X_REG_DS;
				if (mmc328x_i2c_rx_data(data, 1) < 0) {
					return -EFAULT;
				}

				if (data[0] & 0x01) break;
				MD_times++;
				if (MD_times > 2) {
#ifdef MMC328X_SENSOR_DEBUG
					printk("TM not work!!");
#endif
					return -EFAULT;
				}
			}
#endif		
			/* read xyz raw data */
			read_idx++;
			data[0] = MMC328X_REG_DATA;
			if (mmc328x_i2c_rx_data(data, 6) < 0) {
				return -EFAULT;
			}
			vec[0] = data[1] << 8 | data[0];
			vec[1] = data[3] << 8 | data[2];
			vec[2] = 8192 - (data[5] << 8 | data[4]);
			//vec[2] = data[5] << 8 | data[4];
#ifdef MMC328X_SENSOR_DEBUG
			printk("[X - %04x] [Y - %04x] [Z - %04x]\n", 
					vec[0], vec[1], vec[2]);
#endif
			if (copy_to_user(pa, vec, sizeof(vec))) {
				return -EFAULT;
			}

			break;
		case MMC328X_IOC_READSUSPEND:
			flag = atomic_read(&suspend_flag);
			if (copy_to_user(pa, &flag, sizeof(flag)))
				return -EFAULT;
			break;
		default:
			break;
	}

	return 0;
}

static ssize_t mmc328x_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "MMC328X");
	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t mmc328x_readxyz(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count;
	int vec[3] = {0};
	unsigned char data[16] = {0};
	int MD_times = 0;
	/* do RM every MMC328X_RRM_INTV times read */
	if (!(read_idx % MMC328X_RRM_INTV)) {
#ifdef CONFIG_SENSORS_MMC328X 
		data[0] = MMC328X_REG_CTRL;
		data[1] = MMC328X_CTRL_RRM;
		mmc328x_i2c_tx_data(data, 2);
		msleep(MMC328X_DELAY_RRM);
#endif
		/* RM */
		data[0] = MMC328X_REG_CTRL;
		data[1] = MMC328X_CTRL_RM;
		/* not check return value here, assume it always OK */
		mmc328x_i2c_tx_data(data, 2);
		/* wait external capacitor charging done for next RM */
		msleep(MMC328X_DELAY_RM);
	}
	/* send TM cmd before read */
	data[0] = MMC328X_REG_CTRL;
	data[1] = MMC328X_CTRL_TM;
	/* not check return value here, assume it always OK */
	mmc328x_i2c_tx_data(data, 2);
	/* wait TM done for coming data read */
	msleep(MMC328X_DELAY_TM);
#if READMD
	/* Read MD */
	data[0] = MMC328X_REG_DS;
	if (mmc328x_i2c_rx_data(data, 1) < 0) {
		sprintf(buf,"%d,%d,%d\n", -1, -1, -1 );
		return -EFAULT;
	}
	while (!(data[0] & 0x01)) {
		msleep(1);
		/* Read MD again*/
		data[0] = MMC328X_REG_DS;
		if (mmc328x_i2c_rx_data(data, 1) < 0) {
			sprintf(buf,"%d,%d,%d\n", -1, -1, -1 );
			return -EFAULT;
		}

		if (data[0] & 0x01) break;
		MD_times++;
		if (MD_times > 2) {
#ifdef MMC328X_SENSOR_DEBUG
			printk("TM not work!!");
#endif
			sprintf(buf,"%d,%d,%d\n", -1, -1, -1 );
			return -EFAULT;
		}
	}
#endif		
	/* read xyz raw data */
	read_idx++;
	data[0] = MMC328X_REG_DATA;
	if (mmc328x_i2c_rx_data(data, 6) < 0) {
		sprintf(buf,"%d,%d,%d\n", -1, -1, -1 );
		return -EFAULT;
	}
	vec[0] = data[1] << 8 | data[0];
	vec[1] = data[3] << 8 | data[2];
	vec[2] = data[5] << 8 | data[4];
#ifdef MMC328X_SENSOR_DEBUG
	printk("[X - %04x] [Y - %04x] [Z - %04x]\n", 
			vec[0], vec[1], vec[2]);
#endif

	count = sprintf(buf,"%d,%d,%d\n", vec[0], vec[1], vec[2] );
	return count;
}

static ssize_t mmc328x_poweron(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count;
	unsigned char data[16] = {0};

	data[0] = MMC328X_REG_CTRL;
	data[1] = MMC328X_CTRL_TM;
	if (mmc328x_i2c_tx_data(data, 2) < 0) {
		count = sprintf(buf,"%d\n", POWER_OFF);
		return -EFAULT;
	}
	/* wait TM done for coming data read */
	msleep(MMC328X_DELAY_TM);

	count = sprintf(buf,"%d\n", POWER_ON);

	return count;
}

static DEVICE_ATTR(magnetic_readxyz, S_IRUGO, mmc328x_readxyz, NULL);
static DEVICE_ATTR(magnetic_power, S_IRUGO, mmc328x_poweron, NULL);

static struct attribute *mmc328x_mag_attributes[] = {
    &dev_attr_magnetic_readxyz.attr,
    &dev_attr_magnetic_power.attr,
    NULL
};

static const struct attribute_group mmc328x_group = {
	.attrs = mmc328x_mag_attributes,
};

static DEVICE_ATTR(mmc328x, S_IRUGO, mmc328x_show, NULL);

static struct file_operations mmc328x_fops = {
	.owner		= THIS_MODULE,
	.open		= mmc328x_open,
	.release	= mmc328x_release,
	.ioctl		= mmc328x_ioctl,
};

static struct miscdevice mmc328x_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MMC328X_DEV_NAME,
	.fops = &mmc328x_fops,
};

static int mmc328x_init_probe(struct platform_device *pdev)
{
	unsigned char data[16] = {0};
	int res = 0;
	int ret33 = 0;

	if (get_hw_revision() == 0x09) {
		printk("[SENSOR] hw_revision : 0x09\n ");
		vreg_sensor = regulator_get(NULL, "vmmc2");
		ret33 = regulator_enable(vreg_sensor);

		if (ret33) {
			printk("[SENSOR] Error, %s: vreg enable failed (%d)\n", __func__, ret33);
		}

	}

	pr_info("mmc328x driver: probe\n");
	g_client = omap_gpio_i2c_init(OMAP_GPIO_FM_SDA	, OMAP_GPIO_FM_SCL, MMC328X_I2C_ADDR, 200);

	res = misc_register(&mmc328x_device);
	if (res) {
		pr_err("%s: mmc328x_device register failed\n", __FUNCTION__);
		goto out;
	}

	res = device_create_file(mmc328x_device.this_device, &dev_attr_mmc328x);
	if (res) {
		pr_err("%s: device_create_file failed\n", __FUNCTION__);
		goto out_deregister;
	}

	res = sysfs_create_group(&mmc328x_device.this_device->kobj,&mmc328x_group);
	if (res < 0){
		pr_info("failed to create sysfs files\n");
		goto out_sysinfo;
	}

	/* send SET/RESET cmd to mag sensor first of all */
#ifdef CONFIG_SENSORS_MMC328X 
	data[0] = MMC328X_REG_CTRL;
	data[1] = MMC328X_CTRL_RRM;
	if (mmc328x_i2c_tx_data(data, 2) < 0) {
	}
	msleep(MMC328X_DELAY_RRM);
	data[0] = MMC328X_REG_CTRL;
	data[1] = MMC328X_CTRL_TM;
	if (mmc328x_i2c_tx_data(data, 2) < 0) {
	}
	msleep(5*MMC328X_DELAY_TM);
#endif

	data[0] = MMC328X_REG_CTRL;
	data[1] = MMC328X_CTRL_RM;
	if (mmc328x_i2c_tx_data(data, 2) < 0) {
		/* assume RM always success */
	}
#ifndef CONFIG_SENSORS_MMC328X 
	/* wait external capacitor charging done for next RM */
	msleep(MMC328X_DELAY_RM);
#else
	msleep(10*MMC328X_DELAY_RM);
	data[0] = MMC328X_REG_CTRL;
	data[1] = MMC328X_CTRL_TM;
	if (mmc328x_i2c_tx_data(data, 2) < 0) {
	}
#endif

#if defined(CONFIG_HAS_EARLYSUSPEND)
	early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	early_suspend.suspend = (void *)mmc328x_magnetic_suspend;
	early_suspend.resume  = (void *)mmc328x_magnetic_resume;
	register_early_suspend(&early_suspend);
#endif

	return 0;
out_sysinfo:
	sysfs_remove_group(&mmc328x_device.this_device->kobj,&mmc328x_group);
out_deregister:
	misc_deregister(&mmc328x_device);
out:
	return res;
}

static int mmc328x_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&mmc328x_device.this_device->kobj,&mmc328x_group);
	device_remove_file(mmc328x_device.this_device, &dev_attr_mmc328x);
	misc_deregister(&mmc328x_device);
	kfree(g_client);	

#if defined(CONFIG_HAS_EARLYSUSPEND)
	unregister_early_suspend(&early_suspend);
#endif
	return 0;
}


#if defined(CONFIG_HAS_EARLYSUSPEND)
static int mmc328x_magnetic_suspend(struct early_suspend *handler)
{
	atomic_set(&suspend_flag,SUSPEND);
#ifdef MMC328X_SENSOR_DEBUG
	printk("mmc328x suspend mode :%d \n",suspend_flag.counter);
#endif
#if defined(CONFIG_FMRADIO_USE_GPIO_I2C)
	if(1 != gpio_get_value(OMAP_GPIO_FM_nRST))
#endif
		omap_gpio_i2c_deinit(g_client);
	return 0;
}

static int mmc328x_magnetic_resume(struct early_suspend *handler)
{
	unsigned char data[16] = {0};

	atomic_set(&suspend_flag,RESUME);
#ifdef MMC328X_SENSOR_DEBUG
	printk("mmc328x resume mode :%d \n",suspend_flag.counter);
#endif
	if (g_client){
		kfree(g_client);
	}
	g_client = omap_gpio_i2c_init(OMAP_GPIO_FM_SDA	, OMAP_GPIO_FM_SCL, MMC328X_I2C_ADDR, 200);
	msleep(MMC328X_DELAY_RRM);
	/* send SET/RESET cmd to mag sensor first of all */
#ifdef CONFIG_SENSORS_MMC328X 
	data[0] = MMC328X_REG_CTRL;
	data[1] = MMC328X_CTRL_RRM;
	if (mmc328x_i2c_tx_data(data, 2) < 0) {
	}
	msleep(MMC328X_DELAY_RRM);
	data[0] = MMC328X_REG_CTRL;
	data[1] = MMC328X_CTRL_TM;
	if (mmc328x_i2c_tx_data(data, 2) < 0) {
	}
	msleep(5*MMC328X_DELAY_TM);
#endif

	data[0] = MMC328X_REG_CTRL;
	data[1] = MMC328X_CTRL_RM;
	if (mmc328x_i2c_tx_data(data, 2) < 0) {
		/* assume RM always success */
	}
#ifndef CONFIG_SENSORS_MMC328X 
	/* wait external capacitor charging done for next RM */
	msleep(MMC328X_DELAY_RM);
#else
	msleep(10*MMC328X_DELAY_RM);
	data[0] = MMC328X_REG_CTRL;
	data[1] = MMC328X_CTRL_TM;
	if (mmc328x_i2c_tx_data(data, 2) < 0) {
	}
#endif
	return 0;
}
#endif

static struct platform_device *mmc328x_magnetic_device;
static struct platform_driver mmc328x_magnetic_driver = {
	.probe 	 = mmc328x_init_probe,
	.remove	 = mmc328x_remove,
	.driver  = {
		.name = "mmc328x-magnetic", 
		.owner	= THIS_MODULE,
		//.pm = &mmc328x_pm_ops,
	}
};



static int __init mmc328x_init(void)
{
	int result;
	result = platform_driver_register( &mmc328x_magnetic_driver );
	pr_info("mmc328x driver: init : %d\n",result);

	atomic_set(&suspend_flag, 0);
	pr_info("mmc328x suspend mode : %d\n",suspend_flag.counter);
	if( result )
	{
		return result;
	}

	mmc328x_magnetic_device  = platform_device_register_simple( "mmc328x-magnetic", -1, NULL, 0 );
	if( IS_ERR( mmc328x_magnetic_device ) )
	{
		return PTR_ERR( mmc328x_magnetic_device );
	}

	return 0;
}

static void __exit mmc328x_exit(void)
{
	pr_info("mmc328x driver: exit\n");
	platform_device_unregister( mmc328x_magnetic_device );
	platform_driver_unregister( &mmc328x_magnetic_driver );
}

module_init(mmc328x_init);
module_exit(mmc328x_exit);

MODULE_AUTHOR("Dale Hou<byhou@memsic.com>");
MODULE_DESCRIPTION("MEMSIC MMC328X Magnetic Sensor Driver");
MODULE_LICENSE("GPL");

