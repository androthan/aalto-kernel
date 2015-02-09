/*
 * module/samsung_battery/battery_monitor.c
 *
 * SAMSUNG battery driver for Linux
 *
 * Copyright (C) 2009 SAMSUNG ELECTRONICS.
 * Author: EUNGON KIM (egstyle.kim@samsung.com)
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/i2c/twl4030-madc.h>
#include <linux/i2c/twl.h>
#include <linux/power_supply.h>
#include <linux/wakelock.h>
#include <linux/regulator/consumer.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <plat/omap3-gptimer12.h>
#include "common.h"

// To access sysfs file system [+]
#include <linux/uaccess.h>
#include <linux/fs.h>
// To access sysfs file system [-]

#include <linux/gpio.h>
#include <plat/mux.h>

#define DRIVER_NAME "secBattMonitor"

#define TEMP_DEG    0
#define TEMP_ADC    1

/* In module TWL4030_MODULE_PM_RECEIVER */
#define VSEL_VINTANA2_2V75  0x01
#define CARKIT_ANA_CTRL     0xBB
#define SEL_MADC_MCPC       0x08

#define TWL_MODULE_PM_RECEIVER 0x15
#define TWL4030_VMMC2_DEV_GRP		0x2B
#define TWL4030_VMMC2_DEDICATED		0x2E

#define	BATT_AVER_PER	30

#define	BATT_VOL_AVG		0
#define	BATT_VOL_ADC_AVG	1
#define	BATT_TEMP_AVG		2
#define	BATT_TEMP_ADC_AVG	3
#define	BATT_MEASUREMENT	4

//#define _OMS_FEATURES_ // CHINA BORQS CONCEPTS
#define CONFIG_SEC_BATTERY_USE_RECOVERY_MODE

static DEFINE_MUTEX( battery_lock );

static int NCP15WB473_batt_table[] = 
{
    /* -15C ~ 85C */

    /*0[-15] ~  14[-1]*/
    360850,342567,322720,304162,287000,
    271697,255331,241075,227712,215182,
    206463,192394,182034,172302,163156, 
    /*15[0] ~ 34[19]*/
    158214,146469,138859,131694,124947,
    122259,118590,112599,106949,101621,
    95227, 91845, 87363, 83128, 79126,
    74730, 71764, 68379, 65175, 62141,
    /*35[20] ~ 74[59]*/    
    59065, 56546, 53966, 51520, 49201,
    47000, 44912, 42929, 41046, 39257,
    37643, 35942, 34406, 32945, 31555,
    30334, 28972, 27773, 26630, 25542,
    24591, 23515, 22571, 21672, 20813,
    20048, 19211, 18463, 17750, 17068,
    16433, 15793, 15197, 14627, 14082,
    13539, 13060, 12582, 12124, 11685,
    /*75[60] ~ 100[85]*/
    11209, 10862, 10476, 10106,  9751,
    9328,  9083,  8770,  8469,  8180,
    7798,  7635,  7379,  7133,  6896,
    6544,  6450,  6240,  6038,  5843,
    5518,  5475,  5302,  5134,  4973,
    4674
};

struct battery_device_config
// THIS CONFIG IS SET IN BOARD_FILE.(platform_data)
{
    /* SUPPORT MONITORING CHARGE CURRENT FOR CHECKING FULL */
    int MONITORING_CHG_CURRENT;
    int CHG_CURRENT_ADC_PORT;

    /* SUPPORT MONITORING TEMPERATURE OF THE SYSTEM FOR BLOCKING CHARGE */
    int MONITORING_SYSTEM_TEMP;
    int TEMP_ADC_PORT;
};

typedef struct {
	int msize;
	int mindex;
	int data_window[BATT_AVER_PER];
	int sum_data;
	int avg_data;
}BATTERY_AVG;

static BATTERY_AVG battery_avg[BATT_MEASUREMENT] = {
	{0, 0, {0,},0, 0},
	{0, 0, {0,},0, 0},
	{0, 0, {0,},0, 0},
	{0, 0, {0,},0, 0},
};

struct battery_device_info 
{
    struct device *dev;
    struct delayed_work battery_monitor_work;

    // LDO USB1V5, USB1V9 have a same unique operating mode.
    struct regulator *usb1v5;
    struct regulator *usb1v8;
    // LDO USB3V1 have a unique operating mode.
    struct regulator *usb3v1;

    struct power_supply sec_battery;
    struct power_supply sec_ac;
    struct power_supply sec_usb;    
};

static struct device *this_dev;
static struct wake_lock sec_bc_wakelock;

static SEC_battery_charger_info sec_bci;
static struct battery_device_config *device_config;

static struct gptimer12_timer batt_gptimer_12;

static char *samsung_bci_supplied_to[] = {
    "battery",
};


SEC_battery_charger_info *get_sec_bci( void )
{
    return &sec_bci;
}

static int avr_vol = 0;
static int avr_vol_adc = 0;
static int avr_temp = 0;
static int avr_temp_adc = 0;

static int jep01 = 0;

// Prototype
       int _charger_state_change_( int , int, bool );
       int _low_battery_alarm_( void );
       int _get_average_value_( int *, int );
       int _get_t2adc_data_( int );
static int turn_resources_off_for_adc( void );
static int turn_resources_on_for_adc( void );
static int get_elapsed_time_secs( unsigned long long * );
static int t2adc_to_temperature( int , int );
static int do_fuelgauge_reset( void );
static int get_battery_level_adc( bool flag );
static int get_battery_level_ptg( void );
static int get_system_temperature( bool );
static int get_charging_current_adc_val( void );
static int check_full_charge_using_chg_current( int );
static int check_full_charge_using_chg_current2( int );
static void get_system_status_in_sleep( int *, int *, int *, int * );
static int battery_monitor_core( bool );
static void battery_monitor_work_handler( struct work_struct * );
static int battery_monitor_fleeting_wakeup_handler( unsigned long ); 
static int samsung_battery_get_property( struct power_supply *, enum power_supply_property , union power_supply_propval * );
static int samsung_ac_get_property( struct power_supply *, enum power_supply_property, union power_supply_propval * );
static int samsung_usb_get_property( struct power_supply *, enum power_supply_property, union power_supply_propval * );
static void samsung_pwr_external_power_changed( struct power_supply * );
static ssize_t store_event(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t size);

static int __devinit battery_probe( struct platform_device * );
static int __devexit battery_remove( struct platform_device * );
static int battery_suspend( struct platform_device *, pm_message_t );
static int battery_resume( struct platform_device * );
static int __init battery_init( void );
static void __exit battery_exit( void );

// Charger
extern int charger_init( void );
extern void charger_exit( void );
extern int _battery_state_change_( int category, int value, bool is_sleep );
extern int _check_full_charge_dur_sleep_( void );
extern int _cable_status_now_( void );
extern int microusb_enable(void);

// Fuel Guage
extern int fuelgauge_init( void );
extern void fuelgauge_exit( void );
extern int fuelgauge_quickstart( void );
extern int get_fuelgauge_adc_value( int count, bool is_sleep, bool flag );
extern int get_fuelgauge_ptg_value( bool is_sleep );
//+ phill-it: 20110908
#ifdef __BATTERY_COMPENSATION__
extern void adcfg_set_compensate(int mode,int offset,int compensate_value);
#endif

// MAX14577
extern int check_full_charge_current(bool is_sleep);
extern void MD_AL25_ServiceStateMachine( void );

#if 0
extern int update_rcomp_by_temperature(int temp);
#endif

// Sleep i2c, ADC
extern void twl4030_i2c_init( void );
extern void twl4030_i2c_disinit( void );
extern void normal_i2c_init( void );
extern void normal_i2c_disinit( void );
extern s32 t2_adc_data( u8 channel );

extern unsigned long long sched_clock( void );

extern u32 sec_bootmode;

static bool boot_complete = false;
static int boot_monitor_count = 0;

int stop_temperature_overheat = CHARGE_STOP_TEMPERATURE_MAX;
int recover_temperature_overheat = CHARGE_RECOVER_TEMPERATURE_MAX;
int stop_temperature_cold = CHARGE_STOP_TEMPERATURE_MIN;
int recover_temperature_cold = CHARGE_RECOVER_TEMPERATURE_MIN;

#ifdef CONFIG_SEC_BATTERY_USE_RECOVERY_MODE
static int recovery_mode = 0;
module_param(recovery_mode, bool, 0);
#endif  /* CONFIG_SEC_BATTER_USE_RECOVERY_MODE */

// ------------------------------------------------------------------------- // 
//                           sysfs interface                                 //
// ------------------------------------------------------------------------- // 
#define  __ATTR_SHOW_CALLBACK( _name, _ret_val ) \
static ssize_t _name( struct kobject *kobj, \
              struct kobj_attribute *attr, \
              char *buf ) \
{ \
    return sprintf ( buf, "%d\n", _ret_val ); \
} 

static int get_batt_monitor_temp( void )
{
    return sec_bci.battery.support_monitor_temp;
}

static ssize_t store_batt_monitor_temp(struct kobject *kobj,
                    struct kobj_attribute *attr,
                    const char *buf, size_t size)
{
    int flag;
    
    sscanf( buf, "%d", &flag );

#ifdef FCHG_DBG
    printk("[BM] change value %d\n",flag);
#endif

    sec_bci.battery.support_monitor_temp = flag;
    sec_bci.battery.support_monitor_timeout = flag;
    sec_bci.battery.support_monitor_full = flag;

    return size;
}

static ssize_t store_batt_boot_complete(struct kobject *kobj,
                    struct kobj_attribute *attr,
                    const char *buf, size_t size)
{
    int flag;

    sscanf( buf, "%d", &flag );
#ifdef FCHG_DBG
    printk("[BM] boot complete flag:%d, buf:%s, size:%d\n",flag, buf, size);
#endif

    boot_complete = true;

    return size;
}

static ssize_t store_batt_cal(struct kobject *kobj,
                    struct kobj_attribute *attr,
                    const char *buf, size_t size)
{
	int flag;

    sscanf( buf, "%d", &flag );

#ifdef FCHG_DBG
    printk("[BM] Battery ADC Calibration Constant = %d\n", flag);
#endif

	sec_bci.battery.battery_cal = flag;

    return size;
}

/*
__ATTR_SHOW_CALLBACK( show_batt_vol, sec_bci.battery.battery_level_vol )
__ATTR_SHOW_CALLBACK( show_batt_vol_adc, sec_bci.battery.battery_level_vol_adc )
__ATTR_SHOW_CALLBACK( show_batt_temp, sec_bci.battery.battery_temp * 10 )
__ATTR_SHOW_CALLBACK( show_batt_temp_adc, sec_bci.battery.battery_temp_adc )
*/
__ATTR_SHOW_CALLBACK( show_batt_vol, get_battery_level_adc(0) )
__ATTR_SHOW_CALLBACK( show_batt_vol_adc, get_battery_level_adc(1) )
__ATTR_SHOW_CALLBACK( show_batt_temp, get_system_temperature( TEMP_DEG ) * 10 )
__ATTR_SHOW_CALLBACK( show_batt_temp_adc, get_system_temperature( TEMP_ADC ) )

__ATTR_SHOW_CALLBACK( show_batt_v_f_adc, 0 )
//__ATTR_SHOW_CALLBACK( show_batt_capacity, sec_bci.battery.battery_level_ptg )


__ATTR_SHOW_CALLBACK( show_batt_capacity, sec_bci.battery.battery_level_ptg)
//__ATTR_SHOW_CALLBACK( show_batt_capacity, get_battery_level_ptg() )
__ATTR_SHOW_CALLBACK( do_batt_fuelgauge_reset, do_fuelgauge_reset() )
__ATTR_SHOW_CALLBACK( show_batt_monitor_temp, get_batt_monitor_temp() )
__ATTR_SHOW_CALLBACK( show_batt_temp_check, sec_bci.battery.battery_health)
__ATTR_SHOW_CALLBACK( show_batt_full_check, ((sec_bci.charger.charge_status == POWER_SUPPLY_STATUS_FULL) || sec_bci.charger.charge_status == POWER_SUPPLY_STATUS_RECHARGING_FOR_FULL)?1:0)

__ATTR_SHOW_CALLBACK( show_charging_source, sec_bci.charger.cable_status )

__ATTR_SHOW_CALLBACK( show_batt_vol_aver, avr_vol )
__ATTR_SHOW_CALLBACK( show_batt_vol_adc_aver, avr_vol_adc )
__ATTR_SHOW_CALLBACK( show_batt_temp_aver, avr_temp * 10 )
__ATTR_SHOW_CALLBACK( show_batt_temp_adc_aver, avr_temp_adc )

__ATTR_SHOW_CALLBACK( show_batt_cal, sec_bci.battery.battery_cal )
//__ATTR_SHOW_CALLBACK( show_batt_vol_adc_cal, avr_vol_adc + sec_bci.battery.battery_cal )
//__ATTR_SHOW_CALLBACK( show_batt_vol_cal, avr_vol + sec_bci.battery.battery_cal )

#ifdef _OMS_FEATURES_
__ATTR_SHOW_CALLBACK( show_batt_vol_toolow, sec_bci.battery.battery_vol_toolow )
static struct kobj_attribute batt_vol_toolow =
    __ATTR( batt_vol_toolow, 0644, show_batt_vol_toolow, NULL );
#endif


static struct kobj_attribute batt_sysfs_testmode[] = {
	/* Event logging - Put these attributes at first position of this array 
       For using the call back function 'store_event'
	*/
    __ATTR( mp3, 0664, NULL, store_event ), 
    __ATTR( talk_wcdma, 0664, NULL, store_event ), 
    __ATTR( talk_gsm, 0664, NULL, store_event ), 
    __ATTR( data_call, 0664, NULL, store_event ), 
    __ATTR( vt_call, 0664, NULL, store_event ), 
    __ATTR( camera_preview, 0664, NULL, store_event ), 
    __ATTR( camera_recording, 0664, NULL, store_event ), 
    __ATTR( video, 0664, NULL, store_event ), 
    __ATTR( g_map, 0664, NULL, store_event ), 
    __ATTR( e_book, 0664, NULL, store_event ), 
    __ATTR( bt_call, 0664, NULL, store_event ), 
    __ATTR( wap_browsing, 0664, NULL, store_event ), 
    __ATTR( wifi_browsing, 0664, NULL, store_event ), 
	__ATTR( browser, 0664, NULL, store_event ), 
    __ATTR( game, 0664, NULL, store_event ), 
    /* END of Event logging */

    __ATTR( batt_vol, 0644, show_batt_vol, NULL ),
    __ATTR( batt_vol_adc, 0644, show_batt_vol_adc, NULL ),
    __ATTR( batt_temp, 0644, show_batt_temp, NULL ),
    __ATTR( batt_temp_adc, 0644, show_batt_temp_adc, NULL ),
    __ATTR( batt_v_f_adc, 0644, show_batt_v_f_adc, NULL ),
    __ATTR( batt_capacity, 0644, show_batt_capacity, NULL ),
    __ATTR( batt_fuelgauge_reset, 0644, do_batt_fuelgauge_reset, NULL ),
    __ATTR( batt_monitor_temp, 0664, show_batt_monitor_temp, store_batt_monitor_temp ),
    __ATTR( batt_boot_complete, 0664, NULL, store_batt_boot_complete ),
    __ATTR( fg_soc, 0644, show_batt_capacity, NULL ),
    __ATTR( batt_temp_check, 0644, show_batt_temp_check, NULL ),
    __ATTR( batt_full_check, 0644, show_batt_full_check, NULL ),    
    __ATTR( charging_source, 0644, show_charging_source, NULL ), 

    __ATTR( batt_vol_aver, 0644, show_batt_vol_aver, NULL ),
    __ATTR( batt_vol_adc_aver, 0644, show_batt_vol_adc_aver, NULL ),
    __ATTR( batt_temp_aver, 0644, show_batt_temp_aver, NULL ),
    __ATTR( batt_temp_adc_aver, 0644, show_batt_temp_adc_aver, NULL ),

    __ATTR( batt_cal, 0644, show_batt_cal, store_batt_cal ),
//    __ATTR( batt_vol_adc_cal, 0644, show_batt_vol_adc_cal, NULL ),
//    __ATTR( batt_vol_cal, 0644, show_batt_vol_cal, NULL ),
};

/* Event logging */
u32 event_logging = 0;

enum{
	MP3 = 0, TALK_WCDMA, TALK_GSM, DATA_CALL, VT_CALL, CAMERA_PREVIEW, CAMERA_RECORDING, 
	VIDEO, G_MAP, E_BOOK, BT_CALL, WAP_BROWSING, WIFI_BROWSING, BROWSER, GAME
};

//+ phill-it: 20110908
static ssize_t store_event(struct kobject *kobj,
                    struct kobj_attribute *attr,
                    const char *buf, size_t size)
{
    int flag;

	const ptrdiff_t off = attr - batt_sysfs_testmode;

    sscanf( buf, "%d", &flag );
	if(flag == 1)
		event_logging |= (0x1 << off);
	else if(flag == 0)
		event_logging &= ~(0x1 << off);

#ifdef __BATTERY_COMPENSATION__
	switch (off) {
			case DATA_CALL:
			case TALK_WCDMA:
			case TALK_GSM:
			case VT_CALL:
			case G_MAP:
			case BT_CALL:
			case WAP_BROWSING:
			case GAME:
    					adcfg_set_compensate(flag, OFFSET_WAP_BROWSING, COMPENSATE_DEFAULT);
					break;
			case MP3:
    					adcfg_set_compensate(flag, OFFSET_MP3_PLAY, COMPENSATE_MP3);
					break;
			case CAMERA_PREVIEW:
    					adcfg_set_compensate(flag, OFFSET_CAMERA_PREVIEW,COMPENSATE_CAMERA_PREVIEW );
					break;
			case CAMERA_RECORDING:
    					adcfg_set_compensate(flag, OFFSET_CAMERA_RECORDING, COMPENSATE_CAMERA_RECORDING);
					break;
			case VIDEO:
    					adcfg_set_compensate(flag, OFFSET_VIDEO_PLAY,COMPENSATE_VIDEO );
					break;
			case E_BOOK:
    					adcfg_set_compensate(flag, OFFSET_EBOOK, COMPENSATE_EBOOK );
					break;
			case WIFI_BROWSING:
    					adcfg_set_compensate(flag, OFFSET_WIFI_BROWSING, COMPENSATE_WIFI_BROWSING );
					break;
			case BROWSER:
    					adcfg_set_compensate(flag, OFFSET_BROWSER, COMPENSATE_BROWSER );
					break;
			default:
					dbgPrintk("not exist offset=%d", off);
					break;
	}
#endif
	dbgPrintk("[BM] %s, offset=%d, value=%d, evt:%x",__func__, off, flag, event_logging);

    return size;
}

/* END of Event logging */

int _charger_state_change_( int category, int value, bool is_sleep )
{   
#ifdef FCHG_DBG
    printk( "[BM] cate: %d, value: %d\n", category, value );
#endif

    if( category == STATUS_CATEGORY_CABLE )
    {
        switch( value )
        {
            case POWER_SUPPLY_TYPE_BATTERY :
                /*Stop monitoring the batt. level for Re-charging*/
                sec_bci.battery.monitor_field_rechg_vol = false;

                /*Stop monitoring the temperature*/
                sec_bci.battery.monitor_field_temp = false;

                sec_bci.battery.confirm_full_by_current = 0;
                sec_bci.battery.confirm_recharge = 0;

                sec_bci.charger.charging_timeout = DEFAULT_CHARGING_TIMEOUT;

                sec_bci.charger.full_charge_dur_sleep = 0x0;
                break;

            case POWER_SUPPLY_TYPE_MAINS :
                sec_bci.charger.charging_timeout = DEFAULT_CHARGING_TIMEOUT;
                wake_lock_timeout( &sec_bc_wakelock , HZ );         
                break;

            case POWER_SUPPLY_TYPE_USB :            
                break;

            default :
                break;
        }

        goto Out_Charger_State_Change;
    }
    else if( category == STATUS_CATEGORY_CHARGING )
    {
        switch( value )
        {
            case POWER_SUPPLY_STATUS_UNKNOWN :
            case POWER_SUPPLY_STATUS_NOT_CHARGING :
                //sec_bci.charger.full_charge = false;
                
                /*Stop monitoring the batt. level for Re-charging*/
                sec_bci.battery.monitor_field_rechg_vol = false;

                if( sec_bci.battery.battery_health != POWER_SUPPLY_HEALTH_OVERHEAT 
                    && sec_bci.battery.battery_health != POWER_SUPPLY_HEALTH_COLD )
                {
                    /*Stop monitoring the temperature*/
                    sec_bci.battery.monitor_field_temp = false;
                }

                break;

            case POWER_SUPPLY_STATUS_DISCHARGING :
                break;

            case POWER_SUPPLY_STATUS_FULL :
                break;

            case POWER_SUPPLY_STATUS_CHARGING :
                /*Start monitoring the temperature*/
                sec_bci.battery.monitor_field_temp = true;

                /*Stop monitoring the batt. level for Re-charging*/
                sec_bci.battery.monitor_field_rechg_vol = false;

                break;

            case POWER_SUPPLY_STATUS_RECHARGING_FOR_FULL :
                /*Start monitoring the temperature*/
                sec_bci.battery.monitor_field_temp = true;

                /*Stop monitoring the batt. level for Re-charging*/
                sec_bci.battery.monitor_field_rechg_vol = false;

				/*Not change the battery bar - keep battery full screen*/
				//goto Out_Charger_State_Change;
                break;

            case POWER_SUPPLY_STATUS_RECHARGING_FOR_TEMP :
                /*Start monitoring the temperature*/
                sec_bci.battery.monitor_field_temp = true;

                /*Stop monitoring the batt. level for Re-charging*/
                sec_bci.battery.monitor_field_rechg_vol = false;

                break;

		case POWER_SUPPLY_STATUS_FULL_CUT_OFF:
                sec_bci.battery.monitor_field_rechg_vol = true;

                /*Stop monitoring the temperature*/
                sec_bci.battery.monitor_field_temp = false;

                wake_lock_timeout( &sec_bc_wakelock , HZ );
			break;

            default :
                break;
        }

    }

   	 if( !is_sleep )
   	 {
   	     struct battery_device_info *di;
   	     struct platform_device *pdev;

   	     pdev = to_platform_device( this_dev );
   	     di = platform_get_drvdata( pdev );
   	     cancel_delayed_work( &di->battery_monitor_work );
   	     queue_delayed_work( sec_bci.sec_battery_workq, &di->battery_monitor_work, 5 * HZ ); 

   	     power_supply_changed( &di->sec_battery );
   	     power_supply_changed( &di->sec_ac );
   	     power_supply_changed( &di->sec_usb );
   	 }
   	 else
   	 {
   	     release_gptimer12( &batt_gptimer_12 );
   	     request_gptimer12( &batt_gptimer_12 );
   	 }

Out_Charger_State_Change :
    return 0;
}

int _low_battery_alarm_()
{
    struct battery_device_info *di;
    struct platform_device *pdev;
    int level;

    pdev = to_platform_device( this_dev );
    di = platform_get_drvdata( pdev );

    level = get_battery_level_ptg();
    //if ( level == 1 )
    //    sec_bci.battery.battery_level_ptg = 0;
    //else 
        sec_bci.battery.battery_level_ptg = level;

    wake_lock_timeout( &sec_bc_wakelock , HZ );
    power_supply_changed( &di->sec_battery );

    return 0;
}

int _get_average_value_( int *data, int count )
{
    int average;
    int min = 0;
    int max = 0;
    int sum = 0;
    int i;

    if ( count >= 5 )
    {
        min = max = data[0];
        for( i = 0; i < count; i++ )
        {
            if( data[i] < min )
                min = data[i];

            if( data[i] > max )
                max = data[i];

            sum += data[i];
        }
        average = ( sum - max - min ) / ( count - 2 );
    }
    else
    {
        for( i = 0; i < count; i++ )
            sum += data[i];

        average = sum / count;
    }

    return average; 
}

int get_vbat_adc(bool is_sleep)
{

    struct twl4030_madc_request req;
	int adc;

	if (is_sleep != true)
	{
		req.channels = ( 1 << 12 );
		req.do_avg = 0;
		req.method = TWL4030_MADC_SW1;
		req.active = 0;
		req.func_cb = NULL;


		twl4030_madc_conversion( &req );
		adc = req.rbuf[12];
	}
	else /* while sleeping, normal i2c operation isn't possible which use schedule functions */
	{
		adc = t2_adc_data(6);	/* vbat adc channel = 12 */
		//printk("%s : VBAT ADC in sleep => %d\n", __func__, adc);
	}

	adc += sec_bci.battery.battery_cal;	/* battery VBAT calibartion */

	return adc;
}

int _get_t2adc_data_( int ch )
{
    int ret = 0;
    int val[5];
    int i;
    struct twl4030_madc_request req;

    // To control thermal sensor power
 //   gpio_set_value(OMAP_GPIO_EN_TEMP_VDD, 1);
#if ( CONFIG_SAMSUNG_REL_HW_REV >= 4 )
/* AALTO(taejin.hyeon) 2011.03.18 Aalto vmmc2 is used by TSP, not usb. */
//	twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, 0x20, TWL4030_VMMC2_DEV_GRP );
/* AALTO(taejin.hyeon) 2011.03.18. end */
#endif

	if ( ch >= 1 && ch <= 7 ){
	    turn_resources_on_for_adc();
	    twl_i2c_write_u8( TWL4030_MODULE_USB, SEL_MADC_MCPC, CARKIT_ANA_CTRL );

	    msleep(100);
	}

    req.channels = ( 1 << ch );
    req.do_avg = 0;
    req.method = TWL4030_MADC_SW1;
    req.active = 0;
    req.func_cb = NULL;



    #if 0
    twl4030_madc_conversion( &req );
    ret = req.rbuf[ch];
    #else
    for ( i = 0; i < 5 ; i++ )
    {
        twl4030_madc_conversion( &req );
        val[i] = req.rbuf[ch];
    }

    ret = _get_average_value_( val, 5 );
    #endif

	if ( ch >= 1 && ch <= 7 ){
	    turn_resources_off_for_adc();
	}
    // To control thermal sensor power
//    gpio_set_value(OMAP_GPIO_EN_TEMP_VDD, 0);
#if ( CONFIG_SAMSUNG_REL_HW_REV >= 4 )
/* AALTO(taejin.hyeon) 2011.03.18 Aalto vmmc2 is used by TSP, not usb. */
//	twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, 0x0, TWL4030_VMMC2_DEV_GRP );
/* AALTO(taejin.hyeon) 2011.03.18. end */
#endif

    return ret;
}

int turn_resources_on_for_adc()
{
    int ret;
    u8 val = 0; 
    
    struct battery_device_info *di;
    struct platform_device *pdev;

    pdev = to_platform_device( this_dev );
    di = platform_get_drvdata( pdev );

    ret = twl_i2c_read_u8( TWL4030_MODULE_MADC, &val, TWL4030_MADC_CTRL1 );
    val &= ~TWL4030_MADC_MADCON;
    ret = twl_i2c_write_u8( TWL4030_MODULE_MADC, val, TWL4030_MADC_CTRL1 );
    msleep( 10 );

    ret = twl_i2c_read_u8( TWL4030_MODULE_MADC, &val, TWL4030_MADC_CTRL1 );
    val |= TWL4030_MADC_MADCON;
    ret = twl_i2c_write_u8( TWL4030_MODULE_MADC, val, TWL4030_MADC_CTRL1 );
    
    if(device_config->TEMP_ADC_PORT != 0)
    {
        ret = regulator_enable( di->usb3v1 );
        if ( ret )
            printk("[BM] Regulator 3v1 error!!\n");

        ret = regulator_enable( di->usb1v8 );
        if ( ret )
            printk("[BM] Regulator 1v8 error!!\n");

        ret = regulator_enable( di->usb1v5 );
        if ( ret )
            printk("[BM] Regulator 1v5 error!!\n");
    }    

    twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, 0x14, 0x7D/*VUSB_DEDICATED1*/ );
    twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, 0x0, 0x7E/*VUSB_DEDICATED2*/ );
    twl_i2c_read_u8( TWL4030_MODULE_USB, &val, 0xFE/*PHY_CLK_CTRL*/ );
    val |= 0x1;
    twl_i2c_write_u8( TWL4030_MODULE_USB, val, 0xFE/*PHY_CLK_CTRL*/ );

    twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, VSEL_VINTANA2_2V75, TWL4030_VINTANA2_DEDICATED );    
    twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, 0x20, TWL4030_VINTANA2_DEV_GRP );

    return 0;
}

int turn_resources_off_for_adc()
{
    u8 val = 0; 
    struct battery_device_info *di;
    struct platform_device *pdev;

    pdev = to_platform_device( this_dev );
    di = platform_get_drvdata( pdev );

    if ( sec_bci.charger.cable_status == POWER_SUPPLY_TYPE_BATTERY )
    {
        twl_i2c_read_u8( TWL4030_MODULE_USB, &val, 0xFE/*PHY_CLK_CTRL*/ );
        val &= 0xFE;
        twl_i2c_write_u8( TWL4030_MODULE_USB, val, 0xFE/*PHY_CLK_CTRL*/ );
    }

    
    if(device_config->TEMP_ADC_PORT != 0)
    {
        regulator_disable( di->usb1v5 );
        regulator_disable( di->usb1v8 );
        regulator_disable( di->usb3v1 );
    }
    return 0;
}

static int get_elapsed_time_secs( unsigned long long *start )
{
    unsigned long long now;
    unsigned long long diff;
    unsigned long long max = 0xFFFFFFFF;

    max = ( max << 32 ) | 0xFFFFFFFF;
    
    now = sched_clock();

    if ( now >= *start )
    {
        diff = now - *start;
    }
    else 
    {
		sec_bci.charger.charge_start_time = now;
		diff = 0;
		//diff = max - *start + now;
    }

    do_div(diff, 1000000000L);
	/*
    printk( KERN_DEBUG "[BM] now: %llu, start: %llu, diff:%d\n",
        now, *start, (int)diff );
	*/
    return (int)diff;       
}

static int t2adc_to_temperature( int value, int channel )
{
    int mvolt, r;
    int temp;

    /*Caluclating voltage(adc) and resistance of thermistor */
    if( channel == 5 )
    {
        // vol = conv_result * step-size / R (TWLTRM Table 9-4)
        mvolt = value * 2500 / 1023; // mV
    }
    else if (channel == 0)
    {
        mvolt = value * 1500 / 1023;
    }
    else
    {
        mvolt = 0;
    }

    //printk("[BM] TEMP. adc: %d, mVolt: %dmA\n", value , mvolt );

    // for ZEUS - VDD: 3000mV, Rt = ( 100K * Vadc  ) / ( VDD - 2 * Vadc )
    r = ( 100000 * mvolt ) / ( 3000 - 2 * mvolt );

    /*calculating temperature*/
    for( temp = 100; temp >= 0; temp-- )
    {
        if( ( NCP15WB473_batt_table[temp] - r ) >= 0 )
            break;
    }
    temp -= 15;
    temp=temp-1;

    return temp;
}

static int do_fuelgauge_reset( void )
{
    fuelgauge_quickstart();
    return 1;
}

//+ phill-it: bhji
static int average_battery(int type, int value)
{
	int res;


#if 0 // for input range
	if ((type < 0) || (type >= BATT_MEASUREMENT) || (value < 0))
		return -1;
#endif

	if (battery_avg[type].msize >= BATT_AVER_PER) {
		if (++battery_avg[type].mindex >= BATT_AVER_PER)
			battery_avg[type].mindex = 0;
		battery_avg[type].sum_data -= battery_avg[type].data_window[battery_avg[type].mindex];
		battery_avg[type].sum_data += value;
		battery_avg[type].data_window[battery_avg[type].mindex] = value;
	} else {
		battery_avg[type].data_window[battery_avg[type].mindex] = value;
		battery_avg[type].msize++;
		battery_avg[type].mindex = battery_avg[type].msize;
		battery_avg[type].sum_data += value;
	}

	res = battery_avg[type].sum_data / battery_avg[type].msize;

	return res;
}
//- 

static int get_battery_level_adc( bool flag )
{
	int value;

	if (!flag) {
		value = get_fuelgauge_adc_value( 5, CHARGE_DUR_ACTIVE, 0 );
		if (value < 0) {
			value = sec_bci.battery.battery_level_vol;
			//printk("[BM]get_fuelgauge_adc_value return data < 0 %d\n",sec_bci.battery.battery_level_vol);
		}

		avr_vol = average_battery(BATT_VOL_AVG, value);
		sec_bci.battery.battery_level_vol = value;

#ifdef CONFIG_SAMSUNG_BATTERY_TESTMODE
    return 4100;
#else
	//printk("[BM] %s return data %d\n",__func__,value);
    return value;
#endif
	} else {
		value = get_fuelgauge_adc_value( 5, CHARGE_DUR_ACTIVE, 1 );
		if (value < 0) {
			value = sec_bci.battery.battery_level_vol_adc;
			//printk("[BM]flag get_fuelgauge_adc_value return data < 0 %d\n",sec_bci.battery.battery_level_vol_adc);
		}

		avr_vol_adc = average_battery(BATT_VOL_ADC_AVG, value);
		sec_bci.battery.battery_level_vol_adc = value;
		//printk("[BM] %s flag return data %d\n",__func__,value);

		return value;
	}
}

static int get_adjusted_battery_ptg(int value)
{
	if(value == 100)
		value = 100;
	else if(value < 30)
		value = ((value*100*4/3) + 50)/115;
	else if(value < 76)
		value = value + 5;
	else
		value = ((value*100-7600)*8/10+50)/80+81;
	
	if(value > 100)
		value = 100;
	return value;
}

static int get_battery_level_ptg( void )
{
    int value;

    value = get_fuelgauge_ptg_value( CHARGE_DUR_ACTIVE );
	//printk("[BM] %s return data %d\n",__func__,value);

	/* adjust percentage value for Latona */
	//if(!sec_bci.charger.is_charging)
//	value = get_adjusted_battery_ptg(value);

	/*
    if ( sec_bci.charger.is_charging && value >= 100)
        value = 99;
    */
	//P110106-0134
    if (( sec_bci.charger.charge_status == POWER_SUPPLY_STATUS_FULL ) || ( sec_bci.charger.charge_status == POWER_SUPPLY_STATUS_RECHARGING_FOR_FULL))
		value = 100;

    if(!boot_complete && value <= 0)
        value = 1;

	//printk("[BM] %s return data %d\n",__func__,value);
//    sec_bci.battery.battery_level_ptg = value;
#ifdef CONFIG_SAMSUNG_BATTERY_TESTMODE
    return 60;
#else
    return value;
#endif
}

static int get_system_temperature( bool flag )
{
	int adc;
	int temp;

	u8 regval = 0;

	twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
				  &regval, TWL4030_BCI_BCICTL1);
	twl_i2c_write_u8(TWL4030_MODULE_MAIN_CHARGE,
				   0x0, TWL4030_BCI_BCICTL1);
    

    adc = _get_t2adc_data_( device_config->TEMP_ADC_PORT );


	if (adc < 0) {
		adc = sec_bci.battery.battery_temp_adc;
	}

    if (flag) {
		avr_temp_adc = average_battery(BATT_TEMP_ADC_AVG, adc);
		sec_bci.battery.battery_temp_adc = adc;
		return adc;
	}

	temp = t2adc_to_temperature( adc, device_config->TEMP_ADC_PORT );
	//if (temp < 0) {
	//	temp = sec_bci.battery.battery_temp;
	//}

	avr_temp = average_battery(BATT_TEMP_AVG, temp);
	sec_bci.battery.battery_temp = temp;

	return temp;
}

static int get_charging_current_adc_val( void )
{
    int adc;
    
    adc = _get_t2adc_data_( device_config->CHG_CURRENT_ADC_PORT );

    return adc;
}

static int check_full_charge_using_chg_current( int charge_current_adc )
{

    if ( sec_bci.battery.battery_level_vol < 4130 )
    {
        sec_bci.battery.confirm_full_by_current = 0;
        return 0;
    }

    if (sec_bci.battery.support_monitor_full)
    {
        if ( charge_current_adc <= CHARGE_FULL_CURRENT_ADC )
        {
            sec_bci.battery.confirm_full_by_current++;

            // changing freq. of monitoring adc to Burst.
            batt_gptimer_12.expire_time = 5;
            sec_bci.battery.monitor_duration = 5;
        }
        else
        {
            sec_bci.battery.confirm_full_by_current = 0;
            // changing freq. of monitoring adc to Default.
            batt_gptimer_12.expire_time = MONITOR_DURATION_DUR_SLEEP;
            sec_bci.battery.monitor_duration = MONITOR_DEFAULT_DURATION;
        }

        if ( sec_bci.battery.confirm_full_by_current >= 4 )
        {
            batt_gptimer_12.expire_time = MONITOR_DURATION_DUR_SLEEP;
            sec_bci.battery.monitor_duration = MONITOR_DEFAULT_DURATION;
            sec_bci.battery.confirm_full_by_current = 0;

            return 1;
        }   
    }
    return 0; 
}

static int check_full_charge_using_chg_current2( int charge_current_flag )
{

//    if ( sec_bci.battery.battery_level_vol < 4100 )
    if ( sec_bci.battery.battery_level_vol < BATT_VOL_100 )
    {
        sec_bci.battery.confirm_full_by_current = 0;
        return 0;
    }
	if (charge_current_flag==2) {
		if (avr_vol < CHARGE_RECHG_VOLTAGE) {
			sec_bci.battery.confirm_full_by_current = 0;
			return 0;
		}
	}

    if (sec_bci.battery.support_monitor_full)
    {
//        if ( charge_current_adc <= CHARGE_FULL_CURRENT_ADC )
        if ( charge_current_flag)
        {

#ifdef FCHG_DBG
	printk("[BM] charge_current_flag = %d\n",charge_current_flag);
#endif

            sec_bci.battery.confirm_full_by_current++;

            // changing freq. of monitoring adc to Burst.
            batt_gptimer_12.expire_time = 5;
            sec_bci.battery.monitor_duration = 5;
        }
        else
        {
            sec_bci.battery.confirm_full_by_current = 0;
            // changing freq. of monitoring adc to Default.
            batt_gptimer_12.expire_time = MONITOR_DURATION_DUR_SLEEP;
            sec_bci.battery.monitor_duration = MONITOR_DEFAULT_DURATION;
        }

        if ( sec_bci.battery.confirm_full_by_current >= 4 )
        {
            batt_gptimer_12.expire_time = MONITOR_DURATION_DUR_SLEEP;
            sec_bci.battery.monitor_duration = MONITOR_DEFAULT_DURATION;
            sec_bci.battery.confirm_full_by_current = 0;

            return 1;
        }   
    }
    return 0; 
}

static void get_system_status_in_sleep( int *battery_level_ptg, 
                    int *battery_level_vol, 
                    int *battery_temp, 
                    int *charge_current_flag)
{
    int temp_adc;
    int ptg_val;
    int val;

    twl4030_i2c_init();
    normal_i2c_init();

    ptg_val = get_fuelgauge_ptg_value( CHARGE_DUR_SLEEP );
//	ptg_val = get_adjusted_battery_ptg(ptg_val);

    if ( ptg_val >= 0 )
    {
        *battery_level_ptg = ptg_val;
    }

    *battery_level_vol = get_fuelgauge_adc_value( 5, CHARGE_DUR_SLEEP, 0 );
	avr_vol = average_battery(BATT_VOL_AVG, *battery_level_vol);

	val = get_fuelgauge_adc_value(5, CHARGE_DUR_SLEEP, 1);
	avr_vol_adc = average_battery(BATT_VOL_ADC_AVG, val);

    temp_adc = t2_adc_data( device_config->TEMP_ADC_PORT );
	avr_temp_adc = average_battery(BATT_TEMP_ADC_AVG, temp_adc);

    //temp_adc = _get_t2adc_data_( device_config->TEMP_ADC_PORT );

#if 0
/*
    if ( device_config->MONITORING_CHG_CURRENT )
        //*charge_current_adc = _get_t2adc_data_ ( device_config->CHG_CURRENT_ADC_PORT );
        *charge_current_adc = t2_adc_data ( device_config->CHG_CURRENT_ADC_PORT );
//*/
#endif

	*charge_current_flag = check_full_charge_current(CHARGE_DUR_SLEEP);

    normal_i2c_disinit();
    twl4030_i2c_disinit();

    *battery_temp = t2adc_to_temperature( temp_adc, device_config->TEMP_ADC_PORT ); 
	avr_temp = average_battery(BATT_TEMP_AVG, *battery_temp);

#ifdef FCHG_DBG
	printk( "[BM-S] (avg_vol:%d, vol:%d) (avg_vol_adc:%d, vol_adc:%d)\n",
					avr_vol,
					*battery_level_vol,
					avr_vol_adc,
					val);
	printk( "[BM-S] (avg_temp:%d, temp:%d) (avg_temp_adc:%d, temp_adc:%d)\n",
					avr_temp,
					*battery_temp,
					avr_temp_adc,
					temp_adc);
#endif
}

static int battery_monitor_core( bool is_sleep )
{

    int charging_time;
	//int rechg_voltage;

#if 0
	if(event_logging)
	{
		stop_temperature_overheat = CHARGE_STOP_TEMPERATURE_EVENT;
		recover_temperature_overheat = CHARGE_RECOVER_TEMPERATURE_EVENT;
	}
	else
	{
		stop_temperature_overheat = CHARGE_STOP_TEMPERATURE_MAX;
		recover_temperature_overheat = CHARGE_RECOVER_TEMPERATURE_MAX;	
	}
#endif
	
    /*Monitoring the system temperature*/
    if ( sec_bci.battery.monitor_field_temp )
    {
        if ( sec_bci.battery.support_monitor_timeout )
        {
            /*Check charging time*/
            charging_time = get_elapsed_time_secs( &sec_bci.charger.charge_start_time );
            if ( charging_time >= sec_bci.charger.charging_timeout )
            {
                if ( is_sleep ) 
                    sec_bci.charger.full_charge_dur_sleep = 0x2;
                else
                    _battery_state_change_( STATUS_CATEGORY_CHARGING, 
                                POWER_SUPPLY_STATUS_CHARGING_OVERTIME, 
                                is_sleep );

                return -1;
            }
        }

        if ( sec_bci.battery.support_monitor_temp )
        {
            if ( sec_bci.battery.battery_health == POWER_SUPPLY_HEALTH_OVERHEAT 
                || sec_bci.battery.battery_health == POWER_SUPPLY_HEALTH_COLD )
            {
                if ( sec_bci.battery.battery_temp <= recover_temperature_overheat //CHARGE_RECOVER_TEMPERATURE_MAX 
                    && sec_bci.battery.battery_temp >= recover_temperature_cold ) //CHARGE_RECOVER_TEMPERATURE_MIN )
                {
                    sec_bci.battery.battery_health = POWER_SUPPLY_HEALTH_GOOD;
                    _battery_state_change_( STATUS_CATEGORY_TEMP, 
                                BATTERY_TEMPERATURE_NORMAL, 
                                is_sleep );
                }

            }
            else
            {
                if ( sec_bci.battery.monitor_duration > MONITOR_TEMP_DURATION )
                    sec_bci.battery.monitor_duration = MONITOR_TEMP_DURATION;

                if ( sec_bci.battery.battery_temp >= stop_temperature_overheat) //CHARGE_STOP_TEMPERATURE_MAX )
                {
#ifdef FCHG_DBG
                	printk("[TA] Temperature is high (%d*)\n", sec_bci.battery.battery_temp);
#endif
                    if ( sec_bci.battery.battery_health != POWER_SUPPLY_HEALTH_OVERHEAT )
                    {
                        sec_bci.battery.battery_health = POWER_SUPPLY_HEALTH_OVERHEAT;

                        _battery_state_change_( STATUS_CATEGORY_TEMP, 
                                    BATTERY_TEMPERATURE_HIGH, 
                                    is_sleep );
                    }
                }
                else if ( sec_bci.battery.battery_temp <= stop_temperature_cold ) //CHARGE_STOP_TEMPERATURE_MIN )
                {
#ifdef FCHG_DBG
					printk("[TA] Temperature is low (%d*)\n", sec_bci.battery.battery_temp);
#endif
                    if ( sec_bci.battery.battery_health != POWER_SUPPLY_HEALTH_COLD )
                    {
                        sec_bci.battery.battery_health = POWER_SUPPLY_HEALTH_COLD;

                        _battery_state_change_( STATUS_CATEGORY_TEMP, 
                                    BATTERY_TEMPERATURE_LOW, 
                                    is_sleep );
                    }
                }
                else
                {
                    if ( sec_bci.battery.battery_health != POWER_SUPPLY_HEALTH_GOOD )
                    {
                        sec_bci.battery.battery_health = POWER_SUPPLY_HEALTH_GOOD;
                        _battery_state_change_( STATUS_CATEGORY_TEMP, 
                                    BATTERY_TEMPERATURE_NORMAL, 
                                    is_sleep );
                    }
                }
            }
        }
    }

    /*Monitoring the battery level for Re-charging*/
    if ( sec_bci.battery.monitor_field_rechg_vol && (sec_bci.charger.rechg_count <= 0 || is_sleep))
    {

        if ( sec_bci.battery.monitor_duration > MONITOR_RECHG_VOL_DURATION )
            sec_bci.battery.monitor_duration = MONITOR_RECHG_VOL_DURATION;

/*
		if(sec_bootmode == 5) // offmode charging
			rechg_voltage = CHARGE_RECHG_VOLTAGE_OFFMODE;
		else
			rechg_voltage = CHARGE_RECHG_VOLTAGE;
//*/

	//rechg_voltage = BATT_VOL_95;	//jineokpark
	//rechg_voltage = BATT_VOL_100;
	//rechg_voltage = 4117;
#ifdef FCHG_DBG
	printk( "[BM] avg:%d vol:%d\n",
					avr_vol,
					sec_bci.battery.battery_level_vol);
#endif
//        if (sec_bci.battery.battery_level_vol <= rechg_voltage )
//	if (avr_vol <= rechg_voltage)
		if (avr_vol < CHARGE_RECHG_VOLTAGE)
        {
            sec_bci.battery.confirm_recharge++;
            if ( sec_bci.battery.confirm_recharge >= 2 )
            {
#ifdef FCHG_DBG
					printk( "[BM] RE-charging !!!\n");
			        printk( "[BM] CORE monitor BATT.(%d%%, %dmV, %d, count=%d, charging=%d)\n", 
				    sec_bci.battery.battery_level_ptg,
				    sec_bci.battery.battery_level_vol,
				    sec_bci.battery.battery_temp,
				    boot_monitor_count,
				    sec_bci.charger.is_charging);
#endif
                sec_bci.battery.confirm_recharge = 0;   

                //if ( is_sleep )
	            //        sec_bci.charger.full_charge_dur_sleep = 0x4;
                //else
	                _battery_state_change_( STATUS_CATEGORY_CHARGING, 
	                            POWER_SUPPLY_STATUS_RECHARGING_FOR_FULL, 
	                            is_sleep );
		return -1;
            }
        }
        else
        {
            sec_bci.battery.confirm_recharge = 0;
        }
    }

    return 0;   
}

extern is_twl4030_madc_ready(void);

static void battery_monitor_work_handler( struct work_struct *work )
{
    int is_full = 0;
    int charge_current_flag;
    struct battery_device_info *di = container_of( work,
                            struct battery_device_info,
                            battery_monitor_work.work );

	if (0 == is_twl4030_madc_ready())
	{
		printk("%s : TWL4030 MADC is not yet ready! Need to wait..\n", __func__);
		queue_delayed_work( sec_bci.sec_battery_workq, &di->battery_monitor_work, HZ);
		return;
	}

#if 1
//	MD_AL25_ServiceStateMachine();
#endif

    #if 0
    printk( "[BM] battery monitor [Level:%d, ADC:%d, TEMP.:%d, cable: %d] \n",\
        get_battery_level_ptg(),\
        get_battery_level_adc(0),\
        get_system_temperature(),\
        sec_bci.charger.cable_status );
    #endif

    boot_monitor_count++;
    if(!boot_complete && boot_monitor_count >= 2)
    {
#ifdef FCHG_DBG
        printk("[BM] boot complete \n");
#endif
        boot_complete = true;
		sec_bci.battery.monitor_duration = MONITOR_DEFAULT_DURATION;
    }
/*
	if(sec_bci.charger.rechg_count > 0)
		sec_bci.charger.rechg_count--;
//*/

	if(sec_bci.charger.rechg_count < 5 && sec_bci.charger.rechg_count > 0)
		sec_bci.charger.rechg_count--;

//	printk("[BM] MMC2_DAT0 : %x\n", omap_readw(0x4800215c));

/*
    if ( device_config->MONITORING_SYSTEM_TEMP )
        sec_bci.battery.battery_temp = get_system_temperature( TEMP_DEG );
    else
        sec_bci.battery.battery_temp = 0;
*/

        sec_bci.battery.battery_temp = get_system_temperature( TEMP_DEG );
#if 0	//high temperature test, show this process value for spare parts --> battery information --> temperature parts
	static int a = 0, b = 0;
	if(!b){
		if(a < 3){
			sec_bci.battery.battery_temp = get_system_temperature( TEMP_DEG );
			a++;
			}
		else{
			sec_bci.battery.battery_temp = 47 + a;
			a++;
			if(a >= 23)	b = 1;			
			}
		}
	else{
		sec_bci.battery.battery_temp = 47 + a;
		a--;
		if(a < 3)	b = 0;
		}
#endif

#if 0
//low temperature test, show this process value for spare parts --> battery information --> temperature parts
	static int a = 0, b = 0;
	if(!b){
		if(a < 3){
			sec_bci.battery.battery_temp = get_system_temperature( TEMP_DEG );
			a++;
			}
		else{
			sec_bci.battery.battery_temp = 8 - a;
			a++;
			if(a >= 13)	b = 1;
			}
		}
	else{
		sec_bci.battery.battery_temp = 8 - a;
		a--;
		if(a < 3)	b = 0;
		}
#endif
	sec_bci.battery.battery_temp_adc = get_system_temperature( TEMP_ADC );

    #if 0
	update_rcomp_by_temperature(sec_bci.battery.battery_temp);
	#endif
	
    /* Monitoring the battery info. */
    sec_bci.battery.battery_level_vol= get_battery_level_adc(0);
    sec_bci.battery.battery_level_vol_adc = get_battery_level_adc(1);
	sec_bci.battery.battery_level_ptg = get_battery_level_ptg();
//    msleep(10);

	dbgPrintk("ptg: %d, adc:%d", 
		    sec_bci.battery.battery_level_ptg,
			sec_bci.battery.battery_level_vol_adc);

#ifdef FCHG_DBG

    printk( "[BM] monitor BATT.(%d%%, %dmV, %d*, count=%d, charging=%d)\n", 
		    sec_bci.battery.battery_level_ptg,
		    sec_bci.battery.battery_level_vol,
		    sec_bci.battery.battery_temp,
		    boot_monitor_count,
		    sec_bci.charger.is_charging
	  );
	printk( "[BM] avg_vol:%d avg_temp:%d\n", avr_vol, avr_temp);
#endif

	charge_current_flag = check_full_charge_current(CHARGE_DUR_ACTIVE);

    if( !( sec_bci.battery.monitor_field_temp ) && !( sec_bci.battery.monitor_field_rechg_vol ) )
    {
        sec_bci.battery.monitor_duration = MONITOR_DEFAULT_DURATION;
    }
    else
    {
        // Workaround : check status of cabel at this point.
        if ( !_cable_status_now_() )
        {
            _battery_state_change_( STATUS_CATEGORY_ETC, 
                        ETC_CABLE_IS_DISCONNECTED, 
                        CHARGE_DUR_ACTIVE );
        }

        if ( sec_bci.charger.is_charging && device_config->MONITORING_CHG_CURRENT )
        {
            // in charging && enable monitor_chg_current
//            charge_current_adc = get_charging_current_adc_val();
//            is_full = check_full_charge_using_chg_current( charge_current_adc );
		is_full = check_full_charge_using_chg_current2( charge_current_flag );

#ifdef FCHG_DBG
	printk("[BM] charge_current_flag = %d, is_full = %d\n",charge_current_flag, is_full);
	printk("[BM] sec_bci.charger.charge_status = %d, POWER_SUPPLY_STATUS_FULL = %d\n", sec_bci.charger.charge_status,POWER_SUPPLY_STATUS_FULL);
#endif

		if( is_full )
		{
			if(charge_current_flag == 1 && ( sec_bci.charger.charge_status != POWER_SUPPLY_STATUS_FULL )){
			_battery_state_change_( STATUS_CATEGORY_CHARGING, 
				POWER_SUPPLY_STATUS_FULL, 
				CHARGE_DUR_ACTIVE );
			}
			else if(charge_current_flag == 2){
//			else if(charge_current_flag == 2 && ( sec_bci.charger.charge_status != POWER_SUPPLY_STATUS_FULL )){
			_battery_state_change_( STATUS_CATEGORY_CHARGING, 
				POWER_SUPPLY_STATUS_FULL_CUT_OFF, 
				CHARGE_DUR_ACTIVE );
//				change_charge_status( POWER_SUPPLY_STATUS_FULL_CUT_OFF, CHARGE_DUR_ACTIVE );
			}
			else{	
				battery_monitor_core( CHARGE_DUR_ACTIVE );
				}
            	}
		else{
			battery_monitor_core(CHARGE_DUR_ACTIVE);
		}
        }
        else
        {
            battery_monitor_core( CHARGE_DUR_ACTIVE );
        }
    }

#if 0 
	printk( "[BM] monitor BATT.(%d%%, %dmV, %d*, count=%d, charging=%d)\n", 
			sec_bci.battery.battery_level_ptg,
			sec_bci.battery.battery_level_vol,
			sec_bci.battery.battery_temp,
			boot_monitor_count,
			sec_bci.charger.is_charging
	      );
#endif
	//printk("[BM] adc 167 -> %d^, adc 198 -> %d^\n", t2adc_to_temperature(927, 0), t2adc_to_temperature(884, 0));

    power_supply_changed( &di->sec_battery );
    power_supply_changed( &di->sec_ac );
    power_supply_changed( &di->sec_usb );

    queue_delayed_work( sec_bci.sec_battery_workq, &di->battery_monitor_work, sec_bci.battery.monitor_duration * (HZ/4));
}

static int battery_monitor_fleeting_wakeup_handler(unsigned long arg) 
{
	int ret = 0;
	int is_full = 0;
	int charge_current_flag= 0;


	get_system_status_in_sleep(&sec_bci.battery.battery_level_ptg,
					&sec_bci.battery.battery_level_vol,
					&sec_bci.battery.battery_temp,
					&charge_current_flag);

	is_full = check_full_charge_using_chg_current2(charge_current_flag);

#ifdef FCHG_DBG
	//printk("[BM] battery_monitor_fleeting_wakeup_handler running\n");
	printk("[BM] S@ charge_current_flag = %d, is_full = %d\n",
					charge_current_flag, is_full);
	printk("[BM] S@ status:%d rechg_vol:%d rechg_cnt:%d is_charging:%d\n",
					sec_bci.charger.charge_status,
					sec_bci.battery.monitor_field_rechg_vol,
					sec_bci.charger.rechg_count,
					sec_bci.charger.is_charging);
#endif

	if (sec_bci.charger.is_charging && device_config->MONITORING_CHG_CURRENT) {
		if (is_full) {
			if (charge_current_flag == 1 &&
				(sec_bci.charger.charge_status != POWER_SUPPLY_STATUS_FULL)) {
				//sec_bci.charger.full_charge_dur_sleep = 0x1;
				_battery_state_change_(STATUS_CATEGORY_CHARGING,
								POWER_SUPPLY_STATUS_FULL,
								CHARGE_DUR_SLEEP);
				return -1;
			} else if (charge_current_flag == 2) {
				//sec_bci.charger.full_charge_dur_sleep = 0x3;
				_battery_state_change_(STATUS_CATEGORY_CHARGING,
								POWER_SUPPLY_STATUS_FULL_CUT_OFF,
								CHARGE_DUR_SLEEP);
				return -1;
			}
		}
	}

	sec_bci.charger.full_charge_dur_sleep = 0x0;
	ret = battery_monitor_core(CHARGE_DUR_SLEEP);
    if (ret >= 0)
		request_gptimer12(&batt_gptimer_12);

	return ret;
}

// ------------------------------------------------------------------------- // 
//                            Power supply monitor                           //
// ------------------------------------------------------------------------- // 

static enum power_supply_property samsung_battery_props[] = {
    POWER_SUPPLY_PROP_STATUS,
    POWER_SUPPLY_PROP_HEALTH,
    POWER_SUPPLY_PROP_PRESENT,
    POWER_SUPPLY_PROP_ONLINE,
    POWER_SUPPLY_PROP_TECHNOLOGY,
    POWER_SUPPLY_PROP_VOLTAGE_NOW,
	#ifdef _OMS_FEATURES_
    POWER_SUPPLY_PROP_TEMP,
	#endif
    POWER_SUPPLY_PROP_CAPACITY, // in percents
};

static enum power_supply_property samsung_ac_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
    POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static enum power_supply_property samsung_usb_props[] = {
    POWER_SUPPLY_PROP_ONLINE,
    POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static int samsung_battery_get_property( struct power_supply *psy,
                    enum power_supply_property psp,
                    union power_supply_propval *val )
{
    switch ( psp ) 
    {
        case POWER_SUPPLY_PROP_STATUS:
#if 0
            if( sec_bci.charger.charge_status == POWER_SUPPLY_STATUS_RECHARGING_FOR_FULL 
                || sec_bci.charger.charge_status == POWER_SUPPLY_STATUS_RECHARGING_FOR_TEMP )
                val->intval = POWER_SUPPLY_STATUS_CHARGING;
            else
                val->intval = sec_bci.charger.charge_status;
#endif
			switch (sec_bci.charger.charge_status) {
			case POWER_SUPPLY_STATUS_CHARGING_OVERTIME:
			case POWER_SUPPLY_STATUS_FULL_DUR_SLEEP:
			case POWER_SUPPLY_STATUS_FULL_CUT_OFF:
            	val->intval = POWER_SUPPLY_STATUS_FULL;
				break;
			case POWER_SUPPLY_STATUS_RECHARGING_FOR_FULL:
			case POWER_SUPPLY_STATUS_RECHARGING_FOR_TEMP:
                val->intval = POWER_SUPPLY_STATUS_CHARGING;
				break;
			default:
                val->intval = sec_bci.charger.charge_status;
				break;
			}

            break;

        case POWER_SUPPLY_PROP_HEALTH:
            val->intval = sec_bci.battery.battery_health;
            break;

        case POWER_SUPPLY_PROP_PRESENT:
            val->intval = sec_bci.battery.battery_level_vol * 1000;
            val->intval = val->intval <= 0 ? 0 : 1;
            break;

        case POWER_SUPPLY_PROP_ONLINE :
            val->intval = sec_bci.charger.cable_status;
            break;

        case POWER_SUPPLY_PROP_TECHNOLOGY :
            val->intval = sec_bci.battery.battery_technology;
            break;

        case POWER_SUPPLY_PROP_VOLTAGE_NOW :
            val->intval = sec_bci.battery.battery_level_vol * 1000;
            break;
#ifdef _OMS_FEATURES_
        case POWER_SUPPLY_PROP_TEMP :
            val->intval = sec_bci.battery.battery_temp * 10;
            break;
#endif
        case POWER_SUPPLY_PROP_CAPACITY : /* in percents! */
            val->intval = sec_bci.battery.battery_level_ptg;
            break;


        default :
            return -EINVAL;
    }

    //printk("[BM] GET %d, %d  !!! \n", psp, val->intval );
    return 0;
}

static int samsung_ac_get_property( struct power_supply *psy,
                enum power_supply_property psp,
                union power_supply_propval *val )
{

    switch ( psp ) 
    {        
        case POWER_SUPPLY_PROP_ONLINE :
            if ( sec_bci.charger.cable_status == POWER_SUPPLY_TYPE_MAINS )
                val->intval = 1;
            else 
                val->intval = 0;
            break;
			
        case POWER_SUPPLY_PROP_VOLTAGE_NOW :
            val->intval = sec_bci.battery.battery_level_vol * 1000;
            break;

        default :
            return -EINVAL;
    }

    return 0;
}

static int samsung_usb_get_property( struct power_supply *psy,
                enum power_supply_property psp,
                union power_supply_propval *val )
{

    switch ( psp ) 
    {        
        case POWER_SUPPLY_PROP_ONLINE :
            if ( sec_bci.charger.cable_status == POWER_SUPPLY_TYPE_USB )
                val->intval = 1;
            else 
                val->intval = 0;

            break;
			
        case POWER_SUPPLY_PROP_VOLTAGE_NOW :
            val->intval = sec_bci.battery.battery_level_vol * 1000;
            break;

        default :
            return -EINVAL;
    }

    return 0;
}

static void samsung_pwr_external_power_changed( struct power_supply *psy ) 
{
    //cancel_delayed_work(&di->twl4030_bci_monitor_work);
    //schedule_delayed_work(&di->twl4030_bci_monitor_work, 0);
}

// ------------------------------------------------------------------------- // 
//                           Driver interface                                //
// ------------------------------------------------------------------------- // 
static int __devinit battery_probe( struct platform_device *pdev )
{
    int ret = 0;
    int i = 0;

    struct battery_device_info *di;
    
    printk( "[BM] Battery Probe... bootmode \n\n");

    this_dev = &pdev->dev; 

    di = kzalloc( sizeof(*di), GFP_KERNEL );
    if(!di)
        return -ENOMEM;

    platform_set_drvdata( pdev, di );
    
    di->dev = &pdev->dev;
    device_config = pdev->dev.platform_data;

    INIT_DELAYED_WORK( &di->battery_monitor_work, battery_monitor_work_handler );

    /*Create power supplies*/
    di->sec_battery.name = "battery";
    di->sec_battery.type = POWER_SUPPLY_TYPE_BATTERY;
    di->sec_battery.properties = samsung_battery_props;
    di->sec_battery.num_properties = ARRAY_SIZE( samsung_battery_props );
    di->sec_battery.get_property = samsung_battery_get_property;
    di->sec_battery.external_power_changed = samsung_pwr_external_power_changed;

    di->sec_ac.name = "ac";
    di->sec_ac.type = POWER_SUPPLY_TYPE_MAINS;
    di->sec_ac.supplied_to = samsung_bci_supplied_to;
    di->sec_ac.num_supplicants = ARRAY_SIZE( samsung_bci_supplied_to );
    di->sec_ac.properties = samsung_ac_props;
    di->sec_ac.num_properties = ARRAY_SIZE( samsung_ac_props );
    di->sec_ac.get_property = samsung_ac_get_property;
    di->sec_ac.external_power_changed = samsung_pwr_external_power_changed;

    di->sec_usb.name = "usb";
    di->sec_usb.type = POWER_SUPPLY_TYPE_USB;
    di->sec_usb.supplied_to = samsung_bci_supplied_to;
    di->sec_usb.num_supplicants = ARRAY_SIZE( samsung_bci_supplied_to );
    di->sec_usb.properties = samsung_usb_props;
    di->sec_usb.num_properties = ARRAY_SIZE( samsung_usb_props );
    di->sec_usb.get_property = samsung_usb_get_property;
    di->sec_usb.external_power_changed = samsung_pwr_external_power_changed;

    // USE_REGULATOR [+]
    di->usb3v1 = regulator_get( &pdev->dev, "usb3v1" );
    if( IS_ERR( di->usb3v1 ) )
        goto fail_regulator1;

    di->usb1v8 = regulator_get( &pdev->dev, "usb1v8" );
    if( IS_ERR( di->usb1v8 ) )
        goto fail_regulator2;

    di->usb1v5 = regulator_get( &pdev->dev, "usb1v5" );
    if( IS_ERR( di->usb1v5 ) )
        goto fail_regulator3;
    // USE_REGULATOR [-]

    ret = power_supply_register( &pdev->dev, &di->sec_battery );
    if( ret )
    {
        printk( "[BM] Failed to register main battery, charger\n" );
        goto batt_regi_fail1;
    }

    ret = power_supply_register( &pdev->dev, &di->sec_ac );
    if( ret )
    {
        printk( "[BM] Failed to register ac\n" );
        goto batt_regi_fail2;
    }

    ret = power_supply_register( &pdev->dev, &di->sec_usb );
    if( ret )
    {
        printk( "[BM] Failed to register usb\n" );
        goto batt_regi_fail3;
    }


#ifdef _OMS_FEATURES_
    // Create battery sysfs files for sharing battery information with platform.
    ret = sysfs_create_file( &di->sec_battery.dev->kobj, &batt_vol_toolow.attr );
    if ( ret )
    {
        printk( "[BM] sysfs create fail - %s\n", batt_vol_toolow.attr.name );
    }
#endif

    for( i = 0; i < ARRAY_SIZE( batt_sysfs_testmode ); i++ )
    {
        ret = sysfs_create_file( &di->sec_battery.dev->kobj, 
                     &batt_sysfs_testmode[i].attr );
        if ( ret )
        {
            printk( "[BM] sysfs create fail - %s\n", batt_sysfs_testmode[i].attr.name );
        }
    }
/*
    // Set GPIO to control thermal sensor power
    if (gpio_is_valid(OMAP_GPIO_EN_TEMP_VDD))
    {
        printk(KERN_ERR "[BM] OMAP_GPIO_EN_TEMP_VDD is valid\n");

        ret = gpio_request(OMAP_GPIO_EN_TEMP_VDD, NULL);
        if (ret < 0)
            printk(KERN_ERR "[BM] Failed to request OMAP_GPIO_EN_TEMP_VDD\n");

        gpio_direction_output(OMAP_GPIO_EN_TEMP_VDD, 0);
   }
*/
#if ( CONFIG_SAMSUNG_REL_HW_REV >= 4 )
/* AALTO(taejin.hyeon) 2011.03.18 Aalto vmmc2 is used by TSP, not usb. */
//	twl_i2c_write_u8( TWL4030_MODULE_PM_RECEIVER, 0x0B, TWL4030_VMMC2_DEDICATED );
/* AALTO(taejin.hyeon) 2011.03.18. end */
#endif

    // Init. ADC
    turn_resources_on_for_adc();
    twl_i2c_write_u8( TWL4030_MODULE_USB, SEL_MADC_MCPC, CARKIT_ANA_CTRL );
    turn_resources_off_for_adc();
	
    // Set gptimer12 for checking battery status in sleep mode.
    batt_gptimer_12.name = "samsung_battery_timer";
    batt_gptimer_12.expire_time =(unsigned int) MONITOR_DURATION_DUR_SLEEP;
    batt_gptimer_12.expire_callback = &battery_monitor_fleeting_wakeup_handler;
    batt_gptimer_12.data = (unsigned long) di;

#ifdef CONFIG_SEC_BATTERY_USE_RECOVERY_MODE

	//printk("[BM] !!!!!!!!!!!!!!!!!!\n");
    if (likely(recovery_mode == 0))
    	{
    	printk("[BM] recovery_mode == 0\n");
        //queue_delayed_work( sec_bci.sec_battery_workq, &di->battery_monitor_work, HZ*2 );
        queue_delayed_work( sec_bci.sec_battery_workq, &di->battery_monitor_work, HZ );
    	}
    else
    	{
    	printk("[BM] recovery_mode != 0\n");
        queue_delayed_work( sec_bci.sec_battery_workq, &di->battery_monitor_work, 0 );
    	}
	//printk("[BM] !!!!!!!!!!!!!!!!!!\n");
#else
    queue_delayed_work( sec_bci.sec_battery_workq, &di->battery_monitor_work, HZ/2 );
#endif
/*
	if (sec_bootmode == 5) {// offmode charging
		stop_temperature_overheat    = CHARGE_OFFMODE_STOP_TEMPERATURE_MAX;
		recover_temperature_overheat = CHARGE_OFFMODE_RECOVER_TEMPERATURE_MAX;
		stop_temperature_cold        = CHARGE_OFFMODE_STOP_TEMPERATURE_MIN;
		recover_temperature_cold     = CHARGE_OFFMODE_RECOVER_TEMPERATURE_MIN;
	}
*/
    sec_bci.ready = true;

    return 0;

batt_regi_fail3:
    power_supply_unregister( &di->sec_ac );

batt_regi_fail2:
    power_supply_unregister( &di->sec_battery );

batt_regi_fail1:
// USE_REGULATOR [+]
    regulator_put( di->usb1v5 );
    di->usb1v5 = NULL;

fail_regulator3:
    regulator_put( di->usb1v8 );
    di->usb1v8 = NULL;

fail_regulator2:
    regulator_put( di->usb3v1 );
    di->usb3v1 = NULL;

fail_regulator1:
// USE_REGULATOR [-]
    kfree(di);

    return ret;
}

static int __devexit battery_remove( struct platform_device *pdev )
{
    struct battery_device_info *di = platform_get_drvdata( pdev );

    flush_scheduled_work();
    cancel_delayed_work( &di->battery_monitor_work );

    power_supply_unregister( &di->sec_ac );
    power_supply_unregister( &di->sec_battery );

    // USE_REGULATOR [+]
    regulator_put( di->usb1v5 );
    regulator_put( di->usb1v8 );
    regulator_put( di->usb3v1 );
    // USE_REGULATOR [-]

    platform_set_drvdata( pdev, NULL );
    kfree( di );

    return 0;
}

static int battery_suspend( struct platform_device *pdev,
                            pm_message_t state )
{
    struct battery_device_info *di = platform_get_drvdata( pdev );

#ifdef FCHG_DBG
	printk("%s dur_sleep:%d, avr_vol:%d, level_vol:%d\n",
					__func__,
					sec_bci.charger.full_charge_dur_sleep,
					avr_vol,
					sec_bci.battery.battery_level_vol);
#endif

    cancel_delayed_work( &di->battery_monitor_work );

	sec_bci.charger.rechg_count = 0;
    if( sec_bci.charger.cable_status == POWER_SUPPLY_TYPE_MAINS )
    {
#if 0
        struct file *filp;
        char buf;
        int count;
        int retval = 0;
        mm_segment_t fs;

        fs = get_fs();
        set_fs(KERNEL_DS);
        filp = filp_open("/sys/power/vdd1_lock", 
                00000001/*O_WRONLY*/|00010000/*O_SYNC*/, 
                0x0);
        buf='1';
        count=filp->f_op->write(filp, &buf, 1, &filp->f_pos);
        retval = filp_close(filp, NULL);
        set_fs(fs);
#endif

        request_gptimer12( &batt_gptimer_12 );
    }

    return 0;
}

static int battery_resume( struct platform_device *pdev )
{
    struct battery_device_info *di = platform_get_drvdata( pdev );

#ifdef FCHG_DBG
	printk("%s dur_sleep:%d, avr_vol:%d, level_vol:%d\n",
					__func__,
					sec_bci.charger.full_charge_dur_sleep,
					avr_vol,
					sec_bci.battery.battery_level_vol);
#endif

    if ( batt_gptimer_12.active )
    {
#if 0
        struct file *filp;
        char buf;
        int count;
        int retval = 0;
        mm_segment_t fs;

        fs = get_fs();
        set_fs(KERNEL_DS);
        filp = filp_open("/sys/power/vdd1_lock", 
                00000001/*O_WRONLY*/|00010000/*O_SYNC*/, 
                0x0);
        buf='0';
        count=filp->f_op->write(filp, &buf, 1, &filp->f_pos);
        retval = filp_close(filp, NULL);
        set_fs(fs);
#endif
        release_gptimer12( &batt_gptimer_12 );
    }

    switch ( sec_bci.charger.full_charge_dur_sleep )
    {
        case 0x1 : 
            _battery_state_change_( STATUS_CATEGORY_CHARGING, 
                        POWER_SUPPLY_STATUS_FULL_DUR_SLEEP, 
                        CHARGE_DUR_ACTIVE );
            break;

        case 0x2 : 
            _battery_state_change_( STATUS_CATEGORY_CHARGING, 
                        POWER_SUPPLY_STATUS_CHARGING_OVERTIME, 
                        CHARGE_DUR_ACTIVE );
            break;

        case 0x3 : 
            _battery_state_change_( STATUS_CATEGORY_CHARGING, 
                        POWER_SUPPLY_STATUS_FULL_CUT_OFF, 
                        CHARGE_DUR_ACTIVE );
            break;
	case 0x4:
            _battery_state_change_( STATUS_CATEGORY_CHARGING, 
                        POWER_SUPPLY_STATUS_RECHARGING_FOR_FULL,
                        CHARGE_DUR_ACTIVE );
		break;

        default : 
            break;
    }

	wake_lock_timeout( &sec_bc_wakelock , HZ * 3 );

    power_supply_changed( &di->sec_battery );
    power_supply_changed( &di->sec_ac );
    power_supply_changed( &di->sec_usb );

    sec_bci.charger.full_charge_dur_sleep = 0x0;
    
    queue_delayed_work( sec_bci.sec_battery_workq, &di->battery_monitor_work, HZ * 2 );

    return 0;
}

struct platform_driver battery_platform_driver = {
    .probe      = battery_probe,
    .remove     = __devexit_p( battery_remove ),
    .suspend    = &battery_suspend,
    .resume     = &battery_resume,
    .driver     = {
        .name = DRIVER_NAME,
    },
};


static int __init battery_init( void )
{
    int ret;

    printk( "\n[BM] Battery Init.\n" );

    sec_bci.ready = false;

    sec_bci.battery.battery_health = POWER_SUPPLY_HEALTH_GOOD;
    sec_bci.battery.battery_technology = POWER_SUPPLY_TECHNOLOGY_LION;
    sec_bci.battery.battery_level_ptg = 0;
    sec_bci.battery.battery_level_vol = 0;
    //sec_bci.battery.monitor_duration = MONITOR_DEFAULT_DURATION;
    sec_bci.battery.monitor_duration = 4;
    sec_bci.battery.monitor_field_temp = false;
    sec_bci.battery.monitor_field_rechg_vol = false;
    sec_bci.battery.confirm_full_by_current = 0;
    sec_bci.battery.support_monitor_temp = 1;
    sec_bci.battery.support_monitor_timeout = 1;
    sec_bci.battery.support_monitor_full = 1;
    sec_bci.battery.confirm_recharge = 0;
	sec_bci.battery.battery_cal = 0;

    sec_bci.charger.prev_cable_status = -1;
    sec_bci.charger.cable_status = -1;
    sec_bci.charger.prev_charge_status = 0;
    sec_bci.charger.charge_status = 0;
    sec_bci.charger.full_charge_dur_sleep = 0x0;
    sec_bci.charger.is_charging = false;
    sec_bci.charger.charge_start_time = 0;
    sec_bci.charger.charged_time = 0;
    sec_bci.charger.charging_timeout = DEFAULT_CHARGING_TIMEOUT;
    sec_bci.charger.use_ta_nconnected_irq = false;
	sec_bci.charger.rechg_count = 0;
    sec_bci.sec_battery_workq = create_singlethread_workqueue("sec_battery_workq");


    init_gptimer12();
    printk( "[BM] Init Gptimer called \n" );
		
    /* Get the charger driver */
    if( ( ret = charger_init() < 0 ) )
    {
        printk( "[BM] Fail to get charger driver.\n" );
        return ret;
    }

    /* Get the fuelgauge driver */
    if( ( ret = fuelgauge_init() < 0 ) )
    {
        printk( "[BM] Fail to get fuelgauge driver.\n" );        
        return ret;
    }

    wake_lock_init( &sec_bc_wakelock, WAKE_LOCK_SUSPEND, "samsung-battery" );
	
    ret = platform_driver_register( &battery_platform_driver );

    return ret;
}
module_init( battery_init );

static void __exit battery_exit( void )
{
    /*Remove the charger driver*/
    charger_exit();
    /*Remove the fuelgauge driver*/
    fuelgauge_exit();

    finish_gptimer12();
    
    platform_driver_unregister( &battery_platform_driver );
    printk( KERN_ALERT "[BM] Battery Driver Exit.\n" );
}

module_exit( battery_exit );

MODULE_AUTHOR( "EUNGON KIM <egstyle.kim@samsung.com>" );
MODULE_DESCRIPTION( "Samsung Battery monitor driver" );
MODULE_LICENSE( "GPL" );
