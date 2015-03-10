/* drivers/input/touchscreen/melfas_ts.c
 *
 * Copyright (C) 2010 Melfas, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#if defined (CONFIG_MSM8225_COOPERPLUS_BOARD)
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <linux/input/melfas_ts.h>
#else
#include <plat/gpio.h>
#include <plat/mux.h>
#include <linux/regulator/consumer.h>
#include <linux/input/melfas_ts.h>
#endif

#if defined (CONFIG_MSM8225_COOPERPLUS_BOARD)
#define TSP_LDO_ON 133 
#endif

#define TS_MAX_X_COORD			320
#define TS_MAX_Y_COORD			480
#define TS_MAX_Z_TOUCH			255
#define TS_MAX_W_TOUCH		30
#define MTSI_V07				1
#define DEBUG_PRINT 		    0	
#define SET_DOWNLOAD 			1
#define LOW_LEVEL_DETECT		1

#define TS_READ_START_ADDR 		0x10
#define TS_READ_HW_VERSION_ADDR	0x30
#define TS_READ_FW_VERSION_ADDR	0x31
#define FW_VERSION					0x07
#define HW_VERSION					0x00

#if DEBUG_PRINT
#define mprintk(fmt, x... ) printk( "%s(%d): " fmt, __FUNCTION__ ,__LINE__, ## x)
#else
#define mprintk(x...) do { } while (0)
#endif

#if MTSI_V07
#define TS_READ_REGS_LEN 6
#else
#define TS_READ_REGS_LEN 5
#endif
#define I2C_RETRY_CNT	10
#define PRESS_KEY	1
#define RELEASE_KEY	0
#define MMS136_MAX_TOUCH	5
#define MELFAS_MAX_TOUCH	5

#define MELFAS_WDOG_ENABLE	// Aalto Wdog Code

#if SET_DOWNLOAD
#include <linux/input/melfas_download.h>
#endif 


enum
{
	None = 0, 
	TOUCH_SCREEN, 
	TOUCH_KEY
};

struct muti_touch_info
{
#if MTSI_V07
	int width;
#endif
	int strength;
	int posX;
	int posY;
};

struct melfas_ts_data
{
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct melfas_tsi_platform_data *pdata;
	uint32_t flags;
	int (*power)(int on);
	void (*power_enable)(int en);
	struct early_suspend early_suspend;
	uint8_t tsp_hw_version; /* Current TSP versions */
	uint8_t tsp_fw_version;
};


static int Firm_ver = -1;
static int FW_Major_ver = -1;
static int FW_Minor_ver = -1;

static int HW_ver = -1;
static int SW_ver = -1;

static int firmware_ret_val = -1;

static struct melfas_ts_data *ts_global;
static int init_complete = 0;
static int tsp_state = 0;

static bool empty_chip = false;

int melfas_ts_read_ver(void);
int firm_update( void );

extern int board_hw_revision;

/* sys fs */
struct class *touch_class;
EXPORT_SYMBOL(touch_class);
struct device *firmware_dev;
EXPORT_SYMBOL(firmware_dev);


static ssize_t firmware_phone_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_phone_store( struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static ssize_t firmware_part_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t firmware_part_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR(firmware_phone, S_IRUGO | S_IWUSR, firmware_phone_show, firmware_phone_store);
static DEVICE_ATTR(firmware_part, S_IRUGO | S_IWUSR, firmware_part_show, firmware_part_store);


#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif

//#define CONFIG_TOUCHSCREEN_MELFAS_KEYLED 1
// #define CONFIG_TOUCHSCREEN_MELFAS_KEYLED 0
#if defined (CONFIG_TOUCHSCREEN_MELFAS_KEYLED)
static int melfas_keyled_init(void);
static int melfas_keyled_enable(void);
static int melfas_keyled_disable(void);
#endif

static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];

static struct regulator *vmmc2 = NULL;
#ifdef MELFAS_WDOG_ENABLE
static int melfas_set_vdd(int enable);
static void melfas_wdog_wq_body(struct work_struct* p_work);
DECLARE_DELAYED_WORK(melfas_wdog_wq, melfas_wdog_wq_body);

static void melfas_wdog_wq_body(struct work_struct* p_work)
{
	int ret = 0, i;
	uint8_t buf[TS_READ_REGS_LEN];

	buf[0] = TS_READ_START_ADDR;
	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_master_send(ts_global->client, buf, 1);
		if (ret >= 0) {
			ret = i2c_master_recv(ts_global->client, buf, TS_READ_REGS_LEN);
			if (ret >= 0) {
				//printk("melfas_wdog_wq_body : i2c successed\n");
				break; // i2c success
			}
		}
	}

	if (ret < 0 || buf[0] == 0x7F) {
		printk(KERN_ERR "melfas_ts: Wdog error. reset IC\n");
		melfas_set_vdd(0);
		mdelay(100);
		melfas_set_vdd(1);
	}
	schedule_delayed_work(&melfas_wdog_wq, msecs_to_jiffies(1200));
}

static void melfas_wdog_ctrl(int enable)
{
	if (!empty_chip) { 
		if (enable) {
			printk("melfas_wdog_ctrl : wdog enable\n");
			schedule_delayed_work(&melfas_wdog_wq, msecs_to_jiffies(1200));
		} else {
			printk("melfas_wdog_ctrl : wdog disable\n");
			cancel_delayed_work_sync(&melfas_wdog_wq);
		}
	}
}
#endif

static int melfas_set_vdd(int enable)
{
	printk(KERN_INFO "(%s) enable=[%d] \n", __func__, enable);
	
	int ret; 
	if (vmmc2 == NULL) {
		printk("regulator is null.. \n");
		vmmc2 = regulator_get( NULL, "vmmc2" );
		if( IS_ERR( vmmc2 ) )
			printk( "Fail to register vmmc2 in melfas_set_vdd using regulator framework!\n" );
	}

	if (enable == 0){
		ret = regulator_disable( vmmc2 );
		if (ret) {
			printk("Regulator vmmc2 error!!\n");
			return -EIO;
		}
	} else if (enable == 1){
		ret = regulator_enable( vmmc2 );
		if (ret) {
			printk("Regulator vmmc2 error!!\n");
			return -EIO;
		}
	} else {
		ret == regulator_enable(vmmc2);
		if (ret) {
			printk("Regulator vmmc2 error!!\n");
			return -EIO;
		}
		ret == regulator_disable(vmmc2);
		if (ret) {
			printk("Regulator vmmc2 error!!\n");
			return -EIO;
		}
		mdelay(200);
		ret == regulator_enable(vmmc2);
		if (ret) {
			printk("Regulator vmmc2 error!!\n");
			return -EIO;
		}
	}
	mdelay(100);

}
/*
static int melfas_init_panel(struct melfas_ts_data *ts)
{
	char buf = 0x00;
	int ret = 0;
	
	melfas_set_vdd(1);	

//deleted because VDD is used
//	ret = i2c_master_send(ts->client, &buf, 1);
//	if(ret <0) {
//		printk(KERN_ERR "(%s) i2c_master_send() failed\n [%d]", __func__, ret);	
//		return 0;
//	}

	return true;
}
*/
static void melfas_ts_get_data(struct melfas_ts_data *ts)
{

	int ret = 0, i;
	uint8_t buf[TS_READ_REGS_LEN];
	int touchType = 0, touchState = 0, touchID = 0, posX = 0, posY = 0;
	int width = 0, strength = 0, keyID = 0, reportID = 0;

	//mprintk("start\n");

#if DEBUG_PRINT
	if(ts ==NULL) printk(KERN_ERR "melfas_ts_get_data : TS NULL\n");
#endif 

#if 0
	/*
	 SMBus Block Read:	 S Addr Wr [A] Comm [A]
	 S Addr Rd [A] [Data] A [Data] A ... A [Data] NA P
	 */
	ret = i2c_smbus_read_i2c_block_data(ts->client, TS_READ_START_ADDR, TS_READ_REGS_LEN, buf);
	if (ret < 0) {
		printk(KERN_ERR "melfas_ts_work_func: i2c_smbus_read_i2c_block_data(), failed\n");
	}
#else
	/*
	 Simple send transaction:	 S Addr Wr [A]  Data [A] Data [A] ... [A] Data [A] P
	 Simple recv transaction:	 S Addr Rd [A]  [Data] A [Data] A ... A [Data] NA P
	 */

	buf[0] = TS_READ_START_ADDR;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_master_send(ts->client, buf, 1);
		//mprintk("i2c_master_send [%d]\n", ret);
		
		if (ret >= 0) {
			ret = i2c_master_recv(ts->client, buf, TS_READ_REGS_LEN);
			//mprintk(KERN_ERR "i2c_master_recv [%d]\n", ret);

			if (ret >= 0)
				break; // i2c success
		}
	}
#endif

	if (ret < 0) {
		printk(KERN_ERR "melfas_ts_get_data: i2c failed\n");
		return;
	} else if (buf[0] == 0x7f) {
		printk(KERN_ERR "melfas_ts: ESD detected. reset IC\n");
		melfas_set_vdd(0);
		mdelay(100);
		melfas_set_vdd(1);
		return;
	} else {
		touchType = (buf[0] >> 5) & 0x03;
		touchState = (buf[0] >> 4) & 0x01;
		reportID = (buf[0] & 0x0f);
		posX = ((buf[1] & 0x0F) << (8)) + buf[2];
		posY = (((buf[1] & 0xF0) >> 4) << (8)) + buf[3];
		keyID = reportID;
		
#if MTSI_V07
		width = buf[4];
		strength = buf[5];
#else
		strength = buf[4];
#endif

		#if 0 /* nathan */
		if ( ts->tsp_hw_version == 0 )	{
			keyID = strength = buf[4];
		}
		#endif

		touchID = reportID - 1;

		//if (touchID > MELFAS_MAX_TOUCH - 1) {
		if(reportID > MELFAS_MAX_TOUCH || reportID == 0) {
			mprintk("Touch ID: %d\n", touchID);
            //enable_irq(ts->client->irq);
       		//in case of thread method, don't need enable_irq function.
			return;
		}

		if (touchType == TOUCH_SCREEN)
		{
			g_Mtouch_info[touchID].posX = posX;
			g_Mtouch_info[touchID].posY = posY;

#if MTSI_V07
			if ( ts->tsp_hw_version == 0 )	{
				g_Mtouch_info[touchID].width = 5;
			} else {
				if (touchState)
					g_Mtouch_info[touchID].width = width;
				else
					g_Mtouch_info[touchID].width = 0;
			}
#endif
			if (touchState)
				g_Mtouch_info[touchID].strength = strength;
			else
				g_Mtouch_info[touchID].strength = 0;

			for (i = 0; i < MELFAS_MAX_TOUCH; i++)
			{
				if (g_Mtouch_info[i].strength == -1)
					continue;

				input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X,
						g_Mtouch_info[i].posX);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y,
						g_Mtouch_info[i].posY);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR,
						g_Mtouch_info[i].strength);
#if MTSI_V07
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR,
						g_Mtouch_info[i].width);
#endif
				input_mt_sync(ts->input_dev);
				printk(KERN_INFO,"Touch ID: %d, State : %d, x: %d, y: %d, z: %d (%d)(%d)\n", 
					i, touchState, g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength,
					ts->tsp_hw_version, ts->tsp_fw_version);
/*
				printk("Touch ID: %d, State : %d, x: %d, y: %d, z: %d (%d)(%d)\n", 
					i, touchState, g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength,
					ts->tsp_hw_version, ts->tsp_fw_version);
*/
				if (g_Mtouch_info[i].strength == 0)
					g_Mtouch_info[i].strength = -1;
			}

		}
		else if (touchType == TOUCH_KEY)
		{
		    //TSP gabbage clear
        	for (i = 0; i < MELFAS_MAX_TOUCH; i++) /* _SUPPORT_MULTITOUCH_ */
        		g_Mtouch_info[i].strength = -1;

			if (keyID == 0x1)
				input_report_key(ts->input_dev, KEY_MENU,
						touchState ? PRESS_KEY : RELEASE_KEY);
			if (keyID == 0x2)
				input_report_key(ts->input_dev, KEY_HOME,
						touchState ? PRESS_KEY : RELEASE_KEY);
			if (keyID == 0x3)
				input_report_key(ts->input_dev, KEY_BACK,
						touchState ? PRESS_KEY : RELEASE_KEY);
			if (keyID == 0x4)
				input_report_key(ts->input_dev, KEY_SEARCH,
						touchState ? PRESS_KEY : RELEASE_KEY);
			mprintk("keyID : %d, touchState: %d\n", keyID, touchState);

		}

		input_sync(ts->input_dev);
	}


}

static irqreturn_t melfas_ts_irq_handler(int irq, void *handle)
{
	struct melfas_ts_data *ts = (struct melfas_ts_data *) handle;
#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_irq_handler\n");
#endif

	melfas_ts_get_data(ts);


	return IRQ_HANDLED;
}

uint8_t melfas_get_tsp_fw_version(struct melfas_ts_data *ts)
{
	uint8_t buf = 0x0;
	uint8_t ret = 0x0;
	buf = TS_READ_FW_VERSION_ADDR; // F/W Version Address.
			
	ret = i2c_master_send(ts->client, &buf, 1);
	if (ret < 0)
		printk	(KERN_ERR "melfas_ts_get_data : i2c_master_send [%d]\n", ret);

	ret = i2c_master_recv(ts->client, &buf, 1);
	if(ret < 0)
		printk(KERN_ERR "melfas_ts_get_data : i2c_master_recv [%d]\n", ret);

	return buf;
}

uint8_t melfas_get_tsp_hw_version(struct melfas_ts_data *ts)
{
	uint8_t buf = 0x0;
	uint8_t ret = 0x0;
    uint8_t i = 0;
	buf = TS_READ_HW_VERSION_ADDR; // HW Version Address.
		
    for(i = 0; i < 5; i++) {		
	ret = i2c_master_send(ts->client, &buf, 1);
    	if (ret < 0) {
		printk(KERN_ERR "(%s) i2c_master_send [%d]\n",  __func__, ret);
            continue;
        }
        else    
            break;
    }
	
    if(i == 5) {
        ret = mcsdl_download_binary_data();

        #if defined(CONFIG_MSM8225_COOPERPLUS_BOARD)
		gpio_tlmm_config(GPIO_CFG(70, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(71, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(119, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		//enable_irq(ts->client->irq); 
		//in case of thread method, don't need enable_irq function.
		#endif
        ret = i2c_master_send(ts->client, &buf, 1);
    	if (ret < 0) {
    		printk(KERN_ERR "(%s) i2c_master_send [%d]\n",  __func__, ret);
        }
            
    }
	ret = i2c_master_recv(ts->client, &buf, 1);
	if(ret < 0)
		printk(KERN_ERR "(%s) i2c_master_recv [%d]\n", __func__, ret);

	return buf;
}


static int melfas_ts_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{	
	struct melfas_ts_data *ts;
	int ret = 0, i;
	uint8_t buf = 0x40;	
	//bool empty_chip = false;

	mprintk("\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "melfas_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	ts = kmalloc(sizeof(struct melfas_ts_data), GFP_KERNEL);
	if (ts == NULL) {
		printk(KERN_ERR "melfas_ts_probe: failed to create a state of melfas-ts\n");
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	ts->pdata = client->dev.platform_data;

	ts->client = client;
	i2c_set_clientdata(client, ts);
	
	ts_global = ts;

	melfas_set_vdd(2);
//	melfas_init_panel(ts);

#if SET_DOWNLOAD
	ts->tsp_hw_version = melfas_get_tsp_hw_version(ts);
// 	ts->tsp_fw_version = melfas_get_tsp_fw_version(ts);

	buf = TS_READ_FW_VERSION_ADDR; // F/W Version Address.
			
	ret = i2c_master_send(ts->client, &buf, 1);
	if (ret < 0) {
		empty_chip = true;
		printk	(KERN_ERR "melfas_ts_get_data : i2c_master_send [%d]\n", ret);
	}

	ret = i2c_master_recv(ts->client, &buf, 1);
	if(ret < 0)
		printk(KERN_ERR "melfas_ts_get_data : i2c_master_recv [%d]\n", ret);

	ts->tsp_fw_version = buf;

 	printk("(%s) Information : H/W Ver. = [0x%x], F/W Ver. = [0x%x] \n", 
 		__func__, ts->tsp_hw_version, ts->tsp_fw_version);
	if (empty_chip)
		printk("TSP firmware is empty or TSP is not attached\n");

//	if (empty_chip || ts->tsp_fw_version < FW_VERSION || ts->tsp_fw_version > 0x80) {
	if (!empty_chip) {
	if (ts->tsp_fw_version < FW_VERSION || ts->tsp_fw_version > 0x50) {
		ret = mcsdl_download_binary_data();
		if(ret > 0)
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
					
		#if defined(CONFIG_MSM8225_COOPERPLUS_BOARD)
		gpio_tlmm_config(GPIO_CFG(70, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(71, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
		gpio_tlmm_config(GPIO_CFG(119, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		//enable_irq(ts->client->irq); 
		//in case of thread method, don't need enable_irq function.
		#endif		
	}
	}
	
#else
	buf = TS_READ_START_ADDR;
	ret = i2c_master_send(ts->client, &buf, 1);
	if (ret < 0) {
		printk(KERN_ERR "melfas_ts_probe: i2c_master_send() failed\n");
		ret = -EIO;
		goto err_detect_failed;
	}
	else
		ret = i2c_master_recv(ts->client, &buf, 1);

	if(ret <0) {
		printk(KERN_ERR "melfas_ts_probe: i2c_master_recv() failed\n [%d]", ret);
		goto err_detect_failed;
	}
#endif // SET_DOWNLOAD

	mprintk("i2c_master_send() [%d], Add[%d]\n", ret, ts->client->addr);

continued_process:
	ts->input_dev = input_allocate_device();
	if (!ts->input_dev)
	{
		printk(KERN_ERR "melfas_ts_probe: Not enough memory\n");
		ret = -ENOMEM;
		goto err_input_dev_alloc_failed;
	}

	ts->input_dev->name = MELFAS_TS_NAME ;
	ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
	ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
	ts->input_dev->keybit[BIT_WORD(KEY_HOME)] |= BIT_MASK(KEY_HOME);
	ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);
	ts->input_dev->keybit[BIT_WORD(KEY_SEARCH)] |= BIT_MASK(KEY_SEARCH);

	//	__set_bit(BTN_TOUCH, ts->input_dev->keybit);
	//	__set_bit(EV_ABS,  ts->input_dev->evbit);
	//	ts->input_dev->evbit[0] =  BIT_MASK(EV_SYN) | BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, TS_MAX_X_COORD, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, TS_MAX_Y_COORD, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, MELFAS_MAX_TOUCH-1, 0, 0);
#if MTSI_V07
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0);
#endif
	//	__set_bit(EV_SYN, ts->input_dev->evbit);
	//	__set_bit(EV_KEY, ts->input_dev->evbit);

	ret = input_register_device(ts->input_dev);
	if (ret) {
		printk(KERN_ERR "melfas_ts_probe: Failed to register device\n");
		ret = -ENOMEM;
		goto err_input_register_device_failed;
	}

	if (ts->client->irq)
	{
#if DEBUG_PRINT
		printk(KERN_ERR "melfas_ts_probe: trying to request irq: %s-%d\n", ts->client->name, ts->client->irq);
#endif

#if LOW_LEVEL_DETECT
		ret = request_threaded_irq(client->irq, NULL, melfas_ts_irq_handler,IRQF_ONESHOT|IRQF_TRIGGER_LOW, ts->client->name, ts);
#else // Falling edge detect
		ret = request_threaded_irq(client->irq, NULL, melfas_ts_irq_handler,IRQF_TRIGGER_FALLING, ts->client->name, ts);
#endif 

		if (ret > 0) {
			printk(KERN_ERR "melfas_ts_probe: Can't allocate irq %d, ret %d\n", ts->client->irq, ret);
			ret = -EBUSY;
			goto err_request_irq;
		}
	}

#ifdef MELFAS_WDOG_ENABLE
	printk("melfas_ts_probe : Run Wdog\n");
	melfas_wdog_ctrl(1);
#endif

	touch_class = class_create(THIS_MODULE, "touch");
	if (IS_ERR(touch_class))
		pr_err("Failed to create class(touch)!\n");

    firmware_dev = device_create(touch_class, NULL, 0, NULL, "firmware");
	if (IS_ERR(firmware_dev))
		pr_err("Failed to create device(firmware)!\n");

	if (device_create_file(firmware_dev, &dev_attr_firmware_phone) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_firmware_phone.attr.name);

    if (device_create_file(firmware_dev, &dev_attr_firmware_part) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_firmware_part.attr.name);
	
	for (i = 0; i < MELFAS_MAX_TOUCH; i++) /* _SUPPORT_MULTITOUCH_ */
		g_Mtouch_info[i].strength = -1;

	mprintk("succeed to register input device\n");

#if CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = melfas_ts_early_suspend;
	ts->early_suspend.resume = melfas_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	mprintk("Start touchscreen. name: %s, irq: %d\n", ts->client->name, ts->client->irq);

#if defined (CONFIG_TOUCHSCREEN_MELFAS_KEYLED)
	melfas_keyled_init();
	melfas_keyled_enable();
#endif

	return 0;

err_request_irq:
	printk(KERN_ERR "melfas-ts: err_request_irq failed\n");
	free_irq(client->irq, ts);
err_input_register_device_failed:
	printk(KERN_ERR "melfas-ts: err_input_register_device failed\n");
	input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
	printk(KERN_ERR "melfas-ts: err_input_dev_alloc failed\n");
err_alloc_data_failed:
	printk(KERN_ERR "melfas-ts: err_alloc_data failed_\n");
err_detect_failed:
	printk(KERN_ERR "melfas-ts: err_detect failed\n");
	kfree(ts);
err_check_functionality_failed:
	printk(KERN_ERR "melfas-ts: err_check_functionality failed_\n");

	return ret;
}

static int melfas_ts_remove(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);

#if DEBUG_PRINT
	printk(KERN_INFO "melfas_ts_remove");
#endif

	unregister_early_suspend(&ts->early_suspend);
	free_irq(client->irq, ts);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	
	return 0;
}

static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct melfas_ts_data *ts = i2c_get_clientdata(client);
#if DEBUG_PRINT
		printk(KERN_INFO "melfas_ts_suspend");
#endif

//delete

//	ret = i2c_smbus_write_byte_data(client, 0x01, 0x00); /* deep sleep */
//	if (ret < 0)
//		printk(KERN_ERR "melfas_ts_suspend: i2c_smbus_write_byte_data failed\n");

#if defined (CONFIG_MSM8225_COOPERPLUS_BOARD)
	melfas_set_vdd(0);	
#endif

	melfas_set_vdd(0);	
#if defined (CONFIG_TOUCHSCREEN_MELFAS_KEYLED)
	melfas_keyled_disable();
#endif

	return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{
    int i = 0;
	struct melfas_ts_data *ts = i2c_get_clientdata(client);
//	melfas_init_panel(ts);
	melfas_set_vdd(1);
    //TSP gabbage clear
	for (i = 0; i < MELFAS_MAX_TOUCH; i++) /* _SUPPORT_MULTITOUCH_ */
		g_Mtouch_info[i].strength = -1;

#if defined (CONFIG_TOUCHSCREEN_MELFAS_KEYLED)
	melfas_keyled_enable();
#endif

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h)
{
	struct melfas_ts_data *ts;
	ts = container_of(h, struct melfas_ts_data, early_suspend);
	printk("\n\n\n\nearly_suspend\n\n\n\n");
#if defined (CONFIG_MSM8225_COOPERPLUS_BOARD)
	melfas_ts_suspend(ts->client, PMSG_SUSPEND);
#endif

#ifdef MELFAS_WDOG_ENABLE
	melfas_wdog_ctrl(0);
#endif
	melfas_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void melfas_ts_late_resume(struct early_suspend *h)
{
	struct melfas_ts_data *ts;
	ts = container_of(h, struct melfas_ts_data, early_suspend);
	printk("\n\n\n\nlate_resume\n\n\n\n");
#if defined (CONFIG_MSM8225_COOPERPLUS_BOARD)
	melfas_ts_resume(ts->client);
#endif
#ifdef MELFAS_WDOG_ENABLE
	melfas_wdog_ctrl(1);
#endif

	melfas_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id melfas_ts_id[] = {
{ MELFAS_TS_NAME, 0 },
	{ }
};

static struct i2c_driver melfas_ts_driver = {
	.driver		= {
	.name = MELFAS_TS_NAME,
	},
	.id_table		= melfas_ts_id,
	.probe		= melfas_ts_probe,
		.remove = __devexit_p (melfas_ts_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend		= melfas_ts_suspend,
	.resume		= melfas_ts_resume,
#endif
		};

static int __devinit melfas_ts_init(void)
{
	return i2c_add_driver(&melfas_ts_driver);
}

static void __exit melfas_ts_exit(void)
{
	i2c_del_driver(&melfas_ts_driver);
}

#if defined (CONFIG_TOUCHSCREEN_MELFAS_KEYLED)
static int melfas_keyled_init(void)
{
	int ret = 0;
	struct vreg *vreg_keyled;

	printk(KERN_INFO "%s has been called.\n", __func__);
	
	vreg_keyled = vreg_get(NULL, "xo_out");

	ret = vreg_set_level(vreg_keyled, 3300);
	
	if (ret) {
		printk(KERN_ERR "%s: vreg set level failed (%d)\n", __func__, ret);
		return -EIO;
	}
	
	ret = vreg_disable(vreg_keyled);
	if (ret) {
		printk(KERN_ERR "%s: vreg disable failed (%d)\n",	__func__, ret);
		return -EIO;
	}
	
	return 0;
}

static int melfas_keyled_enable(void)
{
	int ret = 0;
	struct vreg *vreg_keyled;
	
	printk(KERN_INFO "%s has been called.\n", __func__);

	vreg_keyled = vreg_get(NULL, "xo_out");
		
	ret = vreg_enable(vreg_keyled);
	if (ret) {
		printk(KERN_ERR "%s: vreg enable failed (%d)\n", __func__, ret);
		ret=-EIO;	
	}

	return 0;
}

static int melfas_keyled_disable(void)
{
	int ret = 0;
	struct vreg *vreg_keyled;
	
	printk(KERN_INFO "%s has been called.\n", __func__);
	
	vreg_keyled = vreg_get(NULL, "xo_out");
		
	ret = vreg_disable(vreg_keyled);
	if (ret) {
		printk(KERN_ERR "%s: vreg enable failed (%d)\n", __func__, ret);
		ret=-EIO;	
	}

	return 0;
}
#endif




static int melfas_ts_check_firmware_and_update(void)
{
	int ret = melfas_ts_read_ver();

	if (ret < 0) {
#ifdef EMERGENCY_FIRMWARE_UPDATE
		printk(KERN_ERR "melfas_ts_probe: emergancy update[%d]\n", ret);
		ret = firm_update();
#endif			
		return ret;
	}

#if 0
	if((board_hw_revision == 0x00 || board_hw_revision == 0x02) && HW_ver <= 1){
		if (working_TSP_for_fwupdate == INNER)	{
			if (SW_ver < TS_INNER_HW01_SW_VER)
				firm_update();
		}
		else {
			if (SW_ver < TS_OUTER_HW01_SW_VER)
				firm_update();
		}			
	}
#endif

	printk(KERN_INFO "[TSP] board_rev=%d\n", board_hw_revision);
	return ret;
}

static int melfas_ts_power_off_all(void)
{
	int ret = 1; 
	struct vreg *vreg_touch;

#if 1
	printk("[TSP] %s : start\n",__func__);



    melfas_set_vdd(0);

#if 0
	// turn off inner tsp
	vreg_touch = vreg_get(NULL, "ldo19");

	ret = vreg_disable(vreg_touch);
	if (ret) {
		printk(KERN_ERR "%s: vreg disable failed (%d)\n",__func__, ret);
		return -EIO;
	}

	// turn off outer tsp
	vreg_touch = vreg_get(NULL, "ldo6");

	ret = vreg_disable(vreg_touch);
	if (ret) {
		printk(KERN_ERR "%s: vreg disable failed (%d)\n",__func__, ret);
		return -EIO;
	}
#endif

#endif

	return ret;
}


int melfas_ts_read_ver(void)
{
	int ret = 0;
	int i = 0;
	uint8_t i2c_addr = 0x30;
	uint8_t buf_tmp[2]={0};


printk(KERN_ERR,"melfas_ts_read_ver1\n" );


	buf_tmp[0] = MCSTS_MODULE_VER_REG;



	for(i=0; i<I2C_RETRY_CNT; i++)
	{
		ret = i2c_master_send(ts_global->client, buf_tmp, 1);
//#if DEBUG_PRINT
		printk(KERN_ERR "melfas_ts_read_ver : i2c_master_send [%d]\n", ret);
//#endif 
		mdelay(1); 
		if(ret >=0)
		{
			ret = i2c_master_recv(ts_global->client, buf_tmp, 2);
//#if DEBUG_PRINT			
			printk(KERN_ERR "melfas_ts_read_ver : i2c_master_recv [%d]\n", ret);			
//#endif
			if(ret >=0)
			{
				break; // i2c success
			}
		}
	}

	HW_ver = buf_tmp[0];
	SW_ver = buf_tmp[1];



    
	printk("[TSP] %s, ver HW=%x\n", __func__ , HW_ver );
	printk("[TSP] %s, ver SW=%x\n", __func__ , SW_ver );


	return ret;
}



/* firmware - update */
/*
static ssize_t firmware_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
		char *after;
	
		unsigned long value = simple_strtoul(buf, &after, 10);	
		printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);
		firmware_ret_val = -1;
	
		if ( value == 1 )
		{
			printk("[TSP] Firmware update start!!\n" );
			{
//				melfas_ts_power_off_all();				
			
				firmware_ret_val = firm_update( );
//				msleep(0);
				
//				melfas_ts_power_off_all();				
//				melfas_init_panel(ts_global);				
				//firmware_ret_val = 1;				
			}
		}

		return size;
}

*/
static ssize_t firmware_phone_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	melfas_ts_read_ver();
    
	printk("[TSP] %s: phone = %x \n", __func__, FW_VERSION );
	printk("[TSP] %s:/sys/class/touch/firmware/firmware_phone, %02x \n", __func__, FW_VERSION);

	sprintf(buf,"%02X\n", FW_VERSION);
        printk("[TSP] %s: firmware phone ver %s  \n", __func__, buf);
	//# cat /sys/class/touch/firmware/firmware   

	return sprintf(buf, "%s", buf );
}


static ssize_t firmware_part_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	melfas_ts_read_ver();
    
	printk("[TSP] %s: part = %x & HW ver is = %x \n", __func__, SW_ver , HW_ver);
	printk("[TSP] %s:/sys/class/touch/firmware/firmware_part, %02x \n", __func__, SW_ver);

	sprintf(buf,"%02X\n", SW_ver);
        printk("[TSP] %s: firmware phone ver %s  \n", __func__, buf);
	//# cat /sys/class/touch/firmware/firmware   

	return sprintf(buf, "%s", buf );
}

/*

static ssize_t firmware_ret_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	printk("[TSP] %s!\n", __func__);

	return sprintf(buf, "%d", firmware_ret_val );
}
*/

static ssize_t firmware_phone_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	printk("[TSP] %s, operate nothing!\n", __func__);

	return size;
}

static ssize_t firmware_part_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	printk("[TSP] %s, operate nothing!\n", __func__);

	return size;
}

int firm_update( void )
{
	int ret = 0;
	int cnt = 0;
	struct vreg *vreg_touch;

	printk(KERN_INFO "[TSP] %s, %d\n", __func__, __LINE__);

	firmware_ret_val = -1;
    	//firmware_ret_val = 1;

#if 1

#if 1
	//if (ts_global->use_irq)
	{
		printk("[TSP] disable_irq : %d\n", __LINE__ );
		disable_irq(ts_global->client->irq);
	}
#endif
	for (cnt=0; cnt <5 ; cnt++){		
//		msleep(0);	
		printk(KERN_INFO,"[F/W D/L] Entry mcsdl_download_binary_data, try=%d\n", cnt );
		
		//ret = mcsdl_download_binary_data( working_TSP_for_fwupdate );
		if(HW_ver != HW_VERSION) {
            firmware_ret_val = 1;
            break;
        }
		ret = mcsdl_download_binary_data( );

		if(ret > 0) {
			printk(KERN_ERR "SET Download Fail - error code [%d]\n", ret);			
            continue;
          }
        else {
            firmware_ret_val = 1;					
#if defined(CONFIG_MSM8225_COOPERPLUS_BOARD)
    		gpio_tlmm_config(GPIO_CFG(70, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
    		gpio_tlmm_config(GPIO_CFG(71, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_16MA), GPIO_CFG_ENABLE);
    		gpio_tlmm_config(GPIO_CFG(119, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    		//enable_irq(ts_global->client->irq); 
    		//in case of thread method, don't need enable_irq function.
#endif
            break;
         }
#if 0
		if(ret > 0)
		{		
			melfas_ts_power_off_all();				
			melfas_ts_init_selected( working_TSP_for_fwupdate );				
			ret = melfas_ts_read_ver();
			if ( ret> 0) {
				firmware_ret_val = 1;
				printk("[TSP] Firmware update success!\n");
				break;	
			}
		} else {
			printk("[TSP] Firmware update failed.. RESET!, try=%d\n", cnt);
			mcsdl_vdd_off();
			mdelay(100);
			mcsdl_vdd_on();
			mdelay(100);

			if ( cnt >= 4 ) {
				printk("[TSP] Firmware update failed.. RESET!, try=%d\n", cnt);
				printk("[TSP] check i2c lines.\n");
				firmware_ret_val = 0;
				break;	
			}
		}
#endif
	}
	msleep(200);

tsp_out:

#if 1
	//if (ts_global->use_irq)
	{
		enable_irq(ts_global->client->irq);
	}
#endif

#endif

	return (firmware_ret_val == 1 ? 1 : 0);
}


MODULE_DESCRIPTION("Driver for Melfas MTSI Touchscreen Controller");
MODULE_AUTHOR("MinSang, Kim <kimms@melfas.com>");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

module_init(melfas_ts_init);
module_exit(melfas_ts_exit);
