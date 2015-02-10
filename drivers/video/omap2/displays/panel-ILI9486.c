/*
 * ili9486 LDI support
 *
 * Copyright (C) 2009 Samsung Corporation
 * Author: Samsung Electronics..
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <linux/spi/spi.h>
#include <linux/regulator/consumer.h>
#include <linux/err.h>
#include <linux/backlight.h>
#include <linux/i2c/twl.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <plat/gpio.h>
#include <plat/hardware.h>
#include <plat/mux.h>
#include <plat/control.h>
#include <plat/display.h>
#include <asm/mach-types.h>
#include <../../../../arch/arm/mach-omap2/mux.h>

//#if CONFIG_SAMSUNG_REL_HW_REV >= 4
#define ENABLE_ESD_WDOG
//#endif

#ifdef ENABLE_ESD_WDOG
extern int get_hw_revision(void);
static void enable_esd_wdog(void);
static void disable_esd_wdog(void);

static int g_lcd_exist_flag = 0;	/* 0 => lcd doesn't exist. */
struct delayed_work lcd_chk_work;
static void lcd_chk_work_handler(struct work_struct *);
void ili9486_ldi_poweroff_smd(void);
void ili9486_ldi_poweron_smd(void);

static int off_by_normal_proc = 0;
#endif

#define LCD_XRES		320
#define LCD_YRES		480

static int current_panel = 0;	// 0:smd
static int lcd_enabled = 0;
static int is_nt35510_spi_shutdown = 0;

static int current_intensity = 115;	// DEFAULT BRIGHTNESS

// default setting : smd panel. 
static u16 LCD_HBP =	32;
static u16 LCD_HFP =	22; 
static u16 LCD_HSW =	4; 
static u16 LCD_VBP =	8;
static u16 LCD_VFP =	12;
static u16 LCD_VSW =	2; 

#define LCD_PIXCLOCK_MAX	        13090 // 15.4mhz


#define GPIO_LEVEL_LOW   0
#define GPIO_LEVEL_HIGH  1

#define POWER_OFF	0	// set in lcd_poweroff function.
#define POWER_ON	1	// set in lcd_poweron function

static struct spi_device *nt35510lcd_spi;
    

static atomic_t lcd_power_state = ATOMIC_INIT(POWER_ON);	// default is power on because bootloader already turn on LCD.
static atomic_t ldi_power_state = ATOMIC_INIT(POWER_ON);	// ldi power state

int g_lcdlevel = 0x6C;

// ------------------------------------------ // 
//          For Regulator Framework                            //
// ------------------------------------------ // 

struct regulator *vaux2;
struct regulator *vaux4;

#define MAX_NOTIFICATION_HANDLER	10

void nt35510_lcd_poweron(void);
void nt35510_lcd_poweroff(void);
void nt35510_lcd_LDO_on(void);
void nt35510_lcd_LDO_off(void);


// paramter : POWER_ON or POWER_OFF
typedef void (*notification_handler)(const int);
typedef struct
{
	int  state;
	spinlock_t vib_lock;
}timer_state_t;
timer_state_t timer_state;


notification_handler power_state_change_handler[MAX_NOTIFICATION_HANDLER];

int nt35510_add_power_state_monitor(notification_handler handler);
void nt35510_remove_power_state_monitor(notification_handler handler);

EXPORT_SYMBOL(nt35510_add_power_state_monitor);
EXPORT_SYMBOL(nt35510_remove_power_state_monitor);

static void nt35510_notify_power_state_changed(void);
static void aat1402_set_brightness(void);
//extern int omap34xx_pad_set_configs(struct pin_config *pin_configs, int n);
extern int omap34xx_pad_set_config_lcd(u16,u16);

//TSP power control
extern int ts_power_control(int*, u8);
extern void zinitix_resume(void);
#define PM_RECEIVER                     TWL4030_MODULE_PM_RECEIVER

#define ENABLE_VPLL2_DEDICATED          0x05
#define ENABLE_VPLL2_DEV_GRP            0x20
#define TWL4030_VPLL2_DEV_GRP           0x33
#define TWL4030_VPLL2_DEDICATED       	0x36


static struct pin_config  omap34xx_lcd_pins[] = {
/*
 *		Name, reg-offset,
 *		mux-mode | [active-mode | off-mode]
 */
 
	// MCSPI1 AND MCSPI2 PIN CONFIGURATION
	// FOR ALL SPI, SPI_CS0 IS I/O AND OTHER SPI CS ARE PURE OUT.
	// CLK AND SIMO/SOMI LINES ARE I/O.
/*
	// 206 (AB3, L, MCSPI1_CLK, DISPLAY_CLK, O)
	MUX_CFG_34XX("AB3_MCSPI1_CLK", 0x01C8,  OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP)
	// 207 (AB4, L, MCSPI1_SIMO, DISLPAY_SI, I)
	MUX_CFG_34XX("AB4_MCSPI1_SIMO", 0x01CA,  OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN)
	// 208 (AA4, L, MCSPI1_SOMI, DISLPAY_SO, O)
	MUX_CFG_34XX("AA4_MCSPI1_SOMI", 0x01CC,  OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLDOWN)
	// 209 (AC2, H, MCSPI1_CS0, DISPLAY_CS, O)
	MUX_CFG_34XX("AC2_MCSPI1_CS0", 0x01CE,  OMAP34XX_MUX_MODE0 | OMAP34XX_PIN_INPUT_PULLUP)
	*/
	 //omap_mux_init_signal("gpio_163", OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	// omap_mux_init_signal("MCSPI1_SIMO", OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN);
	// omap_mux_init_signal("MCSPI1_SOMI", OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN);
	// 209 (AC2, H, MCSPI1_CS0, DISPLAY_CS, O)
	//omap_mux_init_signal("sys_nirq",OMAP_WAKEUP_EN | OMAP_PIN_INPUT_PULLUP);
	//omap_mux_init_signal("MCSPI1_CS0", OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	 
	
	
};

static struct pin_config  omap34xx_lcd_off_pins[] = {
/*
 *		Name, reg-offset,
 *		mux-mode | [active-mode | off-mode]
 */
 
	// MCSPI1 AND MCSPI2 PIN CONFIGURATION
	// FOR ALL SPI, SPI_CS0 IS I/O AND OTHER SPI CS ARE PURE OUT.
	// CLK AND SIMO/SOMI LINES ARE I/O.

	// 206 (AB3, L, MCSPI1_CLK, DISPLAY_CLK, O)
	//MUX_CFG_34XX("AB3_MCSPI1_CLK", 0x01C8,  OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN)
	// 207 (AB4, L, MCSPI1_SIMO, DISLPAY_SI, I)
	//MUX_CFG_34XX("AB4_MCSPI1_SIMO", 0x01CA,  OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN)
	// 208 (AA4, L, MCSPI1_SOMI, DISLPAY_SO, O)
	//MUX_CFG_34XX("AA4_MCSPI1_SOMI", 0x01CC,  OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN)
	// 209 (AC2, H, MCSPI1_CS0, DISPLAY_CS, O)
	//MUX_CFG_34XX("AC2_MCSPI1_CS0", 0x01CE,  OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN)
	
};

/*  Manual
 * defines HFB, HSW, HBP, VFP, VSW, VBP as shown below
 */

static struct omap_video_timings panel_timings = {0,};
#if 0 
= {
	/* 800 x 480 @ 60 Hz  Reduced blanking VESA CVT 0.31M3-R */
	.x_res          = LCD_XRES,
	.y_res          = LCD_YRES,
	.pixel_clock    = LCD_PIXCLOCK_MAX,
	.hfp            = LCD_HFP,
	.hsw            = LCD_HSW,
	.hbp            = LCD_HBP,
	.vfp            = LCD_VFP,
	.vsw            = LCD_VSW,
	.vbp            = LCD_VBP,
};
#endif

void lcd_en_set(int set)
{
	if (set == 0)
	{
		gpio_set_value(OMAP_GPIO_NEW_LCD_EN_SET, GPIO_LEVEL_LOW);
	}
	else
	{
		gpio_set_value(OMAP_GPIO_NEW_LCD_EN_SET, GPIO_LEVEL_HIGH);
	}
}

int nt35510_add_power_state_monitor(notification_handler handler)
{
	int index = 0;
	if(handler == NULL)
	{
		printk(KERN_ERR "[LCD][%s] param is null\n", __func__);
		return -EINVAL;
	}

	for(; index < MAX_NOTIFICATION_HANDLER; index++)
	{
		if(power_state_change_handler[index] == NULL)
		{
			power_state_change_handler[index] = handler;
			return 0;
		}
	}

	// there is no space this time
	printk(KERN_INFO "[LCD][%s] No spcae\n", __func__);

	return -ENOMEM;
}

void nt35510_remove_power_state_monitor(notification_handler handler)
{
	int index = 0;
	if(handler == NULL)
	{
		printk(KERN_ERR "[LCD][%s] param is null\n", __func__);
		return;
	}
	
	for(; index < MAX_NOTIFICATION_HANDLER; index++)
	{
		if(power_state_change_handler[index] == handler)
		{
			power_state_change_handler[index] = NULL;
		}
	}
}
	
static void nt35510_notify_power_state_changed(void)
{
	int index = 0;
	for(; index < MAX_NOTIFICATION_HANDLER; index++)
	{
		if(power_state_change_handler[index] != NULL)
		{
			power_state_change_handler[index](atomic_read(&lcd_power_state));
		}
	}

}

static __init int setup_current_panel(char *opt)
{
	current_panel = (u32)memparse(opt, &opt);
	return 0;
}
__setup("androidboot.current_panel=", setup_current_panel);

static int nt35510_panel_probe(struct omap_dss_device *dssdev)
{
	printk(KERN_INFO " **** nt35510_panel_probe.\n");
		
	vaux4 = regulator_get( &dssdev->dev, "vaux4" );
	if( IS_ERR( vaux4 ) )
		printk( "Fail to register vaux4 using regulator framework!\n" );	

	vaux2 = regulator_get( &dssdev->dev, "vaux2" );
	if( IS_ERR( vaux2 ) )
		printk( "Fail to register vaux2 using regulator framework!\n" );	

	nt35510_lcd_LDO_on();

	//MLCD pin set to OUTPUT.
	if (gpio_request(OMAP_GPIO_MLCD_RST, "MLCD_RST") < 0) {
		printk(KERN_ERR "\n FAILED TO REQUEST GPIO %d \n", OMAP_GPIO_MLCD_RST);
		return;
	}
	gpio_direction_output(OMAP_GPIO_MLCD_RST, 1);

	/* 320 x 480 @ 90 Hz */
	panel_timings.x_res          = LCD_XRES,
	panel_timings.y_res          = LCD_YRES,
	panel_timings.pixel_clock    = LCD_PIXCLOCK_MAX,
	panel_timings.hfp            = LCD_HFP,
	panel_timings.hsw            = LCD_HSW,
	panel_timings.hbp            = LCD_HBP,
	panel_timings.vfp            = LCD_VFP,
	panel_timings.vsw            = LCD_VSW,
	panel_timings.vbp            = LCD_VBP;

	dssdev->panel.config = OMAP_DSS_LCD_TFT | OMAP_DSS_LCD_ONOFF | OMAP_DSS_LCD_IPC;

	//dssdev->panel.recommended_bpp= 32;  /* 35 kernel  recommended_bpp field is removed */
	dssdev->panel.acb = 0;
	dssdev->panel.timings = panel_timings;
	dssdev->ctrl.pixel_size = 18;
	
	return 0;
}

static void nt35510_panel_remove(struct omap_dss_device *dssdev)
{
	regulator_put( vaux4 );
	regulator_put( vaux2 );
}

static int nt35510_panel_enable(struct omap_dss_device *dssdev)
{
	int r = 0;

	/* HW test team ask power on reset should be done before enabling
	   sync signals. So it's not done with lcd poweron code */
	if (lcd_enabled == 0)	/* don't reset lcd for initial booting */
	{
		nt35510_lcd_LDO_on();

		// Activate Reset
		gpio_set_value(OMAP_GPIO_MLCD_RST, GPIO_LEVEL_LOW);	
		mdelay(1);
		gpio_set_value(OMAP_GPIO_MLCD_RST, GPIO_LEVEL_HIGH);
		mdelay(5);
		
		//MLCD pin set to InputPulldown.
		omap_ctrl_writew(0x010C, 0x1c6);
	}

	r = omapdss_dpi_display_enable(dssdev);
	if (r)
		goto err0;

	/* Delay recommended by panel DATASHEET */
	mdelay(4);
	if (dssdev->platform_enable) {
		r = dssdev->platform_enable(dssdev);
		if (r)
			goto err1;
        }
        dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;
		
	if(lcd_enabled ==1)
		lcd_enabled =0;
	else 
		nt35510_lcd_poweron();

	return r;
err1:
	omapdss_dpi_display_disable(dssdev);
err0:
	return r;
}

static void nt35510_panel_disable(struct omap_dss_device *dssdev)
{
	if (dssdev->state != OMAP_DSS_DISPLAY_ACTIVE)
		return;
	if (is_nt35510_spi_shutdown == 1)
	{
		printk("[%s] skip omapdss_dpi_display_disable..\n", __func__);
		return;
	}
	nt35510_lcd_poweroff();
	if (dssdev->platform_disable)
		dssdev->platform_disable(dssdev);
	mdelay(4);

	omapdss_dpi_display_disable(dssdev);

	dssdev->state = OMAP_DSS_DISPLAY_DISABLED;
}

static int nt35510_panel_suspend(struct omap_dss_device *dssdev)
{
	printk(KERN_INFO " **** nt35510_panel_suspend\n");

#ifdef ENABLE_ESD_WDOG
	disable_esd_wdog();
#endif

	spi_setup(nt35510lcd_spi);

	//gpio_set_value(OMAP_GPIO_LCD_EN_SET, GPIO_LEVEL_LOW);
	lcd_en_set(0);
	mdelay(1);

    #if 0
	nt35510_lcd_poweroff();
    #else
//	nt35510_ldi_standby();
	nt35510_panel_disable(dssdev);
    #endif
   
	dssdev->state = OMAP_DSS_DISPLAY_SUSPENDED;
    return 0;
}

static int nt35510_panel_resume(struct omap_dss_device *dssdev)
{
	printk(KERN_INFO " **** nt35510_panel_resume\n");
	//TSP power control
	//ts_power_control(NULL, 1);
	zinitix_resume();
	spi_setup(nt35510lcd_spi);
    
//	msleep(150);
    
    #if 0
	nt35510_lcd_poweron();
    #else
//	nt35510_ldi_wakeup();
	nt35510_panel_enable(dssdev);
    #endif

	//gpio_set_value(OMAP_GPIO_LCD_EN_SET, GPIO_LEVEL_LOW);
	lcd_en_set(0);
	mdelay(1);
	//gpio_set_value(OMAP_GPIO_LCD_EN_SET, GPIO_LEVEL_HIGH);
	lcd_en_set(1);

	dssdev->state = OMAP_DSS_DISPLAY_ACTIVE;

#ifdef ENABLE_ESD_WDOG
	enable_esd_wdog();
#endif

	return 0;
}

static int nt35510_get_recommended_bpp(struct omap_dss_device *dssdev)
{
	return 24;	/* AALTO lcd has 18bit data line but internally processed by packed 24bit */
}

static struct omap_dss_driver nt35510_driver = {
	.probe          = nt35510_panel_probe,
	.remove         = nt35510_panel_remove,

	.enable         = nt35510_panel_enable,
	.disable        = nt35510_panel_disable,
	.suspend        = nt35510_panel_suspend,
	.resume         = nt35510_panel_resume,

	.get_recommended_bpp = nt35510_get_recommended_bpp,

	.driver		= {
		.name	= "nt35510_panel",
		.owner 	= THIS_MODULE,
	},
};

static void spi1writeindex(u8 index)
{
	volatile unsigned short cmd = 0;
	int ret_cnt = 10;

	while (ret_cnt-- > 0)
	{
		cmd= 0x0000|index;
		if (0 == spi_write(nt35510lcd_spi,(unsigned char*)&cmd,2))
		{
			break;
		}
		else
		{
			printk("%s : fail!\n", __func__);
			mdelay(1);
		}
	}

	udelay(100);
	udelay(100);
}

static void spi1writedata(u8 data)
{
	volatile unsigned short datas = 0;
	int ret_cnt = 10;

	while (ret_cnt-- > 0)
	{
		datas= 0x0100|data;
		if (0 == spi_write(nt35510lcd_spi,(unsigned char*)&datas,2))
		{
			break;
		}
		else
		{
			printk("%s : fail!\n", __func__);
			mdelay(1);
		}
	}

	udelay(100);
	udelay(100);
}

static void spi1write(u8 index, u8 data)
{
	volatile unsigned short cmd = 0;
	volatile unsigned short datas=0;

	cmd = 0x0000 | index;
	datas = 0x0100 | data;
	
	spi_write(nt35510lcd_spi,(unsigned char*)&cmd,2);
	udelay(100);
	spi_write(nt35510lcd_spi,(unsigned char *)&datas,2);
	udelay(100);
	udelay(100);
}

static int lcd_brightness_test(void)
{
	u16 cmd = 0x52;
	u32 buf = 0;

	spi_write_then_read(nt35510lcd_spi, (u8*)&cmd, 2, (u8*)&buf, 2); 

	buf = (buf&0x1ff)>>1;

	//printk("%s : 0x%x\n", __func__, buf);

	return buf;
}

static void lcd_gamma_test(void)
{
	spi1writeindex(0xE2);
	spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a);
	/* 
	spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a);
	spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a);
	spi1writedata(0x0a);
	*/
}

static int lcd_reg_test(void)
{
	u16 cmd = 0x0a;

	u32 buf = 0;

	spi_write_then_read(nt35510lcd_spi, (u8*)&cmd, 2, (u8*)&buf, 2); 

	//printk("%s : 0x%x\n", __func__, buf);

	return (buf&0x1ff) >> 1;
}

void ili9486_ldi_poweron_smd(void)
{
	printk("[LCD] %s()\n", __func__);

	mdelay(10);

	/* power & init setting */
	spi1writeindex(0xC0); spi1writedata(0x15); spi1writedata(0x15);
	spi1writeindex(0xC1); spi1writedata(0x41); spi1writedata(0x00);
	spi1writeindex(0xC2); spi1writedata(0x22);
	spi1writeindex(0xC5); spi1writedata(0x00); spi1writedata(0x4B);
	spi1writeindex(0x2A); spi1writedata(0x00); spi1writedata(0x00); spi1writedata(0x01); spi1writedata(0x3F);
	spi1writeindex(0x2B); spi1writedata(0x00); spi1writedata(0x00); spi1writedata(0x01); spi1writedata(0xDF);
	spi1writeindex(0x36); spi1writedata(0x08);
	spi1writeindex(0x3A); spi1writedata(0x66);
	spi1writeindex(0xB0); spi1writedata(0x0C);
	spi1writeindex(0xB1); spi1writedata(0xA0); spi1writedata(0x10);
	spi1writeindex(0xB4); spi1writedata(0x02);
	spi1writeindex(0xB5); spi1writedata(0x0C); spi1writedata(0x08); spi1writedata(0x2C); spi1writedata(0x26);
	spi1writeindex(0xB6); spi1writedata(0x30); spi1writedata(0x42); spi1writedata(0x3B);
	spi1writeindex(0xB7); spi1writedata(0x07);
	spi1writeindex(0xF9); spi1writedata(0x00); spi1writedata(0x08);

	spi1writeindex(0xE0);
	spi1writedata(0x1F); spi1writedata(0x24); spi1writedata(0x24); spi1writedata(0x0B); spi1writedata(0x0B);
	spi1writedata(0x08); spi1writedata(0x4E); spi1writedata(0x86); spi1writedata(0x3F); spi1writedata(0x05);
	spi1writedata(0x13); spi1writedata(0x04); spi1writedata(0x10); spi1writedata(0x0F); spi1writedata(0x00);

	spi1writeindex(0xE1);
	spi1writedata(0x1F); spi1writedata(0x2A); spi1writedata(0x1F); spi1writedata(0x0B); spi1writedata(0x11);
	spi1writedata(0x04); spi1writedata(0x49); spi1writedata(0x56); spi1writedata(0x34); spi1writedata(0x06);
	spi1writedata(0x0B); spi1writedata(0x04); spi1writedata(0x25); spi1writedata(0x22); spi1writedata(0x00);

	spi1writeindex(0xF8);
	spi1writedata(0x21); spi1writedata(0x07); spi1writedata(0x02);

	spi1writeindex(0xE2);
	spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a);
	spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a);
	spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a); spi1writedata(0x0a);
	spi1writedata(0x0a);

	spi1writeindex(0xE3);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);
	spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06); spi1writedata(0x06);

	spi1writeindex(0xF4);
	spi1writedata(0x00); spi1writedata(0x00); spi1writedata(0x08); spi1writedata(0x91); spi1writedata(0x04);

	spi1writeindex(0xF2);
	spi1writedata(0x1A); spi1writedata(0xA3); spi1writedata(0x12); spi1writedata(0x02); spi1writedata(0x82);
	spi1writedata(0x32); spi1writedata(0xFF); spi1writedata(0x10); spi1writedata(0x00);

	//spi1writeindex(0xFC);
	//spi1writedata(0x00); spi1writedata(0x06); spi1writedata(0x83);

	/* ?? */
	spi1writeindex(0x13); 
	spi1writeindex(0x11); 

	msleep(120);

	spi1writeindex(0xcf);	
	spi1writedata(0x01);	/* pwm out as 35khz */

	/* backlight pwm brightness */
	spi1writeindex(0x51);
	spi1writedata(0x90);

	spi1writeindex(0x53);
	spi1writedata(0x2c);

	/* display on */
	spi1writeindex(0x29); 

	mdelay(40);

	/* LCD BL_CTRL high for aat1402 */
	//gpio_set_value(OMAP_GPIO_LCD_EN_SET, GPIO_LEVEL_HIGH);
	lcd_en_set(1);
	mdelay(1);

	atomic_set(&ldi_power_state, POWER_ON);
}

void ili9486_ldi_poweroff_smd(void)
{
	printk("[LCD] %s\n", __func__);

	/* LCD BL_CTRL low for aat1042 */
	//gpio_set_value(OMAP_GPIO_LCD_EN_SET, GPIO_LEVEL_LOW);
	lcd_en_set(0);
	mdelay(10);

	/* display off */
	spi1writeindex(0x28); 
	msleep(10);

	/* sleep in */
	spi1writeindex(0x10); 
	msleep(50);
	/* booster stop and internal oscillator stop here...by LCD hw */
	msleep(120);
	
	atomic_set(&ldi_power_state, POWER_OFF);
}

/* called by fbsysfs */
void samsung_aalto_set_cabc(const char* level)
{
	int cabc_level = simple_strtol(level, NULL, 10);

	if (cabc_level == 0)
	{
		printk("%s : cabc off\n", __func__);
		spi1writeindex(0x55);	
		spi1writedata(0x00);	/* cabc off */
	}
	else
	{
		spi1writeindex(0x55);	
		spi1writedata(0x03);	/* cabc on for moving image */

		if (cabc_level >= 70 && cabc_level <= 100)
		{
			cabc_level = (-cabc_level + 100) >> 1;	/* see 248pp of ILI9486 manual */
			cabc_level <<= 4;	
		}
		else
		{
			cabc_level = 0x70;	/* default value (86%) for abnormal input */
		}
		
		spi1writeindex(0xc9);	
		spi1writedata(cabc_level);	/* THRES_MOV = cabc_level */

		printk("%s : cabc = 0x%x\n", __func__, cabc_level);
	}
}

void nt35510_lcd_LDO_on(void)
{
	int ret;

	printk("+++ %s\n", __func__);
//	twl_i2c_read_regdump();
#if 1	
	ret = regulator_enable( vaux2 ); //VAUX2 - 1.8V
	if ( ret )
		printk("Regulator vaux2 error!!\n");
#else
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x1F);
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x1, 0x22);
	twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0xe0, 0x1F);
#endif

	mdelay(1);

	ret = regulator_enable( vaux4 ); //VAUX4 - 3.0V
	if ( ret )
		printk("Regulator vaux4 error!!\n");

	mdelay(1);
	
	printk("--- %s\n", __func__);
}

void nt35510_lcd_LDO_off(void)
{
	int ret;
	int ret_cnt = 10;

	printk("+++ %s\n", __func__);

	// Reset Release (reset = L)
	gpio_set_value(OMAP_GPIO_MLCD_RST, GPIO_LEVEL_LOW); 
	mdelay(10);

	// VCI 3.0V OFF

	while (ret_cnt-- > 0)
	{
		ret = regulator_disable( vaux4 );
		if ( ret )
		{
			printk("Regulator vaux4 error!!\n");
			mdelay(10);
		}
		else
		{
			break;
		}
	}
	mdelay(1);

	// VDD3 1.8V OFF
	ret_cnt = 10;
	while (ret_cnt-- > 0)
	{
		ret = regulator_disable( vaux2 );
		if ( ret )
		{
			printk("Regulator vaux2 error!!\n");
			mdelay(10);
		}
		else
		{
			break;
		}
	}

	printk("--- %s\n", __func__);
}

void nt35510_lcd_poweroff(void)
{
	ili9486_ldi_poweroff_smd();

	// turn OFF VCI (3.0V)
	// turn OFF VDD3 (1.8V)
	nt35510_lcd_LDO_off();

#if 1             
		omap_mux_init_signal("mcspi1_clk", OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN);
		omap_mux_init_signal("mcspi1_simo",  OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN);
		omap_mux_init_signal("mcspi1_somi", OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN);
		omap_mux_init_signal("mcspi1_cs0", OMAP_MUX_MODE7 | OMAP_PIN_INPUT_PULLDOWN);
#endif		
}

void nt35510_lcd_poweron(void)
{
	u8 read=0;

#if 1	
	omap_mux_init_signal("mcspi1_clk",OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
	omap_mux_init_signal("mcspi1_simo",OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN);
	omap_mux_init_signal("mcspi1_somi", OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN);
	omap_mux_init_signal("mcspi1_cs0", OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLUP);
#endif	

	/* done by nt35510_panel_enable */
#if 0	
	nt35510_lcd_LDO_on();

	// Activate Reset
	gpio_set_value(OMAP_GPIO_MLCD_RST, GPIO_LEVEL_LOW);	
	mdelay(1);
	gpio_set_value(OMAP_GPIO_MLCD_RST, GPIO_LEVEL_HIGH);
	mdelay(5);
	
	//MLCD pin set to InputPulldown.
	omap_ctrl_writew(0x010C, 0x1c6);
#endif	

	ili9486_ldi_poweron_smd();

	aat1402_set_brightness();
}

// [[ backlight control 
//static int current_intensity = 115;	// DEFAULT BRIGHTNESS
static DEFINE_SPINLOCK(aat1402_bl_lock);

static void aat1402_set_brightness(void)
{
	//printk(KERN_DEBUG" *** aat1402_set_brightness : %d\n", current_intensity);
	printk(" *** aat1402_set_brightness : %d\n", current_intensity);

	//spin_lock_irqsave(&aat1402_bl_lock, flags);
	//spin_lock(&aat1402_bl_lock);

	spi1writeindex(0x51);
	spi1writedata(current_intensity);	// PWM control. default brightness : 115

	//spin_unlock_irqrestore(&aat1402_bl_lock, flags);
	//spin_unlock(&aat1402_bl_lock);

}

static int aat1402_bl_get_intensity(struct backlight_device *bd)
{
	return current_intensity;
}

static int aat1402_bl_set_intensity(struct backlight_device *bd)
{
	int intensity = bd->props.brightness;
	
	if( intensity < 0 || intensity > 255 )
		return;

/*
	while(atomic_read(&ldi_power_state)==POWER_OFF) 
	{
		if(--retry_count == 0)
			break;
		mdelay(5);
	}
*/	
	current_intensity = intensity;

	if(atomic_read(&ldi_power_state)==POWER_OFF) 
	{
		return;
	}

	/* This is to avoid lcd_chk_work_handler() reset lcd 
	   when LCD is turned off by normal procedure (ex. sleep) */
	if (intensity == 0)
	{
		off_by_normal_proc = 1;
	}
	else 
	{
		off_by_normal_proc = 0;
	}

	aat1402_set_brightness();

	return 0;
}

static struct backlight_ops aat1402_bl_ops = {
	.get_brightness = aat1402_bl_get_intensity,
	.update_status  = aat1402_bl_set_intensity,
};


#ifdef ENABLE_ESD_WDOG

/* This is to recover LCD from ESD shock test */
static void lcd_chk_work_handler(struct work_struct *work)
{
	if(atomic_read(&ldi_power_state) != POWER_OFF) 
	{
		if (lcd_reg_test() != 0x9c || (lcd_brightness_test() == 0 && off_by_normal_proc == 0))
		{
			printk("%s: reset lcd!! + \n", __func__);

			// Activate Reset
			gpio_set_value(OMAP_GPIO_MLCD_RST, GPIO_LEVEL_LOW);	
			mdelay(1);
			gpio_set_value(OMAP_GPIO_MLCD_RST, GPIO_LEVEL_HIGH);
			mdelay(5);
			
			//MLCD pin set to InputPulldown.
			omap_ctrl_writew(0x010C, 0x1c6);

			ili9486_ldi_poweron_smd();
			aat1402_set_brightness();
			printk("%s: reset lcd!! - \n", __func__);
		}
		else
		{
			lcd_gamma_test();
		}
	}

	schedule_delayed_work(&lcd_chk_work, HZ*5);
}

void start_lcd_esd_wdog(void)
{
	g_lcd_exist_flag = 1;
	INIT_DELAYED_WORK(&lcd_chk_work, lcd_chk_work_handler);
	enable_esd_wdog();
}
EXPORT_SYMBOL(start_lcd_esd_wdog);

static int g_esd_counter = 0;

static void disable_esd_wdog(void)
{
	if (get_hw_revision() < 5)	/* 5=> Rev0.4 */
	{
		return;
	}

	if (!g_lcd_exist_flag)	/* no need to run wdog if lcd unavailable */
	{
		return;
	}
	
	cancel_delayed_work_sync(&lcd_chk_work);
}

static irqreturn_t esd_wdog_handler(int irq, void *handle)
{
	//printk("%s : %d\n", __func__, g_esd_counter);

	if (++g_esd_counter > 10)
	{
		set_irq_type(irq, IRQ_TYPE_NONE);
		if(atomic_read(&ldi_power_state) == POWER_OFF) 
		{
			mdelay(100);
		}
		else
		{
			printk("%s: reset lcd!!\n", __func__);

			ili9486_ldi_poweroff_smd();
			nt35510_lcd_LDO_off();
			mdelay(100);
			nt35510_lcd_LDO_on();

			// Activate Reset
			gpio_set_value(OMAP_GPIO_MLCD_RST, GPIO_LEVEL_LOW);	
			mdelay(1);
			gpio_set_value(OMAP_GPIO_MLCD_RST, GPIO_LEVEL_HIGH);
			mdelay(5);
			
			//MLCD pin set to InputPulldown.
			omap_ctrl_writew(0x010C, 0x1c6);

			mdelay(100);
			ili9486_ldi_poweron_smd();
			mdelay(100);
			aat1402_set_brightness();
			g_esd_counter = 0;
		}	
		set_irq_type(irq, IRQ_TYPE_LEVEL_LOW);
	}

	return IRQ_HANDLED;
}

static void enable_esd_wdog(void)
{
	if (get_hw_revision() < 5)	/* 5=> Rev0.4 */
	{
		return;
	}

	if (!g_lcd_exist_flag)	/* no need to run wdog if lcd unavailable */
	{
		return;
	}

	schedule_delayed_work(&lcd_chk_work, HZ*13);
}

#endif	/* ENABLE_ESD_WDOG */

static int nt35510_spi_probe(struct spi_device *spi)
{
    struct backlight_properties props;
     int status =0;
		
	printk(KERN_INFO " **** nt35510_spi_probe.\n");
	nt35510lcd_spi = spi;
	nt35510lcd_spi->mode = SPI_MODE_0;
	nt35510lcd_spi->bits_per_word = 9 ;

	printk(" nt35510lcd_spi->chip_select = %x\t, mode = %x\n", nt35510lcd_spi->chip_select,  nt35510lcd_spi->mode);
	printk(" ax_speed_hz  = %x\t modalias = %s", nt35510lcd_spi->max_speed_hz, nt35510lcd_spi->modalias );
	
	status = spi_setup(nt35510lcd_spi);
	printk(" spi_setup ret = %x\n",status );
	
	omap_dss_register_driver(&nt35510_driver);
//	led_classdev_register(&spi->dev, &nt35510_backlight_led);
	struct backlight_device *bd;
	bd = backlight_device_register("omap_bl", &spi->dev, NULL, &aat1402_bl_ops, &props);
	bd->props.max_brightness = 255;
	bd->props.brightness = 125;
	
#ifndef CONFIG_FB_OMAP_BOOTLOADER_INIT
	lcd_enabled = 0;
	nt35510_lcd_poweron();
#else
	lcd_enabled =1;
#endif

	return 0;
}

static int nt35510_spi_remove(struct spi_device *spi)
{
//	led_classdev_unregister(&nt35510_backlight_led);
	omap_dss_unregister_driver(&nt35510_driver);

	return 0;
}
static void nt35510_spi_shutdown(struct spi_device *spi)
{
	printk("*** First power off LCD.\n");
	is_nt35510_spi_shutdown = 1;

#ifdef ENABLE_ESD_WDOG
	disable_esd_wdog();
#endif

	nt35510_lcd_poweroff();
	printk("*** power off - backlight.\n");
	//gpio_set_value(OMAP_GPIO_LCD_EN_SET, GPIO_LEVEL_LOW);
	lcd_en_set(0);
}

static int nt35510_spi_suspend(struct spi_device *spi, pm_message_t mesg)
{
    //spi_send(spi, 2, 0x01);  /* R2 = 01h */
    //mdelay(40);

#if 0
	nt35510lcd_spi = spi;
	nt35510lcd_spi->mode = SPI_MODE_0;
	nt35510lcd_spi->bits_per_word = 16 ;
	spi_setup(nt35510lcd_spi);

	lcd_poweroff();
	zeus_panel_power_enable(0);
#endif

	return 0;
}

static int nt35510_spi_resume(struct spi_device *spi)
{
	/* reinitialize the panel */
#if 0
	zeus_panel_power_enable(1);
	nt35510lcd_spi = spi;
	nt35510lcd_spi->mode = SPI_MODE_0;
	nt35510lcd_spi->bits_per_word = 16 ;
	spi_setup(nt35510lcd_spi);

	lcd_poweron();
#endif
	return 0;
}

static struct spi_driver nt35510_spi_driver = {
	.probe    = nt35510_spi_probe,
	.remove   = nt35510_spi_remove,
	.shutdown = nt35510_spi_shutdown,
	.suspend  = nt35510_spi_suspend,
	.resume   = nt35510_spi_resume,
	.driver   = {
		.name   = "nt35510_disp_spi",
		.bus    = &spi_bus_type,
		.owner  = THIS_MODULE,
	},
};

static int __init nt35510_lcd_init(void)
{
   	return spi_register_driver(&nt35510_spi_driver);
}

static void __exit nt35510_lcd_exit(void)
{
	return spi_unregister_driver(&nt35510_spi_driver);
}

module_init(nt35510_lcd_init);
module_exit(nt35510_lcd_exit);
MODULE_LICENSE("GPL");
