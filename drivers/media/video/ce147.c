/*
 * drivers/media/video/mt9p012.c
 *
 * mt9p012 sensor driver
 *
 *
 * Copyright (C) 2008 Texas Instruments.
 *
 * Leverage OV9640.c
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *****************************************************
 *****************************************************
 * modules/camera/ce147.c
 *
 * CE147 sensor driver source file
 *
 * Modified by paladin in Samsung Electronics
 */
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <mach/gpio.h>
#include <mach/hardware.h>
#include <media/v4l2-int-device.h>

#include <linux/firmware.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>

#include "isp/isp.h"
#include "omap34xxcam.h"
#include "ce147.h"
bool back_cam_in_use= false;
#if (CAM_CE147_DBG_MSG)
#include "dprintk.h"
#else
#define dprintk(x, y...)
#endif

#define I2C_M_WRITE 0x0000 /* write data, from slave to master */
#define I2C_M_READ  0x0001 /* read data, from slave to master */


#define I2C_M_WR 0x0000 /* write data, from slave to master */
#define I2C_M_RD  0x0001 /* read data, from slave to master */

static u32 ce147_curr_state = CE147_STATE_INVALID;
static u32 ce147_pre_state = CE147_STATE_INVALID;

static bool ce147_720p_enable = false;
static bool ce147_touch_state = false;


static struct ce147_sensor ce147 = {
	.timeperframe = {
		.numerator    = 1,
		.denominator  = 30,
	},
	.fps            = 30,
	.bv             = 0,
	.state          = CE147_STATE_PREVIEW,
	.mode           = CE147_MODE_CAMERA,
	.preview_size   = CE147_PREVIEW_SIZE_640_480,
	.capture_size   = CE147_IMAGE_SIZE_2560_1920,
	.detect         = SENSOR_NOT_DETECTED,
	.focus_mode     = CE147_AF_INIT_NORMAL,
	.effect         = CE147_EFFECT_OFF,
	.iso            = CE147_ISO_AUTO,
	.photometry     = CE147_PHOTOMETRY_CENTER,
	.ev             = CE147_EV_DEFAULT,
	.wdr            = CE147_WDR_OFF,
	.contrast       = CE147_CONTRAST_DEFAULT,
	.saturation     = CE147_SATURATION_DEFAULT,
	.sharpness      = CE147_SHARPNESS_DEFAULT,
	.wb             = CE147_WB_AUTO,
	.isc            = CE147_ISC_STILL_OFF,
	.scene          = CE147_SCENE_OFF,
	.aewb           = CE147_AE_UNLOCK_AWB_UNLOCK,
	.antishake      = CE147_ANTI_SHAKE_OFF,
	.flash_capture  = CE147_FLASH_CAPTURE_OFF,
	.flash_movie    = CE147_FLASH_MOVIE_OFF,
	.jpeg_quality   = CE147_JPEG_SUPERFINE, 
	.zoom           = CE147_ZOOM_1P00X,
	.thumb_offset   = CE147_THUMBNAIL_OFFSET,
	.yuv_offset     = CE147_YUV_OFFSET,
	.jpeg_capture_w = JPEG_CAPTURE_WIDTH,
	.jpeg_capture_h = JPEG_CAPTURE_HEIGHT,
};

struct v4l2_queryctrl ce147_ctrl_list[] = {
	{
		.id            = V4L2_CID_SELECT_MODE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "select mode",
		.minimum       = CE147_MODE_CAMERA,
		.maximum       = CE147_MODE_VT,
		.step          = 1,
		.default_value = CE147_MODE_CAMERA,
	},    
	{
		.id            = V4L2_CID_SELECT_STATE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "select state",
		.minimum       = CE147_STATE_PREVIEW,
		.maximum       = CE147_STATE_CAPTURE,
		.step          = 1,
		.default_value = CE147_STATE_PREVIEW,
	},    
	{
		.id            = V4L2_CID_FOCUS_MODE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Focus Mode",
		.minimum       = CE147_AF_INIT_NORMAL,
		.maximum       = CE147_AF_INIT_FACE,
		.step          = 1,
		.default_value = CE147_AF_INIT_NORMAL,
	},
	{
		.id            = V4L2_CID_AF,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Focus Status",
		.minimum       = CE147_AF_START,
		.maximum       = CE147_AF_STOP,
		.step          = 1,
		.default_value = CE147_AF_STOP,
	},
	{
		.id            = V4L2_CID_ZOOM,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Digital Zoom",
		.minimum       = CE147_ZOOM_1P00X,
		.maximum       = CE147_ZOOM_4P00X,
		.step          = 1,
		.default_value = CE147_ZOOM_1P00X,
	},
	{
		.id            = V4L2_CID_JPEG_TRANSFER,
		.name          = "Request JPEG Transfer",
	},
	{
		.id            = V4L2_CID_JPEG_QUALITY,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "JPEG Quality",
		.minimum       = CE147_JPEG_SUPERFINE,
		.maximum       = CE147_JPEG_ECONOMY,
		.step          = 1,
		.default_value = CE147_JPEG_SUPERFINE,
	},
	{
		.id            = V4L2_CID_ISO,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "ISO",
		.minimum       = CE147_ISO_AUTO,
		.maximum       = CE147_ISO_800,
		.step          = 1,
		.default_value = CE147_ISO_AUTO,
	},
	{
		.id            = V4L2_CID_BRIGHTNESS,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Brightness",
		.minimum       = CE147_EV_MINUS_2P0,
		.maximum       = CE147_EV_PLUS_2P0,
		.step          = 1,
		.default_value = CE147_EV_DEFAULT,
	},
	{
		.id            = V4L2_CID_WB,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "White Balance",
		.minimum       = CE147_WB_AUTO,
		.maximum       = CE147_WB_FLUORESCENT,
		.step          = 1,
		.default_value = CE147_WB_AUTO,
	},
	{
		.id            = V4L2_CID_CONTRAST,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Contrast",
		.minimum       = CE147_CONTRAST_MINUS_3,
		.maximum       = CE147_CONTRAST_PLUS_3,
		.step          = 1,
		.default_value = CE147_CONTRAST_DEFAULT,
	},  
	{
		.id            = V4L2_CID_SATURATION,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Saturation",
		.minimum       = CE147_SATURATION_MINUS_3,
		.maximum       = CE147_SATURATION_PLUS_3,
		.step          = 1,
		.default_value = CE147_SATURATION_DEFAULT,
	},
	{
		.id            = V4L2_CID_EFFECT,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Effect",
		.minimum       = CE147_EFFECT_OFF,
		.maximum       = CE147_EFFECT_PURPLE,
		.step          = 1,
		.default_value = CE147_EFFECT_OFF,
	},
	{
		.id            = V4L2_CID_SCENE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Scene",
		.minimum       = CE147_SCENE_OFF,
		.maximum       = CE147_SCENE_FIREWORKS,
		.step          = 1,
		.default_value = CE147_SCENE_OFF,
	},
	{
		.id            = V4L2_CID_PHOTOMETRY,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Photometry",
		.minimum       = CE147_PHOTOMETRY_CENTER,
		.maximum       = CE147_PHOTOMETRY_MATRIX,
		.step          = 1,
		.default_value = CE147_PHOTOMETRY_CENTER,
	},
	{
		.id            = V4L2_CID_WDR,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Wide Dynamic Range",
		.minimum       = CE147_WDR_OFF,
		.maximum       = CE147_WDR_AUTO,
		.step          = 1,
		.default_value = CE147_WDR_OFF,
	},
	{
		.id            = V4L2_CID_SHARPNESS,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Sharpness",
		.minimum       = CE147_SHARPNESS_MINUS_3,
		.maximum       = CE147_SHARPNESS_PLUS_3,
		.step          = 1,
		.default_value = CE147_SHARPNESS_DEFAULT,
	},
	{
		.id            = V4L2_CID_ISC,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Image Stabilization",
		.minimum       = CE147_ISC_STILL_OFF,
		.maximum       = CE147_ISC_MOVIE_ON,
		.step          = 1,
		.default_value = CE147_ISC_STILL_OFF,
	},
	{
		.id            = V4L2_CID_AEWB,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Auto Exposure/Auto White Balance",
		.minimum       = CE147_AE_LOCK_AWB_LOCK,
		.maximum       = CE147_AE_UNLOCK_AWB_UNLOCK,
		.step          = 1,
		.default_value = CE147_AE_UNLOCK_AWB_UNLOCK,
	},
	{
		.id            = V4L2_CID_ANTISHAKE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Anti Shake Setting",
		.minimum       = CE147_ANTI_SHAKE_OFF,
		.maximum       = CE147_ANTI_SHAKE_ON,
		.step          = 1,
		.default_value = CE147_ANTI_SHAKE_OFF,
	},  
	{
		.id            = V4L2_CID_FW_UPDATE,
		.name          = "Firmware Update",
	},
	{
		.id            = V4L2_CID_FLASH_CAPTURE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Capture Flash Setting",
		.minimum       = CE147_FLASH_CAPTURE_OFF,
		.maximum       = CE147_FLASH_CAPTURE_AUTO,
		.step          = 1,
		.default_value = CE147_FLASH_CAPTURE_OFF,
	},
	{
		.id            = V4L2_CID_FLASH_MOVIE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Movie Flash Setting",
		.minimum       = CE147_FLASH_MOVIE_OFF,
		.maximum       = CE147_FLASH_MOVIE_ON,
		.step          = 1,
		.default_value = CE147_FLASH_MOVIE_OFF,
	},
	{
		.id            = V4L2_CID_JPEG_SIZE,
		.name          = "JPEG Image Size",
		.flags         = V4L2_CTRL_FLAG_READ_ONLY,
	},
	{
		.id            = V4L2_CID_THUMBNAIL_SIZE,
		.name          = "Thumbnail Image Size",
		.flags         = V4L2_CTRL_FLAG_READ_ONLY,
	},
};
#define NUM_CE147_CONTROL ARRAY_SIZE(ce147_ctrl_list)

/* list of image formats supported by ce147 sensor */
const static struct v4l2_fmtdesc ce147_formats[] = {
	{
		.description = "YUV422 (UYVY)",
		.pixelformat = V4L2_PIX_FMT_UYVY,
	},
	{
		.description = "YUV422 (YUYV)",
		.pixelformat = V4L2_PIX_FMT_YUYV,
	},
	{
		.description = "JPEG(without header)+ JPEG",
		.pixelformat = V4L2_PIX_FMT_JPEG,
	},
	{
		.description = "JPEG(without header)+ YUV",
		.pixelformat = V4L2_PIX_FMT_MJPEG,
	},
};
#define NUM_CE147_FORMATS ARRAY_SIZE(ce147_formats)

extern struct ce147_platform_data nowplus_ce147_platform_data;

u8 ISP_FW_ver[4] = {0x00,};

//static int nightshot_in_init = 0;

static int ce147_start_preview(void);

/* Arun C: inlining the function
 * need to check if there are any side effects and
 * camera performance improvement
 */
#define REGION_50HZ 50
#define REGION_60HZ 60
/* Arun C: inlining the function
 * need to check if there are any side effects and
 * camera performance improvement
 */

#define DEBUG_PRINTS 0
#if DEBUG_PRINTS
	#define FUNC_ENTR	\
		printk(KERN_INFO "[S5K5BBGX] %s Entered!!!\n", __func__)
	//#define I2C_DEBUG 1
#else
	#define FUNC_ENTR
	//#define I2C_DEBUG 0
#endif

/* FOR S5K5BBGX TUNING */
//#define CONFIG_LOAD_FILE
#ifdef CONFIG_LOAD_FILE
#include <linux/vmalloc.h>
#include <linux/mm.h>
//#define max_size 200000

struct test
{
	char data;
	struct test *nextBuf;
};

struct test *testBuf;
#endif

#define S5K5BBGX_BURST_WRITE_LIST(A)	s5k5bbgx_sensor_burst_write_list(A,(sizeof(A) / sizeof(A[0])),#A);
#define BURST_MODE_BUFFER_MAX_SIZE 2700
unsigned char s5k5bbgx_buf_for_burstmode[BURST_MODE_BUFFER_MAX_SIZE];

static struct s5k5bbgx_exif_info exif_info={0,0,0};

static const struct s5k5bbgx_reg *mode_table[] = {
	[S5K5BBGX_MODE_SENSOR_INIT] = mode_sensor_init,
	[S5K5BBGX_MODE_PREVIEW_640x480] = mode_preview_640x480,
	[S5K5BBGX_MODE_PREVIEW_800x600] = mode_preview_800x600,
	[S5K5BBGX_MODE_CAPTURE_1600x1200] = mode_capture_1600x1200,
};

#if 1 //p5-last

static int s5k5bbgx_read_reg(struct i2c_client *client, u16 addr, u8 *val, u16 length)
{
	int err;
	struct i2c_msg msg[2];
	unsigned char data[2];

	if (!client->adapter)
		return -ENODEV;

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = data;

	/* high byte goes out first */
	data[0] = (u8) (addr >> 8);
	data[1] = (u8) (addr & 0xff);

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = length;
	msg[1].buf = val;
	err = i2c_transfer(client->adapter, msg, 2);

	if (err != 2)
		return -EINVAL;

	return 0;
}
static int s5k5bbgx_write_reg(struct i2c_client *client, u16 addr, u16 val)
{
#if 1
	int err;
	struct i2c_msg msg;
	unsigned char data[4];
	int retry = 0;

	if (!client->adapter)
		return -ENODEV;

	data[0] = (u8) (addr >> 8);
	data[1] = (u8) (addr & 0xff);
	data[2] = (u8) (val >> 8);
	data[3] = (u8) (val & 0xff);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = 4;
	msg.buf = data;

	do {
		err = i2c_transfer(client->adapter, &msg, 1);
		//pr_err("s5k5bbgx: i2c transfer p5 last %4x %4x\n",
		//       addr, val);
		if (err == 1)
			return 0;
		retry++;
		pr_err("s5k5bbgx: i2c transfer failed, retrying %x %x\n",
		       addr, val);
		msleep(3);
	} while (retry <= S5K5BBGX_MAX_RETRIES);
	if (err != 1) {
		pr_err("%s: I2C is not working\n", __func__);
		return -EIO;
	}

	return 0;
#endif
}

static int s5k5bbgx_write_table(struct i2c_client *client, const u32 table[])
{
	int err;
	int i;
	const u32 *next = table;
	u16 val;

	for (next = table; ((*next >> 16) & 0xFFFF) != S5K5BBGX_TABLE_END; next++) {
		if (((*next >> 16) & 0xFFFF) == S5K5BBGX_TABLE_WAIT_MS) {
			msleep(*next  & 0xFFFF);
			continue;
		}

		val = (u16)(*next  & 0xFFFF);

		err = s5k5bbgx_write_reg(client, (*next >> 16) & 0xFFFF, val);
		if (err < 0)
			return err;
	}

	return 0;
}

static int s5k5bbgx_sensor_burst_write_list(const u32 list[], int size, char *name)	
{
	int err = -EINVAL;
	int i = 0;
	int idx = 0;

	u16 subaddr = 0, next_subaddr = 0;
	u16 value = 0;

	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	//struct i2c_msg msg = {  info->i2c_client->addr, 0, 0, s5k5bbgx_buf_for_burstmode};
	struct i2c_msg msg = {  client->addr, 0, 0, s5k5bbgx_buf_for_burstmode};
	


	for (i = 0; i < size; i++)
	{

		if(idx > (BURST_MODE_BUFFER_MAX_SIZE - 10))
		{
			printk("BURST MODE buffer overflow!!!\n");
			 return err;
		}



		subaddr = (list[i] & 0xFFFF0000) >> 16;

		if(subaddr == 0x0F12)
			next_subaddr = (list[i+1] & 0xFFFF0000) >> 16;

		value = list[i] & 0x0000FFFF;
		
		switch(subaddr)
		{

			case 0x0F12:
			{
				// make and fill buffer for burst mode write
				if(idx == 0) 
				{
					s5k5bbgx_buf_for_burstmode[idx++] = 0x0F;
					s5k5bbgx_buf_for_burstmode[idx++] = 0x12;
				}
				s5k5bbgx_buf_for_burstmode[idx++] = value >> 8;
				s5k5bbgx_buf_for_burstmode[idx++] = value & 0xFF;

			
			 	//write in burstmode
				if(next_subaddr != 0x0F12)
				{
					msg.len = idx;
					err = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
					//printk("s5k5ccgx_sensor_burst_write, idx = %d\n",idx);
					idx=0;
				}
				
			}
			break;

			case 0xFFFF:
			{
				pr_info("burst_mode --- end of REGISTER \n");
				err = 0;
				idx = 0;
			}
			break;

			case 0xFFFE:
			{
				pr_info("burst_mode --- s5k5ccgx_i2c_write_twobyte give delay: %d\n", value);				
				msleep(value);
			}
			break;

			default:
			{
			    idx = 0;
			    err = s5k5bbgx_write_reg(client, subaddr, value);
			}
			break;
			
		}

		
	}

	if (unlikely(err < 0))
	{
		printk("%s: register set failed\n",__func__);
		return err;
	}
	return 0;

}



#ifdef CONFIG_LOAD_FILE
static inline int s5k5bbgx_write(struct i2c_client *client,
		u16 addr_reg, u16 data_reg)
{
	struct i2c_msg msg[1];
	unsigned char buf[4];
	int ret = 1;

	buf[0] = ((addr_reg >>8)&0xFF);
	buf[1] = (addr_reg)&0xFF;
	buf[2] = ((data_reg >>8)&0xFF);
	buf[3] = (data_reg)&0xFF;
	
	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 4;
	msg->buf = buf;


	ret = i2c_transfer(client->adapter, &msg, 1);

	if (unlikely(ret < 0)) {
		dev_err(&client->dev, "%s: (a:%x,d:%x) write failed\n", __func__, addr_reg,data_reg);
		return ret;
	}
	
	return (ret != 1) ? -1 : 0;
}

static int s5k5bbgx_write_tuningmode(struct i2c_client *client, char s_name[])
{
	int ret = -EAGAIN;
	u32 temp;
	//unsigned long temp;
	char delay = 0;
	char data[11];
	int searched = 0;
	int size = strlen(s_name);
	int i;
	unsigned int addr_reg;
	unsigned int data_reg;

	struct test *tempData, *checkData;

	dprintk(CAM_DBG, CE147_MOD_NAME "size = %d, string = %s\n", size, s_name);
	tempData = &testBuf[0];
	while(!searched)
	{
		searched = 1;
		for (i = 0; i < size; i++)
		{
			if (tempData->data != s_name[i])
			{
				searched = 0;
				break;
			}
			tempData = tempData->nextBuf;
		}
		tempData = tempData->nextBuf;
	}
	//structure is get..

	while(1)
	{
		if (tempData->data == '{')
			break;
		else
			tempData = tempData->nextBuf;
	}


	while (1)
	{
		searched = 0;
		while (1)
		{
			if (tempData->data == 'x')
			{
				//get 10 strings
				data[0] = '0';
				for (i = 1; i < 11; i++)
				{
					data[i] = tempData->data;
					tempData = tempData->nextBuf;
				}
				//printk("%s\n", data);
				temp = simple_strtoul(data, NULL, 16);
				break;
			}
			else if (tempData->data == '}')
			{
				searched = 1;
				break;
			}
			else
				tempData = tempData->nextBuf;
			if (tempData->nextBuf == NULL)
				return -1;
		}

		if (searched)
			break;

		//let search...
		if ((temp & 0xFFFE0000) == 0xFFFE0000) {                                                    
			delay = temp & 0xFFFF;                                                                              
			msleep(delay);                                                                                      
			continue;                                                                                           
		}
		else if ((temp & 0xFFFFFFFF) == 0xFFFFFFFF) { 
			continue;
		}
//		ret = s5k5bbgx_write(client, temp);

		addr_reg = (temp >> 16)&0xffff;
		data_reg = (temp & 0xffff);

		ret = s5k5bbgx_write(client, addr_reg, data_reg);
		
		//printk("addr = %x    data = %x\n", addr_reg, data_reg);
		//printk("data = %x\n", data_reg);

		/* In error circumstances */
		/* Give second shot */
		if (unlikely(ret)) {
			dev_info(&client->dev,
					"s5k5bbgx i2c retry one more time\n");
			ret = s5k5bbgx_write(client, addr_reg, data_reg);

			/* Give it one more shot */
			if (unlikely(ret)) {
				dev_info(&client->dev,
						"s5k5bbgx i2c retry twice\n");
				ret = s5k5bbgx_write(client, addr_reg, data_reg);
			}
		}
	}
	return ret;
}

static int loadFile(void){

	struct file *fp;
	char *nBuf;
	unsigned int max_size;
	unsigned int l;
	struct test *nextBuf = testBuf;
	int i = 0;

	int starCheck = 0;
	int check = 0;
	int ret = 0;
	loff_t pos;

	mm_segment_t fs = get_fs();
	set_fs(get_ds());

	fp = filp_open("/mnt/sdcard/s5k5bbgx_regs_p5.h", O_RDONLY, 0);

	if (IS_ERR(fp)) {
		printk("%s : file open error\n", __func__);
		return PTR_ERR(fp);
	}

	l = (int) fp->f_path.dentry->d_inode->i_size;

	max_size = l;
	
	printk("l = %d\n", l);
	nBuf = kmalloc(l, GFP_KERNEL);
	testBuf = (struct test*)kmalloc(sizeof(struct test) * l, GFP_KERNEL);

	if (nBuf == NULL) {
		printk( "Out of Memory\n");
		filp_close(fp, current->files);
	}
	pos = 0;
	memset(nBuf, 0, l);
	memset(testBuf, 0, l * sizeof(struct test));

	ret = vfs_read(fp, (char __user *)nBuf, l, &pos);

	if (ret != l) {
		printk("failed to read file ret = %d\n", ret);
		kfree(nBuf);
		kfree(testBuf);
		filp_close(fp, current->files);
		return -1;
	}

	filp_close(fp, current->files);

	set_fs(fs);

	i = max_size;

	printk("i = %d\n", i);

	while (i){
		testBuf[max_size - i].data = *nBuf;
		if (i != 1)
		{
			testBuf[max_size - i].nextBuf = &testBuf[max_size - i + 1];
		}
		else
		{
			testBuf[max_size - i].nextBuf = NULL;
			break;
		}
		i--;
		nBuf++;
	}

	i = max_size;
	nextBuf = &testBuf[0];
#if 1
	while (i - 1){
		if (!check && !starCheck){
			if (testBuf[max_size - i].data == '/')
			{
				if(testBuf[max_size-i].nextBuf != NULL)
				{
					if (testBuf[max_size-i].nextBuf->data == '/')
					{
						check = 1;// when find '//'
						i--;
					}
					else if (testBuf[max_size-i].nextBuf->data == '*')
					{
						starCheck = 1;// when find '/*'
						i--;
					}
				}	
				else
					break;
			}
			if (!check && !starCheck)
				if (testBuf[max_size - i].data != '\t')//ignore '\t'
				{
					nextBuf->nextBuf = &testBuf[max_size-i];
					nextBuf = &testBuf[max_size - i];
				}

		}
		else if (check && !starCheck)
		{
			if (testBuf[max_size - i].data == '/')
			{
				if(testBuf[max_size-i].nextBuf != NULL)
				{
					if (testBuf[max_size-i].nextBuf->data == '*')
					{
						starCheck = 1;// when find '/*'
						check = 0;
						i--;
					}
				}	
				else 
					break;
			}

			if(testBuf[max_size - i].data == '\n' && check) // when find '\n'
			{
				check = 0;
				nextBuf->nextBuf = &testBuf[max_size - i];
				nextBuf = &testBuf[max_size - i];
			}

		}
		else if (!check && starCheck)
		{
			if (testBuf[max_size - i].data == '*')
			{
				if(testBuf[max_size-i].nextBuf != NULL)
				{
					if (testBuf[max_size-i].nextBuf->data == '/')
					{
						starCheck = 0;// when find '*/'
						i--;
					}
				}	
				else
					break;
			}

		}
		i--;
		if (i < 2) {
			nextBuf = NULL;
			break;
		}
		if (testBuf[max_size - i].nextBuf == NULL)
		{
			nextBuf = NULL;
			break;
		}
	}
#endif
#if 0 // for print
	printk("i = %d\n", i);
	nextBuf = &testBuf[0];
	while (1)
	{
		//printk("sdfdsf\n");
		if (nextBuf->nextBuf == NULL)
			break;
		printk("%c", nextBuf->data);
		nextBuf = nextBuf->nextBuf;
	}
#endif
	return 0;
}
#endif
#endif  //p5-last

static int s5k5bbgx_senser_info(struct i2c_client *client)
{
	unsigned short id = 0;
	int ret;
	//u8 r_value[2] = {0};
	u16 t_value;
	u16 iso_value;
	int err;

//#ifdef VIEW_FUNCTION_CALL	
	printk("[s5k5bbgx] %s function %d line launched!\n", __func__, __LINE__);
//#endif


	//err = s5k5bbgx_write_table(client, mode_exif_shutterspeed);
	err =  s5k5bbgx_read_reg(client, 0x0040, &id, 2);
	if (err < 0)
	{
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}
	//t_value = r_value[1] + (r_value[0] << 8);
	//pr_debug("Shutterspeed = %d, r_value[1] = %d, r_value[0] = %d\n", t_value, r_value[1], r_value[0]);
	printk("<=s5k5bbgx_senser_info=> SENSOR FW ID => 0x%x \n", id); 

		//err = s5k5bbgx_write_table(client, mode_exif_shutterspeed);
	err =  s5k5bbgx_read_reg(client, 0x0042, &id, 2);
	if (err < 0)
	{
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}
	//t_value = r_value[1] + (r_value[0] << 8);
	//pr_debug("Shutterspeed = %d, r_value[1] = %d, r_value[0] = %d\n", t_value, r_value[1], r_value[0]);
	printk("<=s5k5bbgx_senser_info=> SENSOR version => 0x%x \n", id); 


	return err;
}


static int ce147_check_dataline()
{
	int err  = -EINVAL;
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	//unsigned char ce147_buf_check_dataline[3] = {0xEC, 0x01, 0x01};

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_check_dataline is called...\n");	
#if 0
	if(ce147_write_reg(client, sizeof(ce147_buf_check_dataline), ce147_buf_check_dataline))
	{
		printk(CE147_MOD_NAME "%s: failed: i2c_write for check_dataline\n", __func__);
		return -EIO;
	}
#endif
	//err = s5k4ba_write_regs(client, sr200pc10_dataline,SR200PC10_DATALINE_REGS, "sr200pc10_dataline"); // init + dataline start
	//if (err < 0)
#ifdef CONFIG_LOAD_FILE
	 if(s5k5bbgx_write_tuningmode(client, "mode_test_pattern")!=0)
#else
	 if(s5k5bbgx_write_table(client, mode_test_pattern)!=0)
#endif	 	
	{
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}
	sensor->check_dataline = 0;
	
	return 0;
}

static int ce147_check_dataline_stop()
{
	int err  = -EINVAL;
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;
#if 0
	unsigned char ce147_buf_check_dataline[3] = {0xEC, 0x00, 0x00};

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_check_dataline_stop is called...\n");	

	if(ce147_write_reg(client, sizeof(ce147_buf_check_dataline), ce147_buf_check_dataline))
	{
		printk(CE147_MOD_NAME "%s: failed: i2c_write for check_dataline\n", __func__);
		return -EIO;
	}
#endif
	dprintk(CAM_INF, CE147_MOD_NAME "ce147_check_dataline_stop is called...\n");	

	//err = s5k4ba_write_regs(client, sr200pc10_dataline_stop,SR200PC10_DATALINE_STOP_REGS, "sr200pc10_dataline_stop"); // preview start
	//if (err < 0)
#ifdef CONFIG_LOAD_FILE
	 if(s5k5bbgx_write_tuningmode(client, "mode_test_pattern_off")!=0)
#else	
	 if(s5k5bbgx_write_table(client, mode_test_pattern_off)!=0)
#endif	 	
	{
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}
	
	sensor->check_dataline = 0;
	sensor->pdata->power_set(V4L2_POWER_OFF);
	mdelay(5);
	sensor->pdata->power_set(V4L2_POWER_ON);
	mdelay(5);
	//err = s5k4ba_write_regs(client, sr200pc10_init0, SR200PC10_INIT0_REGS, "sr200pc10_init0");//LYG
	//if (err < 0)
#ifdef CONFIG_LOAD_FILE
	 if(s5k5bbgx_write_tuningmode(client, "mode_sensor_init")!=0)
#else	
	 if(s5k5bbgx_write_table(client, mode_table[0])!=0)
#endif	 	
	{
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}
	mdelay(10);
	ce147_start_preview();
	//s5ka3dfx_set_init();
	//ce147_set_ev(CE147_EV_DEFAULT);

	return 0;
}

static int ce147_detect(struct i2c_client *client)
{
	struct ce147_sensor *sensor = &ce147;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_detect is called...\n");
#if 0
	/* Start Camera Program */
	if(ce147_write_reg(client, sizeof(ConfigSensorFirmWareInit_list), ConfigSensorFirmWareInit_list))
		goto detect_fail;

	//mdelay(600); //Reduced the delay 

	/* Read Firmware Version */
	if(ce147_write_read_reg(client, sizeof(CameraReadCommand_FWVersion_List), CameraReadCommand_FWVersion_List, 4, ISP_FW_ver))
		goto detect_fail;

	if(*ISP_FW_ver == 0x0 && *(ISP_FW_ver+1) == 0x0 && *(ISP_FW_ver+2) == 0x0 && *(ISP_FW_ver+3) == 0x0)
		goto detect_fail;

	printk(CE147_MOD_NAME " Loader (FW ver)  : 0x%02x, 0x%02x\n", ISP_FW_ver[0], ISP_FW_ver[1]);
	printk(CE147_MOD_NAME " Loader (PRM ver) : 0x%02x, 0x%02x\n", ISP_FW_ver[2], ISP_FW_ver[3]);
	printk(CE147_MOD_NAME " Sensor is Detected!!!\n");
#endif
	return 0;

detect_fail:
	printk(CE147_MOD_NAME " Sensor is not Detected!!!\n");
	return -EINVAL; 
}

static int ce147_set_effect(s32 value) //LYG
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_effect is called...[%d]\n",value);
#ifdef CONFIG_LOAD_FILE
	switch(value)
	{
		case CE147_EFFECT_OFF:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_none")!=0)
				goto effect_fail;
			break;

		case CE147_EFFECT_BW:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_mono")!=0)
				goto effect_fail;
			break;    

		case CE147_EFFECT_GREY:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_mono")!=0)
				goto effect_fail;
			break;      

		case CE147_EFFECT_SEPIA:
             		 if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_sepia")!=0)
				goto effect_fail;
			break;

		case CE147_EFFECT_SHARPEN:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_none")!=0)
				goto effect_fail;
			break;      

		case CE147_EFFECT_NEGATIVE:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_negative")!=0)
				goto effect_fail;
			break;

		case CE147_EFFECT_ANTIQUE:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_none")!=0)
				goto effect_fail;
			break;

		case CE147_EFFECT_AQUA:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_none")!=0)
				goto effect_fail;
			break;

		case CE147_EFFECT_RED:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_none")!=0)
				goto effect_fail;
			break; 

		case CE147_EFFECT_PINK:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_none")!=0)
				goto effect_fail;
			break; 

		case CE147_EFFECT_YELLOW:
			if(s5k5bbgx_write_tuningmode(client,"mode_coloreffect_none")!=0)
				goto effect_fail;
			break;       

		case CE147_EFFECT_GREEN:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_none")!=0)
				goto effect_fail;
			break; 

		case CE147_EFFECT_BLUE:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_none")!=0)
				goto effect_fail;
			break; 

		case CE147_EFFECT_PURPLE:
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_none")!=0)
				goto effect_fail;
			break;       

		default:
			printk(CE147_MOD_NAME "Effect value is not supported!!!\n");
			if(s5k5bbgx_write_tuningmode(client, "mode_coloreffect_none")!=0)
				goto effect_fail;
	}
#else
	switch(value)
	{
		case CE147_EFFECT_OFF:
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
			break;

		case CE147_EFFECT_BW:
			if(s5k5bbgx_write_table(client, mode_coloreffect_mono)!=0)
				goto effect_fail;
			break;    

		case CE147_EFFECT_GREY:
			if(s5k5bbgx_write_table(client, mode_coloreffect_mono)!=0)
				goto effect_fail;
			break;      

		case CE147_EFFECT_SEPIA:
             		 if(s5k5bbgx_write_table(client, mode_coloreffect_sepia)!=0)
				goto effect_fail;
			break;

		case CE147_EFFECT_SHARPEN:
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
			break;      

		case CE147_EFFECT_NEGATIVE:
			if(s5k5bbgx_write_table(client, mode_coloreffect_negative)!=0)
				goto effect_fail;
			break;

		case CE147_EFFECT_ANTIQUE:
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
			break;

		case CE147_EFFECT_AQUA:
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
			break;

		case CE147_EFFECT_RED:
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
			break; 

		case CE147_EFFECT_PINK:
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
			break; 

		case CE147_EFFECT_YELLOW:
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
			break;       

		case CE147_EFFECT_GREEN:
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
			break; 

		case CE147_EFFECT_BLUE:
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
			break; 

		case CE147_EFFECT_PURPLE:
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
			break;       

		default:
			printk(CE147_MOD_NAME "Effect value is not supported!!!\n");
			if(s5k5bbgx_write_table(client, mode_coloreffect_none)!=0)
				goto effect_fail;
	}
#endif	

	sensor->effect = value;
	return 0;

effect_fail:
	printk(CE147_MOD_NAME " ce147_set_effect is failed!!\n");
	return -EINVAL;     
}

static int ce147_set_iso(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_iso is called...[%d]\n",value);

	sensor->iso = value;
	return 0;
}

static int ce147_set_photometry(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_photometry is called...[%d]\n",value);

#ifdef CONFIG_LOAD_FILE
	switch(value)
	{
		case CE147_PHOTOMETRY_CENTER:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_centerweighted")!=0)
				goto photometry_fail;
			break;

		case CE147_PHOTOMETRY_SPOT:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_spot")!=0)
				goto photometry_fail;
			break;

		case CE147_PHOTOMETRY_MATRIX:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_matrix")!=0)
				goto photometry_fail;
			break;

		default:
			printk(CE147_MOD_NAME "Photometry value is not supported!!!\n");
			goto photometry_fail;
	}	
#else
	switch(value)
	{
		case CE147_PHOTOMETRY_CENTER:
			if(s5k5bbgx_write_table(client, mode_exposure_centerweighted)!=0)
				goto photometry_fail;
			break;

		case CE147_PHOTOMETRY_SPOT:
			if(s5k5bbgx_write_table(client, mode_exposure_spot)!=0)
				goto photometry_fail;
			break;

		case CE147_PHOTOMETRY_MATRIX:
			if(s5k5bbgx_write_table(client, mode_exposure_matrix)!=0)
				goto photometry_fail;
			break;

		default:
			printk(CE147_MOD_NAME "Photometry value is not supported!!!\n");
			goto photometry_fail;
	}
#endif
	sensor->photometry = value;

	return 0;

photometry_fail:
	printk(CE147_MOD_NAME " ce147_set_photometry is failed!!\n");
	return -EINVAL;         
}

static int ce147_set_ev_no_apply(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	int err = 0;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_ev is called...[%d]\n",value);

	sensor->ev = value;

	return 0;
}


static int ce147_set_ev(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_ev is called...[%d]\n",value);
#ifdef CONFIG_LOAD_FILE
	switch(value)
	{
		case CE147_EV_MINUS_2P0:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_m4")!=0)
				goto ev_fail;
			break;

		case CE147_EV_MINUS_1P5:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_m3")!=0)
				goto ev_fail;
			break;

		case CE147_EV_MINUS_1P0:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_m2")!=0)
				goto ev_fail;
			break;

		case CE147_EV_MINUS_0P5:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_m1")!=0)
				goto ev_fail;
			break;      

		case CE147_EV_DEFAULT:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_0")!=0)
				goto ev_fail;
			break;

		case CE147_EV_PLUS_0P5:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_p1")!=0)
				goto ev_fail;
			break;

		case CE147_EV_PLUS_1P0:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_p2")!=0)
				goto ev_fail;
			break;

		case CE147_EV_PLUS_1P5:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_p3")!=0)
				goto ev_fail;
			break;

		case CE147_EV_PLUS_2P0:
			if(s5k5bbgx_write_tuningmode(client, "mode_exposure_p4")!=0)
				goto ev_fail;
			break;      

		default:
			printk(CE147_MOD_NAME "EV value is not supported!!!\n");
			goto ev_fail;
	}
#else
	switch(value)
	{
		case CE147_EV_MINUS_2P0:
			if(s5k5bbgx_write_table(client, mode_exposure_m4)!=0)
				goto ev_fail;
			break;

		case CE147_EV_MINUS_1P5:
			if(s5k5bbgx_write_table(client, mode_exposure_m3)!=0)
				goto ev_fail;
			break;

		case CE147_EV_MINUS_1P0:
			if(s5k5bbgx_write_table(client, mode_exposure_m2)!=0)
				goto ev_fail;
			break;

		case CE147_EV_MINUS_0P5:
			if(s5k5bbgx_write_table(client, mode_exposure_m1)!=0)
				goto ev_fail;
			break;      

		case CE147_EV_DEFAULT:
			if(s5k5bbgx_write_table(client, mode_exposure_0)!=0)
				goto ev_fail;
			break;

		case CE147_EV_PLUS_0P5:
			if(s5k5bbgx_write_table(client, mode_exposure_p1)!=0)
				goto ev_fail;
			break;

		case CE147_EV_PLUS_1P0:
			if(s5k5bbgx_write_table(client, mode_exposure_p2)!=0)
				goto ev_fail;
			break;

		case CE147_EV_PLUS_1P5:
			if(s5k5bbgx_write_table(client, mode_exposure_p3)!=0)
				goto ev_fail;
			break;

		case CE147_EV_PLUS_2P0:
			if(s5k5bbgx_write_table(client, mode_exposure_p4)!=0)
				goto ev_fail;
			break;      

		default:
			printk(CE147_MOD_NAME "EV value is not supported!!!\n");
			goto ev_fail;
	}
#endif
	sensor->ev = value;

	return 0;

ev_fail:
	printk(CE147_MOD_NAME " ce147_set_ev is failed!!\n");
	return -EINVAL;           
}

static int ce147_set_wdr(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_wdr is called...[%d]\n",value);

	sensor->wdr = value;

	return 0;	
}

static int ce147_set_saturation(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_saturation is called...[%d]\n",value);

	sensor->saturation = value;

	return 0;
}

static int ce147_set_sharpness(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_sharpness is called...[%d]\n",value);

	sensor->sharpness = value;

	return 0;
}

static int ce147_set_contrast(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_contrast is called...[%d]\n",value);

	sensor->contrast = value;
	return 0;
}

static int ce147_set_wb(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;
	int err;
	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_wb is called...[%d]\n",value);

#ifdef CONFIG_LOAD_FILE
	switch (value) 
	{
		case CE147_WB_AUTO:			
			if(s5k5bbgx_write_tuningmode(client, "mode_WB_auto")!=0)
				goto wb_fail;	
			break;

		case CE147_WB_DAYLIGHT:
			if(s5k5bbgx_write_tuningmode(client, "mode_WB_daylight")!=0)
				goto wb_fail;	
			break;

		case CE147_WB_INCANDESCENT:
			if(s5k5bbgx_write_tuningmode(client, "mode_WB_incandescent")!=0)
				goto wb_fail;	
			break;

		case CE147_WB_FLUORESCENT:
			if(s5k5bbgx_write_tuningmode(client, "mode_WB_fluorescent")!=0)
				goto wb_fail;	
			break;

		case CE147_WB_CLOUDY:
			if(s5k5bbgx_write_tuningmode(client, "mode_WB_cloudy")!=0)
				goto wb_fail;	
			break;

		default:
			printk(CE147_MOD_NAME "WB value is not supported!!!\n");
			goto wb_fail;
	}
#else
	switch(value)
	{
		case CE147_WB_AUTO:
			if(s5k5bbgx_write_table(client, mode_WB_auto)!=0)
				goto wb_fail;	
			break;

		case CE147_WB_DAYLIGHT:
			if(s5k5bbgx_write_table(client, mode_WB_daylight)!=0)
				goto wb_fail;	
			break;

		case CE147_WB_INCANDESCENT:
			if(s5k5bbgx_write_table(client, mode_WB_incandescent)!=0)
				goto wb_fail;	
			break;

		case CE147_WB_FLUORESCENT:
			if(s5k5bbgx_write_table(client, mode_WB_fluorescent)!=0)
				goto wb_fail;	
			break;

		case CE147_WB_CLOUDY:
			if(s5k5bbgx_write_table(client, mode_WB_cloudy)!=0)
				goto wb_fail;	
			break;

		default:
			printk(CE147_MOD_NAME "WB value is not supported!!!\n");
			goto wb_fail;
	}
#endif		

	sensor->wb = value;
	return 0;

wb_fail:
	printk(CE147_MOD_NAME " ce147_set_wb is failed!!\n");
	return -EINVAL;                 
}

static int ce147_set_isc(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_isc is called...[%d]\n",value);

	sensor->isc = value;

	return 0;
}

static int ce147_get_scene(struct v4l2_control *vc)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;
	u8 readdata[2] = {0x00,};

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_get_scene is called...\n");
	vc->value = sensor->scene;
	return 0;

get_scene_fail:
	printk(CE147_MOD_NAME " ce147_get_scene is failed!!\n");
	return -EINVAL;  
}

static int ce147_set_scene(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_scene is called...[%d]\n",value);
#ifdef CONFIG_LOAD_FILE
	if(value !=CE147_SCENE_OFF){
		if(s5k5bbgx_write_tuningmode(client, "mode_scene_OFF")!=0)
			goto scene_fail;
	}
	
	switch(value)
	{
		case CE147_SCENE_OFF:

			if(s5k5bbgx_write_tuningmode(client, "mode_scene_OFF")!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_ASD:
			break;    

		case CE147_SCENE_PORTRAIT:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_PORTRAIT")!=0) 
				goto scene_fail;
			break;

		case CE147_SCENE_LANDSCAPE:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_LANDSCAPE")!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_SUNSET:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_SUNSET")!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_DAWN:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_DAWN")!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_NIGHTSHOT:

			if(s5k5bbgx_write_tuningmode(client, "mode_scene_night")!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_TEXT:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_TEXT")!=0) 
				goto scene_fail;
			break;

		case CE147_SCENE_SPORTS:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_SPORTS")!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_AGAINST_LIGHT:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_BACKLIGHT")!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_INDOORS:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_PARTY")!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_BEACH_SNOW:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_BEACH")!=0) 
				goto scene_fail;
			break;

		case CE147_SCENE_FALLCOLOR:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_FALL")!=0) 
				goto scene_fail;
			break;

		case CE147_SCENE_FIREWORKS:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_FIREWORK")!=0) 
				goto scene_fail;
			break;

		case CE147_SCENE_CANDLELIGHT:
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_CANDLE")!=0)
				goto scene_fail;
			break;

		default:
			printk(CE147_MOD_NAME "Scene value is not supported!!!\n");
			if(s5k5bbgx_write_tuningmode(client, "mode_scene_OFF")!=0)
			goto scene_fail;
	}
#else
	if(value !=CE147_SCENE_OFF){
		if(s5k5bbgx_write_table(client, mode_scene_OFF)!=0)
			goto scene_fail;
	}
	
	switch(value)
	{
		case CE147_SCENE_OFF:

			if(s5k5bbgx_write_table(client, mode_scene_OFF)!=0)
				goto scene_fail;	
			break;

		case CE147_SCENE_ASD:
			break;    

		case CE147_SCENE_PORTRAIT:
			if(s5k5bbgx_write_table(client, mode_scene_PORTRAIT)!=0) 
				goto scene_fail;
			break;

		case CE147_SCENE_LANDSCAPE:
			if(s5k5bbgx_write_table(client, mode_scene_LANDSCAPE)!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_SUNSET:
			if(s5k5bbgx_write_table(client, mode_scene_SUNSET)!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_DAWN:
			if(s5k5bbgx_write_table(client, mode_scene_DAWN)!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_NIGHTSHOT:

			if(s5k5bbgx_write_table(client, mode_scene_night)!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_TEXT:
			if(s5k5bbgx_write_table(client, mode_scene_TEXT)!=0) 
				goto scene_fail;
			break;

		case CE147_SCENE_SPORTS:
			if(s5k5bbgx_write_table(client, mode_scene_SPORTS)!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_AGAINST_LIGHT:
			if(s5k5bbgx_write_table(client, mode_scene_BACKLIGHT)!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_INDOORS:
			if(s5k5bbgx_write_table(client, mode_scene_PARTY)!=0)
				goto scene_fail;
			break;

		case CE147_SCENE_BEACH_SNOW:
			if(s5k5bbgx_write_table(client, mode_scene_BEACH)!=0) 
				goto scene_fail;
			break;

		case CE147_SCENE_FALLCOLOR:
			if(s5k5bbgx_write_table(client, mode_scene_FALL)!=0) 
				goto scene_fail;
			break;

		case CE147_SCENE_FIREWORKS:
			if(s5k5bbgx_write_table(client, mode_scene_FIREWORK)!=0) 
				goto scene_fail;
			break;

		case CE147_SCENE_CANDLELIGHT:
			if(s5k5bbgx_write_table(client, mode_scene_CANDLE)!=0)
				goto scene_fail;
			break;

		default:
			printk(CE147_MOD_NAME "Scene value is not supported!!!\n");
			if(s5k5bbgx_write_table(client, mode_scene_OFF)!=0)
			goto scene_fail;
	}
#endif
	sensor->scene = value;  
	//ce147_start_preview(); //error mode_scene_night
	return 0;

scene_fail:
	printk(CE147_MOD_NAME " ce147_set_scene is failed!!\n");
	return -EINVAL;    
}

static int ce147_set_mode(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	sensor->mode = value;
	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_mode is called... mode = %d\n", sensor->mode); 
	return 0;
}

static int ce147_set_state(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	sensor->state = value;
	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_state is called... state = %d\n", sensor->state); 
	return 0;
}

static int ce147_set_aewb(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_aewb is called...[%d]\n",value);

	sensor->aewb = value;

	return 0;
}

static int ce147_set_antishake(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_antishake is called...[%d]\n",value);

	sensor->antishake = value;

	return 0;
}

static int ce147_set_flash_capture(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_flash_capture is called...[%d]\n",value);

	sensor->flash_capture = value;  

	return 0;
}

static int ce147_set_flash_movie(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_flash_movie is called...%d\n", value);

	sensor->flash_movie = value;

	return 0;
}

static int ce147_set_flash_ctrl(bool value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_flash_ctrl is called...value : %d, mode : %d\n", value, sensor->flash_capture);

	return 0;
}

static int ce147_set_jpeg_quality(s32 value)
{
	struct ce147_sensor *sensor = &ce147;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_jpeg_quality is called... value : %d\n", value);

	switch(value) 
	{
		case CE147_JPEG_SUPERFINE:
			sensor->jpeg_quality = CE147_JPEG_SUPERFINE;
			break;
		case CE147_JPEG_FINE:
			sensor->jpeg_quality = CE147_JPEG_FINE;
			break;
		case CE147_JPEG_NORMAL:
			sensor->jpeg_quality = CE147_JPEG_NORMAL;
			break;
		case CE147_JPEG_ECONOMY:
			sensor->jpeg_quality = CE147_JPEG_ECONOMY;
			break;
		default:
			printk(CE147_MOD_NAME "JPEG quality value is not supported!\n");
			goto jpeg_quality_fail;
	}

	return 0;

jpeg_quality_fail:
	printk(CE147_MOD_NAME "ce147_set_jpeg_quality is failed!!!\n");
	return -EINVAL;    
}

static int ce147_set_focus_status(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	int cnt = 0;

	u8 readdata = 0x00;
	u8 status = 0x00;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_focus_status is called...[%d]\n",value);
	return 0;
}

static int ce147_set_focus_touch(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	u16 touch_x = (u16)sensor->position.x;
	u16 touch_y = (u16)sensor->position.y;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_focus_touch is called... x : %d, y : %d\n", touch_x, touch_y); 

	ce147_touch_state = true;

	return 0;
}

static int ce147_get_focus(struct v4l2_control *vc)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	u8 readdata = 0x00;
	u8 status = 0x00;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_get_auto_focus is called...\n");
	vc->value = sensor->focus_mode;
	return 0;

get_focus_fail:
	printk(CE147_MOD_NAME "ce147_get_focus is failed!!!\n");
	return -EINVAL;      
}

static int ce147_set_focus(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "AF set value = %d\n", value);

	sensor->focus_mode = value;

	return 0;
}

static int ce147_get_zoom(struct v4l2_control *vc)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	u8 readdata[2] = {0x0,};
	vc->value = sensor->zoom;
	dprintk(CAM_INF, CE147_MOD_NAME "ce147_get_zoom is called...\n"); 
	return 0;
}


static int ce147_set_zoom(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_zoom is called... value = %d\n", value); 

	sensor->zoom = value;

	return 0;
}


static int ce147_get_jpeg_size(struct v4l2_control *vc)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;
	u8 readdata[8];

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_get_jpeg_size is called...\n"); 
	vc->value = 0;
	return 0;

get_jpeg_size_fail:
	printk(CE147_MOD_NAME "ce147_get_jpeg_size is failed!!!\n");
	return -EINVAL;     
}

static int ce147_get_thumbnail_size(struct v4l2_control *vc)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;
	u8 readdata[8] = {0x00,};

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_get_thumbnail_size is called...\n");
	vc->value = 0;
	return 0;

get_thumbnail_size_fail:
	printk(CE147_MOD_NAME "ce147_get_thumbnail_size is failed!!!\n");
	return -EINVAL;     
}

static int ce147_prepare_preview(void)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	u8  readdata[1] = {0x00};

	int cnt = 0;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_prepare_preview is called...\n");
	return 0;

prepare_preview_fail:    
	printk(CE147_MOD_NAME "ce147_prepare_preview is failed!!!\n");
	return -EINVAL;     
}

static int ce147_start_preview(void)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_DBG, CE147_MOD_NAME  "ce147_start_preview is called...preview_size = %d\n",sensor->preview_size);
#ifdef CONFIG_LOAD_FILE
	switch(sensor->preview_size)
	{
		case CE147_PREVIEW_SIZE_640_480:
			if(s5k5bbgx_write_tuningmode(client, "mode_preview_640x480")!=0)
				goto start_preview_fail;
		 	break;
		case CE147_PREVIEW_SIZE_320_240:
			if(s5k5bbgx_write_tuningmode(client, "mode_preview_320x240")!=0)
				goto start_preview_fail;
			break;
		case CE147_PREVIEW_SIZE_176_144:
			if(s5k5bbgx_write_tuningmode(client, "mode_preview_176x144")!=0)
				goto start_preview_fail;
			break;
		default:		
			if(s5k5bbgx_write_tuningmode(client, "mode_preview_640x480")!=0)
				goto start_preview_fail;
			break;
			
	}
#else
	switch(sensor->preview_size)
	{
		case CE147_PREVIEW_SIZE_640_480:
			//if(s5k5bbgx_write_table(client, mode_table[S5K5BBGX_MODE_PREVIEW_640x480])!=0)
			if(s5k5bbgx_write_table(client, mode_preview_640x480)!=0)
				goto start_preview_fail;
			break;
		case CE147_PREVIEW_SIZE_320_240:
			//s5k4ba_write_regs(client, sr200pc10_preview_320x240,SR200PC10_PREVIEW_320x240_REGS, "sr200pc10_preview_320x240"); // preview start
			if(s5k5bbgx_write_table(client, mode_preview_320x240)!=0)
				goto start_preview_fail;
			break;
		case CE147_PREVIEW_SIZE_176_144:
			//s5k4ba_write_regs(client, sr200pc10_preview_176x144,SR200PC10_PREVIEW_176x144_REGS, "sr200pc10_preview_176x144"); // preview start
			if(s5k5bbgx_write_table(client, mode_preview_176x144)!=0)
				goto start_preview_fail;
			break;
		default:
			//s5k4ba_write_regs(client, sr200pc10_preview,SR200PC10_PREVIEW_REGS, "sr200pc10_preview"); // preview start
			if(s5k5bbgx_write_table(client, mode_table[S5K5BBGX_MODE_PREVIEW_640x480])!=0)
				goto start_preview_fail;
			break;
			
	}

	//if(sensor->scene ==CE147_SCENE_NIGHTSHOT||sensor->scene ==CE147_SCENE_FIREWORKS ){
	//	ce147_set_scene(sensor->scene);
	//}
	
#endif
	if(sensor->scene ==CE147_SCENE_NIGHTSHOT||sensor->scene ==CE147_SCENE_FIREWORKS ){
		ce147_set_scene(sensor->scene);
	}
#if 0
	/* Preview Start */
	if(ce147_write_reg(client, sizeof(ConfigSensorPreviewStart_list), ConfigSensorPreviewStart_list))
		goto start_preview_fail;
	if(ce147_poll_reg(client, RCommandPreviewStatusCheck))
		goto start_preview_fail;
#endif
	return 0;

start_preview_fail:
	printk(CE147_MOD_NAME "ce147_start_preview is failed\n"); 
	return -EINVAL;  
}

static int ce147_set_preview(void)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	u8  FixedFPS_list[3] = {0x5A, 0x1E, 0x00}; 

	dprintk(CAM_DBG, CE147_MOD_NAME "ce147_set_preview is called...%d\n", sensor->preview_size);

	ce147_pre_state = ce147_curr_state;
	ce147_curr_state = CE147_STATE_PREVIEW;    

	/* Set Preview setting (fps, iso, 720p...) */
	if(ce147_720p_enable) {

	} else {

#ifdef CONFIG_LOAD_FILE
		//int err;
		 if(s5k5bbgx_write_tuningmode(client, "mode_return_camera_preview")!=0)
		 	goto preview_fail;
	
#else
		if(s5k5bbgx_write_table(client, mode_return_camera_preview)!=0)
			goto preview_fail;
#endif		  
		dprintk(CAM_DBG, CE147_MOD_NAME "Preview size is %d\n", sensor->preview_size);
		
		/* Set FPS */
		dprintk(CAM_DBG, CE147_MOD_NAME "fps is %d. set 0x%x\n", sensor->fps, (u8)sensor->fps);

#ifdef CONFIG_LOAD_FILE
		//int err;
		if(sensor->scene ==CE147_SCENE_OFF){
			if(sensor->fps == 25 && sensor->mode == CE147_MODE_CAMCORDER){
				dprintk(CAM_DBG, CE147_MOD_NAME " fps is 25\n");
				if(s5k5bbgx_write_tuningmode(client, "mode_25_FPS")!=0)
						goto preview_fail;
			}
			else if(sensor->fps == 15 && sensor->mode == CE147_MODE_CAMCORDER){
				dprintk(CAM_DBG, CE147_MOD_NAME " fps is 15\n");
				if(s5k5bbgx_write_tuningmode(client, "mode_15_FPS")!=0)
						goto preview_fail;
			}
			else{
				dprintk(CAM_DBG, CE147_MOD_NAME " fps is AUTO\n");
				if(s5k5bbgx_write_tuningmode(client, "mode_Auto_FPS")!=0)
						goto preview_fail;	
			}
		}
	
#else
		if(sensor->scene ==CE147_SCENE_OFF){
			if(sensor->fps == 25 && sensor->mode == CE147_MODE_CAMCORDER){
				dprintk(CAM_DBG, CE147_MOD_NAME " fps is 25\n");
				if(s5k5bbgx_write_table(client, mode_25_FPS)!=0)
						goto preview_fail;
			}
			else if(sensor->fps == 15 && sensor->mode == CE147_MODE_CAMCORDER){
				dprintk(CAM_DBG, CE147_MOD_NAME " fps is 15\n");
				if(s5k5bbgx_write_table(client, mode_15_FPS)!=0)
						goto preview_fail;
			}
			else{
				dprintk(CAM_DBG, CE147_MOD_NAME " fps is AUTO\n");
				if(s5k5bbgx_write_table(client, mode_Auto_FPS)!=0)
						goto preview_fail;	
			}
		}
#endif			
	}
	return 0;

preview_fail:
	printk(CE147_MOD_NAME "ce147_set_preview is failed\n"); 
	return -EINVAL;
}

static int ce147_prepare_capture(void)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	u8  readdata[1] = {0x00};

	int cnt = 0;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_prepare_capture is called...\n");
	return 0;

prepare_capture_fail:    
	printk(CE147_MOD_NAME "ce147_prepare_capture is failed!!!\n");
	return -EINVAL;     
}


int s5k5bbgx_set_exif_info(struct i2c_client *client)
{
	u8 r_value1[2] = {0};
	u8 r_value2[2] = {0};
	u16 t_value=0;
	u16 t_value2=0;
	u32 t_value3=0;
	u16 iso_value;
	int err;

	//Shutter Speed
	err = s5k5bbgx_write_table(client, mode_exif_shutterspeed);
	err =  s5k5bbgx_read_reg(client, 0x0F12, r_value1, 2);
	err =  s5k5bbgx_read_reg(client, 0x0F12, r_value2, 2);
	
	t_value = r_value1[1] + (r_value1[0] << 8);
	t_value2 = r_value2[1] + (r_value2[0] << 8);
	t_value3 = t_value |(t_value2 << 16);

	dprintk(CAM_DBG, CE147_MOD_NAME "Shutterspeed = %d,%x r_value[1] = %x, r_value[0] = %x\n", t_value,t_value, r_value1[1], r_value1[0]);
	dprintk(CAM_DBG, CE147_MOD_NAME "Shutterspeed = %d,%x r_value2[1] = %x, r_value2[0] = %x\n", t_value2,t_value2, r_value2[1], r_value2[0]);
	dprintk(CAM_DBG, CE147_MOD_NAME "Shutterspeed = %d,%x ",t_value3,t_value3);

	exif_info.info_exptime_denumer = t_value3;
	exif_info.T_value = t_value/100;			//add by yongjin - phillit

	//ISO
	err = s5k5bbgx_write_table(client, mode_exif_iso);
	err =  s5k5bbgx_read_reg(client, 0x0F12, r_value1, 2);
	t_value = r_value1[1] + (r_value1[0] << 8);
	dprintk(CAM_DBG, CE147_MOD_NAME "ISO = %d,%x r_value[1] = %x, r_value[0] =%x\n", t_value,t_value, r_value1[1], r_value1[0]);
	
	iso_value = t_value * 100 / 256;

	if (iso_value < 150)
		exif_info.info_iso = 50;
	else if (iso_value < 250)
		exif_info.info_iso = 100;
	else if (iso_value < 350)
		exif_info.info_iso = 200;
	else
		exif_info.info_iso = 400;

	return err;
}

static int ce147_start_capture(void)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;
	struct v4l2_pix_format* pix = &sensor->pix;

	u32 value;
	u8 readdata[3] = {0x0,};
	int err;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_start_capture is called...capture_size = %d\n",sensor->capture_size); 
	
#ifdef CONFIG_LOAD_FILE	
	switch(sensor->capture_size)
	{
		case CE147_IMAGE_SIZE_1600_1200:
			s5k5bbgx_write_tuningmode(client, "mode_capture_1600x1200");
			break;
		case CE147_IMAGE_SIZE_1280_960:
			if(s5k5bbgx_write_tuningmode(client, "mode_capture_1280x960")!=0)
				goto start_capture_fail;
			break;
		case CE147_IMAGE_SIZE_640_480:
			if(s5k5bbgx_write_tuningmode(client, "mode_capture_640x480")!=0)
				goto start_capture_fail;
			break;
		case CE147_IMAGE_SIZE_320_240:
			if(s5k5bbgx_write_tuningmode(client, "mode_capture_320x240")!=0)
				goto start_capture_fail;
			break;
		default:
			s5k5bbgx_write_tuningmode(client,  "mode_capture_1600x1200");
			break;
	}
#else
	switch(sensor->capture_size)	
	{
		case CE147_IMAGE_SIZE_1600_1200:
			s5k5bbgx_write_table(client, mode_table[S5K5BBGX_MODE_CAPTURE_1600x1200]);
			break;
		case CE147_IMAGE_SIZE_1280_960:
			if(s5k5bbgx_write_table(client, mode_capture_1280x960)!=0)
				goto start_capture_fail;
			break;
		case CE147_IMAGE_SIZE_640_480:
			if(s5k5bbgx_write_table(client, mode_capture_640x480)!=0)
				goto start_capture_fail;
			break;
		case CE147_IMAGE_SIZE_320_240:
			if(s5k5bbgx_write_table(client, mode_capture_320x240)!=0)
				goto start_capture_fail;
			break;
		default:
			s5k5bbgx_write_table(client, mode_table[S5K5BBGX_MODE_CAPTURE_1600x1200]);
			break;
	}
#endif

	if(sensor->scene ==CE147_SCENE_NIGHTSHOT||sensor->scene ==CE147_SCENE_FIREWORKS ){
			dprintk(CAM_INF, CE147_MOD_NAME "Night scene or Firework\n"); 
			mdelay(200);
	}

		u8 val[2] = {0};
		u8 retry = 0;
		pr_debug("%s: sensor_ mode is S5K5BBGX_MODE_CAPTURE_1600x1200---!!\n", __func__);
#ifndef CONFIG_LOAD_FILE		
		s5k5bbgx_set_exif_info(client);
#endif
		do {
			dprintk(CAM_INF, CE147_MOD_NAME "ce147_start_capture is called.. while(%d)\n",retry); 
			msleep(20);
#ifndef CONFIG_LOAD_FILE			
			err = s5k5bbgx_write_table(client, mode_check_capture_staus);
#else
			err = s5k5bbgx_write_tuningmode(client, "mode_check_capture_staus");
#endif			
			if (err < 0)
				return err;
			err = s5k5bbgx_read_reg(client, 0x0F12, val, 2);
			if (err < 0)
				return err;
			if ((val[1]+(val[0]<<8)) == 0)
				break;
			retry++;
		} while (retry <= S5K5BBGX_READ_STATUS_RETRIES);

		if(err<0)
			goto start_capture_fail;
	
	return 0;

start_capture_fail:
	printk(CE147_MOD_NAME "ce147_start_capture is failed\n"); 
	return -EINVAL;  
}

static int ce147_set_capture(void)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;
	dprintk(CAM_DBG, CE147_MOD_NAME  "ce147_set_capture is called... %d\n", sensor->capture_size);

	ce147_pre_state = ce147_curr_state;
	ce147_curr_state = CE147_STATE_CAPTURE;
	return 0;

capture_fail:
	printk(CE147_MOD_NAME "ce147_set_capture is failed\n"); 
	return -EINVAL;     
}

static void ce147_set_skip(void)
{
	struct ce147_sensor *sensor = &ce147;

	int skip_frame = 0; 

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_skip is called...\n");

	if(sensor->state == CE147_STATE_PREVIEW)
	{      
		dprintk(CAM_INF, CE147_MOD_NAME "BV level = %d\n", sensor->bv);

		if(sensor->bv > 500) {
			if(sensor->scene == CE147_SCENE_NIGHTSHOT)
				skip_frame = sensor->fps; 
			else
				skip_frame = sensor->fps / 2; 
		} else {
			if(sensor->fps > 30)
				skip_frame = sensor->fps / 5;
			else if(sensor->fps > 15)
				skip_frame = sensor->fps / 4; 
			else if(sensor->fps > 7)
				skip_frame = sensor->fps / 3; 
			else
				skip_frame = sensor->fps / 2; 
		}
	}
	else
		skip_frame = 2;

	dprintk(CAM_INF, CE147_MOD_NAME "skip frame = %d frame\n", skip_frame);

	isp_set_hs_vs(0,skip_frame);
}

static int ioctl_streamoff(struct v4l2_int_device *s)
{
	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_streamoff is called...\n");

	return 0;
}

static int ioctl_streamon(struct v4l2_int_device *s)
{
	struct ce147_sensor *sensor = s->priv;  
	int ret;
	//struct ce147_sensor *sensor = &ce147;
	//struct i2c_client *client = sensor->i2c_client;
	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_streamon is called...(%x)\n", sensor->state);  

	if(sensor->state != CE147_STATE_CAPTURE)
	{
		
		dprintk(CAM_DBG, CE147_MOD_NAME "start preview....................\n");

		//ret = s5k4ba_write_regs(sd, sr200pc10_preview,SR200PC10_PREVIEW_REGS, "sr200pc10_preview"); // preview start
		//ret = s5k4ba_write_regs(client, sr200pc10_preview,SR200PC10_PREVIEW_REGS, "sr200pc10_preview"); // preview start

		//ce147_start_preview();
#if 1 // LYG
		if(sensor->check_dataline)
		{
			if(ce147_check_dataline())
				goto streamon_fail;
		}
		else
		{
			/* Zoom setting */
			//if(ce147_set_zoom(sensor->zoom))
				//goto streamon_fail;

			if(ce147_start_preview())
				goto streamon_fail;    
		}
#endif		
#if 0
		if(sensor->mode == CE147_MODE_CAMCORDER)    
		{
			/* Lens focus setting */
			if(ce147_set_focus(CE147_AF_INIT_CONTINUOUS))
				goto streamon_fail;       
		}
#endif
	}
	else
	{
		dprintk(CAM_DBG, CE147_MOD_NAME "start capture....................\n");
		if(ce147_start_capture())
			goto streamon_fail;
	}

	return 0;

streamon_fail:
	printk(CE147_MOD_NAME "ioctl_streamon is failed\n"); 
	return -EINVAL;   
}

/**
 * ioctl_queryctrl - V4L2 sensor interface handler for VIDIOC_QUERYCTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @qc: standard V4L2 VIDIOC_QUERYCTRL ioctl structure
 *
 * If the requested control is supported, returns the control information
 * from the ce147_ctrl_list[] array.
 * Otherwise, returns -EINVAL if the control is not supported.
 */
static int ioctl_queryctrl(struct v4l2_int_device *s,
		struct v4l2_queryctrl *qc)
{
	int i;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_queryctrl is called...\n");

	for (i = 0; i < NUM_CE147_CONTROL; i++) 
	{
		if (qc->id == ce147_ctrl_list[i].id)
		{
			break;
		}
	}
	if (i == NUM_CE147_CONTROL)
	{
		printk(CE147_MOD_NAME "Control ID is not supported!!\n");
		qc->flags = V4L2_CTRL_FLAG_DISABLED;
		goto queryctrl_fail;
	}

	*qc = ce147_ctrl_list[i];

	return 0;

queryctrl_fail:
	printk(CE147_MOD_NAME "ioctl_queryctrl is failed\n"); 
	return -EINVAL;     
}

#define CMD_VERSION			0x00
#define DATA_VERSION_FW		0x00
#define DATA_VERSION_DATE	0x01
#define DATA_VERSION_SENSOR	0x03
#define DATA_VERSION_AF		0x05
#define DATA_VERSION_SENSOR_MAKER 0xE0

/**
 * ioctl_g_ctrl - V4L2 sensor interface handler for VIDIOC_G_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_G_CTRL ioctl structure
 *
 * If the requested control is supported, returns the control's current
 * value from the ce13 sensor struct.
 * Otherwise, returns -EINVAL if the control is not supported.
 */
static int ioctl_g_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	struct ce147_sensor *sensor = s->priv;

	int retval = 0;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_g_ctrl is called...id(%x)\n", vc->id);

	switch (vc->id) 
	{
		case V4L2_CID_SELECT_MODE:
			vc->value = sensor->mode;
			break;  
		case V4L2_CID_SELECT_STATE:
			vc->value = sensor->state;
			break;       
		case V4L2_CID_FOCUS_MODE:
			vc->value = sensor->focus_mode;
			break;  
		case V4L2_CID_AF:
			retval = ce147_get_focus(vc);
			break;
		case V4L2_CID_ZOOM:
			retval = ce147_get_zoom(vc);
			break;
		case V4L2_CID_JPEG_SIZE:
			retval = ce147_get_jpeg_size(vc);
			break;
		case V4L2_CID_THUMBNAIL_SIZE:
			retval = ce147_get_thumbnail_size(vc);
			break;
		case V4L2_CID_JPEG_QUALITY:
			vc->value = sensor->jpeg_quality;
			break;
		case V4L2_CID_ISO:
			vc->value = sensor->iso;
			break;
		case V4L2_CID_BRIGHTNESS:
			vc->value = sensor->ev;
			break;
		case V4L2_CID_CONTRAST:
			vc->value = sensor->contrast;
			break; 
		case V4L2_CID_WB:
			vc->value = sensor->wb;
			break;      
		case V4L2_CID_SATURATION:
			vc->value = sensor->saturation;
			break;
		case V4L2_CID_EFFECT:
			vc->value = sensor->effect;
			break;
		case V4L2_CID_PHOTOMETRY:
			vc->value = sensor->photometry;
			break;
		case V4L2_CID_WDR:
			vc->value = sensor->wdr;
			break;
		case V4L2_CID_SHARPNESS:
			vc->value = sensor->sharpness;
			break;
		case V4L2_CID_ISC:
			vc->value = sensor->isc;
			break;
		case V4L2_CID_AEWB:
			vc->value = sensor->aewb;
			break;
		case V4L2_CID_ANTISHAKE:
			vc->value = sensor->antishake;  
			break;
		case V4L2_CID_SCENE:
			retval = ce147_get_scene(vc);
			break;      
		case V4L2_CID_FLASH_CAPTURE:
			vc->value = sensor->flash_capture;
			break;
		case V4L2_CID_FLASH_MOVIE:
			vc->value = sensor->flash_movie;
			break;
		case V4L2_CID_FW_THUMBNAIL_OFFSET:
			vc->value = sensor->thumb_offset;
			break;
		case V4L2_CID_FW_YUV_OFFSET:
			vc->value = sensor->yuv_offset;
			break;      
		case V4L2_CID_JPEG_CAPTURE_WIDTH:
			vc->value = sensor->jpeg_capture_w;
			break; 
		case V4L2_CID_JPEG_CAPTURE_HEIGHT:
			vc->value = sensor->jpeg_capture_h;
			break;
		case V4L2_CID_FW_VERSION:
			//FW_ver
			ISP_FW_ver[0] = CE147_FW_MINOR_VERSION;
			ISP_FW_ver[1] = CE147_FW_MAJOR_VERSION;
			ISP_FW_ver[2] = CE147_PRM_MINOR_VERSION;
			ISP_FW_ver[3] = CE147_PRM_MAJOR_VERSION;
			
			vc->value = ISP_FW_ver[3] | (ISP_FW_ver[2] << 8) | (ISP_FW_ver[1] << 16) | (ISP_FW_ver[0] << 24);
			break;
		case V4L2_CID_FW_LASTEST:
			vc->value = CE147_PRM_MAJOR_VERSION | (CE147_PRM_MINOR_VERSION << 8) |(CE147_FW_MAJOR_VERSION << 16) | (CE147_FW_MINOR_VERSION << 24);
			break;
		case V4L2_CID_FW_DATE:
			dprintk(CAM_INF, CE147_MOD_NAME "V4L2_CID_FW_DATE not supported!!!\n"); 
			retval = 0; //ce147_fw_date(vc);
			break;
		case V4L2_CID_CAM_DATE_INFO_YEAR:
			vc->value = sensor->dateinfo.year;
			break; 
		case V4L2_CID_CAM_DATE_INFO_MONTH:
			vc->value = sensor->dateinfo.month;
			break; 
		case V4L2_CID_CAM_DATE_INFO_DATE:
			vc->value = sensor->dateinfo.date;
			break; 
		case V4L2_CID_CAM_SENSOR_VER:
			vc->value = sensor->sensor_version;
			break; 
		case V4L2_CID_CAM_FW_MINOR_VER:
			vc->value = sensor->fw.minor;
			break; 
		case V4L2_CID_CAM_FW_MAJOR_VER:
			vc->value = sensor->fw.major;
			break; 
		case V4L2_CID_CAM_PRM_MINOR_VER:
			vc->value = sensor->prm.minor;
			break; 
		case V4L2_CID_CAM_PRM_MAJOR_VER:
			vc->value = sensor->prm.major;
			break; 
		case V4L2_CID_CAM_SENSOR_MAKER:
			vc->value = sensor->sensor_info.maker;
			break; 
		case V4L2_CID_CAM_SENSOR_OPTICAL:
			vc->value = sensor->sensor_info.optical;
			break; 		
		case V4L2_CID_CAM_AF_VER_LOW:
			vc->value = sensor->af_info.low;
			break; 
		case V4L2_CID_CAM_AF_VER_HIGH:
			vc->value = sensor->af_info.high;
			break; 	
		case V4L2_CID_CAM_GAMMA_RG_LOW:
			vc->value = sensor->gamma.rg_low;
			break; 
		case V4L2_CID_CAM_GAMMA_RG_HIGH:
			vc->value = sensor->gamma.rg_high;
			break; 		
		case V4L2_CID_CAM_GAMMA_BG_LOW:
			vc->value = sensor->gamma.bg_low;
			break; 
		case V4L2_CID_CAM_GAMMA_BG_HIGH:
			vc->value = sensor->gamma.bg_high;
			break; 	
		case V4L2_CID_CAM_GET_DUMP_SIZE:
			vc->value = sensor->fw_dump_size;
			break;		
		case V4L2_CID_MAIN_SW_DATE_INFO_YEAR:
			vc->value = sensor->main_sw_dateinfo.year;
			break; 
		case V4L2_CID_MAIN_SW_DATE_INFO_MONTH:
			vc->value = sensor->main_sw_dateinfo.month;
			break; 
		case V4L2_CID_MAIN_SW_DATE_INFO_DATE:
			vc->value = sensor->main_sw_dateinfo.date;
			break; 
		case V4L2_CID_MAIN_SW_FW_MINOR_VER:
			vc->value = sensor->main_sw_fw.minor;
			break; 
		case V4L2_CID_MAIN_SW_FW_MAJOR_VER:
			vc->value = sensor->main_sw_fw.major;
			break; 
		case V4L2_CID_MAIN_SW_PRM_MINOR_VER:
			vc->value = sensor->main_sw_prm.minor;
			break; 
		case V4L2_CID_MAIN_SW_PRM_MAJOR_VER:
			vc->value = sensor->main_sw_prm.major;
			break; 	  
		default:
			printk(CE147_MOD_NAME "[id]Invalid value is ordered!!!\n");
			break;
	}

	return retval;
}

static int ce147_set_face_lock(s32 value)
{
	struct ce147_sensor *sensor = &ce147;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_set_face_lock is called...[%d]\n",value);	

	return 0;
}

/**
 * ioctl_s_ctrl - V4L2 sensor interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_S_CTRL ioctl structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW (and updates the ce147 sensor struct).
 * Otherwise, * returns -EINVAL if the control is not supported.
 */
static int ioctl_s_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	struct ce147_sensor *sensor = s->priv;
	int retval = 0;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_s_ctrl is called...id(%x), value(%d)\n", vc->id, vc->value);

	switch (vc->id) 
	{
		case V4L2_CID_SELECT_MODE:
			retval = ce147_set_mode(vc->value);
			break;  
		case V4L2_CID_SELECT_STATE:
			retval = ce147_set_state(vc->value);
			break;       
		case V4L2_CID_FOCUS_MODE:
			retval = ce147_set_focus(vc->value);
			break;
		case V4L2_CID_AF:
			retval = ce147_set_focus_status(vc->value);
			break;
		case V4L2_CID_ZOOM:
			retval = ce147_set_zoom(vc->value);
			break;
		case V4L2_CID_JPEG_QUALITY:
			retval = ce147_set_jpeg_quality(vc->value);
			break;
		case V4L2_CID_ISO:
			retval = ce147_set_iso(vc->value);
			break;
		case V4L2_CID_BRIGHTNESS:
			retval = ce147_set_ev(vc->value);
			break;
		case V4L2_CID_CONTRAST:
			retval = ce147_set_contrast(vc->value);
			break;      
		case V4L2_CID_WB:
			retval = ce147_set_wb(vc->value);
			break;
		case V4L2_CID_SATURATION:
			retval = ce147_set_saturation(vc->value);
			break;
		case V4L2_CID_EFFECT:
			retval = ce147_set_effect(vc->value);
			break;
		case V4L2_CID_SCENE:
			retval = ce147_set_scene(vc->value);
			break;
		case V4L2_CID_PHOTOMETRY:
			retval = ce147_set_photometry(vc->value);
			break;
		case V4L2_CID_WDR:
			retval = ce147_set_wdr(vc->value);
			break;
		case V4L2_CID_SHARPNESS:
			retval = ce147_set_sharpness(vc->value);
			break;
		case V4L2_CID_ISC:
			retval = ce147_set_isc(vc->value);
			break;
		case V4L2_CID_AEWB:
			retval = ce147_set_aewb(vc->value);
			break;
		case V4L2_CID_ANTISHAKE:
			retval = ce147_set_antishake(vc->value);
			break;      
		case V4L2_CID_FW_UPDATE:
			dprintk(CAM_INF, CE147_MOD_NAME "V4L2_CID_FW_UPDATE not supported!!!\n"); 
			retval = 0;
			break;
		case V4L2_CID_FLASH_CAPTURE:
			retval = ce147_set_flash_capture(vc->value);
			break;
		case V4L2_CID_FLASH_MOVIE:
			retval = ce147_set_flash_movie(vc->value);
			break;
		case V4L2_CID_CAM_FW_VER:
			dprintk(CAM_INF, CE147_MOD_NAME "V4L2_CID_CAM_FW_VER not supported!!!\n"); 
			retval = 0; //ce147_get_fw_data(s);
			break;
		case V4L2_CID_CAM_DUMP_FW:
			retval = 0; //ce147_dump_fw(s);
			break;	  
		case V4L2_CID_CAMERA_CHECK_DATALINE:
			sensor->check_dataline = vc->value;
			retval = 0;
			break;	
		case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
			retval = ce147_check_dataline_stop();
			break;
		case V4L2_CID_CAMERA_OBJECT_POSITION_X:
			sensor->position.x = vc->value;
			retval = 0;
			break;
		case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
			sensor->position.y = vc->value;
			retval = 0;
			break;
		case V4L2_CID_CAMERA_AE_AWB_LOCKUNLOCK:
			retval = ce147_set_aewb(vc->value);
			break;
		case V4L2_CID_CAMERA_FACEDETECT_LOCKUNLOCK:
			retval = ce147_set_face_lock(vc->value);
			break;
		case V4L2_CID_CAMERA_TOUCH_AF_START_STOP:
			retval = ce147_set_focus_touch(vc->value);
			break; 
		default:
			printk(CE147_MOD_NAME "[id]Invalid value is ordered!!!\n");
			break;
	}

	return retval;
}

/**
 * ioctl_enum_fmt_cap - Implement the CAPTURE buffer VIDIOC_ENUM_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @fmt: standard V4L2 VIDIOC_ENUM_FMT ioctl structure
 *
 * Implement the VIDIOC_ENUM_FMT ioctl for the CAPTURE buffer type.
 */
static int ioctl_enum_fmt_cap(struct v4l2_int_device *s, struct v4l2_fmtdesc *fmt)
{
	int index = 0;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_enum_fmt_cap is called...\n");

	switch (fmt->type) 
	{
		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			switch(fmt->pixelformat)
			{
				case V4L2_PIX_FMT_UYVY:
					index = 0;
					break;

				case V4L2_PIX_FMT_YUYV:
					index = 1;
					break;

				case V4L2_PIX_FMT_JPEG:
					index = 2;
					break;

				case V4L2_PIX_FMT_MJPEG:
					index = 3;
					break;

				default:
					printk(CE147_MOD_NAME "[format]Invalid value is ordered!!!\n");
					goto enum_fmt_cap_fail;
			}
			break;

		default:
			printk(CE147_MOD_NAME "[type]Invalid value is ordered!!!\n");
			goto enum_fmt_cap_fail;
	}

	fmt->flags = ce147_formats[index].flags;
	fmt->pixelformat = ce147_formats[index].pixelformat;
	strlcpy(fmt->description, ce147_formats[index].description, sizeof(fmt->description));

	dprintk(CAM_DBG, CE147_MOD_NAME "ioctl_enum_fmt_cap flag : %d\n", fmt->flags);
	dprintk(CAM_DBG, CE147_MOD_NAME "ioctl_enum_fmt_cap description : %s\n", fmt->description);

	return 0;

enum_fmt_cap_fail:
	printk(CE147_MOD_NAME "ioctl_enum_fmt_cap is failed\n"); 
	return -EINVAL;     
}

/**
 * ioctl_try_fmt_cap - Implement the CAPTURE buffer VIDIOC_TRY_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 VIDIOC_TRY_FMT ioctl structure
 *
 * Implement the VIDIOC_TRY_FMT ioctl for the CAPTURE buffer type.  This
 * ioctl is used to negotiate the image capture size and pixel format
 * without actually making it take effect.
 */
static int ioctl_try_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct ce147_sensor *sensor = s->priv;
	struct v4l2_pix_format *pix2 = &sensor->pix;

	int index = 0;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_try_fmt_cap is called...\n");
	dprintk(CAM_DBG, CE147_MOD_NAME "ioctl_try_fmt_cap. mode : %d\n", sensor->mode);
	dprintk(CAM_DBG, CE147_MOD_NAME "ioctl_try_fmt_cap. state : %d\n", sensor->state);

	ce147_set_skip();  

	if(sensor->state == CE147_STATE_CAPTURE)
	{ 
		for(index = 0; index < ARRAY_SIZE(ce147_image_sizes); index++)
		{
			if(ce147_image_sizes[index].width == pix->width && ce147_image_sizes[index].height == pix->height)
			{
				sensor->capture_size = index;
				break;
			}
		}   

		if(index == ARRAY_SIZE(ce147_image_sizes))
		{
			printk(CE147_MOD_NAME "Capture Image Size is not supported!\n");
			goto try_fmt_fail;
		}

		dprintk(CAM_DBG, CE147_MOD_NAME "CE147--capture size = %d\n", sensor->capture_size);  
		dprintk(CAM_DBG, CE147_MOD_NAME "CE147--capture width : %d\n", ce147_image_sizes[index].width);
		dprintk(CAM_DBG, CE147_MOD_NAME "CE147--capture height : %d\n", ce147_image_sizes[index].height);      

		if(pix->pixelformat == V4L2_PIX_FMT_UYVY || pix->pixelformat == V4L2_PIX_FMT_YUYV)
		{
			pix->field = V4L2_FIELD_NONE;
			pix->bytesperline = pix->width * 2;
			pix->sizeimage = pix->bytesperline * pix->height;
			dprintk(CAM_DBG, CE147_MOD_NAME "V4L2_PIX_FMT_UYVY\n");
		}
		else
		{
			pix->field = V4L2_FIELD_NONE;
			pix->bytesperline = JPEG_CAPTURE_WIDTH * 2;
			pix->sizeimage = pix->bytesperline * JPEG_CAPTURE_HEIGHT;
			dprintk(CAM_DBG, CE147_MOD_NAME "V4L2_PIX_FMT_JPEG\n");
		}

		dprintk(CAM_DBG, CE147_MOD_NAME "set capture....................\n");

		if(ce147_set_capture())
			goto try_fmt_fail;
	}  

	switch (pix->pixelformat) 
	{
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_JPEG:
		case V4L2_PIX_FMT_MJPEG:
			pix->colorspace = V4L2_COLORSPACE_JPEG;
			break;
		case V4L2_PIX_FMT_RGB565:
		case V4L2_PIX_FMT_RGB565X:
		case V4L2_PIX_FMT_RGB555:
		case V4L2_PIX_FMT_SGRBG10:
		case V4L2_PIX_FMT_RGB555X:
		default:
			pix->colorspace = V4L2_COLORSPACE_SRGB;
			break;
	}

	*pix2 = *pix;

	return 0;

try_fmt_fail:
	printk(CE147_MOD_NAME "ioctl_try_fmt_cap is failed\n"); 
	return -EINVAL;

}

/**
 * ioctl_s_fmt_cap - V4L2 sensor interface handler for VIDIOC_S_FMT ioctl
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 VIDIOC_S_FMT ioctl structure
 *
 * If the requested format is supported, configures the HW to use that
 * format, returns error code if format not supported or HW can't be
 * correctly configured.
 */
static int ioctl_s_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct ce147_sensor *sensor = s->priv;
	struct v4l2_pix_format *pix2 = &sensor->pix;

	int index = 0;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_s_fmt_cap is called...\n");

	ce147_720p_enable = false;

	dprintk(CAM_DBG, CE147_MOD_NAME  "camera mode  : %d (1:camera , 2:camcorder, 3:vt)\n", sensor->mode);
	dprintk(CAM_DBG, CE147_MOD_NAME  "camera state : %d (0:preview, 1:snapshot)\n", sensor->state);
	dprintk(CAM_DBG, CE147_MOD_NAME  "set width  : %d\n", pix->width);
	dprintk(CAM_DBG, CE147_MOD_NAME  "set height : %d\n", pix->height); 

	if(sensor->state == CE147_STATE_CAPTURE)
	{ 
		/* check for capture */
		if(ce147_prepare_capture())
			goto s_fmt_fail;   

		ce147_set_skip();  

		for(index = 0; index < ARRAY_SIZE(ce147_image_sizes); index++)
		{
			if(ce147_image_sizes[index].width == pix->width && ce147_image_sizes[index].height == pix->height)
			{
				sensor->capture_size = index;
				break;
			}
		}   

		if(index == ARRAY_SIZE(ce147_image_sizes))
		{
			printk(CE147_MOD_NAME "Capture Image %d x %d Size is not supported!\n", pix->width, pix->height);
			goto s_fmt_fail;
		}

		dprintk(CAM_DBG, CE147_MOD_NAME "CE147--capture size = %d\n", sensor->capture_size);  
		dprintk(CAM_DBG, CE147_MOD_NAME "CE147--capture width : %d\n", ce147_image_sizes[index].width);
		dprintk(CAM_DBG, CE147_MOD_NAME "CE147--capture height : %d\n", ce147_image_sizes[index].height);      

		if(pix->pixelformat == V4L2_PIX_FMT_UYVY || pix->pixelformat == V4L2_PIX_FMT_YUYV)
		{
			pix->field = V4L2_FIELD_NONE;
			pix->bytesperline = pix->width * 2;
			pix->sizeimage = pix->bytesperline * pix->height;
			dprintk(CAM_DBG, CE147_MOD_NAME "V4L2_PIX_FMT_UYVY\n");
		}
		else
		{
			pix->field = V4L2_FIELD_NONE;
			pix->bytesperline = JPEG_CAPTURE_WIDTH * 2;
			pix->sizeimage = pix->bytesperline * JPEG_CAPTURE_HEIGHT;
			dprintk(CAM_DBG, CE147_MOD_NAME "V4L2_PIX_FMT_JPEG\n");
		}

		if(ce147_set_capture())
			goto s_fmt_fail;
	}  
	else
	{  
		/* check for preview */
		if(ce147_prepare_preview())
			goto s_fmt_fail;

		//    ce147_set_skip();  

		for(index = 0; index < ARRAY_SIZE(ce147_preview_sizes); index++)
		{
			if(ce147_preview_sizes[index].width == pix->width && ce147_preview_sizes[index].height == pix->height)
			{
				sensor->preview_size = index;
				break;
			}
		}   

		if(index == ARRAY_SIZE(ce147_preview_sizes))
		{
			printk(CE147_MOD_NAME "Preview Image %d x %d Size is not supported!\n", pix->width, pix->height);
			goto s_fmt_fail;
		}

		if(sensor->mode == CE147_MODE_CAMCORDER)
		{
			if(pix->width == 1280 && pix->height == 720)
			{
				dprintk(CAM_DBG, CE147_MOD_NAME "Preview Image Size is 720P!\n");
				ce147_720p_enable = true;
			}
		}

		dprintk(CAM_DBG, CE147_MOD_NAME "CE147--preview size = %d\n", sensor->preview_size); 
		dprintk(CAM_DBG, CE147_MOD_NAME "CE147--preview width : %d\n", ce147_preview_sizes[index].width);
		dprintk(CAM_DBG, CE147_MOD_NAME "CE147--preview height : %d\n", ce147_preview_sizes[index].height);      

		pix->field = V4L2_FIELD_NONE;
		pix->bytesperline = pix->width * 2;
		pix->sizeimage = pix->bytesperline * pix->height;  
		dprintk(CAM_DBG, CE147_MOD_NAME "V4L2_PIX_FMT_UYVY\n");

		if(ce147_set_preview())
			goto s_fmt_fail;
	}      

	switch (pix->pixelformat) 
	{
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_JPEG:
		case V4L2_PIX_FMT_MJPEG:
			pix->colorspace = V4L2_COLORSPACE_JPEG;
			break;
		case V4L2_PIX_FMT_RGB565:
		case V4L2_PIX_FMT_RGB565X:
		case V4L2_PIX_FMT_RGB555:
		case V4L2_PIX_FMT_SGRBG10:
		case V4L2_PIX_FMT_RGB555X:
		default:
			pix->colorspace = V4L2_COLORSPACE_SRGB;
			break;
	}

	*pix2 = *pix;

	return 0;

s_fmt_fail:
	printk(CE147_MOD_NAME "ioctl_s_fmt_cap is failed\n"); 
	return -EINVAL;  
}

/**
 * ioctl_g_fmt_cap - V4L2 sensor interface handler for ioctl_g_fmt_cap
 * @s: pointer to standard V4L2 device structure
 * @f: pointer to standard V4L2 v4l2_format structure
 *
 * Returns the sensor's current pixel format in the v4l2_format
 * parameter.
 */
static int ioctl_g_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct ce147_sensor *sensor = s->priv;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_g_fmt_cap is called...\n");

	f->fmt.pix = sensor->pix;

	return 0;
}

/**
 * ioctl_g_parm - V4L2 sensor interface handler for VIDIOC_G_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_G_PARM ioctl structure
 *
 * Returns the sensor's video CAPTURE parameters.
 */
static int ioctl_g_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct ce147_sensor *sensor = s->priv;
	struct v4l2_captureparm *cparm = &a->parm.capture;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_g_parm is called...\n");

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
	{
		printk(CE147_MOD_NAME "ioctl_g_parm type not supported.\n");
		goto g_parm_fail;
	}

	memset(a, 0, sizeof(*a));
	a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;
	cparm->timeperframe = sensor->timeperframe;

	return 0;

g_parm_fail:
	printk(CE147_MOD_NAME "ioctl_g_parm is failed\n"); 
	return -EINVAL;  
}

/**
 * ioctl_s_parm - V4L2 sensor interface handler for VIDIOC_S_PARM ioctl
 * @s: pointer to standard V4L2 device structure
 * @a: pointer to standard V4L2 VIDIOC_S_PARM ioctl structure
 *
 * Configures the sensor to use the input parameters, if possible.  If
 * not possible, reverts to the old parameters and returns the
 * appropriate error code.
 */
static int ioctl_s_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct ce147_sensor *sensor = s->priv;
	struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_s_parm is called...\n");

	/* Set mode (camera/camcorder/vt) & state (preview/capture) */
	sensor->mode = a->parm.capture.capturemode;
	sensor->state = a->parm.capture.currentstate;

	if(sensor->mode < 1 || sensor->mode > 3) sensor->mode = CE147_MODE_CAMERA;
	dprintk(CAM_DBG, CE147_MOD_NAME "mode = %d, state = %d\n", sensor->mode, sensor->state);   

	/* Set time per frame (FPS) */
	if((timeperframe->numerator == 0)&&(timeperframe->denominator == 0))
	{
		sensor->fps = 30;
	}
	else
	{
		sensor->fps = timeperframe->denominator / timeperframe->numerator;
	}
	sensor->timeperframe = *timeperframe;
	dprintk(CAM_DBG, CE147_MOD_NAME "fps = %d\n", sensor->fps);  

	return 0;
}

/**
 * ioctl_g_ifparm - V4L2 sensor interface handler for vidioc_int_g_ifparm_num
 * @s: pointer to standard V4L2 device structure
 * @p: pointer to standard V4L2 vidioc_int_g_ifparm_num ioctl structure
 *
 * Gets slave interface parameters.
 * Calculates the required xclk value to support the requested
 * clock parameters in p.  This value is returned in the p
 * parameter.
 */
static int ioctl_g_ifparm(struct v4l2_int_device *s, struct v4l2_ifparm *p)
{
	struct ce147_sensor *sensor = s->priv;
	int rval;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_g_ifparm is called...\n");

	rval = sensor->pdata->ifparm(p);
	if (rval)
	{
		return rval;
	}

	p->u.bt656.clock_curr = CE147_XCLK;

	return 0;
}

/**
 * ioctl_g_priv - V4L2 sensor interface handler for vidioc_int_g_priv_num
 * @s: pointer to standard V4L2 device structure
 * @p: void pointer to hold sensor's private data address
 *
 * Returns device's (sensor's) private data area address in p parameter
 */
static int ioctl_g_priv(struct v4l2_int_device *s, void *p)
{
	struct ce147_sensor *sensor = s->priv;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_g_priv is called...\n");

	if(p == NULL)
	{
		printk(CE147_MOD_NAME "ioctl_g_priv is failed because of null pointer\n"); 
		return -EINVAL;
	}

	return sensor->pdata->priv_data_set(p);
}


/* added following functins for v4l2 compatibility with omap34xxcam */

/**
 * ioctl_enum_framesizes - V4L2 sensor if handler for vidioc_int_enum_framesizes
 * @s: pointer to standard V4L2 device structure
 * @frms: pointer to standard V4L2 framesizes enumeration structure
 *
 * Returns possible framesizes depending on choosen pixel format
 **/
static int ioctl_enum_framesizes(struct v4l2_int_device *s, struct v4l2_frmsizeenum *frms)
{
	struct ce147_sensor* sensor = s->priv;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_enum_framesizes called...\n");   

	if (sensor->state == CE147_STATE_CAPTURE)
	{    
		dprintk(CAM_DBG, CE147_MOD_NAME "Size enumeration for image capture size = %d\n", sensor->capture_size);

		if(sensor->preview_size == ARRAY_SIZE(ce147_image_sizes))
			goto enum_framesizes_fail;

		frms->index = sensor->capture_size;
		frms->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		frms->discrete.width = ce147_image_sizes[sensor->capture_size].width;
		frms->discrete.height = ce147_image_sizes[sensor->capture_size].height;        
	}
	else
	{
		dprintk(CAM_DBG, CE147_MOD_NAME "Size enumeration for image preview size = %d\n", sensor->preview_size);

		if(sensor->preview_size == ARRAY_SIZE(ce147_preview_sizes))
			goto enum_framesizes_fail;

		frms->index = sensor->preview_size;
		frms->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		frms->discrete.width = ce147_preview_sizes[sensor->preview_size].width;
		frms->discrete.height = ce147_preview_sizes[sensor->preview_size].height;        
	}

	dprintk(CAM_DBG, CE147_MOD_NAME "framesizes width : %d\n", frms->discrete.width); 
	dprintk(CAM_DBG, CE147_MOD_NAME "framesizes height : %d\n", frms->discrete.height); 

	return 0;

enum_framesizes_fail:
	printk(CE147_MOD_NAME "ioctl_enum_framesizes is failed\n"); 
	return -EINVAL;   
}

static int ioctl_enum_frameintervals(struct v4l2_int_device *s, struct v4l2_frmivalenum *frmi)
{
	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_enum_frameintervals \n"); 
	dprintk(CAM_DBG, CE147_MOD_NAME "ioctl_enum_frameintervals numerator : %d\n", frmi->discrete.numerator); 
	dprintk(CAM_DBG, CE147_MOD_NAME "ioctl_enum_frameintervals denominator : %d\n", frmi->discrete.denominator); 

	return 0;
}


/**
 * ioctl_s_power - V4L2 sensor interface handler for vidioc_int_s_power_num
 * @s: pointer to standard V4L2 device structure
 * @on: power state to which device is to be set
 *
 * Sets devices power state to requrested state, if possible.
 */
  static int power_Enable =0;
static int ioctl_s_power(struct v4l2_int_device *s, enum v4l2_power on)
{
	struct ce147_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;
	int status;
	int err;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_s_power is called......ON=%x, detect= %x\n", on, sensor->detect);

	 if(power_Enable ==0||power_Enable ==1){
  		power_Enable++;
		return 0;
  	}
//idle current optimisation 
	if(on == V4L2_POWER_ON)
                 back_cam_in_use= 1 ;
	else if(on == V4L2_POWER_OFF)
		  back_cam_in_use = 0 ;
		
//idle current optimisation 
		
/*
	if(on == V4L2_POWER_OFF)
	{
		ce147_write_reg(client, sizeof(Lense_AFoff_list), Lense_AFoff_list);
		//mdelay(200);
	}  
*/
	if(sensor->pdata->power_set(on))
	{
		printk(CE147_MOD_NAME "Can not power on/off " CE147_DRIVER_NAME " sensor\n"); 
		goto s_power_fail;
	}

	switch(on)
	{
		case V4L2_POWER_ON:
			{
				dprintk(CAM_DBG, CE147_MOD_NAME "pwr on-----!\n");
				//if(ce147_detect(client))
				//if(s5k4ca_sensor_get_id(client)!=0) //LYG
				if(s5k5bbgx_senser_info(client)!=0) //LYG
				{
					printk(CE147_MOD_NAME "Unable to detect " CE147_DRIVER_NAME " sensor\n");
					sensor->pdata->power_set(V4L2_POWER_OFF);
					goto s_power_fail;
				}
#ifdef CONFIG_LOAD_FILE
				err = loadFile();
				if (unlikely(err)) {
					printk("%s: failed to init\n", __func__);
					return err;
				}
				status = s5k5bbgx_write_tuningmode(client, "mode_sensor_init");
#else
				//status = s5k5bbgx_write_table(client, mode_table[0]);
				status = S5K5BBGX_BURST_WRITE_LIST(mode_sensor_init);
#endif
				if (status < 0)
				{
					printk(CE147_MOD_NAME "mode_table failed==== " CE147_DRIVER_NAME " sensor\n");
					sensor->pdata->power_set(V4L2_POWER_OFF);
					goto s_power_fail;
				}
				else
				 	printk(CE147_MOD_NAME "s5k5bbgx_write_table mode table== success " CE147_DRIVER_NAME " sensor\n");	
					
				//err = s5k4ba_write_regs(sd, sr200pc10_init0, SR200PC10_INIT0_REGS, "sr200pc10_init0");//LYG
				//s5k4ba_write_regs(client, sr200pc10_init0, SR200PC10_INIT0_REGS, "sr200pc10_init0");//LYG

				/* Make the default detect */
				sensor->detect = SENSOR_DETECTED;     
				
				/* Make the state init */
				ce147_curr_state = CE147_STATE_INVALID;
			}
			break;

		case V4L2_POWER_RESUME:
			{
				dprintk(CAM_DBG, CE147_MOD_NAME "pwr resume-----!\n");
			}  
			break;

		case V4L2_POWER_STANDBY:
			{
				dprintk(CAM_DBG, CE147_MOD_NAME "pwr stanby-----!\n");
			}
			break;

		case V4L2_POWER_OFF:
			{
				dprintk(CAM_DBG, CE147_MOD_NAME "pwr off-----!\n");

				/* Make the default detect */
				sensor->detect = SENSOR_NOT_DETECTED;  

				/* Make the state init */
				ce147_pre_state = CE147_STATE_INVALID;
#ifdef CONFIG_LOAD_FILE				
				kfree(testBuf);
#endif
				  
			}
			break;
	}

	return 0;

s_power_fail:
//idle current optimisation
	back_cam_in_use= 0 ;
//idle current optimisation	
	printk(CE147_MOD_NAME "ioctl_s_power is failed\n");
	return -EINVAL;
}



static int ioctl_g_exif(struct v4l2_int_device *s, struct v4l2_exif *exif)
{
	struct ce147_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;

	u8 Write_Exposure_info[3] = {0x17,0x00,0x00};
	u8 Read_Capture_Data[8] = {0x00,};

	//dprintk(CAM_INF, CE147_MOD_NAME "ioctl_g_exif is called...\n");
	exif->TV_Value =  exif_info.T_value;

	exif->shutter_speed_numerator = exif_info.info_exptime_numer;
	exif->shutter_speed_denominator = exif_info.info_exptime_denumer;

	exif->iso = exif_info.info_iso;
	//dprintk(CAM_INF, CE147_MOD_NAME "shutter_speed_numerator = %d, shutter_speed_denominator = %d, iso = %d\n",exif->shutter_speed_numerator,exif->shutter_speed_denominator,exif->iso);
	
#if 0
	if(ce147_write_read_reg(client, sizeof(Write_Exposure_info), Write_Exposure_info, sizeof(Read_Capture_Data), Read_Capture_Data))
		goto g_exif_fail;

	dprintk(CAM_DBG, CE147_MOD_NAME "Capture Data [0]: 0x%04x, [1]: 0x%04x, [2]: 0x%04x\n", Read_Capture_Data[0], Read_Capture_Data[1], Read_Capture_Data[2]);
	dprintk(CAM_DBG, CE147_MOD_NAME "Capture Data [3]: 0x%04x, [4]: 0x%04x, [5]: 0x%04x\n", Read_Capture_Data[3], Read_Capture_Data[4], Read_Capture_Data[5]);      

	exif->TV_Value =     Read_Capture_Data[0]|(Read_Capture_Data[1]*256);
	exif->SV_Value =     Read_Capture_Data[2]|(Read_Capture_Data[3]*256);
	exif->AV_Value =     Read_Capture_Data[4]|(Read_Capture_Data[5]*256);
	exif->BV_Value =     Read_Capture_Data[6]|(Read_Capture_Data[7]*256);

	dprintk(CAM_DBG, CE147_MOD_NAME "aperture Data : 0x%x, TV_Value: 0x%x,  SV_Value: 0x%x flash: 0x%x\n",
			exif->aperture_numerator, exif->TV_Value, exif->SV_Value, exif->aperture_denominator );
#endif
	return 0;

g_exif_fail:
	printk(CE147_MOD_NAME "ioctl_g_exif is failed\n");
	return -EINVAL;  
}

/**
 * ioctl_deinit - V4L2 sensor interface handler for VIDIOC_INT_DEINIT
 * @s: pointer to standard V4L2 device structure
 *
 * Deinitialize the sensor device
 */
static int ioctl_deinit(struct v4l2_int_device *s)
{
	struct ce147_sensor *sensor = s->priv;

	dprintk(CAM_INF, "ioctl_deinit is called...\n");

	sensor->state = CE147_STATE_INVALID; //init problem

	return 0;
}


/**
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 *
 * Initialize the sensor device (call ce147_configure())
 */
static int ioctl_init(struct v4l2_int_device *s)
{
	struct ce147_sensor *sensor = s->priv;

	dprintk(CAM_INF, CE147_MOD_NAME "ioctl_init is called...\n");

	//init value
	sensor->timeperframe.numerator    = 1;
	sensor->timeperframe.denominator  = 25;
	sensor->fps                       = 25;
	sensor->bv                        = 0;
	sensor->state                     = CE147_STATE_INVALID;
	sensor->mode                      = CE147_MODE_CAMERA;
	sensor->preview_size              = CE147_PREVIEW_SIZE_640_480;
	sensor->capture_size              = CE147_IMAGE_SIZE_2560_1920;
	sensor->detect                    = SENSOR_NOT_DETECTED;
	sensor->focus_mode                = CE147_AF_INIT_NORMAL;
	sensor->effect                    = CE147_EFFECT_OFF;
	sensor->iso                       = CE147_ISO_AUTO;
	sensor->photometry                = CE147_PHOTOMETRY_CENTER;
	sensor->ev                        = CE147_EV_DEFAULT;
	sensor->wdr                       = CE147_WDR_OFF;
	sensor->contrast                  = CE147_CONTRAST_DEFAULT;
	sensor->saturation                = CE147_SATURATION_DEFAULT;
	sensor->sharpness                 = CE147_SHARPNESS_DEFAULT;
	sensor->wb                        = CE147_WB_AUTO;
	sensor->isc                       = CE147_ISC_STILL_OFF;
	sensor->scene                     = CE147_SCENE_OFF;
	sensor->aewb                      = CE147_AE_UNLOCK_AWB_UNLOCK;
	sensor->antishake                 = CE147_ANTI_SHAKE_OFF;
	sensor->flash_capture             = CE147_FLASH_CAPTURE_OFF;
	sensor->flash_movie               = CE147_FLASH_MOVIE_OFF;
	sensor->jpeg_quality              = CE147_JPEG_SUPERFINE;
	sensor->zoom                      = CE147_ZOOM_1P00X;
	sensor->thumb_offset              = CE147_THUMBNAIL_OFFSET;
	sensor->yuv_offset                = CE147_YUV_OFFSET;
	sensor->jpeg_capture_w            = JPEG_CAPTURE_WIDTH;
	sensor->jpeg_capture_h            = JPEG_CAPTURE_HEIGHT;  

	memcpy(&ce147, sensor, sizeof(struct ce147_sensor));

	return 0;
}

static struct v4l2_int_ioctl_desc ce147_ioctl_desc[] = {
	{ .num = vidioc_int_enum_framesizes_num,
		.func = (v4l2_int_ioctl_func *)ioctl_enum_framesizes},
	{ .num = vidioc_int_enum_frameintervals_num,
		.func = (v4l2_int_ioctl_func *)ioctl_enum_frameintervals},
	{ .num = vidioc_int_s_power_num,
		.func = (v4l2_int_ioctl_func *)ioctl_s_power },
	{ .num = vidioc_int_g_priv_num,
		.func = (v4l2_int_ioctl_func *)ioctl_g_priv },
	{ .num = vidioc_int_g_ifparm_num,
		.func = (v4l2_int_ioctl_func *)ioctl_g_ifparm },
	{ .num = vidioc_int_init_num,
		.func = (v4l2_int_ioctl_func *)ioctl_init },
	{ .num = vidioc_int_deinit_num,
		.func = (v4l2_int_ioctl_func *)ioctl_deinit },
	{ .num = vidioc_int_enum_fmt_cap_num,
		.func = (v4l2_int_ioctl_func *)ioctl_enum_fmt_cap },
	{ .num = vidioc_int_try_fmt_cap_num,
		.func = (v4l2_int_ioctl_func *)ioctl_try_fmt_cap },
	{ .num = vidioc_int_g_fmt_cap_num,
		.func = (v4l2_int_ioctl_func *)ioctl_g_fmt_cap },
	{ .num = vidioc_int_s_fmt_cap_num,
		.func = (v4l2_int_ioctl_func *)ioctl_s_fmt_cap },
	{ .num = vidioc_int_g_parm_num,
		.func = (v4l2_int_ioctl_func *)ioctl_g_parm },
	{ .num = vidioc_int_s_parm_num,
		.func = (v4l2_int_ioctl_func *)ioctl_s_parm },
	{ .num = vidioc_int_queryctrl_num,
		.func = (v4l2_int_ioctl_func *)ioctl_queryctrl },
	{ .num = vidioc_int_g_ctrl_num,
		.func = (v4l2_int_ioctl_func *)ioctl_g_ctrl },
	{ .num = vidioc_int_s_ctrl_num,
		.func = (v4l2_int_ioctl_func *)ioctl_s_ctrl },
	{ .num = vidioc_int_streamon_num,
		.func = (v4l2_int_ioctl_func *)ioctl_streamon },
	{ .num = vidioc_int_streamoff_num,
		.func = (v4l2_int_ioctl_func *)ioctl_streamoff },
	{ .num = vidioc_int_g_exif_num,
		.func = (v4l2_int_ioctl_func *)ioctl_g_exif },    
};

static struct v4l2_int_slave ce147_slave = {
	.ioctls = ce147_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(ce147_ioctl_desc),
};

static struct v4l2_int_device ce147_int_device = {
	.module = THIS_MODULE,
	.name = CE147_DRIVER_NAME,
	.priv = &ce147,
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &ce147_slave,
	},
};


/**
 * ce147_probe - sensor driver i2c probe handler
 * @client: i2c driver client device structure
 *
 * Register sensor as an i2c client device and V4L2
 * device.
 */
 extern u32 hw_revision;
static int ce147_probe(struct i2c_client *client, const struct i2c_device_id *device)
{
	struct ce147_sensor *sensor = &ce147;
	
	dprintk(CAM_INF, "YGLEE start OK ce147_probe is called...\n");

	if(hw_revision<=0x03){
		dprintk(CAM_INF, "YGLEE start OK sr200pc10_sensor_init is return...\n");
		return -1;
	}
	
	if (i2c_get_clientdata(client))
	{
		printk(CE147_MOD_NAME "can't get i2c client data!!\n");
		return -EBUSY;
	}

	sensor->pdata = &nowplus_ce147_platform_data;

	if (!sensor->pdata) 
	{
		printk(CE147_MOD_NAME "no platform data!!\n");
		return -ENODEV;
	}

	sensor->v4l2_int_device = &ce147_int_device;
	sensor->i2c_client = client;

	/* Make the default capture size VGA */
	sensor->pix.width = 640;
	sensor->pix.height = 480; 

	/* Make the default capture format V4L2_PIX_FMT_UYVY */
	sensor->pix.pixelformat = V4L2_PIX_FMT_UYVY;
	 
	i2c_set_clientdata(client, sensor);

	if (v4l2_int_device_register(sensor->v4l2_int_device))
	{
		printk(CE147_MOD_NAME "fail to init device register \n");
		i2c_set_clientdata(client, NULL);
	}

	return 0;
}

/**
 * ce147_remove - sensor driver i2c remove handler
 * @client: i2c driver client device structure
 *
 * Unregister sensor as an i2c client device and V4L2
 * device.  Complement of ce147_probe().
 */
static int __exit ce147_remove(struct i2c_client *client)
{
	struct ce147_sensor *sensor = i2c_get_clientdata(client);

	dprintk(CAM_INF, CE147_MOD_NAME "ce147_remove is called...\n");

	if (!client->adapter)
	{
		printk(CE147_MOD_NAME "no i2c client adapter!!");
		return -ENODEV; /* our client isn't attached */
	}

	v4l2_int_device_unregister(sensor->v4l2_int_device);
	i2c_set_clientdata(client, NULL);

	return 0;
}

static const struct i2c_device_id ce147_id[] = {
	{ CE147_DRIVER_NAME, 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, ce147_id);


static struct i2c_driver ce147sensor_i2c_driver = {
	.driver = {
		.name = CE147_DRIVER_NAME,
	},
	.probe = ce147_probe,
	.remove = __exit_p(ce147_remove),
	.id_table = ce147_id,
};

/**
 * ce147_sensor_init - sensor driver module_init handler
 *
 * Registers driver as an i2c client driver.  Returns 0 on success,
 * error code otherwise.
 */
static int __init ce147_sensor_init(void)
{
	int err;

	dprintk(CAM_INF, "start OK ce147_sensor_init is called.hw_revision=0x%02x..\n",hw_revision);
	if(hw_revision<=0x03){
		dprintk(CAM_INF, "start OK ce147_sensor_init is return...\n");
		return -1;
	}

	err = i2c_add_driver(&ce147sensor_i2c_driver);
	if (err) 
	{
		printk(CE147_MOD_NAME "Failed to register" CE147_DRIVER_NAME ".\n");
		return err;
	}

	return 0;
}

module_init(ce147_sensor_init);

/**
 * ce147sensor_cleanup - sensor driver module_exit handler
 *
 * Unregisters/deletes driver as an i2c client driver.
 * Complement of ce147_sensor_init.
 */
static void __exit ce147sensor_cleanup(void)
{
	i2c_del_driver(&ce147sensor_i2c_driver);
}
module_exit(ce147sensor_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CE147 camera sensor driver");
