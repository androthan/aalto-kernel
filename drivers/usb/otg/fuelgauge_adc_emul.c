/*
 * fuelgauge_adc_emul.c
 *
 * SW Fuel Gauge Emulator using VBAT adc
 *
 * Copyright (C) 2011 SAMSUNG ELECTRONICS.
 * Author: Jaemin Yoo
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include "common.h"
#include <linux/power_supply.h>

/* 
   NOTE1 : ADC values are based on OMAP3630 in Aalto project 
   NOTE2 : voltage is no need to be precise. BUT!!!BUT!!!BUT!!!!
           Recharging, Power Off, Battery level rely on voltage
		   value. Keep this in mind!  
   NOTE3 : voltage margin is +-0.04v except recharging voltage for MP3 or MID products
   NOTE4 : Can't we just use HW fuel gauge? -.-
*/

/* Use this if you don't need high(?) precision battery status bar */
#define BATT_LVL6		678    ///* 4.13v ~80% 
#define BATT_LVL5		658    ///* 80% ~ 65% 
#define BATT_LVL4		640    ///* 65% ~ 50% 
#define BATT_LVL3		620    ///* 50% ~ 35% 
#define BATT_LVL2		602    ///* 35% ~ 20% 
#define BATT_LVL1		583    ///* 3.60v   ~5% 
#define BATT_LVL_OFF    575    ///* 3.40v (fixed) 

#define BATT_RECHARGING	700		/* 4.13v (fixed) +-0.02v margin */

#define BATT_ADC_CHK_INTERVAL	200	/* milli-seconds */
#define BATT_ADC_AVG_WINDOW		30	/* # of sampling numbers */
#define BATT_ADC_AVG_WINDOW_MIN		10	/* # of minimum sampling numbers */

#define PMIC_VBAT_ADC	12	/* vbat is ADC12 for TWL */

#define DRIVER_NAME	"secAdcFg"



///*
static BATTERY_INFO battery_table[] = 
{
    /* %,    V,  adc*/
{	BATT_PTG_100	,	BATT_VOL_100	,	BATT_ADC_100	},
{	BATT_PTG_95	,	BATT_VOL_95	,	BATT_ADC_95	},
{	BATT_PTG_90	,	BATT_VOL_90	,	BATT_ADC_90	},
{	BATT_PTG_85	,	BATT_VOL_85	,	BATT_ADC_85	},
{	BATT_PTG_80	,	BATT_VOL_80	,	BATT_ADC_80	},
{	BATT_PTG_75	,	BATT_VOL_75	,	BATT_ADC_75	},
{	BATT_PTG_70	,	BATT_VOL_70	,	BATT_ADC_70	},
{	BATT_PTG_65	,	BATT_VOL_65	,	BATT_ADC_65	},
{	BATT_PTG_60	,	BATT_VOL_60	,	BATT_ADC_60	},
{	BATT_PTG_55	,	BATT_VOL_55	,	BATT_ADC_55	},
{	BATT_PTG_50	,	BATT_VOL_50	,	BATT_ADC_50	},
{	BATT_PTG_45	,	BATT_VOL_45	,	BATT_ADC_45	},
{	BATT_PTG_40	,	BATT_VOL_40	,	BATT_ADC_40	},
{	BATT_PTG_35	,	BATT_VOL_35	,	BATT_ADC_35	},
{	BATT_PTG_30	,	BATT_VOL_30	,	BATT_ADC_30	},
{	BATT_PTG_25	,	BATT_VOL_25	,	BATT_ADC_25	},
{	BATT_PTG_20	,	BATT_VOL_20	,	BATT_ADC_20	},
{	BATT_PTG_15	,	BATT_VOL_15	,	BATT_ADC_15	},
{	BATT_PTG_10	,	BATT_VOL_10	,	BATT_ADC_10	},
{	BATT_PTG_05	,	BATT_VOL_05	,	BATT_ADC_05	},
{	BATT_PTG_04	,	BATT_VOL_04	,	BATT_ADC_04	},
{	BATT_PTG_03	,	BATT_VOL_03	,	BATT_ADC_03	},
{	BATT_PTG_02	,	BATT_VOL_02	,	BATT_ADC_02	},
{	BATT_PTG_01	,	BATT_VOL_01	,	BATT_ADC_01	},
{	BATT_PTG_00	,	BATT_VOL_00	,	BATT_ADC_00	},
};
//*/
	
static BATTERY_INFO batt_avg_info = {0,0,0};

static struct workqueue_struct* adcfg_workq;
static struct delayed_work adcfg_work;

static int adc_compensate = 0;

static SEC_battery_charger_info *sec_bci;

extern SEC_battery_charger_info *get_sec_bci( void );
extern int get_vbat_adc(bool is_sleep);		/* pmic vbat adc */
extern int _low_battery_alarm_();	/* low battery alarm to framework */


//+ phill-it: 20110908
#ifdef __BATTERY_COMPENSATION__
void adcfg_set_compensate(int mode,int offset,int compensate_value)
{
	if (mode) {
		if (!(sec_bci->battery.device_state & offset)) {
			sec_bci->battery.device_state |= offset;
			adc_compensate += compensate_value;
		}
	} else {
		if (sec_bci->battery.device_state & offset) {
			sec_bci->battery.device_state &= ~offset;
			adc_compensate -= compensate_value;
		}
	}
	dbgPrintk("[BAT]:%s: mode :%d, device_state=0x%x, compensation=%d", __func__, mode,sec_bci->battery.device_state, adc_compensate);
}
#endif

int adcfg_get_compensate(void)
{
	return adc_compensate;
}

static int calc_avg_adc(int adc, bool is_sleep)
{
	static int adc_window[BATT_ADC_AVG_WINDOW] = {0,};
	static int total_sum_adc = 0;
	static int size = 0;
	static int index = 0;
	static int old_avg = 0;
	int total_avg = 0;
	int i;
#if 1
	if(total_sum_adc){
			if(adc > ((total_sum_adc / size) + 10)){
					//printk("[BM] battery_level_vol :%d\n",sec_bci->battery.battery_level_vol);
					if (sec_bci->battery.battery_level_vol < BATT_VOL_90) {
							if (sec_bci->charger.cable_status == POWER_SUPPLY_TYPE_USB) {
									//printk("[BM] usb charger calibration!!\n");
									adc-= 13;
							}
							else{
									//printk("[BM] ta charger calibration!!\n");
									adc-= 15;
							}
					}
			}
	}
#endif
	if (is_sleep == true)
	{
		total_sum_adc = adc;
		size = 1;
		index = 1;
		return adc;
	}

	if (adc <= 0) 
	{
		printk("%s : invalid ADC(%d)\n", __func__, adc);
	}
	else if (size >= BATT_ADC_AVG_WINDOW)
	{
		if (++index >= BATT_ADC_AVG_WINDOW)
		{
			index = 0;
		}
		total_sum_adc -= adc_window[index];
		total_sum_adc += adc;
		adc_window[index] = adc;
	}
	else
	{
		adc_window[index] = adc;
		size++;
		index = size;
		total_sum_adc += adc;
	}

	total_avg = total_sum_adc / size;

	if(sec_bci->charger.is_charging){
			if(total_avg < old_avg)
					total_avg = old_avg;
	}
	old_avg = total_avg;
//	printk("[fuelgague]  adc value :%d\n",total_avg);
	return total_avg;
	//return total_sum_adc / size;
}

static int get_batt_adc(bool is_sleep)
{
	int adc = 0;
	int min = -1;
	int max = -1;
	int sum = 0;
	int i = 0;

	int sampling_size = 5;

	adc = get_vbat_adc(is_sleep);
	min = max = adc;

	sum += adc;

	for (i = 1; i < sampling_size; i++)
	{
		adc = get_vbat_adc(is_sleep);
		if (adc <= min)
		{
			min = adc;
		}
		else if (adc >= max)
		{
			max = adc;
		}
		sum += adc;
	}

	sum -= (min+max);

	return sum / (sampling_size-2);
}

#if 1
static void get_batt_info(int count, bool is_sleep)
{
	int array_size = ARRAY_SIZE(battery_table);
	int adc;
	int avg_adc;
	int i;
	static int prev_idx = -1;
	static unsigned char idx_change_cnt = 0;
	static bool booting = false;

	adc = get_batt_adc(is_sleep);
//+ phill-it: 20110908
#ifdef __BATTERY_COMPENSATION__
	if(!sec_bci->charger.is_charging){
		//dbgPrintk("[fuelgauge]: avg_adc= %d \n", avg_adc);
		//dbgPrintk("[fuelgauge]: %s, device_state=0x%x, compensation=%d\n", __func__, sec_bci->battery.device_state, adc_compensate);
		adc += adc_compensate;
		//dbgPrintk("[fuelgauge]: avg_adc + adc_compensate= %d \n", avg_adc);
	}
#endif
	avg_adc = calc_avg_adc(adc, is_sleep);
	dbgPrintk("avg_adc:%d \n", avg_adc);

	for (i = 0; i < array_size; i++)
	{
		if (avg_adc >= battery_table[i].adc) 
		{
			if (prev_idx < 0)
				prev_idx = i;

				if(!is_sleep){
					idx_change_cnt++;
					if (booting) {
						if (idx_change_cnt > 25) {
							idx_change_cnt = 0;
							//printk("[%s] change idx:%d prev_idx:%d\n", __func__, i, prev_idx);
							if (prev_idx != i) {
								if (sec_bci->charger.is_charging)
									prev_idx > i ? prev_idx-- : prev_idx;
								else
									prev_idx > i ? prev_idx : prev_idx++;
							}
						}
					} else {
						if (idx_change_cnt == 0) {
							booting = true;
						} else {
							if (prev_idx != i)
								prev_idx > i ? prev_idx-- : prev_idx++;
						}
					}
				}
				else{
					prev_idx = i;
					idx_change_cnt = 0;
					//printk("[%s] sleep change idx:%d prev_idx:%d\n", __func__, i, prev_idx);
				}

//		printk("[%s] idx:%d prev_idx:%d\n", __func__, i, prev_idx);
		batt_avg_info = battery_table[prev_idx];
		return;
		}
	}
	prev_idx = -1;
	batt_avg_info = battery_table[array_size-1];
}
#endif
#if 0
static void get_batt_info(int count, bool is_sleep)
{
	int array_size = ARRAY_SIZE(battery_table);
	int adc;
	int avg_adc;
	int i;
	static int prev_idx = -1;
	static unsigned char idx_change_cnt = 0;
	static bool booting = false

	adc = get_batt_adc(is_sleep);
//+ phill-it: 20110908
#ifdef __BATTERY_COMPENSATION__
	if(!sec_bci->charger.is_charging){
		//dbgPrintk("[fuelgauge]: avg_adc= %d \n", avg_adc);
		//dbgPrintk("[fuelgauge]: %s, device_state=0x%x, compensation=%d\n", __func__, sec_bci->battery.device_state, adc_compensate);
		adc += adc_compensate;
		//dbgPrintk("[fuelgauge]: avg_adc + adc_compensate= %d \n", avg_adc);
	}
#endif
	avg_adc = calc_avg_adc(adc, is_sleep);
	dbgPrintk("avg_adc:%d \n", avg_adc);

	for (i = 0; i < array_size; i++)
	{
		if (avg_adc >= battery_table[i].adc) 
		{
			if (prev_idx < 0)
				prev_idx = i;

				if(!is_sleep){
					if (++idx_change_cnt > 25) {
						idx_change_cnt = 0;
						//printk("[%s] change idx:%d prev_idx:%d\n", __func__, i, prev_idx);
						if (prev_idx != i) {
							if (sec_bci->charger.is_charging)
								prev_idx > i ? prev_idx-- : prev_idx;
							else
								prev_idx > i ? prev_idx : prev_idx++;
						}
					}
				}
				else{
					prev_idx = i;
					idx_change_cnt = 0;
					//printk("[%s] sleep change idx:%d prev_idx:%d\n", __func__, i, prev_idx);
				}

//		printk("[%s] idx:%d prev_idx:%d\n", __func__, i, prev_idx);
		batt_avg_info = battery_table[prev_idx];
		return;
		}
	}
	prev_idx = -1;
	batt_avg_info = battery_table[array_size-1];
}
#endif

/* 
   Get battery voltage in mV (0~4200mv) 
   count (sampling number), is_sleep=1 (charging during sleep) 
*/
int get_fuelgauge_adc_value( int count, bool is_sleep, bool flag )
{
	if (is_sleep == true) 
	{
		get_batt_info(5, true);
	}

	if(flag)	return batt_avg_info.adc;
	else		return batt_avg_info.voltage;
}

/* Get battery capacity (0~100%) */
int get_fuelgauge_ptg_value( bool is_sleep )
{
	if (is_sleep == true) 
	{
		get_batt_info(5, true);
	}

	return batt_avg_info.capacity;
}

static int adcfg_lowbat_detect(void)
{
	static int first_warn_sent = false;
	static int second_warn_sent = false;
	static int poweroff_sent = false;

	if (sec_bci->charger.is_charging || !sec_bci->ready
					|| batt_avg_info.capacity > BATT_PTG_WARN_1ST)
	{
		first_warn_sent = false;
		second_warn_sent = false;
		poweroff_sent = false;

		return false;
	}

	if (batt_avg_info.capacity <= BATT_PTG_WARN_1ST
					&& batt_avg_info.capacity > BATT_PTG_WARN_2ND)
	{
		if (first_warn_sent == false) 
		{
			dbgPrintk("1st: ptg:%d, adc", batt_avg_info.capacity, batt_avg_info.adc);
			_low_battery_alarm_();
		}
		first_warn_sent = true;
		second_warn_sent = false;
		poweroff_sent = false;
	}
	else if (batt_avg_info.capacity <= BATT_PTG_WARN_2ND
					&& batt_avg_info.capacity > BATT_PTG_WARN_OFF)
	{
		if (second_warn_sent == false) 
		{
			dbgPrintk("2nd: ptg:%d, adc", batt_avg_info.capacity, batt_avg_info.adc);
			_low_battery_alarm_();
		}
		first_warn_sent = false;
		second_warn_sent = true;
		poweroff_sent = false;
	}
	else if (batt_avg_info.capacity <= BATT_PTG_WARN_OFF)
	{
		if (poweroff_sent == false) 
		{
			dbgPrintk("off: ptg:%d, adc", batt_avg_info.capacity, batt_avg_info.adc);
			_low_battery_alarm_();
		}
		first_warn_sent = false;
		second_warn_sent = false;
		poweroff_sent = true;
	}
	else
	{
		printk("%s : can't reach here!\n", __func__);
	}

	return true;
}

extern is_twl4030_madc_ready(void);

static void adcfg_work_handler(struct work_struct *work)
{
	static int twl4030_adc_found = false;

	/* twl4030_madc is a drvier for ADC in TWL4030 PMIC */
	/* It is loaded after adc fuelgauge driver. So we have to wait */
	if (true == twl4030_adc_found || 1 == is_twl4030_madc_ready())
	{
		/* wait for 20ms after twl4030 madc becomes ready. */
		if (twl4030_adc_found == false)
		{
			mdelay(20);
		}
		twl4030_adc_found = true;

		get_batt_info(5, false);
		adcfg_lowbat_detect();

		//printk("[ADCFG] %s : VOL(%d), CAPA(%d), ADC(%d)\n", __func__, batt_avg_info.voltage, batt_avg_info.capacity, batt_avg_info.adc);
	}

	queue_delayed_work(adcfg_workq, &adcfg_work, msecs_to_jiffies(BATT_ADC_CHK_INTERVAL));
}


/* Below are null functions. just for interfacing */

int fuelgauge_init( void )
{
	return 0;	
}

void fuelgauge_exit( void )
{
}

int fuelgauge_quickstart( void )
{
	return 0;
}

static int __devinit adcfg_probe(struct platform_device* pdev)
{
	printk("[ADCFG] %s\n", __func__);

	sec_bci = get_sec_bci();

	adcfg_workq = create_workqueue("ADCFGWORKQ");

	INIT_DELAYED_WORK(&adcfg_work, adcfg_work_handler);

	queue_delayed_work(adcfg_workq, &adcfg_work, 0);

	return 0;
}

static int __devexit adcfg_remove(struct platform_device* pdev)
{
#ifdef FCHG_DBG
	printk("[ADCFG] %s\n", __func__);
#endif

	return 0;
}

static int adcfg_suspend(struct platform_device* pdev, pm_message_t state)
{
#ifdef FCHG_DBG
	printk("[ADCFG] %s\n", __func__);
#endif

	cancel_delayed_work_sync(&adcfg_work);

	return 0;
}

static int adcfg_resume(struct platform_device* pdev)
{
#ifdef FCHG_DBG
	printk("[ADCFG] %s\n", __func__);
#endif

	get_batt_info(5, false);	/* no time to get avg. adc value */
	adcfg_lowbat_detect();
	queue_delayed_work(adcfg_workq, &adcfg_work, msecs_to_jiffies(BATT_ADC_CHK_INTERVAL));

	return 0;
}

struct platform_driver adcfg_platform_driver = {
    .probe      = &adcfg_probe,
    .remove     = __devexit_p(adcfg_remove),
    .suspend    = &adcfg_suspend,
    .resume     = &adcfg_resume,
    .driver     = {
        .name = DRIVER_NAME,
    },
};

static int __init adcfg_init(void)
{
	int ret = 0;

	printk("[ADCFG] %s\n", __func__);

	ret = platform_driver_register(&adcfg_platform_driver);

	return ret;
}

static void __exit adcfg_exit(void)
{
	platform_driver_unregister(&adcfg_platform_driver);
	printk("[ADCFG] %s\n", __func__);
}

module_init(adcfg_init)
module_exit(adcfg_exit)
