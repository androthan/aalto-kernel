#ifndef _COMMON_H_
#define _COMMON_H_

//#define WR_ADC 

#ifdef WR_ADC
/* Workaround to get proper adc value */
#include <linux/i2c/twl.h>
#endif

#define	FULL_TEST	1
#define	FCHG_DBG
#undef	FCHG_DBG

#define	TEMP_TEST	0
#define	VOL_TEST	0

//+ phill-it: bhji
#define APPLY_AUTOSTOP_MODE
#define APPLY_WORKAROUND_LOWBATTERY
//#define __BATTERY_COMPENSATION__ : does not work on newer Android versions -- adrt.15

//#define BATTERY_DEBUG
#ifdef BATTERY_DEBUG
#define dbgPrintk(fmt, ...)\
	printk("[%s]" fmt "\n", __func__, ## __VA_ARGS__);
#else
#define dbgPrintk(fmt, ...)
#endif
//-

//-


// Unit is sec.
#define MONITOR_DURATION_DUR_SLEEP  	30
#define MONITOR_DEFAULT_DURATION    	30
#define MONITOR_TEMP_DURATION       	30
#define MONITOR_RECHG_VOL_DURATION  	30


#if 0
#define DEFAULT_CHARGING_TIMEOUT        6 * 60 * 60
#define DEFAULT_RECHARGING_TIMEOUT        (1 * 60 * 60 + 30 * 60)

#define CHARGE_STOP_TEMPERATURE_MAX     	70		
#define CHARGE_RECOVER_TEMPERATURE_MAX      60		

#define CHARGE_STOP_TEMPERATURE_MIN     	3
#define CHARGE_RECOVER_TEMPERATURE_MIN      6
#define CHARGE_RECHG_VOLTAGE            	4130
#define CHARGE_RECHG_VOLTAGE_OFFMODE       	4130

#define CHARGE_STOP_TEMPERATURE_EVENT		70
#define CHARGE_RECOVER_TEMPERATURE_EVENT    60
#else
#define DEFAULT_CHARGING_TIMEOUT        6 * 60 * 60
#define DEFAULT_RECHARGING_TIMEOUT        1500 // faster charging -- adrt.15

#define CHARGE_STOP_TEMPERATURE_MAX     	65
#define CHARGE_RECOVER_TEMPERATURE_MAX      45 // 55
#define CHARGE_STOP_TEMPERATURE_MIN     	-5 //0
#define CHARGE_RECOVER_TEMPERATURE_MIN      5 //3

#define CHARGE_OFFMODE_STOP_TEMPERATURE_MAX     	55
#define CHARGE_OFFMODE_RECOVER_TEMPERATURE_MAX      35
#define CHARGE_OFFMODE_STOP_TEMPERATURE_MIN     	-10 //-5 //0
#define CHARGE_OFFMODE_RECOVER_TEMPERATURE_MIN      -5 //0 //3

#define CHARGE_RECHG_VOLTAGE            	4130
#define CHARGE_RECHG_VOLTAGE_OFFMODE       	4130

//#define CHARGE_STOP_TEMPERATURE_EVENT		65 //67		// 63
//#define CHARGE_RECOVER_TEMPERATURE_EVENT    43 //60		// 58
#endif
#define CHARGE_FULL_CURRENT_ADC 250

#define CHARGE_DUR_ACTIVE 0
#define CHARGE_DUR_SLEEP  1

#define STATUS_CATEGORY_CABLE       1
#define STATUS_CATEGORY_CHARGING    2
#define STATUS_CATEGORY_TEMP        3
#define STATUS_CATEGORY_ETC         4

//+ phill-it: 20110908
// project-lowpower-aalto: may causes issues                     DEBUG @adrt.15
#ifdef __BATTERY_COMPENSATION__
#define COMPENSATE_DEFAULT          3  //3
#define COMPENSATE_MP3              3  //3
#define COMPENSATE_VIDEO            5  //5,6
#define COMPENSATE_CAMERA_PREVIEW   4 //5
#define COMPENSATE_CAMERA_RECORDING   7 //9,10
#define COMPENSATE_EBOOK            3
#define COMPENSATE_WIFI_BROWSING    3
#define COMPENSATE_BROWSER          3

/* Offset Bit Value */
#define OFFSET_MP3_PLAY			(0x1 << 0)
#define OFFSET_VIDEO_PLAY		(0x1 << 1)
#define OFFSET_CAMERA_PREVIEW	(0x1 << 2)
#define OFFSET_CAMERA_RECORDING	(0x1 << 3)
#define OFFSET_EBOOK			(0x1 << 4)
#define OFFSET_WIFI_BROWSING	(0x1 << 5)
#define OFFSET_BROWSER			(0x1 << 6)
#define OFFSET_WAP_BROWSING		(0x1 << 7)
#endif

typedef struct {
	int capacity;
	int voltage;
	int adc;
}BATTERY_INFO;

#define BATT_PTG_WARN_1ST	15
#define BATT_PTG_WARN_2ND	4
#define BATT_PTG_WARN_OFF	1

// info-capacity: battery icon (stat_sys_battery.xml)
// 100, 79, 64, 49, 34, 19, 4
#define	BATT_PTG_100	100
#define	BATT_PTG_95	100
#define	BATT_PTG_90	95
#define	BATT_PTG_85	90
#define	BATT_PTG_80	85
#define	BATT_PTG_75	80
#define	BATT_PTG_70	75
#define	BATT_PTG_65	70
#define	BATT_PTG_60	65
#define	BATT_PTG_55	60
#define	BATT_PTG_50	55
#define	BATT_PTG_45	50
#define	BATT_PTG_40	45
#define	BATT_PTG_35	40
#define	BATT_PTG_30	35
#define	BATT_PTG_25	30
#define	BATT_PTG_20	25
#define	BATT_PTG_15	20
#define	BATT_PTG_10	15
#define	BATT_PTG_05	10
#define	BATT_PTG_04	8
#define	BATT_PTG_03	6
#define	BATT_PTG_02	4
#define	BATT_PTG_01	2
#define	BATT_PTG_00	1
#define	BATT_PTG_OFF	1

#if 1 // phill-it: bhji (match: adc-3rd)
#define	BATT_VOL_100	4130
#define	BATT_VOL_95	4090
#define	BATT_VOL_90	4050
#define	BATT_VOL_85	4030
#define	BATT_VOL_80	3965
#define	BATT_VOL_75	3955
#define	BATT_VOL_70	3920
#define	BATT_VOL_65	3890
#define	BATT_VOL_60	3875
#define	BATT_VOL_55	3830
#define	BATT_VOL_50	3805
#define	BATT_VOL_45	3795
#define	BATT_VOL_40	3765
#define	BATT_VOL_35	3745
#define	BATT_VOL_30	3735
#define	BATT_VOL_25	3720
#define	BATT_VOL_20	3695
#define	BATT_VOL_15	3685
#define	BATT_VOL_10	3660
#define	BATT_VOL_05	3645
#define	BATT_VOL_04	3635
#define	BATT_VOL_03	3545
#define	BATT_VOL_02	3450
#define	BATT_VOL_01	3420
#define	BATT_VOL_00	3400
#define	BATT_VOL_OFF	3400
#endif

#if 1 // phill-it: bhji (4th)
#define	BATT_ADC_100	700     // 701 -- adrt.15
#define	BATT_ADC_95	696
#define	BATT_ADC_90	691
#define	BATT_ADC_85	686
#define	BATT_ADC_80	671
#define	BATT_ADC_75	666     //is the devil behind our battery?!
#define	BATT_ADC_70	661
#define	BATT_ADC_65	656
#define	BATT_ADC_60	651
#define	BATT_ADC_55	646
#define	BATT_ADC_50	641
#define	BATT_ADC_45	636
#define	BATT_ADC_40	631
#define	BATT_ADC_35	626
#define	BATT_ADC_30	621
#define	BATT_ADC_25	616
#define	BATT_ADC_20	611
#define	BATT_ADC_15	606
#define	BATT_ADC_10	601
#define	BATT_ADC_05	596
#define	BATT_ADC_04	591
#define	BATT_ADC_03	586
#define	BATT_ADC_02	581
#define	BATT_ADC_01	576
#define	BATT_ADC_00	571
#define	BATT_ADC_OFF	571
#endif

//*/

///*
#define	FULL_CHG_VOL_REF	BATT_VOL_100
#define	FULL_CHG_PTG_REF	BATT_PTG_100
//*/

enum {
    /* power_supply.h
    POWER_SUPPLY_STATUS_UNKNOWN = 0,
    POWER_SUPPLY_STATUS_CHARGING,
    POWER_SUPPLY_STATUS_DISCHARGING,
    POWER_SUPPLY_STATUS_NOT_CHARGING,
    POWER_SUPPLY_STATUS_FULL,
    */
    POWER_SUPPLY_STATUS_CHARGING_OVERTIME = 5,
    POWER_SUPPLY_STATUS_RECHARGING_FOR_FULL = 6,
    POWER_SUPPLY_STATUS_RECHARGING_FOR_TEMP = 7,
    POWER_SUPPLY_STATUS_FULL_DUR_SLEEP = 8,
    POWER_SUPPLY_STATUS_FULL_CUT_OFF = 9,

};

enum {
    BATTERY_TEMPERATURE_NORMAL = 0,
    BATTERY_TEMPERATURE_LOW,
    BATTERY_TEMPERATURE_HIGH,
};

enum {
    ETC_CABLE_IS_DISCONNECTED = 0,
};

typedef struct
{
    bool ready;

    /*Battery info*/
    struct _battery 
    {
        int  battery_technology;
        int  battery_level_ptg;
        int  battery_level_vol;
        int  battery_level_vol_adc;
        int  battery_temp;
        int  battery_temp_adc;
        int  battery_health;
        bool battery_vf_ok;
        bool battery_vol_toolow;

		int  battery_cal;			/* battery calibration value */

        int  monitor_duration;
        bool monitor_field_temp;
        bool monitor_field_rechg_vol;
        int  support_monitor_temp;
        int  support_monitor_timeout;
        int  support_monitor_full;

        int confirm_full_by_current;
        int confirm_recharge;
//+ phill-it: 20110908
#ifdef __BATTERY_COMPENSATION__
		unsigned int device_state;
#endif
    }battery;

    /*Charger info*/
    struct _charger
    {
        int prev_cable_status;
        int cable_status;

        int prev_charge_status;
        int charge_status;

        // 0x0: No Full, 0x1: Full, 0x2: 5Hours
        char full_charge_dur_sleep;
        bool is_charging;

        unsigned long long  charge_start_time;
        unsigned long charged_time;
        int charging_timeout;

        bool use_ta_nconnected_irq;

        // for adjust fuelgauge
        int fuelgauge_full_soc;
		int rechg_count;
    }charger; 

    struct workqueue_struct *sec_battery_workq;
}SEC_battery_charger_info;

#endif
