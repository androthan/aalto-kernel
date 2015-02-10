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
 * modules/camera/sr200pc10.c
 *
 * SR200PC10 sensor driver source file
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
#include "s5k4ca.h"
#include "ce147.h"
extern bool back_cam_in_use;
#if (CAM_SR200PC10_DBG_MSG)
#include "dprintk.h"
#else
#define dprintk(x, y...)
#endif

#define I2C_M_WRITE 0x0000 /* write data, from slave to master */
#define I2C_M_READ  0x0001 /* read data, from slave to master */

static u32 sr200pc10_curr_state = SR200PC10_STATE_INVALID;
static u32 sr200pc10_pre_state = SR200PC10_STATE_INVALID;

static bool sr200pc10_720p_enable = false;
static bool sr200pc10_touch_state = false;

static struct sr200pc10_sensor sr200pc10 = {
	.timeperframe = {
		.numerator    = 1,
		.denominator  = 30,
	},
	.fps            = 30,
	.bv             = 0,
	.state          = SR200PC10_STATE_PREVIEW,
	.mode           = SR200PC10_MODE_CAMERA,
	.preview_size   = SR200PC10_PREVIEW_SIZE_640_480,
	.capture_size   = SR200PC10_IMAGE_SIZE_2560_1920,
	.detect         = SENSOR_NOT_DETECTED,
	.focus_mode     = SR200PC10_AF_INIT_NORMAL,
	.effect         = SR200PC_EFFECT_OFF,
	.iso            = SR200PC10_ISO_AUTO,
	.photometry     = SR200PC10_PHOTOMETRY_CENTER,
	.ev             = SR200PC10_EV_DEFAULT,
	.wdr            = SR200PC10_WDR_OFF,
	.contrast       = SR200PC10_CONTRAST_DEFAULT,
	.saturation     = SR200PC10_SATURATION_DEFAULT,
	.sharpness      = SR200PC10_SHARPNESS_DEFAULT,
	.wb             = SR200PC10_WB_AUTO,
	.isc            = SR200PC10_ISC_STILL_OFF,
	.scene          = SR200PC10_SCENE_OFF,
	.aewb           = SR200PC10_AE_UNLOCK_AWB_UNLOCK,
	.antishake      = SR200PC10_ANTI_SHAKE_OFF,
	.flash_capture  = SR200PC10_FLASH_CAPTURE_OFF,
	.flash_movie    = SR200PC10_FLASH_MOVIE_OFF,
	.jpeg_quality   = SR200PC10_JPEG_SUPERFINE, 
	.zoom           = SR200PC10_ZOOM_1P00X,
	.thumb_offset   = SR200PC10_THUMBNAIL_OFFSET,
	.yuv_offset     = SR200PC10_YUV_OFFSET,
	.jpeg_capture_w = JPEG_CAPTURE_WIDTH,
	.jpeg_capture_h = JPEG_CAPTURE_HEIGHT,
};

struct v4l2_queryctrl sr200pc10_ctrl_list[] = {
	{
		.id            = V4L2_CID_SELECT_MODE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "select mode",
		.minimum       = SR200PC10_MODE_CAMERA,
		.maximum       = SR200PC10_MODE_VT,
		.step          = 1,
		.default_value = SR200PC10_MODE_CAMERA,
	},    
	{
		.id            = V4L2_CID_SELECT_STATE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "select state",
		.minimum       = SR200PC10_STATE_PREVIEW,
		.maximum       = SR200PC10_STATE_CAPTURE,
		.step          = 1,
		.default_value = SR200PC10_STATE_PREVIEW,
	},    
	{
		.id            = V4L2_CID_FOCUS_MODE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Focus Mode",
		.minimum       = SR200PC10_AF_INIT_NORMAL,
		.maximum       = SR200PC10_AF_INIT_FACE,
		.step          = 1,
		.default_value = SR200PC10_AF_INIT_NORMAL,
	},
	{
		.id            = V4L2_CID_AF,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Focus Status",
		.minimum       = SR200PC10_AF_START,
		.maximum       = SR200PC10_AF_STOP,
		.step          = 1,
		.default_value = SR200PC10_AF_STOP,
	},
	{
		.id            = V4L2_CID_ZOOM,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Digital Zoom",
		.minimum       = SR200PC10_ZOOM_1P00X,
		.maximum       = SR200PC10_ZOOM_4P00X,
		.step          = 1,
		.default_value = SR200PC10_ZOOM_1P00X,
	},
	{
		.id            = V4L2_CID_JPEG_TRANSFER,
		.name          = "Request JPEG Transfer",
	},
	{
		.id            = V4L2_CID_JPEG_QUALITY,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "JPEG Quality",
		.minimum       = SR200PC10_JPEG_SUPERFINE,
		.maximum       = SR200PC10_JPEG_ECONOMY,
		.step          = 1,
		.default_value = SR200PC10_JPEG_SUPERFINE,
	},
	{
		.id            = V4L2_CID_ISO,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "ISO",
		.minimum       = SR200PC10_ISO_AUTO,
		.maximum       = SR200PC10_ISO_800,
		.step          = 1,
		.default_value = SR200PC10_ISO_AUTO,
	},
	{
		.id            = V4L2_CID_BRIGHTNESS,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Brightness",
		.minimum       = SR200PC10_EV_MINUS_2P0,
		.maximum       = SR200PC10_EV_PLUS_2P0,
		.step          = 1,
		.default_value = SR200PC10_EV_DEFAULT,
	},
	{
		.id            = V4L2_CID_WB,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "White Balance",
		.minimum       = SR200PC10_WB_AUTO,
		.maximum       = SR200PC10_WB_FLUORESCENT,
		.step          = 1,
		.default_value = SR200PC10_WB_AUTO,
	},
	{
		.id            = V4L2_CID_CONTRAST,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Contrast",
		.minimum       = SR200PC10_CONTRAST_MINUS_3,
		.maximum       = SR200PC10_CONTRAST_PLUS_3,
		.step          = 1,
		.default_value = SR200PC10_CONTRAST_DEFAULT,
	},  
	{
		.id            = V4L2_CID_SATURATION,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Saturation",
		.minimum       = SR200PC10_SATURATION_MINUS_3,
		.maximum       = SR200PC10_SATURATION_PLUS_3,
		.step          = 1,
		.default_value = SR200PC10_SATURATION_DEFAULT,
	},
	{
		.id            = V4L2_CID_EFFECT,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Effect",
		.minimum       = SR200PC_EFFECT_OFF,
		.maximum       = SR200PC_EFFECT_PURPLE,
		.step          = 1,
		.default_value = SR200PC_EFFECT_OFF,
	},
	{
		.id            = V4L2_CID_SCENE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Scene",
		.minimum       = SR200PC10_SCENE_OFF,
		.maximum       = SR200PC10_SCENE_FIREWORKS,
		.step          = 1,
		.default_value = SR200PC10_SCENE_OFF,
	},
	{
		.id            = V4L2_CID_PHOTOMETRY,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Photometry",
		.minimum       = SR200PC10_PHOTOMETRY_CENTER,
		.maximum       = SR200PC10_PHOTOMETRY_MATRIX,
		.step          = 1,
		.default_value = SR200PC10_PHOTOMETRY_CENTER,
	},
	{
		.id            = V4L2_CID_WDR,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Wide Dynamic Range",
		.minimum       = SR200PC10_WDR_OFF,
		.maximum       = SR200PC10_WDR_AUTO,
		.step          = 1,
		.default_value = SR200PC10_WDR_OFF,
	},
	{
		.id            = V4L2_CID_SHARPNESS,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Sharpness",
		.minimum       = SR200PC10_SHARPNESS_MINUS_3,
		.maximum       = SR200PC10_SHARPNESS_PLUS_3,
		.step          = 1,
		.default_value = SR200PC10_SHARPNESS_DEFAULT,
	},
	{
		.id            = V4L2_CID_ISC,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Image Stabilization",
		.minimum       = SR200PC10_ISC_STILL_OFF,
		.maximum       = SR200PC10_ISC_MOVIE_ON,
		.step          = 1,
		.default_value = SR200PC10_ISC_STILL_OFF,
	},
	{
		.id            = V4L2_CID_AEWB,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Auto Exposure/Auto White Balance",
		.minimum       = SR200PC10_AE_LOCK_AWB_LOCK,
		.maximum       = SR200PC10_AE_UNLOCK_AWB_UNLOCK,
		.step          = 1,
		.default_value = SR200PC10_AE_UNLOCK_AWB_UNLOCK,
	},
	{
		.id            = V4L2_CID_ANTISHAKE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Anti Shake Setting",
		.minimum       = SR200PC10_ANTI_SHAKE_OFF,
		.maximum       = SR200PC10_ANTI_SHAKE_ON,
		.step          = 1,
		.default_value = SR200PC10_ANTI_SHAKE_OFF,
	},  
	{
		.id            = V4L2_CID_FW_UPDATE,
		.name          = "Firmware Update",
	},
	{
		.id            = V4L2_CID_FLASH_CAPTURE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Capture Flash Setting",
		.minimum       = SR200PC10_FLASH_CAPTURE_OFF,
		.maximum       = SR200PC10_FLASH_CAPTURE_AUTO,
		.step          = 1,
		.default_value = SR200PC10_FLASH_CAPTURE_OFF,
	},
	{
		.id            = V4L2_CID_FLASH_MOVIE,
		.type          = V4L2_CTRL_TYPE_INTEGER,
		.name          = "Movie Flash Setting",
		.minimum       = SR200PC10_FLASH_MOVIE_OFF,
		.maximum       = SR200PC10_FLASH_MOVIE_ON,
		.step          = 1,
		.default_value = SR200PC10_FLASH_MOVIE_OFF,
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
#define NUM_SR200PC10_CONTROL ARRAY_SIZE(sr200pc10_ctrl_list)

/* list of image formats supported by sr200pc10 sensor */
const static struct v4l2_fmtdesc sr200pc10_formats[] = {
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
#define NUM_SR200PC10_FORMATS ARRAY_SIZE(sr200pc10_formats)

extern struct sr200pc10_platform_data nowplus_sr200pc10_platform_data;

extern u8 ISP_FW_ver[4];

static int nightshot_in_init = 0;

static int sr200pc10_start_preview(void);

/* Arun C: inlining the function
 * need to check if there are any side effects and
 * camera performance improvement
 */
static inline int s5k4ca_sensor_read(struct i2c_client *client,
		unsigned short subaddr, unsigned short *data)
{

	int ret;
	unsigned char buf[1] = {0};
	struct i2c_msg msg = { client->addr, 0, 1, buf };

#ifdef VIEW_FUNCTION_CALL	
	printk("<=PCAM=> %s function %d line launched!\n", __func__, __LINE__);
	printk("<=PCAM DEBUG=> read on addr ~ : %x\n", client->addr);
#endif
	buf[0] = subaddr;

	ret = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
	/* if (ret == -EIO) */
	/* Arun C: make the error unlikely */
	if (unlikely(ret == -EIO))
	{
		printk("<=PCAM=> 1. read error~!\n");
		goto error;
	}

	msg.flags = I2C_M_READ;
	
	ret = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
	/* if (ret == -EIO) */
	/* Arun C: make the error unlikely */
	if (unlikely(ret == -EIO))
	{
		printk("<=PCAM=> 2. read error~!\n");
		goto error;
	}

	*data = buf[0];

error:
	return ret;

}


#define REGION_50HZ 50
#define REGION_60HZ 60
/* Arun C: inlining the function
 * need to check if there are any side effects and
 * camera performance improvement
 */
static inline int s5k4ca_sensor_write(struct i2c_client *client, 
		unsigned short subaddr, unsigned short val)
{
	unsigned char buf[2] = {0};
	struct i2c_msg msg = { client->addr, 0, 2, buf };

	buf[0] = subaddr;
	buf[1] = val;

#ifdef VIEW_FUNCTION_CALL	
	printk("<=PCAM=> buf[0]:%x  buf[1]:%x\n", buf[0], buf[1]);
#endif

	return i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
}

//static int s5k4ba_write_regs(struct v4l2_subdev *sd, s5k4ca_short_t regs[], 
//				int size, char *name)
static int s5k4ba_write_regs(struct i2c_client *client, s5k4ca_short_t regs[], 
				int size, char *name)
{

	int i=0;
	//struct i2c_client *client = v4l2_get_subdevdata(sd);
	//struct sr200pc10_sensor *sensor = &sr200pc10;
	//struct i2c_client *client = sensor->i2c_client;
	int err;
	int endAddr =0;

//zzangdol
//#ifdef VIEW_FUNCTION_CALL	
	printk("[S5k4CA] %s function %d line launched!\n", __func__, __LINE__);
	
//#endif

#ifdef CONFIG_LOAD_FILE
	err = sr200pc10_regs_table_write(client, name);
	//s5k4ca_regs_table_write(client, name);
#else

	unsigned char buf[2] = {0};
	struct i2c_msg msg = { client->addr, 0, 2, buf };

//int regionHz = REGION_60HZ; // default Hz is 60Hz
int regionHz = REGION_50HZ; // default Hz is 60Hz
	// Rossi
	if(regionHz != REGION_50HZ ){
		i = size; // use Region 60hz
		endAddr = (size*2) -1;
		size = size * 2;
//#ifdef VIEW_FUNCTION_CALL			
		printk("[LONNIE] %s function %d line launched!\n", __func__, __LINE__);
		
		printk("[LONNIE] Now Use Region 60Hz!!!\n");		
		printk("[LONNIE] start i = %d, endAddr = %d, size = %d \n",i, endAddr, size);		
//#endif		
	}
	else{
//#ifdef VIEW_FUNCTION_CALL			
		printk("[LONNIE] Now Use Region 50Hz!!!\n");	
//#endif
		endAddr = size-1;
	}

	if(  regs[endAddr].subaddr == 0xff ){
//#ifdef VIEW_FUNCTION_CALL			
		printk("[LONNIE]Find 0xff \n");		
//#endif
		do{
			buf[0] = regs[i].subaddr;
			buf[1] = regs[i].value;
			err = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
		}while(i++ < size-1);
/*			
		for (i = 0; i < size-1; i++) {
			buf[0] = regs[i].subaddr;
			buf[1] = regs[i].value;
			i2c_transfer(client->adapter, &msg, 1);
		}
*/		
		mdelay(regs[endAddr].value*10);
//		err = 0;
	}
	else{
//#ifdef VIEW_FUNCTION_CALL			
		printk("[LONNIE] not found 0xff \n");		
//#endif
		do{
			buf[0] = regs[i].subaddr;
			buf[1] = regs[i].value;
			err = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
		}while(i++ < size);
/*		
		for (i = 0; i < size; i++) {
			buf[0] = regs[i].subaddr;
			buf[1] = regs[i].value;
			i2c_transfer(client->adapter, &msg, 1);
		}
*/		
	}

	if (unlikely(err < 0)) {
		v4l_info(client, "%s: register set failed\n", \
		__func__);
		return err;
	}
	printk("[LONNIE]  s5k4ba_write_regs success \n");		
#endif

	return 0;	/* FIXME */
}

//void static night_shot_control(struct v4l2_subdev *sd, int flag)
void static night_shot_control(struct i2c_client *client, int flag)
{
	int Exptime;
	int Expmax;
	unsigned short read_1, read_2, read_3;	

//#ifdef VIEW_FUNCTION_CALL	
	//printk("night_shot_control start\n");
       dprintk(CAM_INF, SR200PC10_MOD_NAME "night_shot_control is called\n");
//#endif

	// lonnie
	//struct i2c_client *client = v4l2_get_subdevdata(sd);
	int err = -EINVAL, i, count;

	
	if(flag == 1) //ON
	{

//#ifdef VIEW_FUNCTION_CALL	
	    printk("PCAM_NIGHT_SHOT ON");
//#endif

	    s5k4ca_sensor_write(client, 0x03, 0x20);
	    s5k4ca_sensor_write(client, 0x10, 0x1C); //AE off for 50Hz

	    s5k4ca_sensor_read(client, 0x80, &read_1);
	    s5k4ca_sensor_read(client, 0x81, &read_2);
	    s5k4ca_sensor_read(client, 0x82, &read_3);


	    Exptime = ((read_1 << 16) | (read_2 << 8) | (read_3));
//	    printk(" read first : %d %d %d\n", read_1, read_2, read_3);


	    s5k4ca_sensor_read(client, 0x88, &read_1);
	    s5k4ca_sensor_read(client, 0x89, &read_2);
	    s5k4ca_sensor_read(client, 0x8A, &read_3);

	    Expmax = ((read_1 << 16) | (read_2 << 8) | (read_3));
//	    printk(" read second : %d %d %d\n", read_1, read_2, read_3);

//	    printk("Night Shot, Exptime : %d, Expmax : %d \n", Exptime, Expmax);


	    if(Exptime < Expmax)
	    {
//#ifdef VIEW_FUNCTION_CALL		    
		printk("<=PCAM=> night 1 set\n");
//#endif

		s5k4ba_write_regs(client, sr200pc_cfg_scene_night1, SR200PC_CFG_SCENE_NIGHT1, "sr200pc_cfg_scene_night1");		

	    }
	    else
	    {
//#ifdef VIEW_FUNCTION_CALL		    
		printk("<=PCAM=> night 2 set\n");
//#endif

		s5k4ba_write_regs(client, sr200pc_cfg_scene_night2, SR200PC_CFG_SCENE_NIGHT2, "sr200pc_cfg_scene_night2");		


	    }
	}

	else
	{
//#ifdef VIEW_FUNCTION_CALL		
		printk("PCAM_NIGHT_SHOT OFF");
//#endif

		s5k4ba_write_regs(client, sr200pc_cfg_scene_normal, SR200PC_CFG_SCENE_NORMAL, "sr200pc_cfg_scene_normal"); 

/*
		s5k4ba_write_regs(sd, sr200pc10_preview, sizeof(sr200pc10_preview)/\
		sizeof(sr200pc10_preview[0]), "sr200pc10_preview"); // preview start
*/
	}

}



// Rossi Project (lonnie.kim) 2010.07.05 - Rossi dont use Burstmode
//#define S5K4CA_USE_BURSTMODE
//#ifdef S5K4CA_USE_BURSTMODE	
#if 0
unsigned char s5k4ca_buf_for_burstmode[2500];
static inline int s5k4ca_sensor_burst_write(struct i2c_client *client, 
		s5k4ca_short_t table[] ,int size)
{
	int i = 0;
	int idx = 0;
	int err = -EINVAL;
	
	struct i2c_msg msg = { client->addr, 0, 0, s5k4ca_buf_for_burstmode };
	
	for (i = 0; i < size; i++) 
	{
		switch( table[i].subaddr )
		{
                     case 0xFCFC :
                     case 0x0028 :
                     case 0x002A :
                         // Set Address
                         idx=0;
                         err = s5k4ca_sensor_write(client, table[i].subaddr, \
     					table[i].value);
                         break;
                     case 0x0F12 :
                         // make and fill buffer for burst mode write
                         if(idx ==0) 
         		 {
         		 	s5k4ca_buf_for_burstmode[idx++] = 0x0F;
         		 	s5k4ca_buf_for_burstmode[idx++] = 0x12;
         		 }
                         s5k4ca_buf_for_burstmode[idx++] = table[i].value>> 8;
     		         s5k4ca_buf_for_burstmode[idx++] = table[i].value & 0xFF;
     		         //write in burstmode	
                         if(table[i+1].subaddr != 0x0F12)
                         {
                         	//printk("%s: write in burstmode. length :%d\n",__func__,idx);
         			msg.len = idx;
         			err = i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
         			idx=0;
         			//msleep(1);
                         }
                         break;
                     case 0xFFFF :
                    	 break;				
		}
	}
	if (unlikely(err < 0))
	{
		v4l_info(client, "%s: register set failed\n", \
			__func__);
		return err;
	}
	return err;
}
#endif /*S5K4CA_USE_BURSTMODE*/

static int s5k4ca_sensor_get_id(struct i2c_client *client)
{
	unsigned short id = 0;
	int ret;
#ifdef VIEW_FUNCTION_CALL	
	printk("[S5k4CA] %s function %d line launched!\n", __func__, __LINE__);
#endif
	
#if 1//PCAM
	ret=s5k4ca_sensor_read(client, 0x04, &id);

	printk("<=PCAM=> SENSOR FW => 0x%x \n", id); 
#else//ORT
	s5k4ca_sensor_write(client, 0x002C, 0x7000);
	s5k4ca_sensor_write(client, 0x002E, 0x01FA);
	s5k4ca_sensor_read(client, 0x0F12, &id);

	printk("Sensor ID(0x%04x) is %s!\n", id, (id == 0x4CA4) ? "Valid" : "Invalid"); 

#endif//PCAM
	return ret;
}


static int sr200pc10_write_read_reg(struct i2c_client *client, u8 writedata_num, const u8* writedata, u8 readdata_num, u8* readdata)
{
	int err, i;
	struct i2c_msg msg[1];
	u8 writebuf[writedata_num];
	u8 readbuf[readdata_num];

	if (!client->adapter) 
	{
		return -ENODEV;
	}

	/* Write */
	msg->addr  = client->addr;
	msg->flags = I2C_M_WRITE;
	msg->len   = writedata_num;
	memcpy(writebuf, writedata, writedata_num);
	msg->buf   = writebuf;

#if (CAM_SR200PC10_I2C_DBG_MSG)
	for (i = 0; i < writedata_num; i++) 
	{
		printk("=0x%02x=\n",writedata[i]);
	}
#endif

	for(i = 0; i < 10; i++)
	{
		err=0;//err = i2c_transfer(client->adapter, msg, 1) == 1 ? 0 : -EIO;
		if (err == 0) 
			break;
		mdelay(1);
	}

	if(i == 10)
	{
		return err;  
	}    

	/* Read */
	msg->addr  = client->addr;
	msg->flags = I2C_M_READ;
	msg->len   = readdata_num;
	memset(readbuf, 0x0, readdata_num);
	msg->buf   = readbuf;

	for(i = 0; i < 10; i++)
	{    
		err=0;//err = i2c_transfer(client->adapter, msg, 1) == 1 ? 0 : -EIO;
		if (err == 0) 
		{
			memcpy(readdata, readbuf, readdata_num);
			return 0;
		}
		mdelay(1);
	}

	return err;
}





static int sr200pc10_write_reg(struct i2c_client *client, u8 length, const u8* readdata)
{
	u8 buf[length], i = 0;
	struct i2c_msg msg = {client->addr, I2C_M_WRITE, length, buf};
	int err = 0;
	if (!client->adapter)
	{
		return -ENODEV;
	}  

#if (CAM_SR200PC10_I2C_DBG_MSG)
	for (i = 0; i < length; i++) 
	{
		buf[i] = readdata[i];
		printk("=0x%02x=\n",buf[i]);
	}
#else
	for (i = 0; i < length; i++) 
	{
		buf[i] = readdata[i];
	}
#endif

	for (i = 0; i < 10; i++)
	{
		err =0;  //i2c_transfer(client->adapter, &msg, 1) == 1 ? 0 : -EIO;
		if(err == 0) 
		{
			return 0;
		}
		mdelay(1);
	}

	return err;
}

static int sr200pc10_poll_reg(struct i2c_client *client, enum ReadCommad CheckStatus)
{
	int i = 0;
	int ret = 0; 
	u8* CheckCommand = NULL;
	u8  readdata[2] = {0x00,};
	int MaxRetryCount = 0;
	int PollingDelay = 0;

	switch(CheckStatus)
	{
		case RCommandPreviewStatusCheck:
			CheckCommand  = (u8*)CameraReadCommand_CaptureStatus;
			MaxRetryCount = 500; // Modified retrycount to fix ECAM no data error
			PollingDelay  = 10;
			break;

		case RCommandCaptureStatusCheck:
			CheckCommand  = (u8*)CameraReadCommand_CaptureStatus;
			MaxRetryCount = 500;
			PollingDelay  = 10;
			break;

		case RCommandFWEndStatusChk:
			CheckCommand  = (u8*)CameraRead_FWEnd_Status_List;
			MaxRetryCount = 500;
			PollingDelay  = 10;
			break;   

		case RCommandDataTransmissionCheck: 
			CheckCommand  = (u8*)DataTransmissionCheck;
			MaxRetryCount = 500;
			PollingDelay  = 10;
			break;   

		case RCommandAFIdleStatus:
		case RCommandAFFinishStatus:
		case RCommandZoomIdleStatus:
		case RCommandZoomReleaseStatus:
		case RCommandZoomFinishStatus:
			CheckCommand  = (u8*)Lense_CheckStatus_List;
			MaxRetryCount = 500;
			PollingDelay  = 10;
			break;

		case RCommandAFZoomTuningStatus:
		case RCommandAFZoomTuningFinishStatus:
			CheckCommand  = (u8*)Lense_CheckStatus_List;
			MaxRetryCount = 1000;
			PollingDelay  = 10;
			break;

		case RCommandBatchReflectionStatus:
			CheckCommand  = (u8*)BatchReflectionCheck_list;
			MaxRetryCount = 500;
			PollingDelay  = 10;
			break;

		case RCommandFlashStatusCheck:
			CheckCommand  = (u8*)FlashStatusCheck;
			MaxRetryCount = 500;
			PollingDelay  = 10;
			break;

		case RCommandFWLoaderStatusChk:
			CheckCommand  = (u8*)CameraRead_FWUpdate_Status_List;
			MaxRetryCount = 1000;
			PollingDelay  = 10;
			break;

		case RCommandFWUpdateStatusChk:
			CheckCommand  = (u8*)CameraRead_FWUpdate_Status_List;
			MaxRetryCount = 1000;
			PollingDelay  = 10;
			break;      

		case RCommandISStatusCheck:
			CheckCommand  = (u8*)IS_Check_list;
			MaxRetryCount = 500;
			PollingDelay  = 10;
			break;

		case RCommandDZoomFinishStatus:
			CheckCommand  = (u8*)Lense_CheckDZoomStatus_List;
			MaxRetryCount = 500;
			PollingDelay  = 10;
			break;

		default:
			break;
	}
goto POLL_STATUS_SUCCESS;
#if 0
	for(i = 0; i < MaxRetryCount; i++)
	{   
		if(CheckStatus == RCommandDZoomFinishStatus)
		{
			ret = sr200pc10_write_read_reg(client, 1, CheckCommand, 2, readdata);
		}
		else
		{
			ret = sr200pc10_write_read_reg(client, 1, CheckCommand, 1, readdata);
		}

		if(ret) 
		{   
			break;
		}

		switch(CheckStatus)
		{
			case RCommandPreviewStatusCheck :
				if(*readdata == 0x08)
					goto POLL_STATUS_SUCCESS;
				break;

			case RCommandDataTransmissionCheck:
			case RCommandCaptureStatusCheck :
			case RCommandFWEndStatusChk :
			case RCommandBatchReflectionStatus:
				if(*readdata == 0x00)
					goto POLL_STATUS_SUCCESS;
				break;

			case RCommandAFIdleStatus :
			case RCommandZoomIdleStatus:
			case RCommandAFZoomTuningStatus:
				if(!(*readdata & 0x01))
				{
					goto POLL_STATUS_SUCCESS;
				}
				break;

			case RCommandAFFinishStatus :
			case RCommandAFZoomTuningFinishStatus:
				if(!(*readdata & 0x05))
				{
					goto POLL_STATUS_SUCCESS;
				}
				break;

			case RCommandZoomReleaseStatus:
				if(*readdata == 0x04)
				{
					goto POLL_STATUS_SUCCESS;
				}
				break;

			case RCommandZoomFinishStatus:
			case RCommandDZoomFinishStatus:
				if(*(readdata+1) == 0x00)
				{
					goto POLL_STATUS_SUCCESS;
				}
				break;

			case RCommandFlashStatusCheck:
				if(*readdata == 0x01)
					goto POLL_STATUS_SUCCESS;
				break;

			case RCommandISStatusCheck:
				if(*readdata == 0x01)
					goto POLL_STATUS_SUCCESS;
				break;

			case RCommandFWLoaderStatusChk:
				if(*readdata == 0x05)
					goto POLL_STATUS_SUCCESS;
				break;

			case RCommandFWUpdateStatusChk:
				if(*readdata == 0x06)
					goto POLL_STATUS_SUCCESS;
				break;        

			default:
				break;
		}

		/* wait and poll again */
		msleep(PollingDelay);
	}

	return -EINVAL;
#endif
POLL_STATUS_SUCCESS:   
	return 0;
}

//====================================================================================

static int sr200pc10_request_firmware(const struct firmware **firmware_p, const char *name,size_t size)
{
	struct file *filep;  
	struct firmware *firmware;
	mm_segment_t oldfs;

	if (!firmware_p)
	{
		return -EINVAL;
	}

	*firmware_p = firmware = kmalloc(sizeof(*firmware), GFP_KERNEL);
	if (!firmware) 
	{
		return -ENOMEM;
	}

	filep = filp_open(name, O_RDONLY, 0) ;

	if (filep )
	{
		firmware->data = (u8 *)kmalloc(size, GFP_KERNEL);
		oldfs = get_fs();
		set_fs(KERNEL_DS);
		firmware->size = filep->f_op->read(filep, (char __user *)firmware->data , size, &filep->f_pos);
		filp_close(filep, current->files);
		set_fs(oldfs);
		(printk(SR200PC10_MOD_NAME "[CAM-SENSOR] =read firmware data size %d\r\n", firmware->size));
	}
	else
	{
		return -EINVAL;
	}

	(printk("SR200PC10_MOD_NAME [CAM-SENSOR] -%s\n",__func__));

	return 0;
}

static int sr200pc10_release_firmware(const struct firmware *fw)
{
	(printk(SR200PC10_MOD_NAME "[CAM-SENSOR] +%s\n",__func__));
	if(fw)
	{
		kfree(fw->data);
		kfree(fw);
	}
	(printk(SR200PC10_MOD_NAME "[CAM-SENSOR] -%s\n",__func__));

	return 0;
}

static int sr200pc10_update_firmware(int type)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	u8 Init_buffer[5] = {0x00,};
	u8 Data_buffer[130] = {0x00,};
	u8 readdata[1] = {0x00};

	u32 F3U_length = 0;
	u32 F3_length = 0;

	int i, ret;

	const struct firmware * fw_F2U_entry;
	const struct firmware * fw_F3U_entry;
	const struct firmware * fw_F2_entry;
	const struct firmware * fw_F3_entry;  

	printk(SR200PC10_MOD_NAME "sr200pc10_update_firmware is called...\n");
	printk(SR200PC10_MOD_NAME "ISP (FW ver)  : 0x%02x, 0x%02x\n", SR200PC10_FW_MINOR_VERSION, SR200PC10_FW_MAJOR_VERSION);
	printk(SR200PC10_MOD_NAME "ISP (PRM ver) : 0x%02x, 0x%02x\n", SR200PC10_PRM_MINOR_VERSION, SR200PC10_PRM_MAJOR_VERSION);  
	printk(SR200PC10_MOD_NAME "Loader (FW ver)  : 0x%02x, 0x%02x\n", ISP_FW_ver[0], ISP_FW_ver[1]);
	printk(SR200PC10_MOD_NAME "Loader (PRM ver) : 0x%02x, 0x%02x\n", ISP_FW_ver[2], ISP_FW_ver[3]);

	if(SR200PC10_FW_MINOR_VERSION == SR200PC10_NOT_FW_UPDATE_OPERATION) 
	{
		printk(SR200PC10_MOD_NAME "Firmware update is denied!!\n");
		ret = SR200PC10_NOT_FW_UPDATE_OPERATION;
		return ret;
	}

	/* Check Firmware Version */
	if(type == IN_IMAGE)
	{
		if(  (ISP_FW_ver[0] == SR200PC10_FW_MINOR_VERSION)
				&& (ISP_FW_ver[1] == SR200PC10_FW_MAJOR_VERSION) 
				&& (ISP_FW_ver[2] == SR200PC10_PRM_MINOR_VERSION) 
				&& (ISP_FW_ver[3] == SR200PC10_PRM_MAJOR_VERSION)
		  )
		{
			printk(SR200PC10_MOD_NAME "Current firmware is the lastest one!!\n");
			return 1;
		}
	}

	if(type == IN_IMAGE)
		printk("SR200PC10_MOD_NAME [CAM-SENSOR] firmware update from memory !\n");
	else
		printk("SR200PC10_MOD_NAME [CAM-SENSOR] firmware update from sdcard !\n");

	if(type == IN_IMAGE)
		ret = sr200pc10_request_firmware(&fw_F2U_entry, SR200PC10_FIRMWARE_F2U_NAME, 4);
	else
		ret = sr200pc10_request_firmware(&fw_F2U_entry, SR200PC10_FIRMWARE_F2U_MMC_NAME, 4);
	if(ret != 0) 
	{
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "firmware[F2U] request error!\n");
		goto update_fail;
	}

	F3U_length = fw_F2U_entry->data[0] + (fw_F2U_entry->data[1] << 8);
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "F3U_length [%x]!\n",F3U_length);

	if(type == IN_IMAGE)
		ret = sr200pc10_request_firmware(&fw_F3U_entry, SR200PC10_FIRMWARE_F3U_NAME, F3U_length * 129);
	else
		ret = sr200pc10_request_firmware(&fw_F3U_entry, SR200PC10_FIRMWARE_F3U_MMC_NAME, F3U_length * 129);

	if(ret != 0) 
	{
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "firmware[F3U] request error!\n");
		goto update_fail;
	}

	if(type == IN_IMAGE)
		ret = sr200pc10_request_firmware(&fw_F2_entry, SR200PC10_FIRMWARE_F2_NAME, 4);
	else
		ret = sr200pc10_request_firmware(&fw_F2_entry, SR200PC10_FIRMWARE_F2_MMC_NAME, 4);

	if(ret != 0) 
	{
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "firmware[F2] request error!\n");
		goto update_fail;
	}

	F3_length = fw_F2_entry->data[0] + (fw_F2_entry->data[1] << 8);

	if(type == IN_IMAGE)
		ret = sr200pc10_request_firmware(&fw_F3_entry, SR200PC10_FIRMWARE_F3_NAME, F3_length * 129);
	else
		ret = sr200pc10_request_firmware(&fw_F3_entry, SR200PC10_FIRMWARE_F3_MMC_NAME, F3_length * 129);

	if(ret != 0) 
	{
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "firmware[F3] request error!\n");
		goto update_fail;
	}

	/* send F2U.bin */
	Init_buffer[0] = 0xF2;
	memcpy(&Init_buffer[1], fw_F2U_entry->data, 4);
	if(sr200pc10_write_reg(client, sizeof(Init_buffer), Init_buffer))
		goto update_fail;
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "firmware[F2U] write complete!!\n");
	mdelay(100);

	/* send F3U.bin */
	Data_buffer[0] = 0xF3;
#if 0	
	for(i = 0; i < F3U_length; i++)
	{
		memcpy(Data_buffer+1, fw_F3U_entry->data + (i * 129), 129);
		if(sr200pc10_write_read_reg(client, sizeof(Data_buffer), Data_buffer, 1, readdata))
			goto update_fail;
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "F3U write remain count : %d\n", F3U_length - i);
		mdelay(1);
	}
#endif	
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "firmware[F3U] write complete!!\n");
	mdelay(20);

	/* check F5 status */
	if(sr200pc10_poll_reg(client, RCommandFWLoaderStatusChk))
	{
		goto update_fail;
	}  
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "firmware loader update success!!\n");
	mdelay(50);

	/* send F2.bin */
	Init_buffer[0] = 0xF2;
	memcpy(&Init_buffer[1], fw_F2_entry->data, 4);
	if(sr200pc10_write_reg(client, sizeof(Init_buffer), Init_buffer))
		goto update_fail;
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "firmware[F2] write complete!!\n"); 
	mdelay(100);

	/* send F3.bin */
	Data_buffer[0] = 0xF4;
	for(i = 0; i < F3_length; i++)
	{
		memcpy(Data_buffer+1, fw_F3_entry->data + (i * 129), 129);
		if(sr200pc10_write_reg(client, sizeof(Data_buffer), Data_buffer))
			goto update_fail;
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "F3 write remain count : %d\n", F3_length - i);
		mdelay(1);
	}
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "firmware[F3] write complete!!\n");  
	mdelay(10);

	/* check F5 status */
	if(sr200pc10_poll_reg(client, RCommandFWUpdateStatusChk))
	{
		goto update_fail;
	}

	printk(SR200PC10_MOD_NAME "firmware update success!!\n");  
	mdelay(50);

	sr200pc10_release_firmware(fw_F2U_entry);
	sr200pc10_release_firmware(fw_F3U_entry);
	sr200pc10_release_firmware(fw_F2_entry);
	sr200pc10_release_firmware(fw_F3_entry);  

	return 0;

update_fail:
	printk(SR200PC10_MOD_NAME "firmware update failed!!\n");  
	mdelay(50);

	sr200pc10_release_firmware(fw_F2U_entry);
	sr200pc10_release_firmware(fw_F3U_entry);
	sr200pc10_release_firmware(fw_F2_entry);
	sr200pc10_release_firmware(fw_F3_entry);  

	return -EINVAL; 
}

static int sr200pc10_fw_date(struct v4l2_control *vc)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	u8 Init_buffer[3] = {0x00,};  

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_fw_date is called...\n");  
#if 0
	/* Read Firmware Version */
	if(sr200pc10_write_read_reg(client, sizeof(CameraReadCommand_FWDate_List), CameraReadCommand_FWDate_List, 3, Init_buffer))
		goto fw_date_fail;

	printk(" Init_buffer  : [0]0x%02x, [1]0x%02x, [2]0x%02x,\n", Init_buffer[0], Init_buffer[1], Init_buffer[2]);

	vc->value = Init_buffer[2] | (Init_buffer[1] << 8) | (Init_buffer[0] << 16) ;
	printk(" date[%x]\n",  vc->value); 
#endif
	return 0;

fw_date_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_fw_date is failed!!\n");
	return -EINVAL;   
}

//=============================================================================================
static int sr200pc10_check_dataline()
{
	int err  = -EINVAL;
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	//unsigned char sr200pc10_buf_check_dataline[3] = {0xEC, 0x01, 0x01};

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_check_dataline is called...\n");	
#if 0
	if(sr200pc10_write_reg(client, sizeof(sr200pc10_buf_check_dataline), sr200pc10_buf_check_dataline))
	{
		printk(SR200PC10_MOD_NAME "%s: failed: i2c_write for check_dataline\n", __func__);
		return -EIO;
	}
#endif
	err = s5k4ba_write_regs(client, sr200pc10_dataline,SR200PC10_DATALINE_REGS, "sr200pc10_dataline"); // init + dataline start
	if (err < 0)
	{
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}
	sensor->check_dataline = 0;
	
	return 0;
}

static int sr200pc10_check_dataline_stop()
{
	int err  = -EINVAL;
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;
#if 0
	unsigned char sr200pc10_buf_check_dataline[3] = {0xEC, 0x00, 0x00};

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_check_dataline_stop is called...\n");	

	if(sr200pc10_write_reg(client, sizeof(sr200pc10_buf_check_dataline), sr200pc10_buf_check_dataline))
	{
		printk(SR200PC10_MOD_NAME "%s: failed: i2c_write for check_dataline\n", __func__);
		return -EIO;
	}
#endif
	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_check_dataline_stop is called...\n");	

	err = s5k4ba_write_regs(client, sr200pc10_dataline_stop,SR200PC10_DATALINE_STOP_REGS, "sr200pc10_dataline_stop"); // preview start
	if (err < 0)
	{
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}
	
	sensor->check_dataline = 0;
	sensor->pdata->power_set(V4L2_POWER_OFF);
	mdelay(5);
	sensor->pdata->power_set(V4L2_POWER_ON);
	mdelay(5);
	err = s5k4ba_write_regs(client, sr200pc10_init0, SR200PC10_INIT0_REGS, "sr200pc10_init0");//LYG
	if (err < 0)
	{
		v4l_info(client, "%s: register set failed\n", __func__);
		return -EIO;
	}
	sr200pc10_start_preview();
	//s5ka3dfx_set_init();
	//sr200pc10_set_ev(SR200PC10_EV_DEFAULT);

	return 0;
}

static int sr200pc10_detect(struct i2c_client *client)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_detect is called...\n");

	/* Start Camera Program */
	if(sr200pc10_write_reg(client, sizeof(ConfigSensorFirmWareInit_list), ConfigSensorFirmWareInit_list))
		goto detect_fail;

	mdelay(600); //Reduced the delay 
#if 0
	/* Read Firmware Version */
	if(sr200pc10_write_read_reg(client, sizeof(CameraReadCommand_FWVersion_List), CameraReadCommand_FWVersion_List, 4, ISP_FW_ver))
		goto detect_fail;

	if(*ISP_FW_ver == 0x0 && *(ISP_FW_ver+1) == 0x0 && *(ISP_FW_ver+2) == 0x0 && *(ISP_FW_ver+3) == 0x0)
		goto detect_fail;

	printk(SR200PC10_MOD_NAME " Loader (FW ver)  : 0x%02x, 0x%02x\n", ISP_FW_ver[0], ISP_FW_ver[1]);
	printk(SR200PC10_MOD_NAME " Loader (PRM ver) : 0x%02x, 0x%02x\n", ISP_FW_ver[2], ISP_FW_ver[3]);
	printk(SR200PC10_MOD_NAME " Sensor is Detected!!!\n");
#endif
	return 0;

detect_fail:
	printk(SR200PC10_MOD_NAME " Sensor is not Detected!!!\n");
	return -EINVAL; 
}

static int sr200pc10_set_effect(s32 value) //LYG
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_effect is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC_EFFECT_OFF:
			//if(sr200pc10_write_reg(client, sizeof(Effect_None_List), Effect_None_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
			break;

		case SR200PC_EFFECT_BW:
			//if(sr200pc10_write_reg(client, sizeof(Effect_BW_List), Effect_BW_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_mono, \
				SR200PC10_EFFECT_MONO\
				, "sr200pc10_effect_mono")!=0)
				goto effect_fail;
			break;    

		case SR200PC_EFFECT_GREY:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Grey_List), Effect_Grey_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_mono, \
				SR200PC10_EFFECT_MONO\
				, "sr200pc10_effect_mono")!=0)
				goto effect_fail;
			break;      

		case SR200PC_EFFECT_SEPIA:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Sepia_List), Effect_Sepia_List))
		if( s5k4ba_write_regs(client, \
				sr200pc10_effect_sepia, \
				SR200PC10_EFFECT_SEPIA, \
                                "sr200pc10_effect_sepia")!=0)
				goto effect_fail;
			break;

		case SR200PC_EFFECT_SHARPEN:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Sharpen_List), Effect_Sharpen_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
			break;      

		case SR200PC_EFFECT_NEGATIVE:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Negative_List), Effect_Negative_List))
			if(s5k4ba_write_regs(client, \
				sr200pc10_effect_negative, \
				SR200PC10_EFFECT_NEGATIVE, \
				"sr200pc10_effect_negative")!=0)
				goto effect_fail;
			break;

		case SR200PC_EFFECT_ANTIQUE:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Antique_List), Effect_Antique_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
			break;

		case SR200PC_EFFECT_AQUA:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Aqua_List), Effect_Aqua_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
			break;

		case SR200PC_EFFECT_RED:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Red_List), Effect_Red_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
			break; 

		case SR200PC_EFFECT_PINK:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Pink_List), Effect_Pink_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
			break; 

		case SR200PC_EFFECT_YELLOW:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Yellow_List), Effect_Yellow_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
			break;       

		case SR200PC_EFFECT_GREEN:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Green_List), Effect_Green_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
			break; 

		case SR200PC_EFFECT_BLUE:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Blue_List), Effect_Blue_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
			break; 

		case SR200PC_EFFECT_PURPLE:
			//if(sr200pc10_write_reg(client, sizeof(Effect_Purple_List), Effect_Purple_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
			break;       

		default:
			printk(SR200PC10_MOD_NAME "Effect value is not supported!!!\n");
			if( s5k4ba_write_regs(client, \
				sr200pc10_effect_normal, \
				SR200PC10_EFFECT_NORMAL\
				, "sr200pc10_effect_normal")!=0)
				goto effect_fail;
	}

	sensor->effect = value;
#if 0
	if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
		goto effect_fail;
	if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
		goto effect_fail;
#endif
	return 0;

effect_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_effect is failed!!\n");
	return -EINVAL;     
}

static int sr200pc10_set_iso(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_iso is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_ISO_AUTO:
			if(sr200pc10_write_reg(client, sizeof(ISOAuto_list), ISOAuto_list))
				goto iso_fail;
			break;

		case SR200PC10_ISO_50:
			if(sr200pc10_write_reg(client, sizeof(ISO50_list), ISO50_list))
				goto iso_fail;
			break;      

		case SR200PC10_ISO_100:
			if(sr200pc10_write_reg(client, sizeof(ISO100_list), ISO100_list))
				goto iso_fail;
			break;      

		case SR200PC10_ISO_200:
			if(sr200pc10_write_reg(client, sizeof(ISO200_list), ISO200_list))
				goto iso_fail;
			break;      

		case SR200PC10_ISO_400:
			if(sr200pc10_write_reg(client, sizeof(ISO400_list), ISO400_list))
				goto iso_fail;
			break;

		case SR200PC10_ISO_800:
			if(sr200pc10_write_reg(client, sizeof(ISO800_list), ISO800_list))
				goto iso_fail;
			break; 

		case SR200PC10_ISO_1600:
			if(sr200pc10_write_reg(client, sizeof(ISO1600_list), ISO1600_list))
				goto iso_fail;
			break;

		default:
			printk(SR200PC10_MOD_NAME "ISO value is not supported!!!\n");
			goto iso_fail;
	}

	sensor->iso = value;

	if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
		goto iso_fail;
	if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
		goto iso_fail;

	return 0;

iso_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_iso is failed!!\n");
	return -EINVAL;       
}

static int sr200pc10_set_photometry(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_photometry is called...[%d]\n",value);
#if 0
	// Set for 720P 
	if (sr200pc10_720p_enable) {
		if(sr200pc10_write_reg(client, sizeof(Photometry_MatrixHD_List), Photometry_MatrixHD_List))
			goto photometry_fail;
		if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
			goto photometry_fail;
		if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
			goto photometry_fail;

		return 0;
	}  
#endif
	switch(value)
	{
		case SR200PC10_PHOTOMETRY_CENTER:
			//if(sr200pc10_write_reg(client, sizeof(Photometry_Center_List), Photometry_Center_List))
			if(s5k4ba_write_regs(client, \
				sr200pc10_exposure_centerweighted, \
				SR200PC10_EXPOSURE_CENTERWEIGHTED,\
				"sr200pc10_exposure_centerweighted")!=0)
				goto photometry_fail;
			break;

		case SR200PC10_PHOTOMETRY_SPOT:
			//if(sr200pc10_write_reg(client, sizeof(Photometry_Spot_List), Photometry_Spot_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_exposure_spot, \
				SR200PC10_EXPOSURE_SPOT, \
				"sr200pc10_exposure_spot")!=0)
				goto photometry_fail;
			break;

		case SR200PC10_PHOTOMETRY_MATRIX:
			//if(sr200pc10_write_reg(client, sizeof(Photometry_Matrix_List), Photometry_Matrix_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_exposure_matrix, \
				SR200PC10_EXPOSURE_MATRIX,\
				"sr200pc10_exposure_matrix")!=0)
				goto photometry_fail;
			break;

		default:
			printk(SR200PC10_MOD_NAME "Photometry value is not supported!!!\n");
			goto photometry_fail;
	}

	sensor->photometry = value;
#if 0
	if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
		goto photometry_fail;
	if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
		goto photometry_fail;
#endif

	return 0;

photometry_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_iso is failed!!\n");
	return -EINVAL;         
}

static int sr200pc10_set_ev_no_apply(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	int err = 0;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_ev is called...[%d]\n",value);

	sensor->ev = value;
	switch(sensor->ev)
	{
		case SR200PC10_EV_MINUS_2P0:
			if(sr200pc10_720p_enable)
				sr200pc10_write_reg(client, sizeof(EV_Minus_2P0_HD_List), EV_Minus_2P0_HD_List);	 
			else
				sr200pc10_write_reg(client, sizeof(EV_Minus_2P0_List), EV_Minus_2P0_List);
			break;

		case SR200PC10_EV_MINUS_1P5:
			if(sr200pc10_720p_enable)
				sr200pc10_write_reg(client, sizeof(EV_Minus_1P5_HD_List), EV_Minus_1P5_HD_List);	 
			else
				sr200pc10_write_reg(client, sizeof(EV_Minus_1P5_List), EV_Minus_1P5_List);
			break;

		case SR200PC10_EV_MINUS_1P0:
			if(sr200pc10_720p_enable)
				sr200pc10_write_reg(client, sizeof(EV_Minus_1P0_HD_List), EV_Minus_1P0_HD_List);	 
			else
				sr200pc10_write_reg(client, sizeof(EV_Minus_1P0_List), EV_Minus_1P0_List);
			break;

		case SR200PC10_EV_MINUS_0P5:
			if(sr200pc10_720p_enable)
				sr200pc10_write_reg(client, sizeof(EV_Minus_0P5_HD_List), EV_Minus_0P5_HD_List);	 
			else
				sr200pc10_write_reg(client, sizeof(EV_Minus_0P5_List), EV_Minus_0P5_List);
			break;      

		case SR200PC10_EV_DEFAULT:
			if(sr200pc10_720p_enable) 		
				sr200pc10_write_reg(client, sizeof(EV_Default_HD_List), EV_Default_HD_List);	 
			else
				sr200pc10_write_reg(client, sizeof(EV_Default_List), EV_Default_List);
			break;

		case SR200PC10_EV_PLUS_0P5:
			if(sr200pc10_720p_enable)
				sr200pc10_write_reg(client, sizeof(EV_Plus_0P5_HD_List), EV_Plus_0P5_HD_List);	 
			else
				sr200pc10_write_reg(client, sizeof(EV_Plus_0P5_List), EV_Plus_0P5_List);
			break;

		case SR200PC10_EV_PLUS_1P0:
			if(sr200pc10_720p_enable)
				sr200pc10_write_reg(client, sizeof(EV_Plus_1P0_HD_List), EV_Plus_1P0_HD_List);	 
			else
				sr200pc10_write_reg(client, sizeof(EV_Plus_1P0_List), EV_Plus_1P0_List);
			break;

		case SR200PC10_EV_PLUS_1P5:
			if(sr200pc10_720p_enable)
				sr200pc10_write_reg(client, sizeof(EV_Plus_1P5_HD_List), EV_Plus_1P5_HD_List);	 
			else
				sr200pc10_write_reg(client, sizeof(EV_Plus_1P5_List), EV_Plus_1P5_List);
			break;

		case SR200PC10_EV_PLUS_2P0:
			if(sr200pc10_720p_enable)
				sr200pc10_write_reg(client, sizeof(EV_Plus_2P0_HD_List), EV_Plus_2P0_HD_List);	 
			else
				sr200pc10_write_reg(client, sizeof(EV_Plus_2P0_List), EV_Plus_2P0_List);
			break;      

		default:
			printk(SR200PC10_MOD_NAME "EV value is not supported!!!\n");
			return -EINVAL;
	}

	return err;
}


static int sr200pc10_set_ev(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_ev is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_EV_MINUS_2P0:
			//if(sr200pc10_write_reg(client, sizeof(EV_Minus_2P0_List), EV_Minus_2P0_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_cfg_brightness_0, \
				SR200PC10_CFG_BRIGHTNESS, \
				"sr200pc10_cfg_brightness_0")!=0)
				goto ev_fail;
			break;

		case SR200PC10_EV_MINUS_1P5:
			//if(sr200pc10_write_reg(client, sizeof(EV_Minus_1P5_List), EV_Minus_1P5_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_cfg_brightness_1, \
				SR200PC10_CFG_BRIGHTNESS, \
				"sr200pc10_cfg_brightness_1")!=0)
				goto ev_fail;
			break;

		case SR200PC10_EV_MINUS_1P0:
			//if(sr200pc10_write_reg(client, sizeof(EV_Minus_1P0_List), EV_Minus_1P0_List))
			if(s5k4ba_write_regs(client, \
				sr200pc10_cfg_brightness_2, \
				SR200PC10_CFG_BRIGHTNESS, \
				"sr200pc10_cfg_brightness_2")!=0)
				goto ev_fail;
			break;

		case SR200PC10_EV_MINUS_0P5:
			//if(sr200pc10_write_reg(client, sizeof(EV_Minus_0P5_List), EV_Minus_0P5_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_cfg_brightness_3, \
				SR200PC10_CFG_BRIGHTNESS, \
				"sr200pc10_cfg_brightness_3")!=0)
				goto ev_fail;
			break;      

		case SR200PC10_EV_DEFAULT:
			//if(sr200pc10_write_reg(client, sizeof(EV_Default_List), EV_Default_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_cfg_brightness_4, \
				SR200PC10_CFG_BRIGHTNESS, \
				"sr200pc10_cfg_brightness_4")!=0)			
				goto ev_fail;
			break;

		case SR200PC10_EV_PLUS_0P5:
			//if(sr200pc10_write_reg(client, sizeof(EV_Plus_0P5_List), EV_Plus_0P5_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_cfg_brightness_5, \
				SR200PC10_CFG_BRIGHTNESS, \
				"sr200pc10_cfg_brightness_5")!=0)
				goto ev_fail;
			break;

		case SR200PC10_EV_PLUS_1P0:
			//if(sr200pc10_write_reg(client, sizeof(EV_Plus_1P0_List), EV_Plus_1P0_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_cfg_brightness_6, \
				SR200PC10_CFG_BRIGHTNESS, \
				"sr200pc10_cfg_brightness_6")!=0)
				goto ev_fail;
			break;

		case SR200PC10_EV_PLUS_1P5:
			//if(sr200pc10_write_reg(client, sizeof(EV_Plus_1P5_List), EV_Plus_1P5_List))
			if(s5k4ba_write_regs(client, \
				sr200pc10_cfg_brightness_7, \
				SR200PC10_CFG_BRIGHTNESS, \
				"sr200pc10_cfg_brightness_7")!=0)
				goto ev_fail;
			break;

		case SR200PC10_EV_PLUS_2P0:
			//if(sr200pc10_write_reg(client, sizeof(EV_Plus_2P0_List), EV_Plus_2P0_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_cfg_brightness_8, \
				SR200PC10_CFG_BRIGHTNESS, \
				"sr200pc10_cfg_brightness_8")!=0)
				goto ev_fail;
			break;      

		default:
			printk(SR200PC10_MOD_NAME "EV value is not supported!!!\n");
			goto ev_fail;
	}

	sensor->ev = value;
#if 0
	if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
		goto ev_fail;
	if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
		goto ev_fail;
#endif

	return 0;

ev_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_ev is failed!!\n");
	return -EINVAL;           
}

static int sr200pc10_set_wdr(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_wdr is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_WDR_OFF:
			if(sr200pc10_write_reg(client, sizeof(WDR_OFF_list), WDR_OFF_list))
				goto wdr_fail;
			break;

		case SR200PC10_WDR_ON:
			if(sr200pc10_write_reg(client, sizeof(WDR_ON_list), WDR_ON_list))
				goto wdr_fail;
			break;

		default:
			printk(SR200PC10_MOD_NAME "WDR value is not supported!!!\n");
			goto wdr_fail;
	}

	sensor->wdr = value;

	return 0;

wdr_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_ev is failed!!\n");
	return -EINVAL;             
}

static int sr200pc10_set_saturation(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_saturation is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_SATURATION_MINUS_3:
			if(sr200pc10_write_reg(client, sizeof(Saturation_Minus3_List), Saturation_Minus3_List))
				goto saturation_fail;
			break;

		case SR200PC10_SATURATION_MINUS_2:
			if(sr200pc10_write_reg(client, sizeof(Saturation_Minus2_List), Saturation_Minus2_List))
				goto saturation_fail;
			break;

		case SR200PC10_SATURATION_MINUS_1:
			if(sr200pc10_write_reg(client, sizeof(Saturation_Minus1_List), Saturation_Minus1_List))
				goto saturation_fail;
			break;

		case SR200PC10_SATURATION_DEFAULT:
			if(sr200pc10_write_reg(client, sizeof(Saturation_Default_List), Saturation_Default_List))
				goto saturation_fail;
			break;

		case SR200PC10_SATURATION_PLUS_1:
			if(sr200pc10_write_reg(client, sizeof(Saturation_Plus1_List), Saturation_Plus1_List))
				goto saturation_fail;
			break;

		case SR200PC10_SATURATION_PLUS_2:
			if(sr200pc10_write_reg(client, sizeof(Saturation_Plus2_List), Saturation_Plus2_List))
				goto saturation_fail;
			break;

		case SR200PC10_SATURATION_PLUS_3:
			if(sr200pc10_write_reg(client, sizeof(Saturation_Plus3_List), Saturation_Plus3_List))
				goto saturation_fail;
			break;      

		default:
			printk(SR200PC10_MOD_NAME "Saturation value is not supported!!!\n");
			goto saturation_fail;
	}

	sensor->saturation = value;

	if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
		goto saturation_fail;
	if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
		goto saturation_fail;

	return 0;

saturation_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_saturation is failed!!\n");
	return -EINVAL;             
}

static int sr200pc10_set_sharpness(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_sharpness is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_SHARPNESS_MINUS_3:
			if(sr200pc10_write_reg(client, sizeof(SharpnessMinus3_list), SharpnessMinus3_list))
				goto sharpness_fail;
			break;

		case SR200PC10_SHARPNESS_MINUS_2:
			if(sr200pc10_write_reg(client, sizeof(SharpnessMinus2_list), SharpnessMinus2_list))
				goto sharpness_fail;
			break;

		case SR200PC10_SHARPNESS_MINUS_1:
			if(sr200pc10_write_reg(client, sizeof(SharpnessMinus1_list), SharpnessMinus1_list))
				goto sharpness_fail;
			break;

		case SR200PC10_SHARPNESS_DEFAULT:
			if(sr200pc10_write_reg(client, sizeof(SharpnessDefault_list), SharpnessDefault_list))
				goto sharpness_fail;
			break;

		case SR200PC10_SHARPNESS_PLUS_1:
			if(sr200pc10_write_reg(client, sizeof(SharpnessPlus1_list), SharpnessPlus1_list))
				goto sharpness_fail;
			break;

		case SR200PC10_SHARPNESS_PLUS_2:
			if(sr200pc10_write_reg(client, sizeof(SharpnessPlus2_list), SharpnessPlus2_list))
				goto sharpness_fail;
			break;

		case SR200PC10_SHARPNESS_PLUS_3:
			if(sr200pc10_write_reg(client, sizeof(SharpnessPlus3_list), SharpnessPlus3_list))
				goto sharpness_fail;
			break;      

		default:
			printk(SR200PC10_MOD_NAME "Sharpness value is not supported!!!\n");
			goto sharpness_fail;
	}

	sensor->sharpness = value;

	if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
		goto sharpness_fail;
	if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
		goto sharpness_fail;

	return 0;

sharpness_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_sharpness is failed!!\n");
	return -EINVAL;               
}

static int sr200pc10_set_contrast(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_contrast is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_CONTRAST_MINUS_3:
			if(sr200pc10_write_reg(client, sizeof(Contrast_Minus3_List), Contrast_Minus3_List))
				goto contrast_fail;
			break;

		case SR200PC10_CONTRAST_MINUS_2:
			if(sr200pc10_write_reg(client, sizeof(Contrast_Minus2_List), Contrast_Minus2_List))
				goto contrast_fail;
			break;

		case SR200PC10_CONTRAST_MINUS_1:
			if(sr200pc10_write_reg(client, sizeof(Contrast_Minus1_List), Contrast_Minus1_List))
				goto contrast_fail;
			break;

		case SR200PC10_CONTRAST_DEFAULT:
			if(sr200pc10_write_reg(client, sizeof(Contrast_default_List), Contrast_default_List))
				goto contrast_fail;
			break;

		case SR200PC10_CONTRAST_PLUS_1:
			if(sr200pc10_write_reg(client, sizeof(Contrast_Plus1_List), Contrast_Plus1_List))
				goto contrast_fail;
			break;

		case SR200PC10_CONTRAST_PLUS_2:
			if(sr200pc10_write_reg(client, sizeof(Contrast_Plus2_List), Contrast_Plus2_List))
				goto contrast_fail;
			break;

		case SR200PC10_CONTRAST_PLUS_3:
			if(sr200pc10_write_reg(client, sizeof(Contrast_Plus3_List), Contrast_Plus3_List))
				goto contrast_fail;
			break;      

		default:
			printk(SR200PC10_MOD_NAME "Contrast value is not supported!!!\n");
			goto contrast_fail;
	}

	sensor->contrast = value;

	if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
		goto contrast_fail;
	if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
		goto contrast_fail;

	return 0;

contrast_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_contrast is failed!!\n");
	return -EINVAL;               
}

static int sr200pc10_set_wb(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_wb is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_WB_AUTO:
			
			//if(sr200pc10_write_reg(client, sizeof(WB_Auto_Ctrl_List), WB_Auto_Ctrl_List)) //LYG
			if(s5k4ba_write_regs(client, \
				sr200pc10_cfg_wb_auto, \
				SR200PC_CFG_WB_AUTO, "sr200pc10_cfg_wb_auto")!=0)
				goto wb_fail;
			//if(sr200pc10_write_reg(client, sizeof(WB_Auto_List), WB_Auto_List))
				//goto wb_fail;
			break;

		case SR200PC10_WB_DAYLIGHT:
			//if(sr200pc10_write_reg(client, sizeof(WB_Daylight_Ctrl_List), WB_Daylight_Ctrl_List))
			if(s5k4ba_write_regs(client, \
				sr200pc10_cfg_wb_daylight, \
				SR200PC_CFG_WB_DAYLIGHT, "sr200pc10_cfg_wb_daylight")!=0)
				goto wb_fail;
			//if(sr200pc10_write_reg(client, sizeof(WB_Daylight_List), WB_Daylight_List))
				//goto wb_fail;
			break;

		case SR200PC10_WB_INCANDESCENT:
			//if(sr200pc10_write_reg(client, sizeof(WB_Incandescent_Ctrl_List), WB_Incandescent_Ctrl_List))
			if( s5k4ba_write_regs(client, \
				sr200pc10_cfg_wb_incandescent, \
				SR200PC_CFG_WB_INCANDESCENT, "sr200pc10_cfg_wb_incandescent")!=0)
				goto wb_fail;
			//if(sr200pc10_write_reg(client, sizeof(WB_Incandescent_List), WB_Incandescent_List))
				//goto wb_fail;
			break;

		case SR200PC10_WB_FLUORESCENT:
			//if(sr200pc10_write_reg(client, sizeof(WB_Fluorescent_Ctrl_List), WB_Fluorescent_Ctrl_List))
			if(s5k4ba_write_regs(client, \
				sr200pc10_cfg_wb_fluorescent, \
				SR200PC_CFG_WB_FLUORESCENT, "sr200pc10_cfg_wb_fluorescent")!=0)
				goto wb_fail;
			//if(sr200pc10_write_reg(client, sizeof(WB_Fluorescent_List), WB_Fluorescent_List))
				//goto wb_fail;
			break;

		case SR200PC10_WB_CLOUDY:
			//if(sr200pc10_write_reg(client, sizeof(WB_Cloudy_Ctrl_List), WB_Cloudy_Ctrl_List))
			if(s5k4ba_write_regs(client, \
				sr200pc10_cfg_wb_cloudy, \
				SR200PC_CFG_WB_CLOUDY, "sr200pc10_cfg_wb_cloudy")!=0)
				goto wb_fail;
			//if(sr200pc10_write_reg(client, sizeof(WB_Cloudy_List), WB_Cloudy_List))
				//goto wb_fail;
			break;

		default:
			printk(SR200PC10_MOD_NAME "WB value is not supported!!!\n");
			goto wb_fail;
	}

	sensor->wb = value;
#if 0
	if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
		goto wb_fail;
	if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
		goto wb_fail;
#endif
	return 0;

wb_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_wb is failed!!\n");
	return -EINVAL;                 
}

static int sr200pc10_set_isc(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_isc is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_ISC_STILL_OFF:
			if(sr200pc10_write_reg(client, sizeof(IS_Still_Off_list), IS_Still_Off_list))
				goto isc_fail;
			break;
		case SR200PC10_ISC_STILL_ON:
			if(sr200pc10_write_reg(client, sizeof(IS_Still_On_list), IS_Still_On_list))
				goto isc_fail;
			break;
		case SR200PC10_ISC_STILL_AUTO:
			if(sr200pc10_write_reg(client, sizeof(IS_Still_Auto_list), IS_Still_Auto_list))
				goto isc_fail;
			break;
		case SR200PC10_ISC_MOVIE_ON:
			if(sr200pc10_write_reg(client, sizeof(IS_Movie_On_list), IS_Movie_On_list))
				goto isc_fail;
			break;
		default:
			printk(SR200PC10_MOD_NAME SR200PC10_MOD_NAME "ISC value is not supported!!!\n");
			goto isc_fail;
	}

	sensor->isc = value;

	return 0;

isc_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_isc is failed!!\n");
	return -EINVAL;
}

static int sr200pc10_get_scene(struct v4l2_control *vc)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;
	u8 readdata[2] = {0x00,};

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_get_scene is called...\n"); 
#if 0
	if(sr200pc10_write_read_reg(client, 1, ASD_CheckStatus_List, 2, readdata))
		goto get_scene_fail;

	if(readdata[0] != 0x02)
	{
		vc->value = -EINVAL;
		printk(SR200PC10_MOD_NAME "Scene is not searched!!!\n");
	}
	else
	{
		vc->value = readdata[1];
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "Scene value is %d\n", vc->value);
	}
#endif
	return 0;

get_scene_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_get_scene is failed!!\n");
	return -EINVAL;  
}

static int sr200pc10_set_scene(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_scene is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_SCENE_OFF:
			night_shot_control(client, 0);  //LYG
			nightshot_in_init = 0;
			s5k4ba_write_regs(client, sr200pc10_preview,SR200PC10_PREVIEW_REGS, "sr200pc10_preview"); // preview start
#if 0			
			if(sr200pc10_write_reg(client, sizeof(SceneModeOff_List1), SceneModeOff_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeOff_List2), SceneModeOff_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeOff_List3), SceneModeOff_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeOff_List4), SceneModeOff_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeOff_List5), SceneModeOff_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeOff_List6), SceneModeOff_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeOff_List7), SceneModeOff_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeOff_List8), SceneModeOff_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeOff_List9), SceneModeOff_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
#endif			
			break;

		case SR200PC10_SCENE_ASD:
			if(sr200pc10_write_reg(client, sizeof(SceneModeASD_List1), SceneModeASD_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeASD_List2), SceneModeASD_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeASD_List3), SceneModeASD_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeASD_List4), SceneModeASD_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeASD_List5), SceneModeASD_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeASD_List6), SceneModeASD_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeASD_List7), SceneModeASD_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeASD_List8), SceneModeASD_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeASD_List9), SceneModeASD_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;    

		case SR200PC10_SCENE_PORTRAIT:
			if(sr200pc10_write_reg(client, sizeof(SceneModePortrait_List1), SceneModePortrait_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModePortrait_List2), SceneModePortrait_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModePortrait_List3), SceneModePortrait_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModePortrait_List4), SceneModePortrait_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModePortrait_List5), SceneModePortrait_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModePortrait_List6), SceneModePortrait_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModePortrait_List7), SceneModePortrait_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModePortrait_List8), SceneModePortrait_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModePortrait_List9), SceneModePortrait_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_LANDSCAPE:
			if(sr200pc10_write_reg(client, sizeof(SceneModeLanscape_List1), SceneModeLanscape_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeLanscape_List2), SceneModeLanscape_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeLanscape_List3), SceneModeLanscape_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeLanscape_List4), SceneModeLanscape_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeLanscape_List5), SceneModeLanscape_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeLanscape_List6), SceneModeLanscape_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeLanscape_List7), SceneModeLanscape_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeLanscape_List8), SceneModeLanscape_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeLanscape_List9), SceneModeLanscape_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_SUNSET:
			if(sr200pc10_write_reg(client, sizeof(SceneModeSunset_List1), SceneModeSunset_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSunset_List2), SceneModeSunset_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSunset_List3), SceneModeSunset_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSunset_List4), SceneModeSunset_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSunset_List5), SceneModeSunset_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSunset_List6), SceneModeSunset_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSunset_List7), SceneModeSunset_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSunset_List8), SceneModeSunset_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSunset_List9), SceneModeSunset_List9))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSunset_List10), SceneModeSunset_List10))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_DAWN:
			if(sr200pc10_write_reg(client, sizeof(SceneModeDawn_List1), SceneModeDawn_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeDawn_List2), SceneModeDawn_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeDawn_List3), SceneModeDawn_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeDawn_List4), SceneModeDawn_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeDawn_List5), SceneModeDawn_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeDawn_List6), SceneModeDawn_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeDawn_List7), SceneModeDawn_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeDawn_List8), SceneModeDawn_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeDawn_List9), SceneModeDawn_List9))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeDawn_List10), SceneModeDawn_List10))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_NIGHTSHOT:
			night_shot_control(client, 1);
			nightshot_in_init = 1;
			s5k4ba_write_regs(client, sr200pc10_preview,SR200PC10_PREVIEW_REGS, "sr200pc10_preview"); // preview start
#if 0			
			if(sr200pc10_write_reg(client, sizeof(SceneModeNightShot_List1), SceneModeNightShot_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeNightShot_List2), SceneModeNightShot_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeNightShot_List3), SceneModeNightShot_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeNightShot_List4), SceneModeNightShot_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeNightShot_List5), SceneModeNightShot_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeNightShot_List6), SceneModeNightShot_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeNightShot_List7), SceneModeNightShot_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeNightShot_List8), SceneModeNightShot_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeNightShot_List9), SceneModeNightShot_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
#endif			
			break;

		case SR200PC10_SCENE_TEXT:
			if(sr200pc10_write_reg(client, sizeof(SceneModeText_List1), SceneModeText_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeText_List2), SceneModeText_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeText_List3), SceneModeText_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeText_List4), SceneModeText_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeText_List5), SceneModeText_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeText_List6), SceneModeText_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeText_List7), SceneModeText_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeText_List8), SceneModeText_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeText_List9), SceneModeText_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_SPORTS:
			if(sr200pc10_write_reg(client, sizeof(SceneModeSports_List1), SceneModeSports_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSports_List2), SceneModeSports_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSports_List3), SceneModeSports_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSports_List4), SceneModeSports_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSports_List5), SceneModeSports_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSports_List6), SceneModeSports_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSports_List7), SceneModeSports_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSports_List8), SceneModeSports_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeSports_List9), SceneModeSports_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_AGAINST_LIGHT:
			if(sr200pc10_write_reg(client, sizeof(SceneModeAgainstLight_List1), SceneModeAgainstLight_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeAgainstLight_List2), SceneModeAgainstLight_List2))
				goto scene_fail;
			if(sensor->flash_capture == SR200PC10_FLASH_CAPTURE_ON)
			{
				if(sr200pc10_write_reg(client, sizeof(SceneModeAgainstLight_List3_0), SceneModeAgainstLight_List3_0))
					goto scene_fail;
			}
			else
			{
				if(sr200pc10_write_reg(client, sizeof(SceneModeAgainstLight_List3_1), SceneModeAgainstLight_List3_1))
					goto scene_fail;
			}
			if(sr200pc10_write_reg(client, sizeof(SceneModeAgainstLight_List4), SceneModeAgainstLight_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeAgainstLight_List5), SceneModeAgainstLight_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeAgainstLight_List6), SceneModeAgainstLight_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeAgainstLight_List7), SceneModeAgainstLight_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeAgainstLight_List8), SceneModeAgainstLight_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeAgainstLight_List9), SceneModeAgainstLight_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_INDOORS:
			if(sr200pc10_write_reg(client, sizeof(SceneModeIndoor_List1), SceneModeIndoor_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeIndoor_List2), SceneModeIndoor_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeIndoor_List3), SceneModeIndoor_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeIndoor_List4), SceneModeIndoor_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeIndoor_List5), SceneModeIndoor_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeIndoor_List6), SceneModeIndoor_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeIndoor_List7), SceneModeIndoor_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeIndoor_List8), SceneModeIndoor_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeIndoor_List9), SceneModeIndoor_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_BEACH_SNOW:
			if(sr200pc10_write_reg(client, sizeof(SceneModeBeachSnow_List1), SceneModeBeachSnow_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeBeachSnow_List2), SceneModeBeachSnow_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeBeachSnow_List3), SceneModeBeachSnow_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeBeachSnow_List4), SceneModeBeachSnow_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeBeachSnow_List5), SceneModeBeachSnow_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeBeachSnow_List6), SceneModeBeachSnow_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeBeachSnow_List7), SceneModeBeachSnow_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeBeachSnow_List8), SceneModeBeachSnow_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeBeachSnow_List9), SceneModeBeachSnow_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_FALLCOLOR:
			if(sr200pc10_write_reg(client, sizeof(SceneModeFallColor_List1), SceneModeFallColor_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFallColor_List2), SceneModeFallColor_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFallColor_List3), SceneModeFallColor_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFallColor_List4), SceneModeFallColor_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFallColor_List5), SceneModeFallColor_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFallColor_List6), SceneModeFallColor_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFallColor_List7), SceneModeFallColor_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFallColor_List8), SceneModeFallColor_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFallColor_List9), SceneModeFallColor_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_FIREWORKS:
			if(sr200pc10_write_reg(client, sizeof(SceneModeFireWorks_List1), SceneModeFireWorks_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFireWorks_List2), SceneModeFireWorks_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFireWorks_List3), SceneModeFireWorks_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFireWorks_List4), SceneModeFireWorks_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFireWorks_List5), SceneModeFireWorks_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFireWorks_List6), SceneModeFireWorks_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFireWorks_List7), SceneModeFireWorks_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFireWorks_List8), SceneModeFireWorks_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeFireWorks_List9), SceneModeFireWorks_List9))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		case SR200PC10_SCENE_CANDLELIGHT:
			if(sr200pc10_write_reg(client, sizeof(SceneModeCandlelight_List1), SceneModeCandlelight_List1))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeCandlelight_List2), SceneModeCandlelight_List2))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeCandlelight_List3), SceneModeCandlelight_List3))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeCandlelight_List4), SceneModeCandlelight_List4))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeCandlelight_List5), SceneModeCandlelight_List5))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeCandlelight_List6), SceneModeCandlelight_List6))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeCandlelight_List7), SceneModeCandlelight_List7))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeCandlelight_List8), SceneModeCandlelight_List8))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeCandlelight_List9), SceneModeCandlelight_List9))
				goto scene_fail;
			if(sr200pc10_write_reg(client, sizeof(SceneModeCandlelight_List10), SceneModeCandlelight_List10))
				goto scene_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto scene_fail;
			break;

		default:
			printk(SR200PC10_MOD_NAME "Scene value is not supported!!!\n");
			goto scene_fail;
	}

	sensor->scene = value;  

	return 0;

scene_fail:
	printk(SR200PC10_MOD_NAME " sr200pc10_set_scene is failed!!\n");
	return -EINVAL;    
}

static int sr200pc10_set_mode(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	sensor->mode = value;
	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_mode is called... mode = %d\n", sensor->mode); 
	return 0;
}

static int sr200pc10_set_state(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	sensor->state = value;
	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_state is called... state = %d\n", sensor->state); 
	return 0;
}

static int sr200pc10_set_aewb(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_aewb is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_AE_LOCK_AWB_LOCK:
			if(sr200pc10_write_reg(client, sizeof(AEWB_Lock_list), AEWB_Lock_list))
				goto aewb_fail;
			break;
		case SR200PC10_AE_LOCK_AWB_UNLOCK:
			if(sr200pc10_write_reg(client, sizeof(AE_Lock_AWB_Unlock_list), AE_Lock_AWB_Unlock_list))
				goto aewb_fail;
			break;
		case SR200PC10_AE_UNLOCK_AWB_LOCK:
			if(sr200pc10_write_reg(client, sizeof(AE_Unlock_AWB_Lock_list), AE_Unlock_AWB_Lock_list))
				goto aewb_fail;
			break;
		case SR200PC10_AE_UNLOCK_AWB_UNLOCK:
			if(sr200pc10_write_reg(client, sizeof(AEWB_UnLock_list), AEWB_UnLock_list))
				goto aewb_fail;
			break;
		default:
			printk(SR200PC10_MOD_NAME "AE/AWB value is not supported!!!\n");
			goto aewb_fail;
	}

	sensor->aewb = value;

	return 0;

aewb_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_aewb is failed!!!\n");
	return -EINVAL;
}

static int sr200pc10_set_antishake(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_antishake is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_ANTI_SHAKE_OFF:
			if(sr200pc10_write_reg(client, sizeof(AntiShake_OFF_list), AntiShake_OFF_list))
				goto antishake_fail;
			break;
		case SR200PC10_ANTI_SHAKE_ON:
			if(sr200pc10_write_reg(client, sizeof(AntiShake_ON_list), AntiShake_ON_list))
				goto antishake_fail;
			break;
		default:
			printk(SR200PC10_MOD_NAME "Anti-shake value is not supported!!!\n");
			goto antishake_fail;
	}

	sensor->antishake = value;

	return 0;

antishake_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_antishake is failed!!!\n");
	return -EINVAL;  
}

static int sr200pc10_set_flash_capture(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_flash_capture is called...[%d]\n",value);

	switch(value)
	{
		case SR200PC10_FLASH_CAPTURE_OFF:
			if(sr200pc10_write_reg(client, sizeof(FlashCaptureOff_list), FlashCaptureOff_list))
				goto flash_capture_fail;
			break;
		case SR200PC10_FLASH_CAPTURE_ON:
			if(sr200pc10_write_reg(client, sizeof(FlashCaptureOn_list), FlashCaptureOn_list))
				goto flash_capture_fail;
			break;
		case SR200PC10_FLASH_CAPTURE_AUTO:
			if(sr200pc10_write_reg(client, sizeof(FlashCaptureAuto_list), FlashCaptureAuto_list))
				goto flash_capture_fail;
			break;
		default:
			printk(SR200PC10_MOD_NAME "Flash value is not supported!!!\n");
			goto flash_capture_fail;
	}

	sensor->flash_capture = value;  

	return 0;

flash_capture_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_flash_capture is failed!!!\n");  
	return -EINVAL;    
}

static int sr200pc10_set_flash_movie(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_flash_movie is called...%d\n", value);

	switch(value)
	{
		case SR200PC10_FLASH_MOVIE_OFF:
			if(sr200pc10_write_reg(client, sizeof(FlashMovieOff_list), FlashMovieOff_list))
				goto flash_movie_fail;
			break;

		case SR200PC10_FLASH_MOVIE_ON:
			if(sr200pc10_write_reg(client, sizeof(FlashMovieOn_list), FlashMovieOn_list))
				goto flash_movie_fail;
			break;

		default:
			printk(SR200PC10_MOD_NAME "Flash value is not supported!!!\n");
			goto flash_movie_fail;
	}

	sensor->flash_movie = value;

	return 0;

flash_movie_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_flash_movie is failed!!!\n");  
	return -EINVAL;      
}

static int sr200pc10_set_flash_ctrl(bool value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_flash_ctrl is called...value : %d, mode : %d\n", value, sensor->flash_capture);

	switch(sensor->flash_capture)
	{
		case SR200PC10_FLASH_CAPTURE_OFF:
			if(sr200pc10_write_reg(client, sizeof(FlashAFOff_list), FlashAFOff_list))
				goto flash_ctrl_fail;
			if(sr200pc10_write_reg(client, sizeof(FlashCaptureOff_list), FlashCaptureOff_list))
				goto flash_ctrl_fail;
			break;

		case SR200PC10_FLASH_CAPTURE_ON:
			if(value)
			{
				if(sr200pc10_write_reg(client, sizeof(FlashCaptureOn_list), FlashCaptureOn_list))
					goto flash_ctrl_fail;
				if(sr200pc10_write_reg(client, sizeof(FlashAFOff_list), FlashAFOff_list))
					goto flash_ctrl_fail;
			}
			else
			{
				if(sr200pc10_write_reg(client, sizeof(FlashAFOn_list), FlashAFOn_list))
					goto flash_ctrl_fail;
				if(sr200pc10_write_reg(client, sizeof(FlashCaptureOff_list), FlashCaptureOff_list))
					goto flash_ctrl_fail;
			}
			break;

		case SR200PC10_FLASH_CAPTURE_AUTO:
			if(value)
			{
				if(sr200pc10_write_reg(client, sizeof(FlashCaptureAuto_list), FlashCaptureAuto_list))
					goto flash_ctrl_fail;
				if(sr200pc10_write_reg(client, sizeof(FlashAFOff_list), FlashAFOff_list))
					goto flash_ctrl_fail;
			}
			else
			{
				if(sr200pc10_write_reg(client, sizeof(FlashAFAuto_list), FlashAFAuto_list))
					goto flash_ctrl_fail;
				if(sr200pc10_write_reg(client, sizeof(FlashCaptureOff_list), FlashCaptureOff_list))
					goto flash_ctrl_fail;
			}
			break;

		default:
			printk(SR200PC10_MOD_NAME "Flash value is not supported!!!\n");
			goto flash_ctrl_fail;
	}

	return 0;

flash_ctrl_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_flash_ctrl is failed!!!\n");
	return -EINVAL;   
}

static int sr200pc10_set_jpeg_quality(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_jpeg_quality is called... value : %d\n", value);

	switch(value) 
	{
		case SR200PC10_JPEG_SUPERFINE:
			sensor->jpeg_quality = SR200PC10_JPEG_SUPERFINE;
			break;
		case SR200PC10_JPEG_FINE:
			sensor->jpeg_quality = SR200PC10_JPEG_FINE;
			break;
		case SR200PC10_JPEG_NORMAL:
			sensor->jpeg_quality = SR200PC10_JPEG_NORMAL;
			break;
		case SR200PC10_JPEG_ECONOMY:
			sensor->jpeg_quality = SR200PC10_JPEG_ECONOMY;
			break;
		default:
			printk(SR200PC10_MOD_NAME "JPEG quality value is not supported!\n");
			goto jpeg_quality_fail;
	}

	return 0;

jpeg_quality_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_jpeg_quality is failed!!!\n");
	return -EINVAL;    
}

static int sr200pc10_set_focus_status(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	int cnt = 0;

	u8 readdata = 0x00;
	u8 status = 0x00;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_focus_status is called...[%d]\n",value);

	if(sr200pc10_curr_state != SR200PC10_STATE_PREVIEW)
	{
		printk(SR200PC10_MOD_NAME "Sensor is not preview state!!");
		goto focus_status_fail;
	}

	switch(value) 
	{
		case SR200PC10_AF_START :
			dprintk(CAM_DBG, SR200PC10_MOD_NAME "AF start.\n");
			if(!sr200pc10_touch_state)
			{
				sr200pc10_write_reg(client, sizeof(Lense_TouchAFOff_List), Lense_TouchAFOff_List);
			}

			/* Lenz stop check */

			do
			{
				if(sr200pc10_write_read_reg(client, 1, Lense_CheckStatus_List, 1, &readdata))
					goto focus_status_fail;

				status = (readdata & 0x0F);
				if(status == 0x01 || status == 0x05)
				{
					mdelay(10);
					cnt++; 
				}
				else
				{
					break;
				}
			}while((status == 0x01 || status == 0x05) && cnt < 200);

			if(sr200pc10_write_reg(client, sizeof(Lense_AFStart_List), Lense_AFStart_List))
				goto focus_status_fail;
			sr200pc10_touch_state = false;
			break;

		case SR200PC10_AF_STOP :
			dprintk(CAM_DBG, SR200PC10_MOD_NAME "AF stop.\n");

			if(sr200pc10_write_reg(client, sizeof(Lense_AFCancel_List), Lense_AFCancel_List))
				goto focus_status_fail;
#if 0
			/* Lenz stop check */
			do
			{
				if(sr200pc10_write_read_reg(client, 1, Lense_CheckStatus_List, 1, &readdata))
					goto focus_status_fail;

				status = (readdata & 0x0F);
				if(status == 0x01 || status == 0x05)
				{
					mdelay(10);
					cnt++; 
				}
				else
				{
					break;
				}
			}while((status == 0x01 || status == 0x05) && cnt < 200);
#endif
			if(sensor->focus_mode == SR200PC10_AF_INIT_MACRO) // Macro off
			{
				if(sr200pc10_write_reg(client, sizeof(Lense_AFMacroMode_list), Lense_AFMacroMode_list)) // Set Base Position
					goto focus_status_fail;
			}
			else
			{
				if(sr200pc10_write_reg(client, sizeof(Lense_AFStandardMode_list), Lense_AFStandardMode_list)) // Set Base Position
					goto focus_status_fail;
			}      

			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto focus_status_fail;

			/* AE/AWB Unlock */
			if(sensor->focus_mode == SR200PC10_AF_INIT_FACE)
			{
				if(sr200pc10_write_reg(client, sizeof(FaceDetection_UnLock_list), FaceDetection_UnLock_list))
					goto focus_status_fail;
				if(sr200pc10_write_reg(client, sizeof(FaceDetection_box_On_list), FaceDetection_box_On_list))
					goto focus_status_fail;
			}
			else
			{
				if(sr200pc10_write_reg(client, sizeof(Lense_TouchAFOff_List), Lense_TouchAFOff_List))
					goto focus_status_fail;	  
				if(sr200pc10_write_reg(client, sizeof(AEWB_UnLock_list), AEWB_UnLock_list))  // AEWB unlock
					goto focus_status_fail;
			}
			
			break;

		default:
			printk(SR200PC10_MOD_NAME "[af]Invalid value is ordered!!!\n");
			goto focus_status_fail;
	}

	return 0;

focus_status_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_focus_status is failed!!!\n");  
	return -EINVAL;    
}

static int sr200pc10_set_focus_touch(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	u16 touch_x = (u16)sensor->position.x;
	u16 touch_y = (u16)sensor->position.y;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_focus_touch is called... x : %d, y : %d\n", touch_x, touch_y); 

	u8 x_offset = 0x34;
	u8 y_offset = 0x24;  

	u8 Touch_AF_list[12] = {0x4D, 0x01, 0x03, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00};

	Touch_AF_list[4]  = ((touch_x - x_offset) & 0x00FF);
	Touch_AF_list[5]  = (((touch_x - x_offset) & 0xFF00) >> 8);
	Touch_AF_list[6]  = ((touch_y - y_offset) & 0x00FF);
	Touch_AF_list[7]  = (((touch_y - y_offset) & 0xFF00) >> 8);
	Touch_AF_list[8]  = ((touch_x + x_offset) & 0x00FF);
	Touch_AF_list[9]  = (((touch_x + x_offset) & 0xFF00) >> 8);
	Touch_AF_list[10] = ((touch_y + y_offset) & 0x00FF);
	Touch_AF_list[11] = (((touch_y + y_offset) & 0xFF00) >> 8);  

	if(sr200pc10_write_reg(client, sizeof(Touch_AF_list), Touch_AF_list))
		goto focus_touch_fail;

	sr200pc10_touch_state = true;

	return 0;

focus_touch_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_focus_touch is failed!!!\n");
	return -EINVAL;     
}
/*
   static int sr200pc10_set_focus_touch_on(void)
   {
   struct sr200pc10_sensor *sensor = &sr200pc10;
   struct i2c_client *client = sensor->i2c_client;

   u16 touch_x = (u16)(sr200pc10_preview_sizes[sensor->preview_size].width / 2);
   u16 touch_y = (u16)(sr200pc10_preview_sizes[sensor->preview_size].height / 2);

   dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_focus_touch_on is called... x : %d, y : %d\n", touch_x, touch_y); 

   u8 x_offset = 0x2b;
   u8 y_offset = 0x1b;  

   u8 Touch_AF_list[12] = {0x4D, 0x01, 0x03, 0x01,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00};

   Touch_AF_list[4]  = ((touch_x - x_offset) & 0x00FF);
   Touch_AF_list[5]  = (((touch_x - x_offset) & 0xFF00) >> 8);
   Touch_AF_list[6]  = ((touch_y - y_offset) & 0x00FF);
   Touch_AF_list[7]  = (((touch_y - y_offset) & 0xFF00) >> 8);
   Touch_AF_list[8]  = ((touch_x + x_offset) & 0x00FF);
   Touch_AF_list[9]  = (((touch_x + x_offset) & 0xFF00) >> 8);
   Touch_AF_list[10] = ((touch_y + y_offset) & 0x00FF);
   Touch_AF_list[11] = (((touch_y + y_offset) & 0xFF00) >> 8);  

   if(sr200pc10_write_reg(client, sizeof(Touch_AF_list), Touch_AF_list))
   goto focus_touch_fail;

   return 0;

focus_touch_fail:
printk(SR200PC10_MOD_NAME "sr200pc10_set_focus_touch_on is failed!!!\n");
return -EINVAL;     
}

static int sr200pc10_set_focus_touch_off(void)
{
struct sr200pc10_sensor *sensor = &sr200pc10;
struct i2c_client *client = sensor->i2c_client;

dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_focus_touch_off is called...\n"); 

u8 Touch_AF_list[12] = {0x4D, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00};

if(sr200pc10_write_reg(client, sizeof(Touch_AF_list), Touch_AF_list))
goto focus_touch_fail;

return 0;

focus_touch_fail:
printk(SR200PC10_MOD_NAME "sr200pc10_set_focus_touch_off is failed!!!\n");
return -EINVAL;     
}
 */

static int sr200pc10_get_focus(struct v4l2_control *vc)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	u8 readdata = 0x00;
	u8 status = 0x00;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_get_auto_focus is called...\n"); 
#if 0
	if(sr200pc10_write_read_reg(client, 1, Lense_CheckStatus_List, 1, &readdata))
		goto get_focus_fail;

	status = (readdata & 0x0F);

	/*
	   - 0x00 : unfocused + stopped
	   - 0x01 : unfocused + lens moving   
	   - 0x02 : focused + stopped (AF)    
	   - 0x04 : status invalid + stopped    
	   - 0x05 : status invalid + lens moving (AF )
	*/    

	switch(status){
		case 0 :
			vc->value = SR200PC10_AF_STATUS_FAIL;
			break;
		case 2 :
			vc->value = SR200PC10_AF_STATUS_SUCCESS;
			/* AE/AWB Lock */
			if(sensor->focus_mode == SR200PC10_AF_INIT_NORMAL || sensor->focus_mode == SR200PC10_AF_INIT_MACRO)
			{
				sr200pc10_write_reg(client, sizeof(AEWB_Lock_list), AEWB_Lock_list);
			}
			break;
		case 4 :
			//break;
		default :
			vc->value = SR200PC10_AF_STATUS_PROGRESS;		
	}

	dprintk(CAM_DBG, SR200PC10_MOD_NAME "AF result = %d (1.progress, 2.success, 3.fail)\n", vc->value);
#endif
	return 0;

get_focus_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_get_focus is failed!!!\n");
	return -EINVAL;      
}

static int sr200pc10_set_focus(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "AF set value = %d\n", value);

	switch(value) 
	{
		case SR200PC10_AF_INIT_NORMAL : 
			if(sr200pc10_write_reg(client, sizeof(FaceDetection_Off_list), FaceDetection_Off_list))
				goto focus_fail;
			if(sr200pc10_write_reg(client, sizeof(Lense_AFStandardMode_list), Lense_AFStandardMode_list))
				goto focus_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto focus_fail;
			break;

		case SR200PC10_AF_INIT_MACRO : 
			if(sr200pc10_write_reg(client, sizeof(FaceDetection_Off_list), FaceDetection_Off_list))
				goto focus_fail;
			if(sr200pc10_write_reg(client, sizeof(Lense_AFMacroMode_list), Lense_AFMacroMode_list))
				goto focus_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto focus_fail;
			break;

		case SR200PC10_AF_INIT_FACE :
			if(sr200pc10_write_reg(client, sizeof(Lense_AFStandardMode_list), Lense_AFStandardMode_list))
				goto focus_fail;
			if(sr200pc10_poll_reg(client, RCommandAFIdleStatus))
				goto focus_fail;
			if(sr200pc10_write_reg(client, sizeof(FaceDetection_On_list), FaceDetection_On_list))
				goto focus_fail;
			break;  

		case SR200PC10_AF_INIT_CONTINUOUS :
			if(sr200pc10_write_reg(client, sizeof(Lense_TouchAFOff_List), Lense_TouchAFOff_List))
				goto focus_fail;
			if(sr200pc10_write_reg(client, sizeof(FaceDetection_Off_list), FaceDetection_Off_list))
				goto focus_fail;
			if(sr200pc10_write_reg(client, sizeof(Lense_AFContinuousMode_list), Lense_AFContinuousMode_list))
				goto focus_fail;
			if(sr200pc10_poll_reg(client, RCommandZoomReleaseStatus))
				goto focus_fail;
			break;  	  
		default:
			printk(SR200PC10_MOD_NAME "[af]Invalid value is ordered!!!\n");
			goto focus_fail;   
	}

	sensor->focus_mode = value;

	return 0;

focus_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_focus is failed!!!\n");
	return -EINVAL;     
}

static int sr200pc10_get_zoom(struct v4l2_control *vc)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	u8 readdata[2] = {0x0,};

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_get_zoom is called...\n"); 
#if 0
	if(sr200pc10_write_read_reg(client, 1, Lense_CheckDZoomStatus_List, 2, readdata))
		goto get_zoom_fail;
	vc->value = (u8)(2560/(readdata[0] +1));
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "Zoom value... %d \n", vc->value);
#endif
	return 0;

get_zoom_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_get_zoom is failed!!!\n");
	return -EINVAL;   
}


static int sr200pc10_set_zoom(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_zoom is called... value = %d\n", value); 

	unsigned char sr200pc10_buf_dzoom_List[2] = { 0xB9, 0x00 };

	//sr200pc10_buf_dzoom_List[1] = sr200pc10_buf_set_dzoom[value];

	if(sr200pc10_write_reg(client, sizeof(sr200pc10_buf_dzoom_List), sr200pc10_buf_dzoom_List))
		goto zoom_fail;

	/*  switch(value)
	    {
	    case SR200PC10_ZOOM_1P00X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_1p00x_List), Lense_SetDZoom_1p00x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_1P25X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_1p25x_List), Lense_SetDZoom_1p25x_List))
	    goto zoom_fail;
	    break;    

	    case SR200PC10_ZOOM_1P50X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_1p50x_List), Lense_SetDZoom_1p50x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_1P75X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_1p75x_List), Lense_SetDZoom_1p75x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_2P00X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_2p00x_List), Lense_SetDZoom_2p00x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_2P25X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_2p25x_List), Lense_SetDZoom_2p25x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_2P50X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_2p50x_List), Lense_SetDZoom_2p50x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_2P75X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_2p75x_List), Lense_SetDZoom_2p75x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_3P00X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_3p00x_List), Lense_SetDZoom_3p00x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_3P25X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_3p25x_List), Lense_SetDZoom_3p25x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_3P50X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_3p50x_List), Lense_SetDZoom_3p50x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_3P75X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_3p75x_List), Lense_SetDZoom_3p75x_List))
	    goto zoom_fail;
	    break;

	    case SR200PC10_ZOOM_4P00X:
	    if(sr200pc10_write_reg(client, sizeof(Lense_SetDZoom_4p00x_List), Lense_SetDZoom_4p00x_List))
	    goto zoom_fail;
	    break;

	    default:
	    printk(SR200PC10_MOD_NAME "[zoom]Invalid value is ordered!!!\n");
	    goto zoom_fail;
	    }
	*/
		if(sr200pc10_poll_reg(client, RCommandDZoomFinishStatus))
			goto zoom_fail;  

	sensor->zoom = value;

	return 0;

zoom_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_zoom is failed!!!\n");
	return -EINVAL;   
}


static int sr200pc10_get_jpeg_size(struct v4l2_control *vc)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;
	u8 readdata[8];

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_get_jpeg_size is called...\n"); 
#if 0	
	if(sr200pc10_write_read_reg(client, sizeof(MakeImagesInLump_Status), MakeImagesInLump_Status, 8, readdata))
		goto get_jpeg_size_fail;

	vc->value = (readdata[3]<<16) + (readdata[2]<<8) + readdata[1];
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "sr200pc10_get_jpeg_size::Main JPEG Size reading... 0x%x      Q value...0x%x\n", vc->value, readdata[0]);
#endif
	return 0;

get_jpeg_size_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_get_jpeg_size is failed!!!\n");
	return -EINVAL;     
}

static int sr200pc10_get_thumbnail_size(struct v4l2_control *vc)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;
	u8 readdata[8] = {0x00,};

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_get_thumbnail_size is called...\n");  
#if 0
	if(sr200pc10_write_read_reg(client, sizeof(MakeImagesInLump_Status1), MakeImagesInLump_Status1, 8, readdata))
		goto get_thumbnail_size_fail;

	vc->value = (readdata[3]<<16) + (readdata[2]<<8) + readdata[1];
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "Thumb JPEG Size reading... 0x%x      Q value...0x%x\n", vc->value, readdata[4]);
#endif
	return 0;

get_thumbnail_size_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_get_thumbnail_size is failed!!!\n");
	return -EINVAL;     
}

static int sr200pc10_prepare_preview(void)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	u8  readdata[1] = {0x00};

	int cnt = 0;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_prepare_preview is called...\n");
#if 0 // 0602 
	/* AF check idle status */
	do
	{
		if(sr200pc10_write_read_reg(client, 1, Lense_CheckStatus_List, 1, readdata))
			goto prepare_preview_fail;
		if((*readdata & 0x0F) == 0x01 || (*readdata & 0x0F) == 0x05) //moving
		{
			mdelay(10);
			cnt++; 
		}
		else
		{
			break;
		}
	}while(((*readdata & 0x0F) == 0x01 || (*readdata & 0x0F) == 0x05) && cnt < 200);  

	/* stop check */
	if(sr200pc10_write_read_reg(client, 1, CameraReadCommand_CaptureStatus, 1, readdata))
		goto prepare_preview_fail;

	if(*readdata != 0x00)
	{
		printk(SR200PC10_MOD_NAME "Preview is not stop. Need to stop!!\n");
		if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreviewStop_list), ConfigSensorPreviewStop_list))
			goto prepare_preview_fail;
		if(sr200pc10_poll_reg(client, RCommandCaptureStatusCheck))
			goto prepare_preview_fail;
	}
#endif 
	return 0;

prepare_preview_fail:    
	printk(SR200PC10_MOD_NAME "sr200pc10_prepare_preview is failed!!!\n");
	return -EINVAL;     
}

static int sr200pc10_start_preview(void)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	printk(SR200PC10_MOD_NAME "sr200pc10_start_preview is called...preview_size = %d\n",sensor->preview_size);
	switch(sensor->preview_size)
	{
		case SR200PC10_PREVIEW_SIZE_640_480:
			//s5k4ba_write_regs(client, sr200pc10_preview,SR200PC10_PREVIEW_REGS, "sr200pc10_preview"); // preview start
			s5k4ba_write_regs(client, sr200pc10_preview_640x480,SR200PC10_PREVIEW_640x480_REGS, "sr200pc10_preview_640x480"); // preview start
			break;
		case SR200PC10_PREVIEW_SIZE_320_240:
			s5k4ba_write_regs(client, sr200pc10_preview_320x240,SR200PC10_PREVIEW_320x240_REGS, "sr200pc10_preview_320x240"); // preview start
			break;
		case SR200PC10_PREVIEW_SIZE_176_144:
			s5k4ba_write_regs(client, sr200pc10_preview_176x144,SR200PC10_PREVIEW_176x144_REGS, "sr200pc10_preview_176x144"); // preview start
			break;
		default:
			s5k4ba_write_regs(client, sr200pc10_preview,SR200PC10_PREVIEW_REGS, "sr200pc10_preview"); // preview start
			break;
			//goto start_preview_fail;
	}
	
#if 0
	/* Preview Start */
	if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreviewStart_list), ConfigSensorPreviewStart_list))
		goto start_preview_fail;
	if(sr200pc10_poll_reg(client, RCommandPreviewStatusCheck))
		goto start_preview_fail;
#endif
	return 0;

start_preview_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_start_preview is failed\n"); 
	return -EINVAL;  
}

static int sr200pc10_set_preview(void)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	u8  FixedFPS_list[3] = {0x5A, 0x1E, 0x00}; 

	printk(SR200PC10_MOD_NAME "sr200pc10_set_preview is called...%d\n", sensor->preview_size);

	sr200pc10_pre_state = sr200pc10_curr_state;
	sr200pc10_curr_state = SR200PC10_STATE_PREVIEW;    

	/* Set Preview setting (fps, iso, 720p...) */
	if(sr200pc10_720p_enable) {

		dprintk(CAM_DBG, SR200PC10_MOD_NAME "Preview is 720P.!!\n");
		if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreview720P_on_list), ConfigSensorPreview720P_on_list))
			goto preview_fail;
		mdelay(5); 
		//    if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreview720P_chk_list), ConfigSensorPreview720P_chk_list))
		//      goto preview_fail;

		/* Set preview size */  
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "Preview size is %d\n", sensor->preview_size);
		if(sensor->fps > 30) { // slow motion
			if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreview_Fast[sensor->preview_size + 6]), ConfigSensorPreview_Fast[sensor->preview_size + 6]))
				goto preview_fail;
		} else {
			if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreview[sensor->preview_size]), ConfigSensorPreview[sensor->preview_size]))
				goto preview_fail;
		}

		/* Fliker 50Hz */
		if(sr200pc10_write_reg(client, sizeof(ConfigSensorfliker_list_50Hz), ConfigSensorfliker_list_50Hz))
			goto preview_fail;	

		/* Set FPS */
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "fps is %d. set 0x%x\n", sensor->fps, (u8)sensor->fps);
		if(sensor->fps > 120) {
			printk(SR200PC10_MOD_NAME "Invalid fps not supported\n"); 
			goto preview_fail;
		} else {  
			FixedFPS_list[1] = (u8)sensor->fps;
			if(sr200pc10_write_reg(client, sizeof(FixedFPS_list), FixedFPS_list))
				goto preview_fail;
		}  

		// Set ISO
		if(sr200pc10_write_reg(client, sizeof(ISOCamcorderHD_list), ISOCamcorderHD_list))
			goto preview_fail;
		// Set HD Matrix 
		if(sr200pc10_write_reg(client, sizeof(Photometry_MatrixHD_List), Photometry_MatrixHD_List))
			goto preview_fail;
		// Set HD EV
		if(sr200pc10_set_ev_no_apply(sensor->ev))
			goto preview_fail;
		// Set AE Speed Ctrl for HD
		if(sr200pc10_write_reg(client, sizeof(AE_Speed_Ctrl_2sec_HD), AE_Speed_Ctrl_2sec_HD))
			goto preview_fail;
		// Set HD Gamma 
		if(sr200pc10_write_reg(client, sizeof(Gammal_HD), Gammal_HD))
			goto preview_fail;
		// Set UV Mapping
		if(sr200pc10_write_reg(client, sizeof(Preview_UV_HD), Preview_UV_HD))
			goto preview_fail;

		if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
			goto preview_fail;
		if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
			goto preview_fail;

	} else {

		if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreview720P_off_list), ConfigSensorPreview720P_off_list))
			goto preview_fail;
		mdelay(5); 
		if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreview720P_chk_list), ConfigSensorPreview720P_chk_list))
			goto preview_fail;

		/* Set preview size */  
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "Preview size is %d\n", sensor->preview_size);
		if(sensor->fps > 30) { // slow motion
			if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreview_Fast[sensor->preview_size + 6]), ConfigSensorPreview_Fast[sensor->preview_size + 6]))
				goto preview_fail;
		} else {
			if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreview[sensor->preview_size]), ConfigSensorPreview[sensor->preview_size]))
				goto preview_fail;
		}

		/* Fliker 50Hz */
		if(sr200pc10_write_reg(client, sizeof(ConfigSensorfliker_list_50Hz), ConfigSensorfliker_list_50Hz))
			goto preview_fail;	

		/* Set FPS */
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "fps is %d. set 0x%x\n", sensor->fps, (u8)sensor->fps);
		if(sensor->fps > 120) {
			printk(SR200PC10_MOD_NAME "Invalid fps not supported\n"); 
			goto preview_fail;
		} else {  
			FixedFPS_list[1] = (u8)sensor->fps;
			if(sr200pc10_write_reg(client, sizeof(FixedFPS_list), FixedFPS_list))
				goto preview_fail;
		}		

		if(sensor->mode == SR200PC10_MODE_CAMCORDER) {        
			//fixed 30fps
			if(sr200pc10_write_reg(client, sizeof(ISOCamcorder_list), ISOCamcorder_list))
				goto preview_fail;
		} else {
			//unfixed fps
			if(sensor->scene == SR200PC10_SCENE_OFF)
				if(sr200pc10_set_iso(sensor->iso))
					goto preview_fail;
		}

		if(sr200pc10_write_reg(client, sizeof(AE_Speed_Ctrl_Normal), AE_Speed_Ctrl_Normal))
			goto preview_fail;
		if(sr200pc10_write_reg(client, sizeof(Gamma_Normal), Gamma_Normal))
			goto preview_fail;
		if(sr200pc10_write_reg(client, sizeof(Preview_UV), Preview_UV))
			goto preview_fail;

		if(sr200pc10_write_reg(client, sizeof(BatchReflectionRequest_list), BatchReflectionRequest_list))
			goto preview_fail;
		if(sr200pc10_poll_reg(client, RCommandBatchReflectionStatus))
			goto preview_fail;
	}

	/* AE/AWB Enable */
	if(sr200pc10_write_reg(client, sizeof(AEWB_UnLock_list), AEWB_UnLock_list))
		goto preview_fail;

	/* Capture & AF Flash off */ 
	//  if(sr200pc10_set_flash_ctrl(false))
	//    goto preview_fail;

	return 0;

preview_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_preview is failed\n"); 
	return -EINVAL;
}

static int sr200pc10_prepare_capture(void)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	u8  readdata[1] = {0x00};

	int cnt = 0;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_prepare_capture is called...\n");
#if 0
	/* AF check idle status */
	do
	{
		if(sr200pc10_write_read_reg(client, 1, Lense_CheckStatus_List, 1, readdata))
			goto prepare_capture_fail;
		if((*readdata & 0x0F) == 0x01 || (*readdata & 0x0F) == 0x05) //moving
		{
			mdelay(10);
			cnt++; 
		}
		else
		{
			break;
		}
	}while(((*readdata & 0x0F) == 0x01 || (*readdata & 0x0F) == 0x05) && cnt < 200);  

	/* stop check */
	if(sr200pc10_write_read_reg(client, 1, CameraReadCommand_CaptureStatus, 1, readdata))
		goto prepare_capture_fail;

	if(*readdata != 0x08)
	{
		printk(SR200PC10_MOD_NAME "Preview is not start. Need to start!!\n");
		if(sr200pc10_write_reg(client, sizeof(ConfigSensorPreviewStart_list), ConfigSensorPreviewStart_list))
			goto prepare_capture_fail;
		if(sr200pc10_poll_reg(client, RCommandPreviewStatusCheck))
			goto prepare_capture_fail;
	}
#endif
	return 0;

prepare_capture_fail:    
	printk(SR200PC10_MOD_NAME "sr200pc10_prepare_capture is failed!!!\n");
	return -EINVAL;     
}

static int sr200pc10_start_capture(void)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;
	struct v4l2_pix_format* pix = &sensor->pix;

	u32 value;
	u8 readdata[3] = {0x0,};

	printk(SR200PC10_MOD_NAME "sr200pc10_start_capture is called...capture_size = %d\n",sensor->capture_size); 
	
#if 0
	/* Data Output Setting */
	if(pix->pixelformat == V4L2_PIX_FMT_UYVY || pix->pixelformat == V4L2_PIX_FMT_YUYV)
	{
		printk(SR200PC10_MOD_NAME "sr200pc10_start_capture is called...V4L2_PIX_FMT_UYVY\n"); 
		 s5k4ba_write_regs(client, \
				sr200pc10_capture_table, \
				SR200PC10_CAPTURE_REGS, \
				"sr200pc10_capture_table");
		if(sr200pc10_write_read_reg(client, sizeof(YUVDataOutputSetting), YUVDataOutputSetting, 3, readdata))
			goto start_capture_fail;
	}
	else
	{
		printk(SR200PC10_MOD_NAME "sr200pc10_start_capture is called...JPEGDataOutputSetting\n"); 
		s5k4ba_write_regs(client, \
				sr200pc10_capture_table, \
				SR200PC10_CAPTURE_REGS, \
				"sr200pc10_capture_table");
		if(sr200pc10_write_read_reg(client, sizeof(JPEGDataOutputSetting), JPEGDataOutputSetting, 3, readdata))
			goto start_capture_fail;
	}
	value = (readdata[2]<<16) + (readdata[1]<<8) + readdata[0];
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "Main Image Output Size reading... 0x%x \n", value);

	/* Data Output Request */
	if(sr200pc10_write_read_reg(client, sizeof(DataOutputRequest), DataOutputRequest, 3, readdata))
		goto start_capture_fail;
	if(sr200pc10_poll_reg(client, RCommandDataTransmissionCheck))
		goto start_capture_fail;
	value = (readdata[2]<<16) + (readdata[1]<<8) + readdata[0];
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "Main Image Transmit Size reading... 0x%x \n", value);
#endif

	switch(sensor->capture_size)
	{
		case SR200PC10_IMAGE_SIZE_1600_1200:
			s5k4ba_write_regs(client, sr200pc10_capture_1600x1200_table, SR200PC10_CAPTURE_1600x1200_REGS, "sr200pc10_capture_1600x1200_table");
			break;
		case SR200PC10_IMAGE_SIZE_1280_960:
			s5k4ba_write_regs(client, sr200pc10_capture_1280x960_table, SR200PC10_CAPTURE_1280x960_REGS, "sr200pc10_capture_1280x960_table");
			break;
		case SR200PC10_IMAGE_SIZE_640_480:
			s5k4ba_write_regs(client, sr200pc10_capture_640x480_table, SR200PC10_CAPTURE_640x480_REGS, "sr200pc10_capture_640x480_table");
			break;
		case SR200PC10_IMAGE_SIZE_320_240:
			s5k4ba_write_regs(client, sr200pc10_capture_320x240_table, SR200PC10_CAPTURE_320x240_REGS, "sr200pc10_capture_320x240_table");
			break;
		default:
			s5k4ba_write_regs(client, sr200pc10_capture_table, SR200PC10_CAPTURE_REGS, "sr200pc10_capture_table");
			break;
			//goto start_preview_fail;
	}
	
	return 0;

start_capture_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_start_capture is failed\n"); 
	return -EINVAL;  
}

static int sr200pc10_set_capture(void)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	u8 Write_BV_Data[3] = {0x17,0x00,0x00};
	u8 Read_BV_Data[8] = {0x00,};  

	printk(SR200PC10_MOD_NAME "sr200pc10_set_capture is called... %d\n", sensor->capture_size);

	sr200pc10_pre_state = sr200pc10_curr_state;
	sr200pc10_curr_state = SR200PC10_STATE_CAPTURE;
#if 0
	/* AE/AWB unlock */
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "AE/AWB lock.\n");
	if(sr200pc10_write_reg(client, sizeof(AEWB_UnLock_list), AEWB_UnLock_list))
		goto capture_fail;

	/* Capture flash setting */
	if(sr200pc10_set_flash_ctrl(true))
		goto capture_fail;

	/* Set Capture Size */  
	dprintk(CAM_INF, SR200PC10_MOD_NAME "capture size is %d\n", sensor->capture_size);
	if(sr200pc10_write_reg(client, sizeof(ConfigSensorBufferingCapture[sensor->capture_size]), ConfigSensorBufferingCapture[sensor->capture_size]))
		goto capture_fail;

	/* Zoom setting */
	if(sr200pc10_set_zoom(sensor->zoom))
		goto capture_fail;

	/* Get BV */ 
	if(sr200pc10_write_read_reg(client, sizeof(Write_BV_Data), Write_BV_Data, sizeof(Read_BV_Data), Read_BV_Data))
		goto capture_fail;
	sensor->bv = Read_BV_Data[6]|(Read_BV_Data[7]*256);    

	/* AE/AWB lock */
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "AE/AWB lock.\n");
	if(sr200pc10_write_reg(client, sizeof(AEWB_Lock_list), AEWB_Lock_list))
		goto capture_fail;

	/* Buffering Capture Start! */
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "Buffering Capture Start.\n");
	if(sr200pc10_write_reg(client, sizeof(BufferingCaptureStart_list), BufferingCaptureStart_list))
		goto capture_fail;
	if(sr200pc10_poll_reg(client, RCommandCaptureStatusCheck))
		goto capture_fail;

	/* Set JPEG quality */
	switch(sensor->jpeg_quality)
	{
		dprintk(CAM_INF, "Quality is %d\n", sensor->jpeg_quality);
		case SR200PC10_JPEG_SUPERFINE:
		if(sr200pc10_write_reg(client, sizeof(JpegCompression_SuperFine_List), JpegCompression_SuperFine_List))
			goto capture_fail;
		break;

		case SR200PC10_JPEG_FINE:
		if(sr200pc10_write_reg(client, sizeof(JpegCompression_Fine_List), JpegCompression_Fine_List))
			goto capture_fail;
		break;

		case SR200PC10_JPEG_NORMAL:
		if(sr200pc10_write_reg(client, sizeof(JpegCompression_Normal_List), JpegCompression_Normal_List))
			goto capture_fail;
		break;

		case SR200PC10_JPEG_ECONOMY:
		if(sr200pc10_write_reg(client, sizeof(JpegCompression_Economy_List), JpegCompression_Economy_List))
			goto capture_fail;
		break;

		default:
		printk(SR200PC10_MOD_NAME "Capture quality not supported\n");
		goto capture_fail;
	}

	/* Start JPEG compression */
	if(sr200pc10_write_reg(client, sizeof(MakeImagesInLump[sensor->capture_size]), MakeImagesInLump[sensor->capture_size]))
		goto capture_fail;
	if(sr200pc10_poll_reg(client, RCommandCaptureStatusCheck))
		goto capture_fail;
#endif
	return 0;

capture_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_capture is failed\n"); 
	return -EINVAL;     
}

static void sr200pc10_set_skip(void)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;

	int skip_frame = 0; 

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_skip is called...\n");

	if(sensor->state == SR200PC10_STATE_PREVIEW)
	{      
		dprintk(CAM_INF, SR200PC10_MOD_NAME "BV level = %d\n", sensor->bv);

		if(sensor->bv > 500) {
			if(sensor->scene == SR200PC10_SCENE_NIGHTSHOT)
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
		skip_frame = 1;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "skip frame = %d frame\n", skip_frame);

	isp_set_hs_vs(0,skip_frame);
}

static int ioctl_streamoff(struct v4l2_int_device *s)
{
	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_streamoff is called...\n");

	return 0;
}

static int ioctl_streamon(struct v4l2_int_device *s)
{
	struct sr200pc10_sensor *sensor = s->priv;  
	int ret;
	//struct sr200pc10_sensor *sensor = &sr200pc10;
	//struct i2c_client *client = sensor->i2c_client;
	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_streamon is called...(%x)\n", sensor->state);  

	if(sensor->state != SR200PC10_STATE_CAPTURE)
	{
		
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "start preview....................\n");

		//ret = s5k4ba_write_regs(sd, sr200pc10_preview,SR200PC10_PREVIEW_REGS, "sr200pc10_preview"); // preview start
		//ret = s5k4ba_write_regs(client, sr200pc10_preview,SR200PC10_PREVIEW_REGS, "sr200pc10_preview"); // preview start

		//sr200pc10_start_preview();
#if 1 // LYG
		if(sensor->check_dataline)
		{
			if(sr200pc10_check_dataline())
				goto streamon_fail;
		}
		else
		{
			/* Zoom setting */
			//if(sr200pc10_set_zoom(sensor->zoom))
				//goto streamon_fail;

			if(sr200pc10_start_preview())
				goto streamon_fail;    
		}
#endif		
#if 0
		if(sensor->mode == SR200PC10_MODE_CAMCORDER)    
		{
			/* Lens focus setting */
			if(sr200pc10_set_focus(SR200PC10_AF_INIT_CONTINUOUS))
				goto streamon_fail;       
		}
#endif
	}
	else
	{
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "start capture....................\n");
		if(sr200pc10_start_capture())
			goto streamon_fail;
	}

	return 0;

streamon_fail:
	printk(SR200PC10_MOD_NAME "ioctl_streamon is failed\n"); 
	return -EINVAL;   
}

/**
 * ioctl_queryctrl - V4L2 sensor interface handler for VIDIOC_QUERYCTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @qc: standard V4L2 VIDIOC_QUERYCTRL ioctl structure
 *
 * If the requested control is supported, returns the control information
 * from the sr200pc10_ctrl_list[] array.
 * Otherwise, returns -EINVAL if the control is not supported.
 */
static int ioctl_queryctrl(struct v4l2_int_device *s,
		struct v4l2_queryctrl *qc)
{
	int i;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_queryctrl is called...\n");

	for (i = 0; i < NUM_SR200PC10_CONTROL; i++) 
	{
		if (qc->id == sr200pc10_ctrl_list[i].id)
		{
			break;
		}
	}
	if (i == NUM_SR200PC10_CONTROL)
	{
		printk(SR200PC10_MOD_NAME "Control ID is not supported!!\n");
		qc->flags = V4L2_CTRL_FLAG_DISABLED;
		goto queryctrl_fail;
	}

	*qc = sr200pc10_ctrl_list[i];

	return 0;

queryctrl_fail:
	printk(SR200PC10_MOD_NAME "ioctl_queryctrl is failed\n"); 
	return -EINVAL;     
}

#define CMD_VERSION			0x00
#define DATA_VERSION_FW		0x00
#define DATA_VERSION_DATE	0x01
#define DATA_VERSION_SENSOR	0x03
#define DATA_VERSION_AF		0x05
#define DATA_VERSION_SENSOR_MAKER 0xE0

static int sr200pc10_get_version(struct v4l2_int_device *sd, int object_id, unsigned char version_info[])
{
	struct sr200pc10_sensor *sensor = sd->priv;
	struct i2c_client *client = sensor->i2c_client;

	unsigned char cmd_buf[3] = {0x00,0x00,0x00};
	int err;

	cmd_buf[0] = CMD_VERSION;
	switch(object_id)
	{
		case DATA_VERSION_FW:
		case DATA_VERSION_DATE:
		case DATA_VERSION_SENSOR:
		case DATA_VERSION_AF:	
		case DATA_VERSION_SENSOR_MAKER:
			cmd_buf[1] = object_id;
			break;
		default:
			return -EINVAL;
	}
#if 0	
	err = sr200pc10_write_read_reg(client, sizeof(cmd_buf), cmd_buf, 4, version_info);
	if(err < 0)
		return -EIO;
#endif
	return 0;
}

static int sr200pc10_get_fw_version(struct v4l2_int_device *sd)
{
	struct sr200pc10_sensor *sensor = sd->priv;
	unsigned char version_info[4] = {0x00, 0x00, 0x00, 0x00};
	int err = -1;

	err = sr200pc10_get_version(sd, DATA_VERSION_FW, version_info);

	if(err < 0) 
		return  err;

	sensor->fw.minor = version_info[0];
	sensor->fw.major = version_info[1];

	sensor->prm.minor = version_info[2];
	sensor->prm.major = version_info[3];

	return 0;
}

static int sr200pc10_get_dateinfo(struct v4l2_int_device *sd)
{
	struct sr200pc10_sensor *sensor = sd->priv;
	unsigned char version_info[4] = {0x00, 0x00, 0x00, 0x00};
	int err = -1;

	err = sr200pc10_get_version(sd, DATA_VERSION_DATE, version_info);

	if(err < 0) 
		return  err;

	sensor->dateinfo.year  = version_info[0] - 'A' + 2007;
	sensor->dateinfo.month = version_info[1] - 'A' + 1;
	sensor->dateinfo.date  = version_info[2];

	return 0;
}

static int sr200pc10_get_sensor_version(struct v4l2_int_device *sd)
{
	struct sr200pc10_sensor *sensor = sd->priv;
	unsigned char version_info[4] = {0x00, 0x00, 0x00, 0x00};
	int err = -1;

	err = sr200pc10_get_version(sd, DATA_VERSION_SENSOR, version_info);

	if(err < 0) 
		return  err;

	sensor->sensor_version = version_info[0];

	return 0;
}

static int sr200pc10_get_sensor_maker_version(struct v4l2_int_device *sd)
{
	struct sr200pc10_sensor *sensor = sd->priv;
	unsigned char version_info[4] = {0x00, 0x00, 0x00, 0x00};
	int err = -1;

	err = sr200pc10_get_version(sd, DATA_VERSION_SENSOR_MAKER, version_info);

	if(err < 0) 
		return  err;

	sensor->sensor_info.maker = version_info[0];
	sensor->sensor_info.optical = version_info[1];

	return 0;
}

static int sr200pc10_get_af_version(struct v4l2_int_device *sd)
{
	struct sr200pc10_sensor *sensor = sd->priv;
	unsigned char version_info[4] = {0x00, 0x00, 0x00, 0x00};
	int err = -1;

	err = sr200pc10_get_version(sd, DATA_VERSION_AF, version_info);

	if(err < 0) 
		return  err;

	//printk("sr200pc10_get_af_version: data0: 0x%02x, data1: 0x%02x\n", version_info[0], version_info[1]);	

	sensor->af_info.low = version_info[1];
	sensor->af_info.high = version_info[0];

	return 0;
}

static int sr200pc10_get_gamma_version(struct v4l2_int_device *sd)
{
	struct sr200pc10_sensor *sensor = sd->priv;
	struct i2c_client *client = sensor->i2c_client;

	unsigned char gamma_info[2] = {0x00, 0x00};
	unsigned int info_len = 2;	
	int err = -1;

	unsigned char rg_low_buf[3]  = {0xE0, 0x0C, 0x00};
	unsigned char rg_high_buf[3] = {0xE0, 0x0D, 0x00};
	unsigned char bg_low_buf[3]  = {0xE0, 0x0E, 0x00};
	unsigned char bg_high_buf[3] = {0xE0, 0x0F, 0x00};	
#if 0
	err = sr200pc10_write_read_reg(client, sizeof(rg_low_buf), rg_low_buf, info_len, gamma_info);
	if(err < 0)
		return -EIO;	

	sensor->gamma.rg_low = gamma_info[1];
	//printk("sr200pc10_get_gamma_version1: data1: 0x%02x, data1: 0x%02x\n", gamma_info[0], gamma_info[1]);		

	err = sr200pc10_write_read_reg(client, sizeof(rg_high_buf), rg_high_buf, info_len, gamma_info);
	if(err < 0)
		return -EIO;	

	sensor->gamma.rg_high = gamma_info[1];
	//printk("sr200pc10_get_gamma_version1: data1: 0x%02x, data1: 0x%02x\n", gamma_info[0], gamma_info[1]);	

	err = sr200pc10_write_read_reg(client, sizeof(bg_low_buf), bg_low_buf, info_len, gamma_info);
	if(err < 0)
		return -EIO;	

	sensor->gamma.bg_low = gamma_info[1];
	//printk("sr200pc10_get_gamma_version1: data1: 0x%02x, data1: 0x%02x\n", gamma_info[0], gamma_info[1]);	

	err = sr200pc10_write_read_reg(client, sizeof(bg_high_buf), bg_high_buf, info_len, gamma_info);
	if(err < 0)
		return -EIO;		
#endif
	sensor->gamma.bg_high= gamma_info[1];
	//printk("sr200pc10_get_gamma_version1: data1: 0x%02x, data1: 0x%02x\n", gamma_info[0], gamma_info[1]);	

	return 0;
}

static int sr200pc10_get_fw_data(struct v4l2_int_device *sd)
{
	struct sr200pc10_sensor *sensor = sd->priv;
	int err = -EINVAL;

	err = sr200pc10_get_fw_version(sd);
	if(err < 0){
		printk(SR200PC10_MOD_NAME "%s: Failed: Reading firmware version\n",__func__);
		return -EIO;
	}

	//printk("sr200pc10_get_fw_data: sr200pc10_get_fw_version is ok\n");

	err = sr200pc10_get_dateinfo(sd);
	if(err < 0){
		printk(SR200PC10_MOD_NAME "%s: Failed: Reading dateinfo\n",__func__);
		return -EIO;
	}

	//printk("sr200pc10_get_fw_data: sr200pc10_get_dateinfo is ok\n");

	err = sr200pc10_get_sensor_version(sd);
	if(err < 0){
		printk(SR200PC10_MOD_NAME "%s: Failed: Reading sensor info\n",__func__);
		return -EIO;
	}

	//printk("sr200pc10_get_fw_data: sr200pc10_get_sensor_version is ok\n");

	err = sr200pc10_get_sensor_maker_version(sd);
	if(err < 0){
		printk(SR200PC10_MOD_NAME "%s: Failed: Reading maker info\n",__func__);
		return -EIO;
	}

	err = sr200pc10_get_af_version(sd);
	if(err < 0){
		printk(SR200PC10_MOD_NAME "%s: Failed: Reading af info\n",__func__);
		return -EIO;
	}

	err = sr200pc10_get_gamma_version(sd);
	if(err < 0){
		printk(SR200PC10_MOD_NAME "%s: Failed: Reading camera gamma info\n",__func__);
		return -EIO;
	}


	printk(SR200PC10_MOD_NAME "FW  Version: %d.%d\n", sensor->fw.major, sensor->fw.minor);
	printk(SR200PC10_MOD_NAME "PRM Version: %d.%d\n", sensor->prm.major, sensor->prm.minor);
	printk(SR200PC10_MOD_NAME "Date(y.m.d): %d.%d.%d\n", sensor->dateinfo.year, sensor->dateinfo.month, sensor->dateinfo.date);	
	printk(SR200PC10_MOD_NAME "Sensor version: %d\n", sensor->sensor_version);	

	return 0;
}


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
	struct sr200pc10_sensor *sensor = s->priv;

	int retval = 0;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_g_ctrl is called...id(%x)\n", vc->id);

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
			retval = sr200pc10_get_focus(vc);
			break;
		case V4L2_CID_ZOOM:
			retval = sr200pc10_get_zoom(vc);
			break;
		case V4L2_CID_JPEG_SIZE:
			retval = sr200pc10_get_jpeg_size(vc);
			break;
		case V4L2_CID_THUMBNAIL_SIZE:
			retval = sr200pc10_get_thumbnail_size(vc);
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
			retval = sr200pc10_get_scene(vc);
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
			vc->value = ISP_FW_ver[3] | (ISP_FW_ver[2] << 8) | (ISP_FW_ver[1] << 16) | (ISP_FW_ver[0] << 24);
			break;
		case V4L2_CID_FW_LASTEST:
			vc->value = SR200PC10_PRM_MAJOR_VERSION | (SR200PC10_PRM_MINOR_VERSION << 8) |(SR200PC10_FW_MAJOR_VERSION << 16) | (SR200PC10_FW_MINOR_VERSION << 24);
			break;
		case V4L2_CID_FW_DATE:
			retval = sr200pc10_fw_date(vc);
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
			printk(SR200PC10_MOD_NAME "[id]Invalid value is ordered!!!\n");
			break;
	}

	return retval;
}

static int sr200pc10_set_face_lock(s32 value)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_set_face_lock is called...[%d]\n",value);	

	switch(value) 
	{
		case FACE_LOCK_ON:
			if(sr200pc10_write_reg(client, sizeof(FaceDetection_Lock_list), FaceDetection_Lock_list))
				goto face_lock_fail;			
			break;
		case FIRST_FACE_TRACKING:
			if(sr200pc10_write_reg(client, sizeof(FaceDetection_Tracking_list), FaceDetection_Tracking_list))
				goto face_lock_fail;	  
			break;
		case FACE_LOCK_OFF:			
		default:
			if(sr200pc10_write_reg(client, sizeof(FaceDetection_UnLock_list), FaceDetection_UnLock_list))
				goto face_lock_fail;	  
			break;
	}

	return 0;

face_lock_fail:
	printk(SR200PC10_MOD_NAME "sr200pc10_set_aewb is failed!!!\n");
	return -EINVAL;	
}

/**
 * ioctl_s_ctrl - V4L2 sensor interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_S_CTRL ioctl structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW (and updates the sr200pc10 sensor struct).
 * Otherwise, * returns -EINVAL if the control is not supported.
 */
static int ioctl_s_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	struct sr200pc10_sensor *sensor = s->priv;
	int retval = 0;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_s_ctrl is called...id(%x), value(%d)\n", vc->id, vc->value);

	switch (vc->id) 
	{
		case V4L2_CID_SELECT_MODE:
			retval = sr200pc10_set_mode(vc->value);
			break;  
		case V4L2_CID_SELECT_STATE:
			retval = sr200pc10_set_state(vc->value);
			break;       
		case V4L2_CID_FOCUS_MODE:
			retval =0;// sr200pc10_set_focus(vc->value);
			break;
		case V4L2_CID_AF:
			retval = 0;sr200pc10_set_focus_status(vc->value);
			break;
		case V4L2_CID_ZOOM:
			retval = 0;sr200pc10_set_zoom(vc->value);
			break;
		case V4L2_CID_JPEG_QUALITY:
			retval = sr200pc10_set_jpeg_quality(vc->value);
			break;
		case V4L2_CID_ISO:
			retval = sr200pc10_set_iso(vc->value);
			break;
		case V4L2_CID_BRIGHTNESS:
			retval = sr200pc10_set_ev(vc->value);
			break;
		case V4L2_CID_CONTRAST:
			retval = sr200pc10_set_contrast(vc->value);
			break;      
		case V4L2_CID_WB:
			retval = sr200pc10_set_wb(vc->value);
			break;
		case V4L2_CID_SATURATION:
			retval = sr200pc10_set_saturation(vc->value);
			break;
		case V4L2_CID_EFFECT:
			retval = sr200pc10_set_effect(vc->value);
			break;
		case V4L2_CID_SCENE:
			retval = sr200pc10_set_scene(vc->value);
			break;
		case V4L2_CID_PHOTOMETRY:
			retval = sr200pc10_set_photometry(vc->value);
			break;
		case V4L2_CID_WDR:
			retval = sr200pc10_set_wdr(vc->value);
			break;
		case V4L2_CID_SHARPNESS:
			retval = sr200pc10_set_sharpness(vc->value);
			break;
		case V4L2_CID_ISC:
			retval = sr200pc10_set_isc(vc->value);
			break;
		case V4L2_CID_AEWB:
			retval = sr200pc10_set_aewb(vc->value);
			break;
		case V4L2_CID_ANTISHAKE:
			retval = 0;//sr200pc10_set_antishake(vc->value);
			break;      
		case V4L2_CID_FW_UPDATE:
			sensor->pdata->power_set(V4L2_POWER_OFF);
			mdelay(100);
			sensor->pdata->power_set(V4L2_POWER_ON); 
			retval = sr200pc10_update_firmware(vc->value);
			break;
		case V4L2_CID_FLASH_CAPTURE:
			retval = sr200pc10_set_flash_capture(vc->value);
			break;
		case V4L2_CID_FLASH_MOVIE:
			retval = sr200pc10_set_flash_movie(vc->value);
			break;
		case V4L2_CID_CAM_FW_VER:
			retval = 0; //sr200pc10_get_fw_data(s);
			break;
		case V4L2_CID_CAM_DUMP_FW:
			retval = 0; //sr200pc10_dump_fw(s);
			break;	  
		case V4L2_CID_CAMERA_CHECK_DATALINE:
			sensor->check_dataline = vc->value;
			retval = 0;
			break;	
		case V4L2_CID_CAMERA_CHECK_DATALINE_STOP:
			retval = sr200pc10_check_dataline_stop();
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
			retval = sr200pc10_set_aewb(vc->value);
			break;
		case V4L2_CID_CAMERA_FACEDETECT_LOCKUNLOCK:
			retval = sr200pc10_set_face_lock(vc->value);
			break;
		case V4L2_CID_CAMERA_TOUCH_AF_START_STOP:
			retval = sr200pc10_set_focus_touch(vc->value);
			break; 
		default:
			printk(SR200PC10_MOD_NAME "[id]Invalid value is ordered!!!\n");
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

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_enum_fmt_cap is called...\n");

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
					printk(SR200PC10_MOD_NAME "[format]Invalid value is ordered!!!\n");
					goto enum_fmt_cap_fail;
			}
			break;

		default:
			printk(SR200PC10_MOD_NAME "[type]Invalid value is ordered!!!\n");
			goto enum_fmt_cap_fail;
	}

	fmt->flags = sr200pc10_formats[index].flags;
	fmt->pixelformat = sr200pc10_formats[index].pixelformat;
	strlcpy(fmt->description, sr200pc10_formats[index].description, sizeof(fmt->description));

	dprintk(CAM_DBG, SR200PC10_MOD_NAME "ioctl_enum_fmt_cap flag : %d\n", fmt->flags);
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "ioctl_enum_fmt_cap description : %s\n", fmt->description);

	return 0;

enum_fmt_cap_fail:
	printk(SR200PC10_MOD_NAME "ioctl_enum_fmt_cap is failed\n"); 
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
	struct sr200pc10_sensor *sensor = s->priv;
	struct v4l2_pix_format *pix2 = &sensor->pix;

	int index = 0;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_try_fmt_cap is called...\n");
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "ioctl_try_fmt_cap. mode : %d\n", sensor->mode);
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "ioctl_try_fmt_cap. state : %d\n", sensor->state);

	sr200pc10_set_skip();  

	if(sensor->state == SR200PC10_STATE_CAPTURE)
	{ 
		for(index = 0; index < ARRAY_SIZE(sr200pc10_image_sizes); index++)
		{
			if(sr200pc10_image_sizes[index].width == pix->width && sr200pc10_image_sizes[index].height == pix->height)
			{
				sensor->capture_size = index;
				break;
			}
		}   

		if(index == ARRAY_SIZE(sr200pc10_image_sizes))
		{
			printk(SR200PC10_MOD_NAME "Capture Image Size is not supported!\n");
			goto try_fmt_fail;
		}

		dprintk(CAM_DBG, SR200PC10_MOD_NAME "SR200PC10--capture size = %d\n", sensor->capture_size);  
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "SR200PC10--capture width : %d\n", sr200pc10_image_sizes[index].width);
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "SR200PC10--capture height : %d\n", sr200pc10_image_sizes[index].height);      

		if(pix->pixelformat == V4L2_PIX_FMT_UYVY || pix->pixelformat == V4L2_PIX_FMT_YUYV)
		{
			pix->field = V4L2_FIELD_NONE;
			pix->bytesperline = pix->width * 2;
			pix->sizeimage = pix->bytesperline * pix->height;
			dprintk(CAM_DBG, SR200PC10_MOD_NAME "V4L2_PIX_FMT_UYVY\n");
		}
		else
		{
			pix->field = V4L2_FIELD_NONE;
			pix->bytesperline = JPEG_CAPTURE_WIDTH * 2;
			pix->sizeimage = pix->bytesperline * JPEG_CAPTURE_HEIGHT;
			dprintk(CAM_DBG, SR200PC10_MOD_NAME "V4L2_PIX_FMT_JPEG\n");
		}

		dprintk(CAM_DBG, SR200PC10_MOD_NAME "set capture....................\n");

		if(sr200pc10_set_capture())
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
	printk(SR200PC10_MOD_NAME "ioctl_try_fmt_cap is failed\n"); 
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
	struct sr200pc10_sensor *sensor = s->priv;
	struct v4l2_pix_format *pix2 = &sensor->pix;

	int index = 0;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_s_fmt_cap is called...\n");

	sr200pc10_720p_enable = false;

	printk(SR200PC10_MOD_NAME "camera mode  : %d (1:camera , 2:camcorder, 3:vt)\n", sensor->mode);
	printk(SR200PC10_MOD_NAME "camera state : %d (0:preview, 1:snapshot)\n", sensor->state);
	printk(SR200PC10_MOD_NAME "set width  : %d\n", pix->width);
	printk(SR200PC10_MOD_NAME "set height : %d\n", pix->height); 

	if(sensor->state == SR200PC10_STATE_CAPTURE)
	{ 
		/* check for capture */
		if(sr200pc10_prepare_capture())
			goto s_fmt_fail;   

		sr200pc10_set_skip();  

		for(index = 0; index < ARRAY_SIZE(sr200pc10_image_sizes); index++)
		{
			if(sr200pc10_image_sizes[index].width == pix->width && sr200pc10_image_sizes[index].height == pix->height)
			{
				sensor->capture_size = index;
				break;
			}
		}   

		if(index == ARRAY_SIZE(sr200pc10_image_sizes))
		{
			printk(SR200PC10_MOD_NAME "Capture Image %d x %d Size is not supported!\n", pix->width, pix->height);
			goto s_fmt_fail;
		}

		dprintk(CAM_DBG, SR200PC10_MOD_NAME "SR200PC10--capture size = %d\n", sensor->capture_size);  
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "SR200PC10--capture width : %d\n", sr200pc10_image_sizes[index].width);
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "SR200PC10--capture height : %d\n", sr200pc10_image_sizes[index].height);      

		if(pix->pixelformat == V4L2_PIX_FMT_UYVY || pix->pixelformat == V4L2_PIX_FMT_YUYV)
		{
			pix->field = V4L2_FIELD_NONE;
			pix->bytesperline = pix->width * 2;
			pix->sizeimage = pix->bytesperline * pix->height;
			dprintk(CAM_DBG, SR200PC10_MOD_NAME "V4L2_PIX_FMT_UYVY\n");
		}
		else
		{
			pix->field = V4L2_FIELD_NONE;
			pix->bytesperline = JPEG_CAPTURE_WIDTH * 2;
			pix->sizeimage = pix->bytesperline * JPEG_CAPTURE_HEIGHT;
			dprintk(CAM_DBG, SR200PC10_MOD_NAME "V4L2_PIX_FMT_JPEG\n");
		}

		if(sr200pc10_set_capture())
			goto s_fmt_fail;
	}  
	else
	{  
		/* check for preview */
		if(sr200pc10_prepare_preview())
			goto s_fmt_fail;

		//    sr200pc10_set_skip();  

		for(index = 0; index < ARRAY_SIZE(sr200pc10_preview_sizes); index++)
		{
			if(sr200pc10_preview_sizes[index].width == pix->width && sr200pc10_preview_sizes[index].height == pix->height)
			{
				sensor->preview_size = index;
				break;
			}
		}   

		if(index == ARRAY_SIZE(sr200pc10_preview_sizes))
		{
			printk(SR200PC10_MOD_NAME "Preview Image %d x %d Size is not supported!\n", pix->width, pix->height);
			goto s_fmt_fail;
		}

		if(sensor->mode == SR200PC10_MODE_CAMCORDER)
		{
			if(pix->width == 1280 && pix->height == 720)
			{
				dprintk(CAM_DBG, SR200PC10_MOD_NAME "Preview Image Size is 720P!\n");
				sr200pc10_720p_enable = true;
			}
		}

		dprintk(CAM_DBG, SR200PC10_MOD_NAME "SR200PC10--preview size = %d\n", sensor->preview_size); 
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "SR200PC10--preview width : %d\n", sr200pc10_preview_sizes[index].width);
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "SR200PC10--preview height : %d\n", sr200pc10_preview_sizes[index].height);      

		pix->field = V4L2_FIELD_NONE;
		pix->bytesperline = pix->width * 2;
		pix->sizeimage = pix->bytesperline * pix->height;  
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "V4L2_PIX_FMT_UYVY\n");

		if(sr200pc10_set_preview())
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
	printk(SR200PC10_MOD_NAME "ioctl_s_fmt_cap is failed\n"); 
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
	struct sr200pc10_sensor *sensor = s->priv;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_g_fmt_cap is called...\n");

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
	struct sr200pc10_sensor *sensor = s->priv;
	struct v4l2_captureparm *cparm = &a->parm.capture;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_g_parm is called...\n");

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
	{
		printk(SR200PC10_MOD_NAME "ioctl_g_parm type not supported.\n");
		goto g_parm_fail;
	}

	memset(a, 0, sizeof(*a));
	a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;
	cparm->timeperframe = sensor->timeperframe;

	return 0;

g_parm_fail:
	printk(SR200PC10_MOD_NAME "ioctl_g_parm is failed\n"); 
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
	struct sr200pc10_sensor *sensor = s->priv;
	struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_s_parm is called...\n");

	/* Set mode (camera/camcorder/vt) & state (preview/capture) */
	sensor->mode = a->parm.capture.capturemode;
	sensor->state = a->parm.capture.currentstate;

	if(sensor->mode < 1 || sensor->mode > 3) sensor->mode = SR200PC10_MODE_CAMERA;
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "mode = %d, state = %d\n", sensor->mode, sensor->state);   

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
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "fps = %d\n", sensor->fps);  

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
	struct sr200pc10_sensor *sensor = s->priv;
	int rval;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_g_ifparm is called...\n");

	rval = sensor->pdata->ifparm(p);
	if (rval)
	{
		return rval;
	}

	p->u.bt656.clock_curr = SR200PC10_XCLK;

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
	struct sr200pc10_sensor *sensor = s->priv;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_g_priv is called...\n");

	if(p == NULL)
	{
		printk(SR200PC10_MOD_NAME "ioctl_g_priv is failed because of null pointer\n"); 
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
	struct sr200pc10_sensor* sensor = s->priv;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_enum_framesizes called...\n");   

	if (sensor->state == SR200PC10_STATE_CAPTURE)
	{    
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "Size enumeration for image capture size = %d\n", sensor->capture_size);

		if(sensor->preview_size == ARRAY_SIZE(sr200pc10_image_sizes))
			goto enum_framesizes_fail;

		frms->index = sensor->capture_size;
		frms->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		frms->discrete.width = sr200pc10_image_sizes[sensor->capture_size].width;
		frms->discrete.height = sr200pc10_image_sizes[sensor->capture_size].height;        
	}
	else
	{
		dprintk(CAM_DBG, SR200PC10_MOD_NAME "Size enumeration for image preview size = %d\n", sensor->preview_size);

		if(sensor->preview_size == ARRAY_SIZE(sr200pc10_preview_sizes))
			goto enum_framesizes_fail;

		frms->index = sensor->preview_size;
		frms->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		frms->discrete.width = sr200pc10_preview_sizes[sensor->preview_size].width;
		frms->discrete.height = sr200pc10_preview_sizes[sensor->preview_size].height;        
	}

	dprintk(CAM_DBG, SR200PC10_MOD_NAME "framesizes width : %d\n", frms->discrete.width); 
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "framesizes height : %d\n", frms->discrete.height); 

	return 0;

enum_framesizes_fail:
	printk(SR200PC10_MOD_NAME "ioctl_enum_framesizes is failed\n"); 
	return -EINVAL;   
}

static int ioctl_enum_frameintervals(struct v4l2_int_device *s, struct v4l2_frmivalenum *frmi)
{
	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_enum_frameintervals \n"); 
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "ioctl_enum_frameintervals numerator : %d\n", frmi->discrete.numerator); 
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "ioctl_enum_frameintervals denominator : %d\n", frmi->discrete.denominator); 

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
	struct sr200pc10_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_s_power is called......ON=%x, detect= %x\n", on, sensor->detect);

//idle current optimisation 
	if(on == V4L2_POWER_ON)
                 back_cam_in_use= 1 ;
	else if(on == V4L2_POWER_OFF)
		  back_cam_in_use = 0 ;
		
//idle current optimisation 
		

	if(on == V4L2_POWER_OFF)
	{
		sr200pc10_write_reg(client, sizeof(Lense_AFoff_list), Lense_AFoff_list);
		mdelay(200);
	}  

	if(sensor->pdata->power_set(on))
	{
		printk(SR200PC10_MOD_NAME "Can not power on/off " SR200PC10_DRIVER_NAME " sensor\n"); 
		goto s_power_fail;
	}

	switch(on)
	{
		case V4L2_POWER_ON:
			{
				dprintk(CAM_DBG, SR200PC10_MOD_NAME "pwr on-----!\n");
				#if 1
				dprintk(CAM_DBG, SR200PC10_MOD_NAME "pwr on-----!\n");
				//if(sr200pc10_detect(client)) 
				if(s5k4ca_sensor_get_id(client)!=0) //LYG
				{
					printk(SR200PC10_MOD_NAME "Unable to detect " SR200PC10_DRIVER_NAME " sensor\n");
					sensor->pdata->power_set(V4L2_POWER_OFF);
					goto s_power_fail;
				}
				#endif
				//err = s5k4ba_write_regs(sd, sr200pc10_init0, SR200PC10_INIT0_REGS, "sr200pc10_init0");//LYG
				s5k4ba_write_regs(client, sr200pc10_init0, SR200PC10_INIT0_REGS, "sr200pc10_init0");//LYG

				/* Make the default detect */
				sensor->detect = SENSOR_DETECTED;     
				
				/* Make the state init */
				sr200pc10_curr_state = SR200PC10_STATE_INVALID;
			}
			break;

		case V4L2_POWER_RESUME:
			{
				dprintk(CAM_DBG, SR200PC10_MOD_NAME "pwr resume-----!\n");
			}  
			break;

		case V4L2_POWER_STANDBY:
			{
				dprintk(CAM_DBG, SR200PC10_MOD_NAME "pwr stanby-----!\n");
			}
			break;

		case V4L2_POWER_OFF:
			{
				dprintk(CAM_DBG, SR200PC10_MOD_NAME "pwr off-----!\n");

				/* Make the default detect */
				sensor->detect = SENSOR_NOT_DETECTED;  

				/* Make the state init */
				sr200pc10_pre_state = SR200PC10_STATE_INVALID; 
		  
			}
			break;
	}

	return 0;

s_power_fail:
//idle current optimisation
	back_cam_in_use= 0 ;
//idle current optimisation		
	printk(SR200PC10_MOD_NAME "ioctl_s_power is failed\n");
	return -EINVAL;
}



static int ioctl_g_exif(struct v4l2_int_device *s, struct v4l2_exif *exif)
{
	struct sr200pc10_sensor *sensor = s->priv;
	struct i2c_client *client = sensor->i2c_client;

	u8 Write_Exposure_info[3] = {0x17,0x00,0x00};
	u8 Read_Capture_Data[8] = {0x00,};

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_g_exif is called...\n");
#if 0
	if(sr200pc10_write_read_reg(client, sizeof(Write_Exposure_info), Write_Exposure_info, sizeof(Read_Capture_Data), Read_Capture_Data))
		goto g_exif_fail;

	dprintk(CAM_DBG, SR200PC10_MOD_NAME "Capture Data [0]: 0x%04x, [1]: 0x%04x, [2]: 0x%04x\n", Read_Capture_Data[0], Read_Capture_Data[1], Read_Capture_Data[2]);
	dprintk(CAM_DBG, SR200PC10_MOD_NAME "Capture Data [3]: 0x%04x, [4]: 0x%04x, [5]: 0x%04x\n", Read_Capture_Data[3], Read_Capture_Data[4], Read_Capture_Data[5]);      

	exif->TV_Value =     Read_Capture_Data[0]|(Read_Capture_Data[1]*256);
	exif->SV_Value =     Read_Capture_Data[2]|(Read_Capture_Data[3]*256);
	exif->AV_Value =     Read_Capture_Data[4]|(Read_Capture_Data[5]*256);
	exif->BV_Value =     Read_Capture_Data[6]|(Read_Capture_Data[7]*256);

	dprintk(CAM_DBG, SR200PC10_MOD_NAME "aperture Data : 0x%x, TV_Value: 0x%x,  SV_Value: 0x%x flash: 0x%x\n",
			exif->aperture_numerator, exif->TV_Value, exif->SV_Value, exif->aperture_denominator );
#endif
	return 0;

g_exif_fail:
	printk(SR200PC10_MOD_NAME "ioctl_g_exif is failed\n");
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
	struct sr200pc10_sensor *sensor = s->priv;

	dprintk(CAM_INF, "ioctl_deinit is called...\n");

	sensor->state = SR200PC10_STATE_INVALID; //init problem

	return 0;
}


/**
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 *
 * Initialize the sensor device (call sr200pc10_configure())
 */
static int ioctl_init(struct v4l2_int_device *s)
{
	struct sr200pc10_sensor *sensor = s->priv;

	dprintk(CAM_INF, SR200PC10_MOD_NAME "ioctl_init is called...\n");

	//init value
	sensor->timeperframe.numerator    = 1;
	sensor->timeperframe.denominator  = 30;
	sensor->fps                       = 30;
	sensor->bv                        = 0;
	sensor->state                     = SR200PC10_STATE_INVALID;
	sensor->mode                      = SR200PC10_MODE_CAMERA;
	sensor->preview_size              = SR200PC10_PREVIEW_SIZE_640_480;
	sensor->capture_size              = SR200PC10_IMAGE_SIZE_2560_1920;
	sensor->detect                    = SENSOR_NOT_DETECTED;
	sensor->focus_mode                = SR200PC10_AF_INIT_NORMAL;
	sensor->effect                    = SR200PC_EFFECT_OFF;
	sensor->iso                       = SR200PC10_ISO_AUTO;
	sensor->photometry                = SR200PC10_PHOTOMETRY_CENTER;
	sensor->ev                        = SR200PC10_EV_DEFAULT;
	sensor->wdr                       = SR200PC10_WDR_OFF;
	sensor->contrast                  = SR200PC10_CONTRAST_DEFAULT;
	sensor->saturation                = SR200PC10_SATURATION_DEFAULT;
	sensor->sharpness                 = SR200PC10_SHARPNESS_DEFAULT;
	sensor->wb                        = SR200PC10_WB_AUTO;
	sensor->isc                       = SR200PC10_ISC_STILL_OFF;
	sensor->scene                     = SR200PC10_SCENE_OFF;
	sensor->aewb                      = SR200PC10_AE_UNLOCK_AWB_UNLOCK;
	sensor->antishake                 = SR200PC10_ANTI_SHAKE_OFF;
	sensor->flash_capture             = SR200PC10_FLASH_CAPTURE_OFF;
	sensor->flash_movie               = SR200PC10_FLASH_MOVIE_OFF;
	sensor->jpeg_quality              = SR200PC10_JPEG_SUPERFINE;
	sensor->zoom                      = SR200PC10_ZOOM_1P00X;
	sensor->thumb_offset              = SR200PC10_THUMBNAIL_OFFSET;
	sensor->yuv_offset                = SR200PC10_YUV_OFFSET;
	sensor->jpeg_capture_w            = JPEG_CAPTURE_WIDTH;
	sensor->jpeg_capture_h            = JPEG_CAPTURE_HEIGHT;  

	memcpy(&sr200pc10, sensor, sizeof(struct sr200pc10_sensor));

	return 0;
}

static struct v4l2_int_ioctl_desc sr200pc10_ioctl_desc[] = {
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

static struct v4l2_int_slave sr200pc10_slave = {
	.ioctls = sr200pc10_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(sr200pc10_ioctl_desc),
};

static struct v4l2_int_device sr200pc10_int_device = {
	.module = THIS_MODULE,
	.name = SR200PC10_DRIVER_NAME,
	.priv = &sr200pc10,
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &sr200pc10_slave,
	},
};


/**
 * sr200pc10_probe - sensor driver i2c probe handler
 * @client: i2c driver client device structure
 *
 * Register sensor as an i2c client device and V4L2
 * device.
 */
  extern u32 hw_revision;
static int sr200pc10_probe(struct i2c_client *client, const struct i2c_device_id *device)
{
	struct sr200pc10_sensor *sensor = &sr200pc10;
	
	dprintk(CAM_INF, "YGLEE start OK sr200pc10_probe is called...\n");
	if(hw_revision>0x03){
		dprintk(CAM_INF, "YGLEE start OK sr200pc10_probe is return...\n");
		return -1;
	}
	
	if (i2c_get_clientdata(client))
	{
		printk(SR200PC10_MOD_NAME "can't get i2c client data!!\n");
		return -EBUSY;
	}

	sensor->pdata = &nowplus_sr200pc10_platform_data;

	if (!sensor->pdata) 
	{
		printk(SR200PC10_MOD_NAME "no platform data!!\n");
		return -ENODEV;
	}

	sensor->v4l2_int_device = &sr200pc10_int_device;
	sensor->i2c_client = client;

	/* Make the default capture size VGA */
	sensor->pix.width = 640;
	sensor->pix.height = 480;

	//sensor->pix.width = 720; //LYG
	//sensor->pix.height = 480;
	//sensor->pix.width = 800; //LYG
	//sensor->pix.height = 600;

	/* Make the default capture format V4L2_PIX_FMT_UYVY */
	//sensor->pix.pixelformat = V4L2_PIX_FMT_UYVY;
	sensor->pix.pixelformat = V4L2_PIX_FMT_YVYU; //LYG

	i2c_set_clientdata(client, sensor);

	if (v4l2_int_device_register(sensor->v4l2_int_device))
	{
		printk(SR200PC10_MOD_NAME "fail to init device register \n");
		i2c_set_clientdata(client, NULL);
	}

	return 0;
}

/**
 * sr200pc10_remove - sensor driver i2c remove handler
 * @client: i2c driver client device structure
 *
 * Unregister sensor as an i2c client device and V4L2
 * device.  Complement of sr200pc10_probe().
 */
static int __exit sr200pc10_remove(struct i2c_client *client)
{
	struct sr200pc10_sensor *sensor = i2c_get_clientdata(client);

	dprintk(CAM_INF, SR200PC10_MOD_NAME "sr200pc10_remove is called...\n");

	if (!client->adapter)
	{
		printk(SR200PC10_MOD_NAME "no i2c client adapter!!");
		return -ENODEV; /* our client isn't attached */
	}

	v4l2_int_device_unregister(sensor->v4l2_int_device);
	i2c_set_clientdata(client, NULL);

	return 0;
}

static const struct i2c_device_id sr200pc10_id[] = {
	{ SR200PC10_DRIVER_NAME, 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, sr200pc10_id);


static struct i2c_driver sr200pc10sensor_i2c_driver = {
	.driver = {
		.name = SR200PC10_DRIVER_NAME,
	},
	.probe = sr200pc10_probe,
	.remove = __exit_p(sr200pc10_remove),
	.id_table = sr200pc10_id,
};

/**
 * sr200pc10_sensor_init - sensor driver module_init handler
 *
 * Registers driver as an i2c client driver.  Returns 0 on success,
 * error code otherwise.
 */
static int __init sr200pc10_sensor_init(void)
{
	int err;

	dprintk(CAM_INF, "YGLEE start OK sr200pc10_sensor_init is called.hw_revision=0x%02x..\n",hw_revision);

	if(hw_revision>0x03){
		dprintk(CAM_INF, "YGLEE start OK sr200pc10_sensor_init is return...\n");
		return -1;
	}	
	err = i2c_add_driver(&sr200pc10sensor_i2c_driver);
	if (err) 
	{
		printk(SR200PC10_MOD_NAME "Failed to register" SR200PC10_DRIVER_NAME ".\n");
		return err;
	}

	return 0;
}

module_init(sr200pc10_sensor_init);

/**
 * sr200pc10sensor_cleanup - sensor driver module_exit handler
 *
 * Unregisters/deletes driver as an i2c client driver.
 * Complement of sr200pc10_sensor_init.
 */
static void __exit sr200pc10sensor_cleanup(void)
{
	i2c_del_driver(&sr200pc10sensor_i2c_driver);
}
module_exit(sr200pc10sensor_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SR200PC10 camera sensor driver");
