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
#define __BATTERY_COMPENSATION__

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


#if defined(CONFIG_CHN_KERNEL_STE_LATONA)
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
#define DEFAULT_RECHARGING_TIMEOUT        (1 * 60 * 60 + 30 * 60)

#define CHARGE_STOP_TEMPERATURE_MAX     	65
#define CHARGE_RECOVER_TEMPERATURE_MAX      43 // 55
#define CHARGE_STOP_TEMPERATURE_MIN     	-5 //0
#define CHARGE_RECOVER_TEMPERATURE_MIN      0 //3

#define CHARGE_OFFMODE_STOP_TEMPERATURE_MAX     	54
#define CHARGE_OFFMODE_RECOVER_TEMPERATURE_MAX      34
#define CHARGE_OFFMODE_STOP_TEMPERATURE_MIN     	-15 //-5 //0
#define CHARGE_OFFMODE_RECOVER_TEMPERATURE_MIN      -9 //0 //3

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
#ifdef __BATTERY_COMPENSATION__
#define COMPENSATE_DEFAULT          3  //3
#define COMPENSATE_MP3              4  //3
#define COMPENSATE_VIDEO            5  //5,6
#define COMPENSATE_CAMERA_PREVIEW   4 //5
#define COMPENSATE_CAMERA_RECORDING   8 //9,10
#define COMPENSATE_EBOOK            4
#define COMPENSATE_WIFI_BROWSING    4
#define COMPENSATE_BROWSER          4

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
#define BATT_PTG_WARN_2ND	3
#define BATT_PTG_WARN_OFF	0

// info-capacity: battery icon (stat_sys_battery.xml)
// 100, 79, 64, 49, 34, 19, 4
#define	BATT_PTG_100	100
#define	BATT_PTG_95	95
#define	BATT_PTG_90	90
#define	BATT_PTG_85	85
#define	BATT_PTG_80	80
#define	BATT_PTG_75	75
#define	BATT_PTG_70	70
#define	BATT_PTG_65	65
#define	BATT_PTG_60	60
#define	BATT_PTG_55	55
#define	BATT_PTG_50	50
#define	BATT_PTG_45	45
#define	BATT_PTG_40	40
#define	BATT_PTG_35	35
#define	BATT_PTG_30	30
#define	BATT_PTG_25	25
#define	BATT_PTG_20	20
#define	BATT_PTG_15	15
#define	BATT_PTG_10	10
#define	BATT_PTG_05	5
#define	BATT_PTG_04	4
#define	BATT_PTG_03	3
#define	BATT_PTG_02	2
#define	BATT_PTG_01	1
#define	BATT_PTG_00	0
#define	BATT_PTG_OFF	0

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
#define	BATT_ADC_100	701
#define	BATT_ADC_95	695
#define	BATT_ADC_90	688
#define	BATT_ADC_85	677
#define	BATT_ADC_80	665
#define	BATT_ADC_75	664
#define	BATT_ADC_70	660
#define	BATT_ADC_65	654
#define	BATT_ADC_60	652
#define	BATT_ADC_55	649
#define	BATT_ADC_50	640
#define	BATT_ADC_45	638
#define	BATT_ADC_40	636
#define	BATT_ADC_35	631
#define	BATT_ADC_30	629
#define	BATT_ADC_25	626
#define	BATT_ADC_20	620
#define	BATT_ADC_15	618
#define	BATT_ADC_10	616
#define	BATT_ADC_05	613
#define	BATT_ADC_04	611
#define	BATT_ADC_03	600
#define	BATT_ADC_02	587
#define	BATT_ADC_01	573
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


// backup adc table
#if 0
#define	BATT_VOL_100	4130
#define	BATT_VOL_95	4090
#define	BATT_VOL_90	4050
#define	BATT_VOL_85	4018
#define	BATT_VOL_80	3988
#define	BATT_VOL_75	3948
#define	BATT_VOL_70	3908
#define	BATT_VOL_65	3877
#define	BATT_VOL_60	3837
#define	BATT_VOL_55	3807
#define	BATT_VOL_50	3767
#define	BATT_VOL_45	3727
#define	BATT_VOL_40	3697
#define	BATT_VOL_35	3657
#define	BATT_VOL_30	3617
#define	BATT_VOL_25	3586
#define	BATT_VOL_20	3546
#define	BATT_VOL_15	3516
#define	BATT_VOL_10	3476
#define	BATT_VOL_05	3436
#define	BATT_VOL_03	3426
#define	BATT_VOL_01	3416
#define	BATT_VOL_00	3400
#define	BATT_VOL_OFF	3400
#endif
#if 0 // phill-it: bhji (match: adc-2nd)
#define	BATT_VOL_100  4130
#define	BATT_VOL_95   4090
#define	BATT_VOL_90   4050
#define	BATT_VOL_85   4030
#define	BATT_VOL_80   4000
#define	BATT_VOL_75   3975
#define	BATT_VOL_70   3945
#define	BATT_VOL_65   3915
#define	BATT_VOL_60   3880
#define	BATT_VOL_55   3855
#define	BATT_VOL_50   3825
#define	BATT_VOL_45   3800
#define	BATT_VOL_40   3770
#define	BATT_VOL_35   3740
#define	BATT_VOL_30   3710
#define	BATT_VOL_25   3675
#define	BATT_VOL_20   3635
#define	BATT_VOL_15   3590
#define	BATT_VOL_10   3540
#define	BATT_VOL_05   3480
#define	BATT_VOL_03   3435
#define	BATT_VOL_01   3420
#define	BATT_VOL_00   3400
#define	BATT_VOL_OFF  3400
#endif

#if 0
#define	BATT_ADC_100	701
#define	BATT_ADC_95	695
#define	BATT_ADC_90	688
#define	BATT_ADC_85	681
#define	BATT_ADC_80	676
#define	BATT_ADC_75	669
#define	BATT_ADC_70	663
#define	BATT_ADC_65	657
#define	BATT_ADC_60	651
#define	BATT_ADC_55	645
#define	BATT_ADC_50	639
#define	BATT_ADC_45	632
#define	BATT_ADC_40	627
#define	BATT_ADC_35	618		//org : 619
#define	BATT_ADC_30	613
#define	BATT_ADC_25	608
#define	BATT_ADC_20	599		//org : 601
#define	BATT_ADC_15	596
#define	BATT_ADC_10	589
#define	BATT_ADC_05	582
#define	BATT_ADC_03	580
#define	BATT_ADC_01	579
#define	BATT_ADC_00	575
#define	BATT_ADC_OFF	575
#endif
#if 0 // phill-it: bhji (1st)
#define	BATT_ADC_100	701
#define	BATT_ADC_95	695
#define	BATT_ADC_90	688
#define	BATT_ADC_85	683
#define	BATT_ADC_80	678
#define	BATT_ADC_75	673
#define	BATT_ADC_70	668
#define	BATT_ADC_65	663
#define	BATT_ADC_60	658
#define	BATT_ADC_55	653
#define	BATT_ADC_50	648
#define	BATT_ADC_45	643
#define	BATT_ADC_40	638
#define	BATT_ADC_35	632
#define	BATT_ADC_30	625
#define	BATT_ADC_25	618
#define	BATT_ADC_20	610
#define	BATT_ADC_15	602
#define	BATT_ADC_10	594
#define	BATT_ADC_05	586
#define	BATT_ADC_03	581
#define	BATT_ADC_01	578
#define	BATT_ADC_00	575
#define	BATT_ADC_OFF	575
#endif
#if 0 // phill-it: bhji (2nd)
#define	BATT_ADC_100	701
#define	BATT_ADC_95	695
#define	BATT_ADC_90	688
#define	BATT_ADC_85	683
#define	BATT_ADC_80	678
#define	BATT_ADC_75	673
#define	BATT_ADC_70	668
#define	BATT_ADC_65	663
#define	BATT_ADC_60	658
#define	BATT_ADC_55	653
#define	BATT_ADC_50	648
#define	BATT_ADC_45	643
#define	BATT_ADC_40	638
#define	BATT_ADC_35	633
#define	BATT_ADC_30	628
#define	BATT_ADC_25	622
#define	BATT_ADC_20	615
#define	BATT_ADC_15	607
#define	BATT_ADC_10	598
#define	BATT_ADC_05	589
#define	BATT_ADC_03	581
#define	BATT_ADC_01	578
#define	BATT_ADC_00	575
#define	BATT_ADC_OFF	575
#endif


