/*
 * linux/arch/arm/mach-omap2/board-3430ldp.c
 *
 * Copyright (C) 2008 Texas Instruments Inc.
 *
 * Modified from mach-omap2/board-3430sdp.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *****************************************************
 *****************************************************
 * modules/camera/sr030pc30_platform.c
 *
 * SR030PC30 sensor driver file related to platform
 *
 * Modified by paladin in Samsung Electronics
 */
 
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <mach/gpio.h>
#include <mach/hardware.h>
//#include <mach/mux.h>
#include <media/v4l2-int-device.h>
#include <linux/mm.h> //LYG
#include "omap34xxcam.h"
#include <../drivers/media/video/isp/ispreg.h>
#include "sr030pc30.h"
#include "cam_pmic.h"

#if (CAM_SR030PC30_DBG_MSG)
#include "dprintk.h"
#else
#define dprintk(x, y...)
#endif
#define SR030PC30_BIGGEST_FRAME_BYTE_SIZE  PAGE_ALIGN(1280 * 720 * 2 *6) //fix for usage of 6 buffers for 720p capture and avoiding camera launch issues.
static struct v4l2_ifparm ifparm_sr030pc30 = {
  .if_type = V4L2_IF_TYPE_BT656, // fix me
  .u = {
    .bt656 = {
      .frame_start_on_rising_vs = 0,
      .latch_clk_inv = 0,
      .mode = V4L2_IF_TYPE_BT656_MODE_NOBT_8BIT, 
      .clock_min = SR030PC30_XCLK,
      .clock_max = SR030PC30_XCLK,
      .clock_curr = SR030PC30_XCLK,
    },
  },
};

static struct omap34xxcam_sensor_config sr030pc30_hwc = {
  .sensor_isp = 1,
   .capture_mem =  SR030PC30_BIGGEST_FRAME_BYTE_SIZE, 	
//  .xclk = OMAP34XXCAM_XCLK_A,
};

struct isp_interface_config sr030pc30_if_config = {
  .ccdc_par_ser = ISP_PARLL,
  .dataline_shift = 0x2, 
  // .dataline_shift = 0x3, //LYG
  .hsvs_syncdetect = ISPCTRL_SYNC_DETECT_VSRISE,
  .wait_hs_vs = 0x03,
  .strobe = 0x0,
  .prestrobe = 0x0,
  .shutter = 0x0,
  .u.par.par_bridge = 0x3,
  .u.par.par_clk_pol = 0x0,
  //.u.par.par_clk_pol = 0x1,//LYG
};

static int sr030pc30_enable_gpio(void)
{
  dprintk(CAM_INF, SR030PC30_MOD_NAME "sr030pc30_enable_gpio is called...\n");
  
  /* Request and configure gpio pins */
  if (gpio_request(OMAP3430_GPIO_CAMERA_EN,"CAM EN") != 0) 
  {
    printk(SR030PC30_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_CAMERA_EN);
    //return -EIO;
  } 
  if (gpio_request(OMAP3430_GPIO_CAMERA_EN2,"CAM EN2") != 0) 
  {
    printk(SR030PC30_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_CAMERA_EN2);
    //return -EIO;
  }
  if (gpio_request(OMAP3430_GPIO_CAMERA_EN3,"CAM EN3") != 0) 
  {
    printk(SR030PC30_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_CAMERA_EN3);
   // return -EIO;
  }

  if (gpio_request(OMAP3430_GPIO_CAMERA_EN4,"CAM EN4") != 0) 
  {
    printk(SR030PC30_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_CAMERA_EN4);
   // return -EIO;
  }

  if (gpio_request(OMAP3430_GPIO_CAMERA_STBY,"VGA STDBY") != 0)
  {
    printk(SR030PC30_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_CAMERA_STBY);
   // return -EIO;
  }

  if (gpio_request(OMAP3430_GPIO_CAMERA_RST,"CAM VGA RST") != 0) 
  {
    printk(SR030PC30_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_CAMERA_RST);
   // return -EIO;
  } 
  
#if 0
  if (gpio_request(OMAP3430_GPIO_CAMERA_SDA,"VGA STDBY") != 0)
  {
    printk(SR030PC30_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_CAMERA_SDA);
    //return -EIO;
  }

  if (gpio_request(OMAP3430_GPIO_CAMERA_SCL,"CAM VGA RST") != 0) 
  {
    printk(SR030PC30_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_CAMERA_SCL);
   // return -EIO;
  }  
 #endif 

  if (gpio_request(OMAP3430_GPIO_VGA_STBY,"VGA STDBY") != 0)
  {
    printk(SR030PC30_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_VGA_STBY);
    //return -EIO;
  }


  if (gpio_request(OMAP3430_GPIO_VGA_RST,"CAM VGA RST") != 0) 
  {
    printk(SR030PC30_MOD_NAME "Could not request GPIO %d", OMAP3430_GPIO_VGA_RST);
   // return -EIO;
  }        

  /* Reset the GPIO pins */
  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN, 0);
  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN2, 0);
  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN3, 0);
    gpio_direction_output(OMAP3430_GPIO_CAMERA_EN4, 0);
    gpio_direction_output(OMAP3430_GPIO_CAMERA_RST, 0);
  gpio_direction_output(OMAP3430_GPIO_CAMERA_STBY, 0);
  //gpio_direction_output(OMAP3430_GPIO_CAMERA_SDA, 0);
  //gpio_direction_output(OMAP3430_GPIO_CAMERA_SCL, 0);
  gpio_direction_output(OMAP3430_GPIO_VGA_STBY, 0);        
  gpio_direction_output(OMAP3430_GPIO_VGA_RST, 0);    

  isp_set_xclk(0,0,0);
  
  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN4, 1);
  udelay(100);
  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN3, 1);
  udelay(100);
  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN2, 1);
  udelay(100);
  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN, 1);
  udelay(100);

  /*8 VGA_EN */
  gpio_direction_output(OMAP3430_GPIO_VGA_STBY, 1);
 mdelay(10);

  /*3 Clock Enable */
  isp_set_xclk(0,SR030PC30_XCLK,0);
  mdelay(10);

      /*7 2M_EN */
  gpio_direction_output(OMAP3430_GPIO_CAMERA_STBY, 1);
  mdelay(10);

     /*4 2M_RST */
  gpio_direction_output(OMAP3430_GPIO_CAMERA_RST, 1);
  //mdelay(2);
  mdelay(10);

        /*7 2M_EN */
  gpio_direction_output(OMAP3430_GPIO_CAMERA_STBY, 0);
  mdelay(10);

   /*9 VGA_RST */
  gpio_direction_output(OMAP3430_GPIO_VGA_RST, 1);
  mdelay(50);
  
  return 0; 
}

static int sr030pc30_disable_gpio(void)
{

dprintk(CAM_INF, SR030PC30_MOD_NAME "sr030pc30_disable_gpio is called...\n");

     /*1 2M_RST */
  gpio_direction_output(OMAP3430_GPIO_CAMERA_RST, 0);
  mdelay(1);
/*2 VGA_Sleep Command */

 /*3 VGA_RST */
 gpio_direction_output(OMAP3430_GPIO_VGA_RST, 0);
  mdelay(1);

  /*4 MCLK LOW(off) */
  isp_set_xclk(0,0,0);
  udelay(20);

 /*5 VGA_EN */
  gpio_direction_output(OMAP3430_GPIO_VGA_STBY, 0);
 mdelay(10);

  /*6 7 8 POWER OFF */

  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN4, 0);
  mdelay(2);
  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN3, 0);
  mdelay(2);
  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN2, 0);
  mdelay(2);
  gpio_direction_output(OMAP3430_GPIO_CAMERA_EN, 0);
  mdelay(2);
  
   gpio_free(OMAP3430_GPIO_CAMERA_EN);
   gpio_free(OMAP3430_GPIO_CAMERA_EN2);
//#if (CONFIG_SAMSUNG_REL_HW_REV >= 1)    
  gpio_free(OMAP3430_GPIO_CAMERA_EN3);
  gpio_free(OMAP3430_GPIO_CAMERA_EN4);
//#endif  
  gpio_free(OMAP3430_GPIO_CAMERA_RST);
  gpio_free(OMAP3430_GPIO_CAMERA_STBY);
  gpio_free(OMAP3430_GPIO_VGA_RST);
  gpio_free(OMAP3430_GPIO_VGA_STBY);

  mdelay(10);
  return 0;
}

static int sr030pc30_sensor_power_set(enum v4l2_power power)
{
  static enum v4l2_power s_previous_pwr = V4L2_POWER_OFF;

  int err = 0;

  printk(SR030PC30_MOD_NAME "sr030pc30_sensor_power_set is called...[%x] (0:OFF, 1:ON)\n", power);

  switch (power) 
  {
    case V4L2_POWER_OFF:
    {
      err = sr030pc30_disable_gpio();
    }      
    break;

    case V4L2_POWER_ON:
    {
      isp_configure_interface(0,&sr030pc30_if_config);
	
      	err = sr030pc30_enable_gpio();  
	  if(err){
		err = sr030pc30_disable_gpio();
		  err = sr030pc30_enable_gpio();  
	  }
    }
    break;

    case V4L2_POWER_STANDBY:
      break;

    case V4L2_POWER_RESUME:
      break;
  }
  
  s_previous_pwr = power;

  return err;
}


static int sr030pc30_ifparm(struct v4l2_ifparm *p)
{
  dprintk(CAM_INF, SR030PC30_MOD_NAME "sr030pc30_ifparm is called...\n");
  
  *p = ifparm_sr030pc30;
  
  return 0;
}


static int sr030pc30_sensor_set_prv_data(void *priv)
{
  struct omap34xxcam_hw_config *hwc = priv;

  dprintk(CAM_INF, SR030PC30_MOD_NAME "sr030pc30_sensor_set_prv_data is called...\n");

//  hwc->u.sensor.xclk = sr030pc30_hwc.xclk;
  hwc->u.sensor.sensor_isp = sr030pc30_hwc.sensor_isp;
 hwc->u.sensor.capture_mem = sr030pc30_hwc.capture_mem;
  hwc->dev_index = 1;  // 1;
  hwc->dev_minor = 5;   //5;
  hwc->dev_type = OMAP34XXCAM_SLAVE_SENSOR;

  return 0;
}


struct sr030pc30_platform_data nowplus_sr030pc30_platform_data = {
  .power_set      = sr030pc30_sensor_power_set,
  .priv_data_set  = sr030pc30_sensor_set_prv_data,
  .ifparm         = sr030pc30_ifparm,
};
