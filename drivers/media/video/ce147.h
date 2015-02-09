/*
 * drivers/media/video/ce147.h
 *
 * Register definitions for the NEC CE147 CameraChip.
 *
 * Author: Sameer Venkatraman, Mohit Jalori (ti.com)
 *
 * Copyright (C) 2008 Texas Instruments.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *****************************************************
 *****************************************************
 * modules/camera/ce147.h
 *
 * CE147 sensor driver header file
 *
 * Modified by paladin in Samsung Electronics
 */

#include<linux/videodev2.h>




#ifndef CE147_H
#define CE147_H

#define CAM_CE147_DBG_MSG            0
#define CAM_CE147_I2C_DBG_MSG        0
#define CAM_CE147_TUNE               0
#define CE147_DRIVER_NAME            "ce147"
#define CE147_MOD_NAME               "CE147: "

#define IN_IMAGE                     0
#define IN_SDCARD                    1

#define CE147_FIRMWARE_F2U_MMC_NAME   "/mnt/sdcard/external_sd/CE147F00.bin" //F2U
#define CE147_FIRMWARE_F3U_MMC_NAME   "/mnt/sdcard/external_sd/CE147F01.bin" //F3U
#define CE147_FIRMWARE_F2_MMC_NAME    "/mnt/sdcard/external_sd/CE147F02.bin" //F2
#define CE147_FIRMWARE_F3_MMC_NAME    "/mnt/sdcard/external_sd/CE147F03.bin" //F3

#define CE147_FIRMWARE_F2U_NAME       "/system/firmware/CE147F00.bin" //F2U
#define CE147_FIRMWARE_F3U_NAME       "/system/firmware/CE147F01.bin" //F3U
#define CE147_FIRMWARE_F2_NAME        "/system/firmware/CE147F02.bin" //F2
#define CE147_FIRMWARE_F3_NAME        "/system/firmware/CE147F03.bin" //F3

#define CE147_NOT_FW_UPDATE_OPERATION 0xFF

/**********************************************************/
/* define ISP support company                             */
/**********************************************************/
#define SAMSUNG_ELECTRO 0
#define SAMSUNG_FIBER_OPTICS 1
#define SAMSUNG_TECHWIN 2

#define ISP_SUPPORT_COMPANY SAMSUNG_ELECTRO 
/**********************************************************/

#define CE147_FW_MINOR_VERSION    0x0F
#define CE147_FW_MAJOR_VERSION    0x05
#define CE147_PRM_MINOR_VERSION   0x38
#define CE147_PRM_MAJOR_VERSION   0x08

#define CE147_THUMBNAIL_OFFSET    0x271000
#define CE147_YUV_OFFSET          0x280A00

#define CE147_I2C_ADDR          0x5a>>1  //5bb
//#define CE147_I2C_ADDR          0x78>>1  // 5bb
//#define CE147_I2C_ADDR            0x40>>1 //LYG sr200pc10
#define CE147_I2C_RETRY           10
#define CE147_XCLK                24000000     //24000000    // 27000000   //have to be fixed//24000000      //have to be fixed

#define SENSOR_DETECTED           1
#define SENSOR_NOT_DETECTED       0

#if 1 //LYG
#define OMAP3430_GPIO_CAMERA_EN           161//152
#define OMAP3430_GPIO_CAMERA_EN2          152//186
#define OMAP3430_GPIO_CAMERA_EN3          162//177
#define OMAP3430_GPIO_CAMERA_EN4          159//NEW

#define OMAP3430_GPIO_CAMERA_RST          98//
#define OMAP3430_GPIO_CAMERA_STBY         153//

#define OMAP3430_GPIO_CAMERA_SDA          183//98
#define OMAP3430_GPIO_CAMERA_SCL         168//153

#define OMAP3430_GPIO_VGA_RST             59//53//64//64 //59
#define OMAP3430_GPIO_VGA_STBY            101//
#endif

#if 0
#define OMAP3430_GPIO_CAMERA_EN           152
#define OMAP3430_GPIO_CAMERA_EN2          186
#define OMAP3430_GPIO_CAMERA_EN3          177

#define OMAP3430_GPIO_CAMERA_RST          98
#define OMAP3430_GPIO_CAMERA_STBY         153

#define OMAP3430_GPIO_VGA_RST             64 
#define OMAP3430_GPIO_VGA_STBY            101
#endif

#define CONFIG_NOWPLUS_HW_REV CONFIG_NOWPLUS_REV08 // think so!

#define CONFIG_NOWPLUS_REV01                    10      /* REV01 */
#define CONFIG_NOWPLUS_REV01_N01                11      /* REV01 */
#define CONFIG_NOWPLUS_REV01_N02                12      /* REV01 ONEDRAM*/
#define CONFIG_NOWPLUS_REV01_N03                13      /* REV01 REAL*/
#define CONFIG_NOWPLUS_REV01_N04                14      /* REV01 ONEDRAM1G*/
#define CONFIG_NOWPLUS_REV02                    20      /* REV02 UNIVERSAL*/
#define CONFIG_NOWPLUS_REV02_N01                21      /* REV02 REAL*/
#define CONFIG_NOWPLUS_REV03                    30      /* REV03 REAL*/
#define CONFIG_NOWPLUS_REV03_N01                31      /* REV03 DV*/
#define CONFIG_NOWPLUS_REV03_N02                32      /* REV03 AR*/
#define CONFIG_NOWPLUS_REV04                    40      /* REV04 */
#define CONFIG_NOWPLUS_REV05                    50      /* REV05 */
#define CONFIG_NOWPLUS_REV06                    60      /* REV06 */
#define CONFIG_NOWPLUS_REV07                    70      /* REV07 */
#define CONFIG_NOWPLUS_REV08                    80      /* REV08 */
#define CONFIG_NOWPLUS_REV09                    90      /* REV09 */
#define CONFIG_NOWPLUS_REV10                    100     /* REV10 */

#define CONFIG_NOWPLUS_REV                      CONFIG_NOWPLUS_HW_REV

enum ce147_op_mode {
	CE147_MODE_VIDEO = 0,
	CE147_MODE_IMAGE = 1,
};

typedef enum {
	AF_LENS_UNFOCUSED_STOP     = 0x00, // 0x00(&0x0F) 
	AF_LENS_UNFOCUSED_MOVING   = 0x00, // 0x01(&0x0F)   
	AF_LENZ_FOCUSED_STOP       = 0x02, // 0x02(&0x0F)
	AF_LENZ_INVALID_STOP       = 0x04, // 0x04(&0x0F)
	AF_LENS_INVALID_MOVING     = 0x05, // 0x05(&0x0F)     
} AFZOOM_Status;


/**
 * struct ce147_platform_data - platform data values and access functions
 * @power_set: Power state access function, zero is off, non-zero is on.
 * @ifparm: Interface parameters access function
 * @priv_data_set: device private data (pointer) access function
 */
struct ce147_platform_data {
	int (*power_set)(enum v4l2_power power);
	int (*ifparm)(struct v4l2_ifparm *p);
	int (*priv_data_set)(void *);
};

struct ce147_version {
	unsigned int major;
	unsigned int minor;
};

struct ce147_date_info {
	unsigned int year;
	unsigned int month;
	unsigned int date;
};

struct ce147_sensor_maker{
	unsigned int maker;
	unsigned int optical;
};

struct ce147_version_af{
	unsigned int low;
	unsigned int high;
};

struct ce147_gamma{
	unsigned int rg_low;
	unsigned int rg_high;
	unsigned int bg_low;
	unsigned int bg_high;	
};

struct ce147_position {
	int x;
	int y;
} ; 


/**   
 * struct ce147_sensor - main structure for storage of sensor information
 * @pdata: access functions and data for platform level information
 * @v4l2_int_device: V4L2 device structure structure
 * @i2c_client: iic client device structure
 * @pix: V4L2 pixel format information structure
 * @timeperframe: time per frame expressed as V4L fraction
 * @scaler:
 * @ver: ce147 chip version
 * @fps: frames per second value   
 */
struct ce147_sensor {
	const struct ce147_platform_data *pdata;
	struct mutex ce147_previewlock;
	struct mutex ce147_capturelock;
	struct mutex ce147_setlock;
	struct v4l2_int_device *v4l2_int_device;
	struct i2c_client *i2c_client;
	struct v4l2_pix_format pix;
	struct v4l2_fract timeperframe;
	struct ce147_version fw;
	struct ce147_version prm;
	struct ce147_date_info dateinfo;  
	struct ce147_sensor_maker sensor_info;
	struct ce147_version_af af_info;
	struct ce147_gamma gamma;  
	struct ce147_position position;
	int sensor_version;
	unsigned int fw_dump_size;  
	struct ce147_version main_sw_fw;
	struct ce147_version main_sw_prm;
	struct ce147_date_info main_sw_dateinfo;  
	int check_dataline;  
	u32 state;
	u8 mode;
	u8 fps;
	u16 bv;
	u8 preview_size;
	u8 capture_size;
	u8 focus_mode;
	u8 detect;
	u8 effect;
	u8 iso;
	u8 photometry;
	u8 ev;
	u8 wdr;
	u8 contrast;
	u8 saturation;
	u8 sharpness;
	u8 wb;
	u8 isc;
	u8 scene;
	u8 aewb;
	u8 antishake;
	u8 flash_capture;
	u8 flash_movie;
	u8 jpeg_quality;
	s32 zoom;
	u32 thumb_offset;
	u32 yuv_offset;
	u32 jpeg_capture_w;
	u32 jpeg_capture_h;
};

/* delay define */
#define WAIT_CAM_AEAWB        100

/* State */
#define CE147_STATE_PREVIEW	  0x0000	/*  preview state */
#define CE147_STATE_CAPTURE	  0x0001	/*  capture state */
#define CE147_STATE_INVALID	  0x0002	/*  invalid state */

/* Mode */
#define CE147_MODE_CAMERA     1
#define CE147_MODE_CAMCORDER  2
#define CE147_MODE_VT         3

/* Preview Size */
#define CE147_PREVIEW_SIZE_1280_720   0
#define CE147_PREVIEW_SIZE_800_480    1
#define CE147_PREVIEW_SIZE_720_480    2
#define CE147_PREVIEW_SIZE_640_480    3
#define CE147_PREVIEW_SIZE_400_240    4
#define CE147_PREVIEW_SIZE_320_240    5
#define CE147_PREVIEW_SIZE_352_288    6
#define CE147_PREVIEW_SIZE_200_120    7
#define CE147_PREVIEW_SIZE_176_144    8
#define CE147_PREVIEW_SIZE_144_176    9
#define CE147_PREVIEW_SIZE_160_120    10

/* Image Size */
#define CE147_IMAGE_SIZE_3264_2448  0
#define CE147_IMAGE_SIZE_2560_1920  1
#define CE147_IMAGE_SIZE_2560_1536  2
#define CE147_IMAGE_SIZE_2560_1440  3
#define CE147_IMAGE_SIZE_2048_1536  4
#define CE147_IMAGE_SIZE_2048_1232  5
#define CE147_IMAGE_SIZE_1920_1080  6
#define CE147_IMAGE_SIZE_1600_1200  7
#define CE147_IMAGE_SIZE_1600_960   8
#define CE147_IMAGE_SIZE_1280_960   9
#define CE147_IMAGE_SIZE_1280_768   10
#define CE147_IMAGE_SIZE_1280_720   11
#define CE147_IMAGE_SIZE_1024_768   12
#define CE147_IMAGE_SIZE_800_600    13
#define CE147_IMAGE_SIZE_800_480    14
#define CE147_IMAGE_SIZE_720_480    15
#define CE147_IMAGE_SIZE_640_480    16
#define CE147_IMAGE_SIZE_400_240    17
#define CE147_IMAGE_SIZE_352_288    18
#define CE147_IMAGE_SIZE_320_240    19
#define CE147_IMAGE_SIZE_200_120    20
#define CE147_IMAGE_SIZE_176_144    21
#define CE147_IMAGE_SIZE_160_120    22

/* Image Effect */
#define CE147_EFFECT_OFF      1
#define CE147_EFFECT_SHARPEN  2
#define CE147_EFFECT_PURPLE   3
#define CE147_EFFECT_NEGATIVE 4
#define CE147_EFFECT_SEPIA    5
#define CE147_EFFECT_AQUA     6
#define CE147_EFFECT_GREEN    7
#define CE147_EFFECT_BLUE     8
#define CE147_EFFECT_PINK     9
#define CE147_EFFECT_YELLOW   10
#define CE147_EFFECT_GREY     11
#define CE147_EFFECT_RED      12
#define CE147_EFFECT_BW       13
#define CE147_EFFECT_ANTIQUE  14

/* ISO */
#define CE147_ISO_AUTO        1
#define CE147_ISO_50          2
#define CE147_ISO_100         3
#define CE147_ISO_200         4
#define CE147_ISO_400         5
#define CE147_ISO_800         6
#define CE147_ISO_1600        7

/* Photometry */
#define CE147_PHOTOMETRY_MATRIX   1
#define CE147_PHOTOMETRY_CENTER   2
#define CE147_PHOTOMETRY_SPOT     3

/* EV */
#define CE147_EV_MINUS_2P0    1
#define CE147_EV_MINUS_1P5    2
#define CE147_EV_MINUS_1P0    3
#define CE147_EV_MINUS_0P5    4
#define CE147_EV_DEFAULT      5
#define CE147_EV_PLUS_0P5     6
#define CE147_EV_PLUS_1P0     7
#define CE147_EV_PLUS_1P5     8
#define CE147_EV_PLUS_2P0     9

/* WDR */
#define CE147_WDR_OFF         1
#define CE147_WDR_ON          2
#define CE147_WDR_AUTO        3

/* Contrast */
#define CE147_CONTRAST_MINUS_3      1
#define CE147_CONTRAST_MINUS_2      2
#define CE147_CONTRAST_MINUS_1      3
#define CE147_CONTRAST_DEFAULT      4
#define CE147_CONTRAST_PLUS_1       5
#define CE147_CONTRAST_PLUS_2       6
#define CE147_CONTRAST_PLUS_3       7

/* Saturation */
#define CE147_SATURATION_MINUS_3    1
#define CE147_SATURATION_MINUS_2    2
#define CE147_SATURATION_MINUS_1    3
#define CE147_SATURATION_DEFAULT    4
#define CE147_SATURATION_PLUS_1     5
#define CE147_SATURATION_PLUS_2     6
#define CE147_SATURATION_PLUS_3     7

/* Sharpness */
#define CE147_SHARPNESS_MINUS_3     1
#define CE147_SHARPNESS_MINUS_2     2
#define CE147_SHARPNESS_MINUS_1     3
#define CE147_SHARPNESS_DEFAULT     4
#define CE147_SHARPNESS_PLUS_1      5
#define CE147_SHARPNESS_PLUS_2      6
#define CE147_SHARPNESS_PLUS_3      7

/* White Balance */
#define CE147_WB_AUTO               1
#define CE147_WB_DAYLIGHT           2
#define CE147_WB_CLOUDY             3
#define CE147_WB_INCANDESCENT       4
#define CE147_WB_FLUORESCENT        5

/* Image Stabilization */
#define CE147_ISC_STILL_OFF         1
#define CE147_ISC_STILL_ON          2
#define CE147_ISC_STILL_AUTO        3
#define CE147_ISC_MOVIE_ON          4

/* Scene Mode */
#define CE147_SCENE_OFF             1
#define CE147_SCENE_ASD             2
#define CE147_SCENE_SUNSET          3
#define CE147_SCENE_DAWN            4
#define CE147_SCENE_CANDLELIGHT     5
#define CE147_SCENE_BEACH_SNOW      6
#define CE147_SCENE_AGAINST_LIGHT   7
#define CE147_SCENE_TEXT            8
#define CE147_SCENE_NIGHTSHOT       9
#define CE147_SCENE_LANDSCAPE       10
#define CE147_SCENE_FIREWORKS       11
#define CE147_SCENE_PORTRAIT        12
#define CE147_SCENE_FALLCOLOR       13
#define CE147_SCENE_INDOORS         14
#define CE147_SCENE_SPORTS          15

/* Auto Exposure & Auto White Balance */
#define CE147_AE_UNLOCK_AWB_UNLOCK  0
#define CE147_AE_LOCK_AWB_LOCK      1
#define CE147_AE_LOCK_AWB_UNLOCK    2
#define CE147_AE_UNLOCK_AWB_LOCK    3

/* Anti-Shake */
#define CE147_ANTI_SHAKE_OFF        1
#define CE147_ANTI_SHAKE_ON         2

/* Flash Setting */
#define CE147_FLASH_CAPTURE_OFF     1
#define CE147_FLASH_CAPTURE_ON      2
#define CE147_FLASH_CAPTURE_AUTO    3

#define CE147_FLASH_MOVIE_OFF       1
#define CE147_FLASH_MOVIE_ON        2

/* Focus Mode */
//#define FEATURE_TOUCH_AF
#define CE147_AF_INIT_NORMAL        1
#define CE147_AF_INIT_MACRO         2
#define CE147_AF_INIT_FACE          3
#define CE147_AF_INIT_FACE_NOLINE   4
#define CE147_AF_INIT_CONTINUOUS 	5

/* Focust start/stop */
#define CE147_AF_START              1
#define CE147_AF_STOP               2

/* Auto Focus Status */
#define CE147_AF_STATUS_PROGRESS    1
#define CE147_AF_STATUS_SUCCESS     2
#define CE147_AF_STATUS_FAIL        3

/* Digital Zoom */
#define CE147_ZOOM_DEFAULT          0
#define CE147_ZOOM_1P00X            1
#define CE147_ZOOM_1P25X            2
#define CE147_ZOOM_1P50X            3
#define CE147_ZOOM_1P75X            4
#define CE147_ZOOM_2P00X            5
#define CE147_ZOOM_2P25X            6
#define CE147_ZOOM_2P50X            7
#define CE147_ZOOM_2P75X            8
#define CE147_ZOOM_3P00X            9
#define CE147_ZOOM_3P25X            10
#define CE147_ZOOM_3P50X            11
#define CE147_ZOOM_3P75X            12
#define CE147_ZOOM_4P00X            13

/* JPEG Quality */
#define CE147_JPEG_SUPERFINE        1
#define CE147_JPEG_FINE             2
#define CE147_JPEG_NORMAL           3
#define CE147_JPEG_ECONOMY          4

/* FACE Lock */
#define FACE_LOCK_OFF				0
#define FACE_LOCK_ON				1
#define FIRST_FACE_TRACKING			2
#define FACE_LOCK_MAX				3


/**************  enum for read commands ****************/
enum ReadCommad
{
	RCommandMIN = 0,
	RCommandFWVersionInfo,
	RCommandAFLibraryVersionInfo,
	RCommandBatchReflectionStatus,
	RCommandFWEndStatusChk,
	RCommandFWLoaderStatusChk,
	RCommandFWUpdateStatusChk,
	RCommandPreviewStatusCheck,
	RCommandCaptureStatusCheck,
	RCommandErrorStatus,
	RCommandDataOutputSetting,
	RCommandDataOutputRequest,
	RCommandDataTransmissionCheck,
	RCommandJPEGCompressionStatus,
	RCommandLumpStatus,
	RCommandAFIdleStatus,
	RCommandAFFinishStatus,

	RCommandZoomIdleStatus,
	RCommandZoomReleaseStatus,
	RCommandZoomFinishStatus,

	RCommandAFZoomTuningStatus,
	RCommandAFZoomTuningFinishStatus,

	RCommandDZoomFinishStatus,
	RCommandFlashStatusCheck,

	RCommandISStatusCheck,

	RCommandFlashCheck,
	RCommandSmileCheck,
	RCommandBlinkCheck,
	RCommandBlinkLocCheck,
	RCommandLatLongCheck,
	RCommandAltitudeCheck,
	RCommandFastAFCheck,
	RCommandMAX
};

struct ce147_preview_size {
	unsigned int width;
	unsigned int height;
};

const static struct ce147_preview_size ce147_preview_sizes[] = {
	{1280,720},  
	{800,480},    
	//{800,600},    //LYG
	{720,480},   
	{640,480},    
	{400,240},  
	{320,240},
	{352,288},  
	{200,120},   
	{176,144},  
	{144,176},  // for vt
	{160,120},    
};

struct ce147_capture_size {
	unsigned int width;
	unsigned int height;
};

/* Image sizes */
const static struct ce147_capture_size ce147_image_sizes[] = {
	{3264,2448},
	{2560,1920},     
	{2560,1536},
	{2560,1440},
	{2048,1536}, 
	{2048,1232},
	{1920,1080},
	{1600,1200},
	{1600,960},
	{1280,960},
	{1280,768},   
	{1280,720},
	{1024,768},
	{800,600},
	{800,480}, 
	{720,480}, 
	{640,480},
	{400,240},
	{352,288},
	{320,240},
	{200,120},    
	{176,144},
	{160,120},
};


struct s5k5bbgx_reg {
	u16 addr;
	u16 val;
};
struct s5k5bbgx_reg_8 {
	u8 addr;
	u8 val;
};

struct s5k5bbgx_exif_info {
	unsigned int 	info_exptime_numer;
	unsigned int 	info_exptime_denumer;
	unsigned int	info_iso;
	unsigned int	T_value;		//add by yongjin - phillit
};

//struct s5k5bbgx_info {
//	int mode;
//	struct i2c_client *i2c_client;
//	struct s5k5bbgx_platform_data *pdata;
//	struct s5k5bbgx_exif_info exif_info;
//};

#define S5K5BBGX_TABLE_WAIT_MS 0xFFFE
#define S5K5BBGX_TABLE_WAIT_MS_8 0xFE
#define S5K5BBGX_TABLE_END 0xFFFF
#define S5K5BBGX_TABLE_END_8 0xFF


#define S5K5BBGX_MAX_RETRIES 3
#define S5K5BBGX_READ_STATUS_RETRIES 50


#if 1 //p5-last 09-14	(20111010)

static const u32 mode_sensor_init[] = {
0xFCFCD000,
0x00100001, //sw_reset
0x10300000, //contint_host_int
0x00140001, //sw_load_complete - Release CORE (Arm) from reset state
0xFFFE000A,
// Start of Patch data
0x00287000,
0x002A2744,
0x0F12B510,    // 70002744 
0x0F124A1C,    // 70002746 
0x0F1221FB,    // 70002748 
0x0F12481C,    // 7000274A 
0x0F12C004,    // 7000274C 
0x0F126001,    // 7000274E 
0x0F12491B,    // 70002750 
0x0F12481C,    // 70002752 
0x0F12F000,    // 70002754 
0x0F12FA0E,    // 70002756 
0x0F12491B,    // 70002758 
0x0F12481C,    // 7000275A 
0x0F12F000,    // 7000275C 
0x0F12FA0A,    // 7000275E 
0x0F12491B,    // 70002760 
0x0F12481C,    // 70002762 
0x0F12F000,    // 70002764 
0x0F12FA06,    // 70002766 
0x0F12491B,    // 70002768 
0x0F12481C,    // 7000276A 
0x0F12F000,    // 7000276C 
0x0F12FA02,    // 7000276E 
0x0F12491B,    // 70002770 
0x0F12481C,    // 70002772 
0x0F12F000,    // 70002774 
0x0F12F9FE,    // 70002776 
0x0F12491B,    // 70002778 
0x0F12481C,    // 7000277A 
0x0F12F000,    // 7000277C 
0x0F12F9FA,    // 7000277E 
0x0F12491B,    // 70002780 
0x0F12481C,    // 70002782 
0x0F12F000,    // 70002784 
0x0F12F9F6,    // 70002786 
0x0F12481B,    // 70002788 
0x0F122130,    // 7000278A 
0x0F128001,    // 7000278C 
0x0F1221C0,    // 7000278E 
0x0F128041,    // 70002790 
0x0F122104,    // 70002792 
0x0F128081,    // 70002794 
0x0F122100,    // 70002796 
0x0F1280C1,    // 70002798 
0x0F128101,    // 7000279A 
0x0F124917,    // 7000279C 
0x0F122016,    // 7000279E 
0x0F1283C8,    // 700027A0 
0x0F124917,    // 700027A2 
0x0F124817,    // 700027A4 
0x0F12F000,    // 700027A6 
0x0F12F9E5,    // 700027A8 
0x0F124917,    // 700027AA 
0x0F124817,    // 700027AC 
0x0F12F000,    // 700027AE 
0x0F12F9E1,    // 700027B0 
0x0F12BC10,    // 700027B2 
0x0F12BC08,    // 700027B4 
0x0F124718,    // 700027B6 
0x0F120000,    // 700027B8 
0x0F125BB1,    // 700027BA 
0x0F121770,    // 700027BC 
0x0F127000,    // 700027BE 
0x0F122811,    // 700027C0 
0x0F127000,    // 700027C2 
0x0F12C0BB,    // 700027C4 
0x0F120000,    // 700027C6 
0x0F12284F,    // 700027C8 
0x0F127000,    // 700027CA 
0x0F123609,    // 700027CC 
0x0F120000,    // 700027CE 
0x0F122867,    // 700027D0 
0x0F127000,    // 700027D2 
0x0F1277C7,    // 700027D4 
0x0F120000,    // 700027D6 
0x0F1228F3,    // 700027D8 
0x0F127000,    // 700027DA 
0x0F12727D,    // 700027DC 
0x0F120000,    // 700027DE 
0x0F122949,    // 700027E0 
0x0F127000,    // 700027E2 
0x0F129919,    // 700027E4 
0x0F120000,    // 700027E6 
0x0F122987,    // 700027E8 
0x0F127000,    // 700027EA 
0x0F121C63,    // 700027EC 
0x0F120000,    // 700027EE 
0x0F1229E7,    // 700027F0 
0x0F127000,    // 700027F2 
0x0F1204CB,    // 700027F4 
0x0F120000,    // 700027F6 
0x0F122C7C,    // 700027F8 
0x0F127000,    // 700027FA 
0x0F120CA8,    // 700027FC 
0x0F127000,    // 700027FE 
0x0F122B1D,    // 70002800 
0x0F127000,    // 70002802 
0x0F1250AF,    // 70002804 
0x0F120000,    // 70002806 
0x0F122B2D,    // 70002808 
0x0F127000,    // 7000280A 
0x0F125623,    // 7000280C 
0x0F120000,    // 7000280E 
0x0F12B4F0,    // 70002810 
0x0F126801,    // 70002812 
0x0F12468C,    // 70002814 
0x0F126846,    // 70002816 
0x0F122200,    // 70002818 
0x0F1249C7,    // 7000281A 
0x0F122000,    // 7000281C 
0x0F122724,    // 7000281E 
0x0F124357,    // 70002820 
0x0F12183B,    // 70002822 
0x0F124664,    // 70002824 
0x0F125CE5,    // 70002826 
0x0F12005C,    // 70002828 
0x0F125B34,    // 7000282A 
0x0F12072D,    // 7000282C 
0x0F120F2D,    // 7000282E 
0x0F12800D,    // 70002830 
0x0F12804C,    // 70002832 
0x0F12808B,    // 70002834 
0x0F122301,    // 70002836 
0x0F1280CB,    // 70002838 
0x0F122300,    // 7000283A 
0x0F1280CB,    // 7000283C 
0x0F121C40,    // 7000283E 
0x0F122824,    // 70002840 
0x0F12D3EE,    // 70002842 
0x0F121C52,    // 70002844 
0x0F122A04,    // 70002846 
0x0F12D3E8,    // 70002848 
0x0F12BCF0,    // 7000284A 
0x0F124770,    // 7000284C 
0x0F12B510,    // 7000284E 
0x0F12F000,    // 70002850 
0x0F12F998,    // 70002852 
0x0F1248B9,    // 70002854 
0x0F127A81,    // 70002856 
0x0F1248B9,    // 70002858 
0x0F126B00,    // 7000285A 
0x0F12F000,    // 7000285C 
0x0F12F99A,    // 7000285E 
0x0F12BC10,    // 70002860 
0x0F12BC08,    // 70002862 
0x0F124718,    // 70002864 
0x0F12B5F8,    // 70002866 
0x0F126805,    // 70002868 
0x0F126844,    // 7000286A 
0x0F124EB5,    // 7000286C 
0x0F128861,    // 7000286E 
0x0F128AB0,    // 70002870 
0x0F128A72,    // 70002872 
0x0F122301,    // 70002874 
0x0F124368,    // 70002876 
0x0F121889,    // 70002878 
0x0F1217C2,    // 7000287A 
0x0F120E12,    // 7000287C 
0x0F121810,    // 7000287E 
0x0F121202,    // 70002880 
0x0F128820,    // 70002882 
0x0F12029B,    // 70002884 
0x0F1218C0,    // 70002886 
0x0F12F000,    // 70002888 
0x0F12F98C,    // 7000288A 
0x0F129000,    // 7000288C 
0x0F128AF6,    // 7000288E 
0x0F124268,    // 70002890 
0x0F12210A,    // 70002892 
0x0F124370,    // 70002894 
0x0F12F000,    // 70002896 
0x0F12F98D,    // 70002898 
0x0F12436E,    // 7000289A 
0x0F120007,    // 7000289C 
0x0F12210A,    // 7000289E 
0x0F120030,    // 700028A0 
0x0F12F000,    // 700028A2 
0x0F12F987,    // 700028A4 
0x0F129A00,    // 700028A6 
0x0F120039,    // 700028A8 
0x0F12F000,    // 700028AA 
0x0F12F989,    // 700028AC 
0x0F120002,    // 700028AE 
0x0F128820,    // 700028B0 
0x0F121880,    // 700028B2 
0x0F128020,    // 700028B4 
0x0F1248A4,    // 700028B6 
0x0F1288C1,    // 700028B8 
0x0F1248A2,    // 700028BA 
0x0F123820,    // 700028BC 
0x0F128B40,    // 700028BE 
0x0F124240,    // 700028C0 
0x0F124350,    // 700028C2 
0x0F12F000,    // 700028C4 
0x0F12F976,    // 700028C6 
0x0F128861,    // 700028C8 
0x0F121840,    // 700028CA 
0x0F128060,    // 700028CC 
0x0F12BCF8,    // 700028CE 
0x0F12BC08,    // 700028D0 
0x0F124718,    // 700028D2 
0x0F12B570,    // 700028D4 
0x0F124C9B,    // 700028D6 
0x0F123C20,    // 700028D8 
0x0F128B20,    // 700028DA 
0x0F12F000,    // 700028DC 
0x0F12F978,    // 700028DE 
0x0F124D99,    // 700028E0 
0x0F1280E8,    // 700028E2 
0x0F128B60,    // 700028E4 
0x0F12F000,    // 700028E6 
0x0F12F97B,    // 700028E8 
0x0F128128,    // 700028EA 
0x0F12BC70,    // 700028EC 
0x0F12BC08,    // 700028EE 
0x0F124718,    // 700028F0 
0x0F12B508,    // 700028F2 
0x0F124895,    // 700028F4 
0x0F124669,    // 700028F6 
0x0F12F000,    // 700028F8 
0x0F12F97A,    // 700028FA 
0x0F124894,    // 700028FC 
0x0F12214D,    // 700028FE 
0x0F128201,    // 70002900 
0x0F122196,    // 70002902 
0x0F128281,    // 70002904 
0x0F12211D,    // 70002906 
0x0F128301,    // 70002908 
0x0F12F7FF,    // 7000290A 
0x0F12FFE3,    // 7000290C 
0x0F12F000,    // 7000290E 
0x0F12F977,    // 70002910 
0x0F12466B,    // 70002912 
0x0F128818,    // 70002914 
0x0F128859,    // 70002916 
0x0F121A40,    // 70002918 
0x0F12498E,    // 7000291A 
0x0F126348,    // 7000291C 
0x0F12488A,    // 7000291E 
0x0F129900,    // 70002920 
0x0F123876,    // 70002922 
0x0F12F000,    // 70002924 
0x0F12F974,    // 70002926 
0x0F12466B,    // 70002928 
0x0F12488A,    // 7000292A 
0x0F128819,    // 7000292C 
0x0F123080,    // 7000292E 
0x0F1284C1,    // 70002930 
0x0F128859,    // 70002932 
0x0F128501,    // 70002934 
0x0F124987,    // 70002936 
0x0F122000,    // 70002938 
0x0F123920,    // 7000293A 
0x0F127088,    // 7000293C 
0x0F123140,    // 7000293E 
0x0F127388,    // 70002940 
0x0F12B001,    // 70002942 
0x0F12BC08,    // 70002944 
0x0F124718,    // 70002946 
0x0F12B570,    // 70002948 
0x0F120004,    // 7000294A 
0x0F126820,    // 7000294C 
0x0F126865,    // 7000294E 
0x0F12F000,    // 70002950 
0x0F12F966,    // 70002952 
0x0F120402,    // 70002954 
0x0F124880,    // 70002956 
0x0F120C12,    // 70002958 
0x0F128142,    // 7000295A 
0x0F12487F,    // 7000295C 
0x0F128801,    // 7000295E 
0x0F122900,    // 70002960 
0x0F12D008,    // 70002962 
0x0F12497E,    // 70002964 
0x0F12002B,    // 70002966 
0x0F126D8A,    // 70002968 
0x0F122105,    // 7000296A 
0x0F121C80,    // 7000296C 
0x0F12F000,    // 7000296E 
0x0F12F95F,    // 70002970 
0x0F126020,    // 70002972 
0x0F12E005,    // 70002974 
0x0F12487B,    // 70002976 
0x0F12002B,    // 70002978 
0x0F122105,    // 7000297A 
0x0F12F000,    // 7000297C 
0x0F12F958,    // 7000297E 
0x0F126020,    // 70002980 
0x0F126820,    // 70002982 
0x0F12E7B2,    // 70002984 
0x0F12B5F8,    // 70002986 
0x0F124975,    // 70002988 
0x0F122200,    // 7000298A 
0x0F123160,    // 7000298C 
0x0F1283CA,    // 7000298E 
0x0F126800,    // 70002990 
0x0F124669,    // 70002992 
0x0F12F000,    // 70002994 
0x0F12F92C,    // 70002996 
0x0F12466B,    // 70002998 
0x0F128818,    // 7000299A 
0x0F12F000,    // 7000299C 
0x0F12F918,    // 7000299E 
0x0F120005,    // 700029A0 
0x0F12466B,    // 700029A2 
0x0F128858,    // 700029A4 
0x0F12F000,    // 700029A6 
0x0F12F91B,    // 700029A8 
0x0F120004,    // 700029AA 
0x0F122101,    // 700029AC 
0x0F121928,    // 700029AE 
0x0F1202C9,    // 700029B0 
0x0F121A08,    // 700029B2 
0x0F120286,    // 700029B4 
0x0F120029,    // 700029B6 
0x0F120030,    // 700029B8 
0x0F12F000,    // 700029BA 
0x0F12F941,    // 700029BC 
0x0F120005,    // 700029BE 
0x0F122701,    // 700029C0 
0x0F1202BF,    // 700029C2 
0x0F120021,    // 700029C4 
0x0F120030,    // 700029C6 
0x0F12F000,    // 700029C8 
0x0F12F93A,    // 700029CA 
0x0F124964,    // 700029CC 
0x0F124A61,    // 700029CE 
0x0F123140,    // 700029D0 
0x0F1232A0,    // 700029D2 
0x0F12800D,    // 700029D4 
0x0F128395,    // 700029D6 
0x0F12804F,    // 700029D8 
0x0F1283D7,    // 700029DA 
0x0F128088,    // 700029DC 
0x0F120011,    // 700029DE 
0x0F123120,    // 700029E0 
0x0F128008,    // 700029E2 
0x0F12E773,    // 700029E4 
0x0F12B510,    // 700029E6 
0x0F12485C,    // 700029E8 
0x0F128980,    // 700029EA 
0x0F122800,    // 700029EC 
0x0F12D001,    // 700029EE 
0x0F12F000,    // 700029F0 
0x0F12F92C,    // 700029F2 
0x0F12E734,    // 700029F4 
0x0F12B5FE,    // 700029F6 
0x0F122600,    // 700029F8 
0x0F12495B,    // 700029FA 
0x0F122000,    // 700029FC 
0x0F126108,    // 700029FE 
0x0F1260C8,    // 70002A00 
0x0F12485A,    // 70002A02 
0x0F128901,    // 70002A04 
0x0F124852,    // 70002A06 
0x0F123020,    // 70002A08 
0x0F1288C0,    // 70002A0A 
0x0F129000,    // 70002A0C 
0x0F120840,    // 70002A0E 
0x0F129002,    // 70002A10 
0x0F124856,    // 70002A12 
0x0F128800,    // 70002A14 
0x0F129001,    // 70002A16 
0x0F124853,    // 70002A18 
0x0F123040,    // 70002A1A 
0x0F128B05,    // 70002A1C 
0x0F122900,    // 70002A1E 
0x0F12D01F,    // 70002A20 
0x0F124E52,    // 70002A22 
0x0F122700,    // 70002A24 
0x0F1280F7,    // 70002A26 
0x0F122400,    // 70002A28 
0x0F12E016,    // 70002A2A 
0x0F12494E,    // 70002A2C 
0x0F120060,    // 70002A2E 
0x0F1239C0,    // 70002A30 
0x0F121841,    // 70002A32 
0x0F122020,    // 70002A34 
0x0F125E08,    // 70002A36 
0x0F129902,    // 70002A38 
0x0F121840,    // 70002A3A 
0x0F129900,    // 70002A3C 
0x0F12F000,    // 70002A3E 
0x0F12F8FF,    // 70002A40 
0x0F124B4A,    // 70002A42 
0x0F120202,    // 70002A44 
0x0F1200A1,    // 70002A46 
0x0F12330C,    // 70002A48 
0x0F12505A,    // 70002A4A 
0x0F120002,    // 70002A4C 
0x0F124342,    // 70002A4E 
0x0F120210,    // 70002A50 
0x0F121DDA,    // 70002A52 
0x0F1232F9,    // 70002A54 
0x0F125050,    // 70002A56 
0x0F121C64,    // 70002A58 
0x0F1242A5,    // 70002A5A 
0x0F12DCE6,    // 70002A5C 
0x0F128137,    // 70002A5E 
0x0F12E042,    // 70002A60 
0x0F122400,    // 70002A62 
0x0F12E031,    // 70002A64 
0x0F124940,    // 70002A66 
0x0F120060,    // 70002A68 
0x0F1239C0,    // 70002A6A 
0x0F121841,    // 70002A6C 
0x0F122020,    // 70002A6E 
0x0F125E08,    // 70002A70 
0x0F129902,    // 70002A72 
0x0F121840,    // 70002A74 
0x0F129900,    // 70002A76 
0x0F12F000,    // 70002A78 
0x0F12F8E2,    // 70002A7A 
0x0F124A3B,    // 70002A7C 
0x0F1200A1,    // 70002A7E 
0x0F12320C,    // 70002A80 
0x0F125853,    // 70002A82 
0x0F123A0C,    // 70002A84 
0x0F128852,    // 70002A86 
0x0F124353,    // 70002A88 
0x0F123380,    // 70002A8A 
0x0F120A1F,    // 70002A8C 
0x0F1223FF,    // 70002A8E 
0x0F121C5B,    // 70002A90 
0x0F121A9B,    // 70002A92 
0x0F12469C,    // 70002A94 
0x0F124343,    // 70002A96 
0x0F1218FF,    // 70002A98 
0x0F124B34,    // 70002A9A 
0x0F12330C,    // 70002A9C 
0x0F12505F,    // 70002A9E 
0x0F120003,    // 70002AA0 
0x0F124343,    // 70002AA2 
0x0F124660,    // 70002AA4 
0x0F124343,    // 70002AA6 
0x0F124831,    // 70002AA8 
0x0F125840,    // 70002AAA 
0x0F124350,    // 70002AAC 
0x0F123080,    // 70002AAE 
0x0F124A2F,    // 70002AB0 
0x0F120A00,    // 70002AB2 
0x0F121818,    // 70002AB4 
0x0F125050,    // 70002AB6 
0x0F120039,    // 70002AB8 
0x0F124379,    // 70002ABA 
0x0F123180,    // 70002ABC 
0x0F120A09,    // 70002ABE 
0x0F124281,    // 70002AC0 
0x0F12D201,    // 70002AC2 
0x0F121A40,    // 70002AC4 
0x0F121986,    // 70002AC6 
0x0F121C64,    // 70002AC8 
0x0F1242A5,    // 70002ACA 
0x0F12DCCB,    // 70002ACC 
0x0F120868,    // 70002ACE 
0x0F121980,    // 70002AD0 
0x0F120029,    // 70002AD2 
0x0F12F000,    // 70002AD4 
0x0F12F8B4,    // 70002AD6 
0x0F123008,    // 70002AD8 
0x0F120900,    // 70002ADA 
0x0F129A01,    // 70002ADC 
0x0F124923,    // 70002ADE 
0x0F124290,    // 70002AE0 
0x0F12D901,    // 70002AE2 
0x0F128888,    // 70002AE4 
0x0F1280C8,    // 70002AE6 
0x0F124920,    // 70002AE8 
0x0F1288C8,    // 70002AEA 
0x0F122800,    // 70002AEC 
0x0F12D001,    // 70002AEE 
0x0F121E40,    // 70002AF0 
0x0F1280C8,    // 70002AF2 
0x0F129801,    // 70002AF4 
0x0F122800,    // 70002AF6 
0x0F12D00D,    // 70002AF8 
0x0F124A1B,    // 70002AFA 
0x0F123AC0,    // 70002AFC 
0x0F127B90,    // 70002AFE 
0x0F122802,    // 70002B00 
0x0F12D001,    // 70002B02 
0x0F122803,    // 70002B04 
0x0F12D106,    // 70002B06 
0x0F1288C9,    // 70002B08 
0x0F122900,    // 70002B0A 
0x0F12D003,    // 70002B0C 
0x0F122100,    // 70002B0E 
0x0F120040,    // 70002B10 
0x0F121880,    // 70002B12 
0x0F128081,    // 70002B14 
0x0F12BCFE,    // 70002B16 
0x0F12BC08,    // 70002B18 
0x0F124718,    // 70002B1A 
0x0F12B510,    // 70002B1C 
0x0F126800,    // 70002B1E 
0x0F12F000,    // 70002B20 
0x0F12F89C,    // 70002B22 
0x0F124911,    // 70002B24 
0x0F122001,    // 70002B26 
0x0F128108,    // 70002B28 
0x0F12E699,    // 70002B2A 
0x0F12B510,    // 70002B2C 
0x0F12F000,    // 70002B2E 
0x0F12F89D,    // 70002B30 
0x0F12F7FF,    // 70002B32 
0x0F12FF60,    // 70002B34 
0x0F12E693,    // 70002B36 
0x0F124140,    // 70002B38 
0x0F12D000,    // 70002B3A 
0x0F1219DC,    // 70002B3C 
0x0F127000,    // 70002B3E 
0x0F121B60,    // 70002B40 
0x0F127000,    // 70002B42 
0x0F120DD4,    // 70002B44 
0x0F127000,    // 70002B46 
0x0F1222AC,    // 70002B48 
0x0F127000,    // 70002B4A 
0x0F120F88,    // 70002B4C 
0x0F127000,    // 70002B4E 
0x0F121E8C,    // 70002B50 
0x0F127000,    // 70002B52 
0x0F12214C,    // 70002B54 
0x0F127000,    // 70002B56 
0x0F121A10,    // 70002B58 
0x0F127000,    // 70002B5A 
0x0F123780,    // 70002B5C 
0x0F127000,    // 70002B5E 
0x0F122384,    // 70002B60 
0x0F127000,    // 70002B62 
0x0F12065C,    // 70002B64 
0x0F127000,    // 70002B66 
0x0F121C8C,    // 70002B68 
0x0F127000,    // 70002B6A 
0x0F122C7C,    // 70002B6C 
0x0F127000,    // 70002B6E 
0x0F122D88,    // 70002B70 
0x0F127000,    // 70002B72 
0x0F124778,    // 70002B74 
0x0F1246C0,    // 70002B76 
0x0F12C000,    // 70002B78 
0x0F12E59F,    // 70002B7A 
0x0F12FF1C,    // 70002B7C 
0x0F12E12F,    // 70002B7E 
0x0F12CE77,    // 70002B80 
0x0F120000,    // 70002B82 
0x0F124778,    // 70002B84 
0x0F1246C0,    // 70002B86 
0x0F12C000,    // 70002B88 
0x0F12E59F,    // 70002B8A 
0x0F12FF1C,    // 70002B8C 
0x0F12E12F,    // 70002B8E 
0x0F123609,    // 70002B90 
0x0F120000,    // 70002B92 
0x0F124778,    // 70002B94 
0x0F1246C0,    // 70002B96 
0x0F12C000,    // 70002B98 
0x0F12E59F,    // 70002B9A 
0x0F12FF1C,    // 70002B9C 
0x0F12E12F,    // 70002B9E 
0x0F129F91,    // 70002BA0 
0x0F120000,    // 70002BA2 
0x0F124778,    // 70002BA4 
0x0F1246C0,    // 70002BA6 
0x0F12C000,    // 70002BA8 
0x0F12E59F,    // 70002BAA 
0x0F12FF1C,    // 70002BAC 
0x0F12E12F,    // 70002BAE 
0x0F122AE3,    // 70002BB0 
0x0F120000,    // 70002BB2 
0x0F124778,    // 70002BB4 
0x0F1246C0,    // 70002BB6 
0x0F12F004,    // 70002BB8 
0x0F12E51F,    // 70002BBA 
0x0F12D1DC,    // 70002BBC 
0x0F120000,    // 70002BBE 
0x0F124778,    // 70002BC0 
0x0F1246C0,    // 70002BC2 
0x0F12C000,    // 70002BC4 
0x0F12E59F,    // 70002BC6 
0x0F12FF1C,    // 70002BC8 
0x0F12E12F,    // 70002BCA 
0x0F126869,    // 70002BCC 
0x0F120000,    // 70002BCE 
0x0F124778,    // 70002BD0 
0x0F1246C0,    // 70002BD2 
0x0F12C000,    // 70002BD4 
0x0F12E59F,    // 70002BD6 
0x0F12FF1C,    // 70002BD8 
0x0F12E12F,    // 70002BDA 
0x0F1268BD,    // 70002BDC 
0x0F120000,    // 70002BDE 
0x0F124778,    // 70002BE0 
0x0F1246C0,    // 70002BE2 
0x0F12C000,    // 70002BE4 
0x0F12E59F,    // 70002BE6 
0x0F12FF1C,    // 70002BE8 
0x0F12E12F,    // 70002BEA 
0x0F1268DB,    // 70002BEC 
0x0F120000,    // 70002BEE 
0x0F124778,    // 70002BF0 
0x0F1246C0,    // 70002BF2 
0x0F12C000,    // 70002BF4 
0x0F12E59F,    // 70002BF6 
0x0F12FF1C,    // 70002BF8 
0x0F12E12F,    // 70002BFA 
0x0F121BC9,    // 70002BFC 
0x0F120000,    // 70002BFE 
0x0F124778,    // 70002C00 
0x0F1246C0,    // 70002C02 
0x0F12C000,    // 70002C04 
0x0F12E59F,    // 70002C06 
0x0F12FF1C,    // 70002C08 
0x0F12E12F,    // 70002C0A 
0x0F1271B9,    // 70002C0C 
0x0F120000,    // 70002C0E 
0x0F124778,    // 70002C10 
0x0F1246C0,    // 70002C12 
0x0F12C000,    // 70002C14 
0x0F12E59F,    // 70002C16 
0x0F12FF1C,    // 70002C18 
0x0F12E12F,    // 70002C1A 
0x0F127249,    // 70002C1C 
0x0F120000,    // 70002C1E 
0x0F124778,    // 70002C20 
0x0F1246C0,    // 70002C22 
0x0F12C000,    // 70002C24 
0x0F12E59F,    // 70002C26 
0x0F12FF1C,    // 70002C28 
0x0F12E12F,    // 70002C2A 
0x0F1298CD,    // 70002C2C 
0x0F120000,    // 70002C2E 
0x0F124778,    // 70002C30 
0x0F1246C0,    // 70002C32 
0x0F12C000,    // 70002C34 
0x0F12E59F,    // 70002C36 
0x0F12FF1C,    // 70002C38 
0x0F12E12F,    // 70002C3A 
0x0F12987F,    // 70002C3C 
0x0F120000,    // 70002C3E 
0x0F124778,    // 70002C40 
0x0F1246C0,    // 70002C42 
0x0F12F004,    // 70002C44 
0x0F12E51F,    // 70002C46 
0x0F12D378,    // 70002C48 
0x0F120000,    // 70002C4A 
0x0F124778,    // 70002C4C 
0x0F1246C0,    // 70002C4E 
0x0F12C000,    // 70002C50 
0x0F12E59F,    // 70002C52 
0x0F12FF1C,    // 70002C54 
0x0F12E12F,    // 70002C56 
0x0F1204CB,    // 70002C58 
0x0F120000,    // 70002C5A 
0x0F124778,    // 70002C5C 
0x0F1246C0,    // 70002C5E 
0x0F12C000,    // 70002C60 
0x0F12E59F,    // 70002C62 
0x0F12FF1C,    // 70002C64 
0x0F12E12F,    // 70002C66 
0x0F1250AF,    // 70002C68 
0x0F120000,    // 70002C6A 
0x0F124778,    // 70002C6C 
0x0F1246C0,    // 70002C6E 
0x0F12C000,    // 70002C70 
0x0F12E59F,    // 70002C72 
0x0F12FF1C,    // 70002C74 
0x0F12E12F,    // 70002C76 
0x0F125623,    // 70002C78 
0x0F120000,    // 70002C7A 
0x002A378C,
0x0F120000, // On/off register bUseOTP
0x002A0DCC,
0x0F120138, //awbb_IntcR
0x0F12011C, //awbb_IntcB
//===================================================================
//Analog & APS Control
//===================================================================
//WRITE    D000F2AC   0020 // analog gain x8: revised by Ana 090126 // 0100 x8, 0080 x4, 0040 x2, 0020 x1
0x0028D000,
0x002AF404,
0x0F120038, //D000F404		// aig_adc_sat[7:0] : 850mV, revised by Ana 20100524
0x0F120001, //D000F406		// aig_ms[2:0] : revised by Ana 20100202
0x0F12000C, //D000F408		// aig_sig_mx[5:0]
0x0F120006, //D000F40A		// aig_rst_mx[5:0]
0x0F120008, //D000F40C		// aig_rmp_option[3] SL_Low_PWR_SAVE On : revised by Ana 20100204
0x002AF418, 					  
0x0F120003, //D000F418		// aig_dbr_clk_sel[1:0] : revised by Ana 20100201
0x002AF41C, 					  
0x0F120140, //D000F41C		// aig_bist_sig_width_e[10:0]
0x0F120140, //D000F41E		// aig_bist_sig_width_o[10:0]
0x0F120066, //D000F420		// aig_bist_sig_width_o[10:0]
0x0F120005, //D000F422		// aig_pix_bias[3:0]
0x002AF426, 					  
0x0F1200D4, //D000F426		// aig_clp_lvl[7:0]
0x002AF42A, 					  
0x0F120001, //D000F42A		// aig_ref_option[0] SL_Low_PWR_SAVE On : revised by Ana 20100204
0x002AF430, 					  
0x0F120001, //D000F430		// aig_pd_cp_rosc[0] : revised by Ana 20100201
0x0F120001, //D000F432		// aig_pd_ncp_rosc[0] : revised by Ana 20100201
0x002AF43A, 					  
0x0F120000, //D000F43A		// aig_pd_fblv[0] : revised by Ana 20100203
0x002AF440, 					  
0x0F120044, //D000F440		// aig_rosc_tune_ncp[7:4] // aig_rosc_tune_cp[3:0]
0x002AF44A, 			
0x0F120000, 				// aig_fb_lv[1:0] : revised by Ana 20100204
0x002AF45C, 	  //D000F44A	  
0x0F120000, 				// aig_dshut_en[0] : revised by APS 20100223
0x0F120000, //D000F45C		// aig_srx_en[0]
0x002AF462, 	  //D000F45E	  
0x0F120001, 				// aig_pdb_atop[0]
0x002AF46E, 	  //D000F462	  
0x0F121F02, 				// aig_cds_test[15:0]
0x002AF474, 	  //D000F46E	  
0x0F12000E, 				// aig_stx_gap[4:0]
		//D000F474		
0x002AE42E,
0x0F120004, // adlc_qec[2:0] : revised by Dithered L-ADLC Designer 20100203

//===================================================================
// AAC Enable
//===================================================================
0x00287000,
0x002A13E0,
0x0F120000, //700013E0 //senHal_bSRX	//SRX OFF
0x002A13C8,
0x0F120001, //700013C8 //senHal_bSRX	//AAC Enable
0x002A12D8,
0x0F120464, //700012D8
0x0F120468, //700012DA
0x002A12F6,
0x0F120000, //700012F6
0x002A13CC, //senHal_bSen11ADLC
0x0F121FC0, // [12:0] : Write tuning value to E404 register
0x002A13EC, //senHal_DarklevelTuneMode
0x0F120001,
0x002A184C,
0x0F121EE1,
0x0028D000,
0x002A1000,
0x0F120001,
0x00287000,
0x002A040E,
0x0F120003, //STBY TnP enable setting STBY TnP enable setting [0] bit 는 STBY enable,[1] bit 는 STBY 복구시 이전에 올린 TnP 도 복구하는 on/off bit
0xFFFE000A,
//H-Digital binning & V-PLA
0x002A1218,
0x0F120002, //subsampling number.
0x0F120002, //subsampling number.
0x002A0C9A,
0x0F120001, // setot_bUseDigitalHbin  0 : do not use bin block, 1 : use binning block
0x002A1438,
0x0F12F468, // senHal_TuneStr_AngTuneData1[0]  : tuning register setting.
0x0F120000, // not use binninb block 0x0000 ,use binnin blcok 0x0000
0x0F120008, // not use binninb block 0x000A ,use binnin blcok 0x0008
0x0F120006, // not use binninb block 0x0005 ,use binnin blcok 0x0006
0x0F120000, // 0x0000 , 0x0000
0xFFFE000A,
// FILESAVE('E:\0_작업장\02_5BB평가\0206\frame_.jpg',5)
// Backup Register
0x002A0416,
0x0F12F400, 	//skl_usStbyBackupReg[0][0]
0x0F120074, 	//skl_usStbyBackupReg[0][1]
0x0F12E42E, 	//skl_usStbyBackupReg[1][0]
0x0F120030, 	//skl_usStbyBackupReg[1][1]

0x002A0F88,
0x0F120210, //0380,	//awbb_GainsInit[0]                         	2   70000F88
0x0F1201c0, //0400,	//awbb_GainsInit[1]                         	2   70000F8A
0x0F120300, //04A0,	//awbb_GainsInit[2]                         	2   70000F8C


//===================================================================
//Tuning Point - FAE
//===================================================================
//===================================================================
// Init Parameters	 MCLK : 24M sys:17.5M pclk :35M
//===================================================================
//CLK Settings
//Input CLK
0x00287000,
0x002A00F4,
0x0F125DC0, //REG_TC_IPRM_InClockLSBs						2	700000F4  //5DC0:24000Khz Mclk
0x0F120000, //REG_TC_IPRM_InClockMSBs						2	700000F6
0x002A0110,
0x0F120002, //REG_TC_IPRM_UseNPviClocks 					2	70000110  //Parallel mode Clock setting (up to 3 cases)
0x0F120000, //REG_TC_IPRM_bBlockInternalPllCalc 			2	70000112  //0:Auto Cal. 1:Manual Cal.
//SYSTEM CLOCK
0x0F12222E, //REG_TC_IPRM_OpClk4KHz_0						2	70000114  //35000Khz/4Khz=222E
0x0F12445C, //REG_TC_IPRM_MinOutRate4KHz_0					2	70000116
0x0F12445C, //REG_TC_IPRM_MaxOutRate4KHz_0					2	70000118
0x0F12222E, //REG_TC_IPRM_OpClk4KHz_1						2	7000011A  
0x0F12222E, //REG_TC_IPRM_MinOutRate4KHz_1					2	7000011C   36000khz/4 =2328hex
0x0F12222E, //REG_TC_IPRM_MaxOutRate4KHz_1					2	7000011E
0x002A0126,
0x0F120001, //REG_TC_IPRM_InitParamsUpdated 				2	70000126
0xFFFE0064,
0x002A1218,
0x0F120002,

0x002A1220,         
0x0F12445C, //expmin

//===================================================================
//Preview0 1600x1200 system 32M PCLK 70M From APS
//===================================================================
//Crop Preview
0x002A0144,
0x0F120640, //70000144	//REG_TC_GP_PreReqInputWidth	0640:1600
0x0F1204B0, //70000146	//REG_TC_GP_PreReqInputHeight	04B0:1200
0x0F120000, //70000148	//REG_TC_GP_PreInputWidthOfs
0x0F120000, //7000014A	//REG_TC_GP_PreInputHeightOfs
//Preview Mode
//Preview0 for 7.5~15fps
0x002A0170,
0x0F120286, //0280	//REG_1TC_PCFG_usWidth							2	70000198  640
0x0F1201E6, //01E0	//REG_1TC_PCFG_usHeight 						2	7000019A  480
0x0F120005, //0005	//0005	//REG_0TC_PCFG_Format							2	70000174  0 RGB565; 1 RGB888; 5 Full YUV422; 6 Reduced YUV422; 7 Bayer
0x0F12222E, //445C	//222E	//REG_0TC_PCFG_usMaxOut4KHzRate 				2	70000176
0x0F12222E, //445C	//222E	//REG_0TC_PCFG_usMinOut4KHzRate 				2	70000178
0x0F120052, //0042	//0042	//REG_0TC_PCFG_PVIMask							2	7000017A
0x0F120010, //0010	//0010	//REG_0TC_PCFG_OIFMask							2	7000017C
0x0F120001, //0000	//0001	//REG_0TC_PCFG_uClockInd						2	7000017E
0x0F120000, //0000	//0000	//REG_0TC_PCFG_usFrTimeType 					2	70000180  0: dynamic 1:fix not accurate 2: fixed_Accurate
0x0F120001, //0000	//0001	//REG_0TC_PCFG_FrRateQualityType				2	70000182  1b: FR (bin) 2b: Quality (no-bin)
0x0F120535, //0535	//0535	//REG_0TC_PCFG_usMaxFrTimeMsecMult10			2	70000184  max frame time : 30fps:014D 15fps:029a 7.5fps:0535 3.75fps:a6a
//0F12029a //bb	//lyg //60Hz	//REG_0TC_PCFG_usMinFrTimeMsecMult10			2	70000186  max frame time : 30fps:014D 15fps:029a 7.5fps:0535 3.75fps:a6a
0x0F1202BC,	//50Hz	//REG_0TC_PCFG_usMinFrTimeMsecMult10			2	70000186  max frame time : 30fps:014D 15fps:029a 7.5fps:0535 3.75fps:a6a
0x0F120000, //0000	//0000	//REG_0TC_PCFG_bSmearOutput 					2	70000188
0x0F120000, //0000	//0000	//REG_0TC_PCFG_sSaturation						2	7000018A
0x0F120000, //0000	//0000	//REG_0TC_PCFG_sSharpBlur						2	7000018C
0x0F120000, //0000	//0000	//REG_0TC_PCFG_sColorTemp						2	7000018E
0x0F120000, //0000	//0000	//REG_0TC_PCFG_uDeviceGammaIndex				2	70000190  1:Mirror(X) 2:Mirror(Y) 4:STAT Mirror(X) 8:STAT Mirror(Y)
0x0F120000, //0000	//0001	//REG_0TC_PCFG_uPrevMirror						2	70000192
0x0F120000, //0000	//0001	//REG_0TC_PCFG_uCaptureMirror					2	70000194
0x0F120000, //0000	//0000	//REG_0TC_PCFG_uRotation						2	70000196

//Capture Mode
0x002A0238,
0x0F120001, //REG_0TC_CCFG_uCaptureMode 					2	70000238
0x0F120640, //REG_0TC_CCFG_usWidth							2	7000023A
0x0F1204B0, //REG_0TC_CCFG_usHeight 						2	7000023C
0x0F120005, //REG_0TC_CCFG_Format							2	7000023E
0x0F12222E, //REG_0TC_CCFG_usMaxOut4KHzRate 				2	70000240
0x0F12222E, //REG_0TC_CCFG_usMinOut4KHzRate 				2	70000242
0x0F120052, //REG_0TC_CCFG_PVIMask							2	70000244
0x0F120000, //REG_0TC_CCFG_OIFMask							2	70000246
0x0F120001, //REG_0TC_CCFG_uClockInd						2	70000248
0x0F120002, //REG_0TC_CCFG_usFrTimeType 					2	7000024A
0x0F120002, //REG_0TC_CCFG_FrRateQualityType				2	7000024C
0x0F120514, //0535	//REG_0TC_CCFG_usMaxFrTimeMsecMult10			2	7000024E
0x0F120514, //0535	//REG_0TC_CCFG_usMinFrTimeMsecMult10			2	70000250
0x0F120000, //REG_0TC_CCFG_bSmearOutput 					2	70000252
0x0F120000, //REG_0TC_CCFG_sSaturation						2	70000254
0x0F120000, //REG_0TC_CCFG_sSharpBlur						2	70000256
0x0F120000, //REG_0TC_CCFG_sColorTemp						2	70000258
0x0F120000, //REG_0TC_CCFG_uDeviceGammaIndex				2	7000025A
0x0F120001, //REG_1TC_CCFG_uCaptureMode 					2	7000025C
0x0F120640, //REG_1TC_CCFG_usWidth							2	7000025E
0x0F1204B0, //REG_1TC_CCFG_usHeight 						2	70000260
0x0F120005, //REG_1TC_CCFG_Format							2	70000262
0x0F12445C,//222E	//REG_1TC_CCFG_usMaxOut4KHzRate 				2	70000264
0x0F12445C,//222E	//REG_1TC_CCFG_usMinOut4KHzRate 				2	70000266
0x0F120052, //REG_1TC_CCFG_PVIMask							2	70000268
0x0F120010,//0000	//REG_1TC_CCFG_OIFMask							2	7000026A
0x0F120000,//0001	//REG_1TC_CCFG_uClockInd						2	7000026C
0x0F120000, //REG_1TC_CCFG_usFrTimeType 					2	7000026E
0x0F120002, //REG_1TC_CCFG_FrRateQualityType				2	70000270
0x0F121388, //REG_1TC_CCFG_usMaxFrTimeMsecMult10			2	70000272
0x0F121388, //REG_1TC_CCFG_usMinFrTimeMsecMult10			2	70000274
0x0F120000, //REG_1TC_CCFG_bSmearOutput 					2	70000276
0x0F120000, //REG_1TC_CCFG_sSaturation						2	70000278
0x0F120000, //REG_1TC_CCFG_sSharpBlur						2	7000027A
0x0F120000, //REG_1TC_CCFG_sColorTemp						2	7000027C
0x0F120000, //REG_1TC_CCFG_uDeviceGammaIndex				2	7000027E
0x002A1218,
0x0F120002, 	//senHal_SenBinFactor
//PREVIEW
0x002A0156,
0x0F120000, 	//REG_TC_GP_ActivePrevConfig
0x002A015E,
0x0F120000, 	//REG_TC_GP_ActiveCapConfig
0x002A015A,
0x0F120001, 	//REG_TC_GP_PrevOpenAfterChange
0x002A0142,
0x0F120001, 	//REG_TC_GP_NewConfigSync
0x002A0158,
0x0F120001, 	//REG_TC_GP_PrevConfigChanged
0x002A0160,
0x0F120001, 	//REG_TC_GP_CapConfigChanged
0x002A013A,
0x0F120001, 	//REG_TC_GP_EnablePreview
0x0F120001, 	//REG_TC_GP_EnablePreviewChanged
0xFFFE0064,
//===================================================================
//AFC
//===================================================================
//Auto//
0x00287000,
0x002A0CC0,
#ifdef CONFIG_TARGET_LOCALE_KOR
0x0F120001, //AFC_Default60Hz	01:60hz 00:50Hz - Korea only
#else
0x0F120000, //AFC_Default50Hz	01:60hz 00:50Hz
#endif
0x002A0374,
0x0F12067F, //REG_TC_DBG_AutoAlgEnBits
//===================================================================
//Shading
//===================================================================
// TVAR_ash_pGAS_high
0x00287000,
0x002A0AD8,
0x0F120F00,
0x0F120F00,
0x0F120F00,
0x0F120F0F,
0x0F120F00,
0x0F120000,
0x0F12000F,
0x0F12000F,
0x0F120F00,
0x0F120F00,
0x0F120F00,
0x0F12000F,
0x0F12000F,
0x0F120F00,
0x0F12000F,
0x0F120000,
0x0F12000F,
0x0F120F00,
0x0F120F00,
0x0F120000,
0x0F120F00,
0x0F120F0F,
0x0F120F00,
0x0F120F00,
0x0F12000F,
0x0F12000F,
0x0F12000F,
0x0F120F00,
0x0F120F00,
0x0F120F0F,
0x0F12000F,
0x0F120F0F,
0x0F12000F,
0x0F12000F,
0x0F12000F,
0x0F120F0F,
0x0F120F00,
0x0F120F00,
0x0F120F00,
0x0F120F0F,
0x0F120F00,
0x0F12000F,
0x0F12000F,
0x0F12000F,
0x0F120F00,
0x0F120F00,
0x0F120F00,
0x0F120000,
0x0F12000F,
0x0F120000,
0x0F120F0F,
0x0F120000,
0x0F12000F,
0x0F120F00,
0x0F120F00,
0x0F120F00,
0x0F120F00,
0x0F120F0F,
0x0F120F00,
0x0F120000,
0x0F12000F,
0x0F12000F,
0x0F120F00,
0x0F120F00,
0x0F120F00,
0x0F12000F,
0x0F12000F,
0x0F12000F,
0x0F120F0F,
0x0F12000F,
0x0F12000F,
0x0F120F00,
// TVAR_ash_pGAS_low
0x0F1271E4,
0x0F12F082,
0x0F12097C,
0x0F12FD84,
0x0F120E5E,
0x0F12EBF0,
0x0F12ECC3,
0x0F12F290,
0x0F120EB6,
0x0F12F363,
0x0F12040D,
0x0F1208C2,
0x0F12F5E9,
0x0F121413,
0x0F12EEAC,
0x0F120B60,
0x0F120494,
0x0F12EC15,
0x0F122370,
0x0F12EC2D,
0x0F120B2F,
0x0F12F90C,
0x0F12FDFB,
0x0F120D39,
0x0F12E8A0,
0x0F12049B,
0x0F120BA7,
0x0F12F81E,
0x0F12F779,
0x0F120D9A,
0x0F1202E1,
0x0F120B71,
0x0F12E455,
0x0F121660,
0x0F120472,
0x0F12E917,
0x0F12847F,
0x0F12EE1C,
0x0F120999,
0x0F120064,
0x0F120A35,
0x0F12EFD9,
0x0F12E7A8,
0x0F12EC10,
0x0F121208,
0x0F12F0AE,
0x0F1211DD,
0x0F12F627,
0x0F12F244,
0x0F1217C7,
0x0F12EE64,
0x0F1206BD,
0x0F12FC6F,
0x0F120264,
0x0F122E55,
0x0F12EA06,
0x0F120F00,
0x0F12FEE1,
0x0F12FC49,
0x0F12FC00,
0x0F12E64D,
0x0F120733,
0x0F12FDEB,
0x0F12FCA8,
0x0F12FEEF,
0x0F121127,
0x0F12FD6B,
0x0F120A8D,
0x0F12F189,
0x0F120C43,
0x0F12FF15,
0x0F12EE96,
0x0F126B41,
0x0F12EC28,
0x0F12134D,
0x0F12ED36,
0x0F121F65,
0x0F12E3EB,
0x0F12ED3B,
0x0F12FC39,
0x0F120395,
0x0F12FE95,
0x0F12F523,
0x0F1214B3,
0x0F12F64A,
0x0F120D0A,
0x0F12F555,
0x0F120C56,
0x0F12098C,
0x0F12DD6D,
0x0F12274F,
0x0F12EDE2,
0x0F120876,
0x0F12ECB6,
0x0F12019A,
0x0F12225F,
0x0F12E25E,
0x0F120245,
0x0F120F49,
0x0F120507,
0x0F12F16F,
0x0F12F711,
0x0F12062D,
0x0F120ECD,
0x0F12E0A6,
0x0F120FD8,
0x0F120B2F,
0x0F12F2E4,
0x0F12737E,
0x0F12F26D,
0x0F1208A1,
0x0F12FBCE,
0x0F1210C6,
0x0F12EB0D,
0x0F12EC6E,
0x0F12EF7E,
0x0F12121D,
0x0F12F26F,
0x0F1200BF,
0x0F120CC3,
0x0F12EFE2,
0x0F121C2A,
0x0F12E442,
0x0F121190,
0x0F120B58,
0x0F12DF9A,
0x0F122F82,
0x0F12DF8F,
0x0F121E5C,
0x0F12E8B4,
0x0F12F931,
0x0F122029,
0x0F12E4CA,
0x0F120B17,
0x0F12F97F,
0x0F1209CC,
0x0F12F9FA,
0x0F12FB87,
0x0F12FD1B,
0x0F120EFC,
0x0F12E892,
0x0F121224,
0x0F12007C,
0x0F12F30A,

0x002A0378,
0x0F120001, //REG_TC_DBG_RelnitCmd
//===================================================================
//Shading - Alpha
//===================================================================
0x002A05E8,
0x0F120090, //00E4TVAR_ash_AwbAshCord[0]						2	70000568
0x0F1200C0, //00F0TVAR_ash_AwbAshCord[1]						2	7000056A
0x0F120100, //0100TVAR_ash_AwbAshCord[2]						2	7000056C
0x0F120120, //0120TVAR_ash_AwbAshCord[3]						2	7000056E
0x0F120140, //0150TVAR_ash_AwbAshCord[4]						2	70000570
0x0F120160, //0180TVAR_ash_AwbAshCord[5]						2	70000572
0x0F12017E, //01A0TVAR_ash_AwbAshCord[6]						2	70000574

0x002A05FE,
0x0F124000, //4000 //TVAR_ash_GASAlpha[0][0]						2	7000057E
0x0F124000, //4000 //TVAR_ash_GASAlpha[0][1]						2	70000580
0x0F124000, //4000 //TVAR_ash_GASAlpha[0][2]						2	70000582
0x0F123800, //3800 //TVAR_ash_GASAlpha[0][3]						2	70000584
0x0F124000, //4000 //TVAR_ash_GASAlpha[1][0]						2	70000586
0x0F124000, //4000 //TVAR_ash_GASAlpha[1][1]						2	70000588
0x0F124000, //4000 //TVAR_ash_GASAlpha[1][2]						2	7000058A
0x0F123800, //3800 //TVAR_ash_GASAlpha[1][3]						2	7000058C
0x0F123500, //3800 //TVAR_ash_GASAlpha[2][0]						2	7000058E
0x0F124000, //4000 //TVAR_ash_GASAlpha[2][1]						2	70000590
0x0F124000, //4000 //TVAR_ash_GASAlpha[2][2]						2	70000592
0x0F123800, //3800 //TVAR_ash_GASAlpha[2][3]						2	70000594
0x0F123500, //3800 //TVAR_ash_GASAlpha[3][0]						2	70000596
0x0F124000, //4000 //TVAR_ash_GASAlpha[3][1]						2	70000598
0x0F124000, //4000 //TVAR_ash_GASAlpha[3][2]						2	7000059A
0x0F123A00, //3C00 //TVAR_ash_GASAlpha[3][3]						2	7000059C
0x0F123C00, //3800 //TVAR_ash_GASAlpha[4][0]						2	7000059E
0x0F124000, //4000 //TVAR_ash_GASAlpha[4][1]						2	700005A0
0x0F124000, //4000 //TVAR_ash_GASAlpha[4][2]						2	700005A2
0x0F123A00, //3C00 //TVAR_ash_GASAlpha[4][3]						2	700005A4
0x0F123C00, //3C00 //TVAR_ash_GASAlpha[5][0]						2	700005A6
0x0F124000, //4000 //TVAR_ash_GASAlpha[5][1]						2	700005A8
0x0F124000, //4000 //TVAR_ash_GASAlpha[5][2]						2	700005AA
0x0F124000, //4000 //TVAR_ash_GASAlpha[5][3]						2	700005AC
0x0F123C00, //3C00 //TVAR_ash_GASAlpha[6][0]						2	700005AE
0x0F124000, //4000 //TVAR_ash_GASAlpha[6][1]						2	700005B0
0x0F124000, //4000 //TVAR_ash_GASAlpha[6][2]						2	700005B2
0x0F124000, //4000 //TVAR_ash_GASAlpha[6][3]						2	700005B4
0x0F123E00, //TVAR_ash_GASOutdoorAlpha[0]					2	700005B6
0x0F124000, //TVAR_ash_GASOutdoorAlpha[1]					2	700005B8
0x0F124000, //TVAR_ash_GASOutdoorAlpha[2]					2	700005BA
0x0F124000, //TVAR_ash_GASOutdoorAlpha[3]					2	700005BC
//===================================================================
//Gamma
//===================================================================
//	param_start SARR_usGammaLutRGBIndoor
//Gamma Indoor R
0x002A0460,
0x0F120000, //0000	//0000	//saRR_usDualGammaLutRGBIndoor[0][0]
0x0F120004, //0008	//0002	//saRR_usDualGammaLutRGBIndoor[0][1]
0x0F12000C, //0013	//0006	//saRR_usDualGammaLutRGBIndoor[0][2]
0x0F120024, //002C	//0011	//saRR_usDualGammaLutRGBIndoor[0][3]
0x0F12006E, //0061	//0036	//saRR_usDualGammaLutRGBIndoor[0][4]
0x0F1200D1, //00C8	//009A	//saRR_usDualGammaLutRGBIndoor[0][5]
0x0F120119, //0113	//00FD	//saRR_usDualGammaLutRGBIndoor[0][6]
0x0F120139, //0132	//0129	//saRR_usDualGammaLutRGBIndoor[0][7]
0x0F120157, //014C	//014B	//saRR_usDualGammaLutRGBIndoor[0][8]
0x0F12018E, //0179	//0184	//saRR_usDualGammaLutRGBIndoor[0][9]
0x0F1201C3, //01A4	//01B8	//saRR_usDualGammaLutRGBIndoor[0][10]
0x0F1201F3, //01CD	//01EA	//saRR_usDualGammaLutRGBIndoor[0][11]
0x0F12021F, //01F4	//0216	//saRR_usDualGammaLutRGBIndoor[0][12]
0x0F120269, //0239	//025E	//saRR_usDualGammaLutRGBIndoor[0][13]
0x0F1202A6, //0278	//0299	//saRR_usDualGammaLutRGBIndoor[0][14]
0x0F1202FF, //02E0	//02F9	//saRR_usDualGammaLutRGBIndoor[0][15]
0x0F120351, //0333	//0341	//saRR_usDualGammaLutRGBIndoor[0][16]
0x0F120395, //037B	//037F	//saRR_usDualGammaLutRGBIndoor[0][17]
0x0F1203CE, //03BF	//03BF	//saRR_usDualGammaLutRGBIndoor[0][18]
0x0F1203FF, //03FF	//03FF	//saRR_usDualGammaLutRGBIndoor[0][19]
//Gamma Indoor G
0x0F120000, //0000	//0000	//saRR_usDualGammaLutRGBIndoor[1][0]
0x0F120004, //0008	//0002	//saRR_usDualGammaLutRGBIndoor[1][1]
0x0F12000C, //0013	//0006	//saRR_usDualGammaLutRGBIndoor[1][2]
0x0F120024, //002C	//0011	//saRR_usDualGammaLutRGBIndoor[1][3]
0x0F12006E, //0061	//0036	//saRR_usDualGammaLutRGBIndoor[1][4]
0x0F1200D1, //00C8	//009A	//saRR_usDualGammaLutRGBIndoor[1][5]
0x0F120119, //0113	//00FD	//saRR_usDualGammaLutRGBIndoor[1][6]
0x0F120139, //0132	//0129	//saRR_usDualGammaLutRGBIndoor[1][7]
0x0F120157, //014C	//014B	//saRR_usDualGammaLutRGBIndoor[1][8]
0x0F12018E, //0179	//0184	//saRR_usDualGammaLutRGBIndoor[1][9]
0x0F1201C3, //01A4	//01B8	//saRR_usDualGammaLutRGBIndoor[1][10]
0x0F1201F3, //01CD	//01EA	//saRR_usDualGammaLutRGBIndoor[1][11]
0x0F12021F, //01F4	//0216	//saRR_usDualGammaLutRGBIndoor[1][12]
0x0F120269, //0239	//025E	//saRR_usDualGammaLutRGBIndoor[1][13]
0x0F1202A6, //0278	//0299	//saRR_usDualGammaLutRGBIndoor[1][14]
0x0F1202FF, //02E0	//02F9	//saRR_usDualGammaLutRGBIndoor[1][15]
0x0F120351, //0333	//0341	//saRR_usDualGammaLutRGBIndoor[1][16]
0x0F120395, //037B	//037F	//saRR_usDualGammaLutRGBIndoor[1][17]
0x0F1203CE, //03BF	//03BF	//saRR_usDualGammaLutRGBIndoor[1][18]
0x0F1203FF, //03FF	//03FF	//saRR_usDualGammaLutRGBIndoor[1][19]
//Gamma Indoor B
0x0F120000, //0000	//0000	//saRR_usDualGammaLutRGBIndoor[2][0]
0x0F120004, //0008	//0002	//saRR_usDualGammaLutRGBIndoor[2][1]
0x0F12000C, //0013	//0006	//saRR_usDualGammaLutRGBIndoor[2][2]
0x0F120024, //002C	//0011	//saRR_usDualGammaLutRGBIndoor[2][3]
0x0F12006E, //0061	//0036	//saRR_usDualGammaLutRGBIndoor[2][4]
0x0F1200D1, //00C8	//009A	//saRR_usDualGammaLutRGBIndoor[2][5]
0x0F120119, //0113	//00FD	//saRR_usDualGammaLutRGBIndoor[2][6]
0x0F120139, //0132	//0129	//saRR_usDualGammaLutRGBIndoor[2][7]
0x0F120157, //014C	//014B	//saRR_usDualGammaLutRGBIndoor[2][8]
0x0F12018E, //0179	//0184	//saRR_usDualGammaLutRGBIndoor[2][9]
0x0F1201C3, //01A4	//01B8	//saRR_usDualGammaLutRGBIndoor[2][10]
0x0F1201F3, //01CD	//01EA	//saRR_usDualGammaLutRGBIndoor[2][11]
0x0F12021F, //01F4	//0216	//saRR_usDualGammaLutRGBIndoor[2][12]
0x0F120269, //0239	//025E	//saRR_usDualGammaLutRGBIndoor[2][13]
0x0F1202A6, //0278	//0299	//saRR_usDualGammaLutRGBIndoor[2][14]
0x0F1202FF, //02E0	//02F9	//saRR_usDualGammaLutRGBIndoor[2][15]
0x0F120351, //0333	//0341	//saRR_usDualGammaLutRGBIndoor[2][16]
0x0F120395, //037B	//037F	//saRR_usDualGammaLutRGBIndoor[2][17]
0x0F1203CE, //03BF	//03BF	//saRR_usDualGammaLutRGBIndoor[2][18]
0x0F1203FF, //03FF	//03FF	//saRR_usDualGammaLutRGBIndoor[2][19]
//Gamma Outdoor R
0x0F120000, //0000	//saRR_usDualGammaLutRGBOutdoor[0][0]
0x0F120007, //0004	//saRR_usDualGammaLutRGBOutdoor[0][1]
0x0F120014, //000C	//saRR_usDualGammaLutRGBOutdoor[0][2]
0x0F120035, //0024	//saRR_usDualGammaLutRGBOutdoor[0][3]
0x0F120078, //006E	//saRR_usDualGammaLutRGBOutdoor[0][4]
0x0F1200E0, //00D1	//saRR_usDualGammaLutRGBOutdoor[0][5]
0x0F120128, //0119	//saRR_usDualGammaLutRGBOutdoor[0][6]
0x0F120146, //0139	//saRR_usDualGammaLutRGBOutdoor[0][7]
0x0F120164, //0157	//saRR_usDualGammaLutRGBOutdoor[0][8]
0x0F120198, //018E	//saRR_usDualGammaLutRGBOutdoor[0][9]
0x0F1201CC, //01C3	//saRR_usDualGammaLutRGBOutdoor[0][10]
0x0F1201F8, //01F3	//saRR_usDualGammaLutRGBOutdoor[0][11]
0x0F12021F, //021F	//saRR_usDualGammaLutRGBOutdoor[0][12]
0x0F120268, //0268	//saRR_usDualGammaLutRGBOutdoor[0][13]
0x0F1202A4, //02A4	//saRR_usDualGammaLutRGBOutdoor[0][14]
0x0F120304, //0304	//saRR_usDualGammaLutRGBOutdoor[0][15]
0x0F12034e, //034e	//saRR_usDualGammaLutRGBOutdoor[0][16]
0x0F12038e, //038e	//saRR_usDualGammaLutRGBOutdoor[0][17]
0x0F1203C8, //03C8	//saRR_usDualGammaLutRGBOutdoor[0][18]
0x0F1203FF, //03FF	//saRR_usDualGammaLutRGBOutdoor[0][19]
//GammmaOutdoor G
0x0F120000,	//saRR_usDualGammaLutRGBOutdoor[1][0]
0x0F120007,	//saRR_usDualGammaLutRGBOutdoor[1][1]
0x0F120014,	//saRR_usDualGammaLutRGBOutdoor[1][2]
0x0F120035,	//saRR_usDualGammaLutRGBOutdoor[1][3]
0x0F120078,	//saRR_usDualGammaLutRGBOutdoor[1][4]
0x0F1200E0,	//saRR_usDualGammaLutRGBOutdoor[1][5]
0x0F120128,	//saRR_usDualGammaLutRGBOutdoor[1][6]
0x0F120146,	//saRR_usDualGammaLutRGBOutdoor[1][7]
0x0F120164,	//saRR_usDualGammaLutRGBOutdoor[1][8]
0x0F120198,	//saRR_usDualGammaLutRGBOutdoor[1][9]
0x0F1201CC,	//saRR_usDualGammaLutRGBOutdoor[1][10]
0x0F1201F8,	//saRR_usDualGammaLutRGBOutdoor[1][11]
0x0F12021F,	//saRR_usDualGammaLutRGBOutdoor[1][12]
0x0F120268,	//saRR_usDualGammaLutRGBOutdoor[1][13]
0x0F1202A4,	//saRR_usDualGammaLutRGBOutdoor[1][14]
0x0F120304,	//saRR_usDualGammaLutRGBOutdoor[1][15]
0x0F12034e,	//saRR_usDualGammaLutRGBOutdoor[1][16]
0x0F12038e,	//saRR_usDualGammaLutRGBOutdoor[1][17]
0x0F1203C8,	//saRR_usDualGammaLutRGBOutdoor[1][18]
0x0F1203FF,	//saRR_usDualGammaLutRGBOutdoor[1][19]
//Gammma Outdoor B
0x0F120000,	//saRR_usDualGammaLutRGBOutdoor[2][0]
0x0F120007,	//saRR_usDualGammaLutRGBOutdoor[2][1]
0x0F120014,	//saRR_usDualGammaLutRGBOutdoor[2][2]
0x0F120035,	//saRR_usDualGammaLutRGBOutdoor[2][3]
0x0F120078,	//saRR_usDualGammaLutRGBOutdoor[2][4]
0x0F1200E0,	//saRR_usDualGammaLutRGBOutdoor[2][5]
0x0F120128,	//saRR_usDualGammaLutRGBOutdoor[2][6]
0x0F120146,	//saRR_usDualGammaLutRGBOutdoor[2][7]
0x0F120164,	//saRR_usDualGammaLutRGBOutdoor[2][8]
0x0F120198,	//saRR_usDualGammaLutRGBOutdoor[2][9]
0x0F1201CC,	//saRR_usDualGammaLutRGBOutdoor[2][10]
0x0F1201F8,	//saRR_usDualGammaLutRGBOutdoor[2][11]
0x0F12021F,	//saRR_usDualGammaLutRGBOutdoor[2][12]
0x0F120268,	//saRR_usDualGammaLutRGBOutdoor[2][13]
0x0F1202A4,	//saRR_usDualGammaLutRGBOutdoor[2][14]
0x0F120304,	//saRR_usDualGammaLutRGBOutdoor[2][15]
0x0F12034e,	//saRR_usDualGammaLutRGBOutdoor[2][16]
0x0F12038e,	//saRR_usDualGammaLutRGBOutdoor[2][17]
0x0F1203C8,	//saRR_usDualGammaLutRGBOutdoor[2][18]
0x0F1203FF,	//saRR_usDualGammaLutRGBOutdoor[2][19]
//===================================================================
//AE - shutter
//===================================================================
//AE_Target
0x002A10C0,
0x0F12003C, //0045	//TVAR_ae_BrAve
0x002A10C6,
0x0F12000F, //ae_StatMode									2	70001046

0x002A19D0,
0x0F1200FA,


//AE_state
0x002A03B2,
0x0F12010E, //#lt_uLimitHigh
0x0F1200F5, //#lt_uLimitLow
//For 60Hz
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //0D40	//#lt_uMaxExp4
0x0F120000, //0003	//#lt_uMaxExp4
0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //0D40 //#lt_uCapMaxExp4
0x0F120000, //0003 //#lt_uCapMaxExp4
0x002A03E4,
0x0F120250, //01C0	//0230	//#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260,   //01D0	//0260	//#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370,   //02D0	//0380	//#lt_uMaxAnGain3
0x0F120880, //0980	//0900	//0790	//#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain
0x0F120230, //01C0	//#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260,   //01D0	//#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380,   //02D0	//#lt_uCapMaxAnGain3
0x0F120880, //0980	//0900	//#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain
//===================================================================
//AE - Weights
//===================================================================
//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]             2 7000108C

//===================================================================
//AWB-BASIC setting
//===================================================================
0x002A0DCC,
0x0F120138, //awbb_IntcR
0x0F12011C, //awbb_IntcB
0x0F1202A7, //awbb_GLocusR
0x0F120343, //awbb_GLocusB
0x002A0DB4,
0x0F12036C, //awbb_CrclLowT_R_c
0x002A0DB8,
0x0F12011D, //awbb_CrclLowT_B_c
0x002A0DBC,
0x0F1262C1, //awbb_CrclLowT_Rad_c
0x002A0DEC,
0x0F1205F0, //awbb_GamutWidthThr1
0x0F1201F4, //awbb_GamutHeightThr1
0x0F12006C, //awbb_GamutWidthThr2
0x0F120038, //awbb_GamutHeightThr2
0x002A0DD8,
0x0F12000C, //awbb_MinNumOfFinalPatches
0x0F12001E, //awbb_MinNumOfLowBrFinalPatches
0x0F120046, //awbb_MinNumOfLowBr0_FinalPatches
0x002A22BA,
0x0F120004,	// #Mon_AWB_ByPassMode // [0]Outdoor [1]LowBr [2]LowTemp


0x002A0F7A,
0x0F120000,	//awbb_RGainOff                             	2   70000EFA
0x0F120000,	//awbb_BGainOff                             	2   70000EFC
0x0F120000,	//awbb_GGainOff                             	2   70000EFE
0x0F1200C2,	//awbb_Alpha_Comp_Mode                      	2   70000F00
0x0F120002,	//awbb_Rpl_InvalidOutDoor                   	2   70000F02
0x0F120001,	//awbb_UseGrThrCorr                         	2   70000F04
0x0F1200E4,	//awbb_Use_Filters                          	2   70000F06

0x002A0F8E,
0x0F120008,	//awbb_WpFilterMinThr                       	2   70000F0E
0x0F120190, //awbb_WpFilterMaxThr							2	70000F10
0x0F1200B0, //awbb_WpFilterCoef 							2	70000F12
0x0F120004, //awbb_WpFilterSize 							2	70000F14
0x0F120001, //awbb_otp_disable								2	70000F16
//===================================================================
//AWB-Zone
//===================================================================
//	param_start awbb_IndoorGrZones_m_BGrid
0x002A0CE0,
0x0F1203BC,	//03BCawbb_IndoorGrZones_m_BGrid[0] 	70000CE0
0x0F120406,	//0406awbb_IndoorGrZones_m_BGrid[1] 		70000CE2
0x0F12038A,	//038Aawbb_IndoorGrZones_m_BGrid[2] 		70000CE4
0x0F120418,	//0418awbb_IndoorGrZones_m_BGrid[3] 		70000CE6
0x0F120356,	//0356awbb_IndoorGrZones_m_BGrid[4] 		70000CE8
0x0F120404,	//0404awbb_IndoorGrZones_m_BGrid[5] 		70000CEA
0x0F120332,	//0332awbb_IndoorGrZones_m_BGrid[6] 		70000CEC
0x0F1203F0,	//03F0awbb_IndoorGrZones_m_BGrid[7] 		70000CEE
0x0F12030E,	//030Eawbb_IndoorGrZones_m_BGrid[8] 		70000CF0
0x0F1203D6,	//03D6awbb_IndoorGrZones_m_BGrid[9] 		70000CF2
0x0F1202F8,	//02F8awbb_IndoorGrZones_m_BGrid[10]		70000CF4
0x0F1203A6,	//03A6awbb_IndoorGrZones_m_BGrid[11]		70000CF6
0x0F1202E0,	//02E0awbb_IndoorGrZones_m_BGrid[12]		70000CF8
0x0F12036A,	//036Aawbb_IndoorGrZones_m_BGrid[13]		70000CFA
0x0F1202D0,	//02D0awbb_IndoorGrZones_m_BGrid[14]		70000CFC
0x0F120336,	//0336awbb_IndoorGrZones_m_BGrid[15]		70000CFE
0x0F1202BA,	//02BAawbb_IndoorGrZones_m_BGrid[16]		70000D00
0x0F1202FC,	//02FCawbb_IndoorGrZones_m_BGrid[17]		70000D02
0x0F12029C,	//029Cawbb_IndoorGrZones_m_BGrid[18]		70000D04
0x0F1202D6,	//02D6awbb_IndoorGrZones_m_BGrid[19]		70000D06
0x0F120278,	//0278awbb_IndoorGrZones_m_BGrid[20]		70000D08
0x0F1202B4,	//02B4awbb_IndoorGrZones_m_BGrid[21]		70000D0A
0x0F120258,	//0248awbb_IndoorGrZones_m_BGrid[22]		70000D0C
0x0F12029A,	//029Aawbb_IndoorGrZones_m_BGrid[23]		70000D0E
0x0F12023A,	//022Aawbb_IndoorGrZones_m_BGrid[24]		70000D10
0x0F120286,	//0286awbb_IndoorGrZones_m_BGrid[25]		70000D12
0x0F12022E,	//0224awbb_IndoorGrZones_m_BGrid[26]		70000D14
0x0F12027A,	//027Aawbb_IndoorGrZones_m_BGrid[27]		70000D16
0x0F12023E,	//023Eawbb_IndoorGrZones_m_BGrid[28]		70000D18
0x0F12025E,	//025Eawbb_IndoorGrZones_m_BGrid[29]		70000D1A
0x0F120000,	//0000awbb_IndoorGrZones_m_BGrid[30]		70000D1C
0x0F120000,	//0000awbb_IndoorGrZones_m_BGrid[31]		70000D1E
0x0F120000,	//0000awbb_IndoorGrZones_m_BGrid[32]		70000D20
0x0F120000,	//0000awbb_IndoorGrZones_m_BGrid[33]		70000D22
0x0F120000,	//0000awbb_IndoorGrZones_m_BGrid[34]		70000D24
0x0F120000,	//0000awbb_IndoorGrZones_m_BGrid[35]		70000D26
0x0F120000,	//0000awbb_IndoorGrZones_m_BGrid[36]		70000D28
0x0F120000,	//0000awbb_IndoorGrZones_m_BGrid[37]		70000D2A
0x0F120000,	//0000awbb_IndoorGrZones_m_BGrid[38]		70000D2C
0x0F120000,	//0000awbb_IndoorGrZones_m_BGrid[39]		70000D2E
//	param_end	awbb_IndoorGrZones_m_BGrid
0x002A0D30,
0x0F120005, 
0x0F120000,

0x0F12000F,
0x0F120000,
 
0x0F1200FE,
0x0F120000,

//	param_start awbb_OutdoorGrZones_m_BGrid
0x002A0D3C,
0x0F1202C2, //02C2 awbb_OutdoorGrZones_m_BGrid[0]	70000D3C
0x0F1202E0, //02E0 awbb_OutdoorGrZones_m_BGrid[1]	70000D3E
0x0F12029C, //029C awbb_OutdoorGrZones_m_BGrid[2]	70000D40
0x0F1202F4, //02F4 awbb_OutdoorGrZones_m_BGrid[3]	70000D42
0x0F120278, //0278 awbb_OutdoorGrZones_m_BGrid[4]	70000D44
0x0F1202EE, //02D6 awbb_OutdoorGrZones_m_BGrid[5]	70000D46
0x0F120266, //0266 awbb_OutdoorGrZones_m_BGrid[6]	70000D48
0x0F1202D0, //02B6 awbb_OutdoorGrZones_m_BGrid[7]	70000D4A
0x0F120278, //0278 awbb_OutdoorGrZones_m_BGrid[8]	70000D4C
0x0F1202A2, //028A awbb_OutdoorGrZones_m_BGrid[9]	70000D4E
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[10]	70000D50
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[11]	70000D52
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[12]	70000D54
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[13]	70000D56
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[14]	70000D58
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[15]	70000D5A
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[16]	70000D5C
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[17]	70000D5E
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[18]	70000D60
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[19]	70000D62
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[20]	70000D64
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[21]	70000D66
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[22]	70000D68
0x0F120000, //0000 awbb_OutdoorGrZones_m_BGrid[23]	70000D6A
//	param_end	awbb_OutdoorGrZones_m_BGrid
0x0F120005, //awbb_OutdoorGrZones_m_Gri
0x002A0D70,
0x0F120005, //awbb_OutdoorGrZones_ZInfo_m_GridSz
0x002A0D74,
0x0F1201E4, //1BA //BA B2 D0 //BC c4	//awbb_OutdoorGrZones_m_Bof
//	param_start awbb_LowBrGrZones_m_BGrid
0x002A0D78,
0x0F1203BE, //03EE awbb_LowBrGrZones_m_BGrid[0]
0x0F120490, //0490 awbb_LowBrGrZones_m_BGrid[1]
0x0F120336, //0368 awbb_LowBrGrZones_m_BGrid[2]
0x0F1204AC, //048C awbb_LowBrGrZones_m_BGrid[3]
0x0F1202CE, //0302 awbb_LowBrGrZones_m_BGrid[4]
0x0F120484, //044C awbb_LowBrGrZones_m_BGrid[5]
0x0F12026E, //02A4 awbb_LowBrGrZones_m_BGrid[6]
0x0F12043A, //0408 awbb_LowBrGrZones_m_BGrid[7]
0x0F120228, //025C awbb_LowBrGrZones_m_BGrid[8]
0x0F1203D2, //039A awbb_LowBrGrZones_m_BGrid[9]
0x0F1201F6, //0226 awbb_LowBrGrZones_m_BGrid[10]
0x0F120372, //034E awbb_LowBrGrZones_m_BGrid[11]
0x0F1201DC, //0202 awbb_LowBrGrZones_m_BGrid[12]
0x0F120332, //030E awbb_LowBrGrZones_m_BGrid[13]
0x0F1201C4, //01EA awbb_LowBrGrZones_m_BGrid[14]
0x0F1202F8, //02DC awbb_LowBrGrZones_m_BGrid[15]
0x0F1201C2, //01E0 awbb_LowBrGrZones_m_BGrid[16]
0x0F1202CC, //02B0 awbb_LowBrGrZones_m_BGrid[17]
0x0F1201EA, //01EA awbb_LowBrGrZones_m_BGrid[18]
0x0F12027C, //027C awbb_LowBrGrZones_m_BGrid[19]
0x0F120000, //0000 awbb_LowBrGrZones_m_BGrid[20]
0x0F120000, //0000 awbb_LowBrGrZones_m_BGrid[21]
0x0F120000, //0000 awbb_LowBrGrZones_m_BGrid[22]
0x0F120000, //0000 awbb_LowBrGrZones_m_BGrid[23]
//	param_end	awbb_LowBrGrZones_m_BGrid
0x002A0DA8, //0DA8
0x0F120006, //0006
0x002A0DAC, //0DAC
0x0F12000A, //000A
0x002A0DB0, //0DB0	
0x0F1200E4, //00E4

//===================================================================
//AWB Scene Detection
//===================================================================
0x002A0E50,
0x0F12FE82, //awbb_SCDetectionMap_SEC_StartR_B				2	70000DD0
0x0F12001E, //awbb_SCDetectionMap_SEC_StepR_B				2	70000DD2
0x0F120960, //awbb_SCDetectionMap_SEC_SunnyNB				2	70000DD4
0x0F120122, //awbb_SCDetectionMap_SEC_StepNB				2	70000DD6
0x0F1200E4, //awbb_SCDetectionMap_SEC_LowTempR_B			2	70000DD8
0x0F120096, //awbb_SCDetectionMap_SEC_SunnyNBZone			2	70000DDA
0x0F12000E, //awbb_SCDetectionMap_SEC_LowTempR_BZone		2	70000DDC
0x002A0E14,
0x0F120000, //awbb_SCDetectionMap_SEC	0000					70000D94	0000  awbb_SCDetectionMap_SEC_SceneDetectionMap
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_0__2_	70000D96	0000
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_0__4_	70000D98	0000
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_1__1_	70000D9A	0000
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_1__3_	70000D9C	0000
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_2__0_	70000D9E	0000
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_2__2_	70000DA0	0000
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_2__4_	70000DA2	0000
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_3__1_	70000DA4	0000
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_3__3_	70000DA6	0000
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_4__0_	70000DA8	0000
0x0F120000, //awbb_SCDetectionMap_SEC_SceneDetectionMap_4__2_	70000DAA	0000
0x0F120500, //awbb_SCDetectionMap_SEC_SceneDetectionMap_4__4_	70000DAC	0500
0x0F125555, //awbb_SCDetectionMap_SEC_SceneDetectionMap_5__1_	70000DAE	5555
0x0F125455, //awbb_SCDetectionMap_SEC_SceneDetectionMap_5__3_	70000DB0	5455
0x0F12AA55, //awbb_SCDetectionMap_SEC_SceneDetectionMap_6__0_	70000DB2	AA55
0x0F12AAAA, //awbb_SCDetectionMap_SEC_SceneDetectionMap_6__2_	70000DB4	AAAA
0x0F12BF54, //awbb_SCDetectionMap_SEC_SceneDetectionMap_6__4_	70000DB6	BF54
0x0F12FFFF, //awbb_SCDetectionMap_SEC_SceneDetectionMap_7__1_	70000DB8	FFFF
0x0F1254FE, //awbb_SCDetectionMap_SEC_SceneDetectionMap_7__3_	70000DBA	54FE
0x0F12FF6F, //awbb_SCDetectionMap_SEC_SceneDetectionMap_8__0_	70000DBC	FF6F
0x0F12FEFF, //awbb_SCDetectionMap_SEC_SceneDetectionMap_8__2_	70000DBE	FEFF
0x0F121B54, //awbb_SCDetectionMap_SEC_SceneDetectionMap_8__4_	70000DC0	1B54
0x0F12FFFF, //awbb_SCDetectionMap_SEC_SceneDetectionMap_9__1_	70000DC2	FFFF
0x0F1254FE, //awbb_SCDetectionMap_SEC_SceneDetectionMap_9__3_	70000DC4	54FE
0x0F12FF06, //awbb_SCDetectionMap_SEC_SceneDetectionMap_10__0_	70000DC6	FF06
0x0F12FEFF, //awbb_SCDetectionMap_SEC_SceneDetectionMap_10__2_	70000DC8	FEFF
0x0F120154, //awbb_SCDetectionMap_SEC_SceneDetectionMap_10__4_	70000DCA	0154
0x0F12BFBF, //awbb_SCDetectionMap_SEC_SceneDetectionMap_11__1_	70000DCC	BFBF
0x0F1254BE, //awbb_SCDetectionMap_SEC_SceneDetectionMap_11__3_	70000DCE	54BE


//===================================================================
//AWB - GridCorrection
//===================================================================
0x002A0F98,
0x0F120002, //awbb_GridEnable								2	70000F18
0x002A0F60,
0x0F12029B, //029B //awbb_GridConst_1[0]							2	70000EE0
0x0F12034B, //034B //awbb_GridConst_1[1]							2	70000EE2
0x0F1203EF, //03EF //awbb_GridConst_1[2]							2	70000EE4
0x0F121036, //1036 //awbb_GridConst_2[0]						2	70000EE6
0x0F1210A3, //10AF //awbb_GridConst_2[1]						2	70000EE8
0x0F121106, //1129 //awbb_GridConst_2[2]						2	70000EEA
0x0F121172, //117E //awbb_GridConst_2[3]						2	70000EEC
0x0F1211CD, //11CD //awbb_GridConst_2[4]						2	70000EEE
0x0F121225, //1225 //awbb_GridConst_2[5]						2	70000EF0
0x0F1202C4, //02C4 //awbb_GridCoeff_R_1							2	70000EF2
0x0F1202E4, //02E4 //awbb_GridCoeff_B_1							2	70000EF4
0x0F1200C3, //00C3 //awbb_GridCoeff_R_2							2	70000EF6
0x0F1200A6, //00A6 //awbb_GridCoeff_B_2							2	70000EF8

0x002A0ED0,
0x0F120014, //awbb_GridCorr_R[0][0] D65 						2	70000ED0
0x0F120028, //awbb_GridCorr_R[0][1] D50 					2	70000ED2
0x0F120014, //awbb_GridCorr_R[0][2] CW						2	70000ED4
0x0F12003C, //awbb_GridCorr_R[0][3] WW						2	70000ED6
0x0F12FFEC, //awbb_GridCorr_R[0][4] A						2	70000ED8
0x0F120014, //awbb_GridCorr_R[0][5] H						2	70000EDA
0x0F120014, //awbb_GridCorr_R[1][0] 						2	70000EDC
0x0F120028, //awbb_GridCorr_R[1][1] 						2	70000EDE
0x0F120014, //awbb_GridCorr_R[1][2] 						2	70000EE0
0x0F12003C, //awbb_GridCorr_R[1][3] 						2	70000EE2
0x0F12FFEC, //awbb_GridCorr_R[1][4] 						2	70000EE4
0x0F120014, //awbb_GridCorr_R[1][5] 						2	70000EE6
0x0F120014, //awbb_GridCorr_R[2][0] 						2	70000EE8
0x0F120028, //awbb_GridCorr_R[2][1] 						2	70000EEA
0x0F120014, //awbb_GridCorr_R[2][2] 						2	70000EEC
0x0F12003C, //awbb_GridCorr_R[2][3] 						2	70000EEE
0x0F12FFEC, //awbb_GridCorr_R[2][4] 						2	70000EF0
0x0F120014, //awbb_GridCorr_R[2][5] 						2	70000EF2
0x0F12FF9C, //awbb_GridCorr_B[0][0] 						2	70000EF4
0x0F12FFD8, //awbb_GridCorr_B[0][1] 						2	70000EF6
0x0F120028, //awbb_GridCorr_B[0][2] 						2	70000EF8
0x0F120000, //awbb_GridCorr_B[0][3] 						2	70000EFA
0x0F12FFEC, //awbb_GridCorr_B[0][4] 						2	70000EFC
0x0F120064, //awbb_GridCorr_B[0][5] 						2	70000EFE
0x0F12FF9C, //awbb_GridCorr_B[1][0] 						2	70000F00
0x0F12FFD8, //awbb_GridCorr_B[1][1] 						2	70000F02
0x0F120028, //awbb_GridCorr_B[1][2] 						2	70000F04
0x0F120000, //awbb_GridCorr_B[1][3] 						2	70000F06
0x0F12FFEC, //awbb_GridCorr_B[1][4] 						2	70000F08
0x0F120064, //awbb_GridCorr_B[1][5] 						2	70000F0A
0x0F12FF9C, //awbb_GridCorr_B[2][0] 						2	70000F0C
0x0F12FFD8, //awbb_GridCorr_B[2][1] 						2	70000F0E
0x0F120028, //awbb_GridCorr_B[2][2] 						2	70000F10
0x0F120000, //awbb_GridCorr_B[2][3] 						2	70000F12
0x0F12FFEC, //awbb_GridCorr_B[2][4] 						2	70000F14
0x0F120064, //awbb_GridCorr_B[2][5] 						2	70000F16

0x0F12FFCE, //FFCE //awbb_GridCorr_R_Out[0][0] 					2	70000F18
0x0F12FFEA, //FFE0 //awbb_GridCorr_R_Out[0][1] 					2	70000F1A
0x0F12000A, //0000 //awbb_GridCorr_R_Out[0][2] 					2	70000F1C
0x0F120000, //0000 //awbb_GridCorr_R_Out[0][3] 					2	70000F1E
0x0F120000, //0000 //awbb_GridCorr_R_Out[0][4] 					2	70000F20
0x0F120000, //0000 //awbb_GridCorr_R_Out[0][5] 					2	70000F22
0x0F12FFCE, //FFCE //awbb_GridCorr_R_Out[1][0] 					2	70000F24
0x0F12FFEA, //FFE0 //awbb_GridCorr_R_Out[1][1] 					2	70000F26
0x0F12000A, //0000 //awbb_GridCorr_R_Out[1][2] 					2	70000F28
0x0F120000, //0000 //awbb_GridCorr_R_Out[1][3] 					2	70000F2A
0x0F120000, //0000 //awbb_GridCorr_R_Out[1][4] 					2	70000F2C
0x0F120000, //0000 //awbb_GridCorr_R_Out[1][5] 					2	70000F2E
0x0F12FFCE, //FFCE //awbb_GridCorr_R_Out[2][0] 					2	70000F30
0x0F12FFEA, //FFE0 //awbb_GridCorr_R_Out[2][1] 					2	70000F32
0x0F12000A, //0000 //awbb_GridCorr_R_Out[2][2] 					2	70000F34
0x0F120000, //0000 //awbb_GridCorr_R_Out[2][3] 					2	70000F36
0x0F120000, //0000 //awbb_GridCorr_R_Out[2][4] 					2	70000F38
0x0F120000, //0000 //awbb_GridCorr_R_Out[2][5] 					2	70000F3A
0x0F120032, //0032 //awbb_GridCorr_B_Out[0][0] 					2	70000F3C
0x0F12000E, //000E //awbb_GridCorr_B_Out[0][1] 					2	70000F3E
0x0F12FFEC, //FFEC //awbb_GridCorr_B_Out[0][2] 					2	70000F40
0x0F120000, //0000 //awbb_GridCorr_B_Out[0][3] 					2	70000F42
0x0F120000, //0000 //awbb_GridCorr_B_Out[0][4] 					2	70000F44
0x0F120000, //0000 //awbb_GridCorr_B_Out[0][5] 					2	70000F46
0x0F120032, //0032 //awbb_GridCorr_B_Out[1][0] 					2	70000F48
0x0F12000E, //000E //awbb_GridCorr_B_Out[1][1] 					2	70000F4A
0x0F12FFEC, //FFEC //awbb_GridCorr_B_Out[1][2] 					2	70000F4C
0x0F120000, //0000 //awbb_GridCorr_B_Out[1][3] 					2	70000F4E
0x0F120000, //0000 //awbb_GridCorr_B_Out[1][4] 					2	70000F50
0x0F120000, //0000 //awbb_GridCorr_B_Out[1][5] 					2	70000F52
0x0F120032, //0032 //awbb_GridCorr_B_Out[2][0] 					2	70000F54
0x0F12000E, //000E //awbb_GridCorr_B_Out[2][1] 					2	70000F56
0x0F12FFEC, //FFEC //awbb_GridCorr_B_Out[2][2] 					2	70000F58
0x0F120000, //0000 //awbb_GridCorr_B_Out[2][3] 					2	70000F5A
0x0F120000, //0000 //awbb_GridCorr_B_Out[2][4] 					2	70000F5C
0x0F120000, //0000 //awbb_GridCorr_B_Out[2][5] 					2	70000F5E
//===================================================================
//CCM
//===================================================================
0x002A05D2,
0x0F1200A0, //--00E4 #SARR_AwbCcmCord			2	70000552
0x0F1200C8, //--00F0 #SARR_AwbCcmCord_1_		2	70000554
0x0F1200F8, //--0100 #SARR_AwbCcmCord_2_		2	70000556
0x0F120110, //--0120 #SARR_AwbCcmCord_3_		2	70000558
0x0F120140, //--0150 #SARR_AwbCcmCord_4_		2	7000055A
0x0F120178, //--0180 #SARR_AwbCcmCord_5_		2	7000055C
//	param_start TVAR_wbt_pBaseCcms
0x002A05C4,
0x0F123800,
0x0F127000,
//3200K
0x002A3800,
0x0F1201D2, //TVAR_wbt_pBaseCcms[0]    
0x0F12FFAF, //TVAR_wbt_pBaseCcms[1]
0x0F12FFE9, //TVAR_wbt_pBaseCcms[2]
0x0F12FF76, //TVAR_wbt_pBaseCcms[3]
0x0F120154, //TVAR_wbt_pBaseCcms[4]
0x0F12FF2E, //TVAR_wbt_pBaseCcms[5]
0x0F12FFE9, //TVAR_wbt_pBaseCcms[6]
0x0F120000, //TVAR_wbt_pBaseCcms[7]
0x0F120222, //TVAR_wbt_pBaseCcms[8]
0x0F1200F6, //TVAR_wbt_pBaseCcms[9]
0x0F1200BD, //TVAR_wbt_pBaseCcms[10]
0x0F12FEA6, //TVAR_wbt_pBaseCcms[11]
0x0F120111, //TVAR_wbt_pBaseCcms[12]
0x0F12FF36, //TVAR_wbt_pBaseCcms[13]
0x0F120125, //TVAR_wbt_pBaseCcms[14]
0x0F12FF4D, //TVAR_wbt_pBaseCcms[15]
0x0F12014D, //TVAR_wbt_pBaseCcms[16]
0x0F1200FB, //TVAR_wbt_pBaseCcms[17]
//3200K
0x0F1201D2, //TVAR_wbt_pBaseCcms[18]   
0x0F12FFAF, //TVAR_wbt_pBaseCcms[19]
0x0F12FFE9, //TVAR_wbt_pBaseCcms[20]
0x0F12FF76, //TVAR_wbt_pBaseCcms[21]
0x0F120154, //TVAR_wbt_pBaseCcms[22]
0x0F12FF2E, //TVAR_wbt_pBaseCcms[23]
0x0F12FFE9, //TVAR_wbt_pBaseCcms[24]
0x0F120000, //TVAR_wbt_pBaseCcms[25]
0x0F120222, //TVAR_wbt_pBaseCcms[26]
0x0F1200F6, //TVAR_wbt_pBaseCcms[27]
0x0F1200BD, //TVAR_wbt_pBaseCcms[28]
0x0F12FEA6, //TVAR_wbt_pBaseCcms[29]
0x0F120111, //TVAR_wbt_pBaseCcms[30]
0x0F12FF36, //TVAR_wbt_pBaseCcms[31]
0x0F120125, //TVAR_wbt_pBaseCcms[32]
0x0F12FF4D, //TVAR_wbt_pBaseCcms[33]
0x0F12014D, //TVAR_wbt_pBaseCcms[34]
0x0F1200FB, //TVAR_wbt_pBaseCcms[35]
//3800K
0x0F1201D7,  //01D7TVAR_wbt_pBaseCcms[36]
0x0F12FFB0,  //FFB0TVAR_wbt_pBaseCcms[37]
0x0F12FFE5,  //FFE5TVAR_wbt_pBaseCcms[38]
0x0F12FF4A,  //FF4ATVAR_wbt_pBaseCcms[39]
0x0F12019B,  //019BTVAR_wbt_pBaseCcms[40]
0x0F12FF78,  //FF78TVAR_wbt_pBaseCcms[41]
0x0F12FFF5,  //FFF5TVAR_wbt_pBaseCcms[42]
0x0F12FFE3,  //FFE3TVAR_wbt_pBaseCcms[43]
0x0F1201AF,  //01AFTVAR_wbt_pBaseCcms[44]
0x0F120124,  //011ATVAR_wbt_pBaseCcms[45]
0x0F120103,  //00F8TVAR_wbt_pBaseCcms[46]
0x0F12FF71,  //FF59TVAR_wbt_pBaseCcms[47]
0x0F120134,  //0134TVAR_wbt_pBaseCcms[48]
0x0F12FF4C,  //FF4CTVAR_wbt_pBaseCcms[49]
0x0F12011C,  //011CTVAR_wbt_pBaseCcms[50]
0x0F12FEE6,  //FEE6TVAR_wbt_pBaseCcms[51]
0x0F120162,  //0162TVAR_wbt_pBaseCcms[52]
0x0F1200F7,  //00F7TVAR_wbt_pBaseCcms[53]  
//4150K
0x0F1201D7,  //TVAR_wbt_pBaseCcms[36]
0x0F12FFB0,  //TVAR_wbt_pBaseCcms[37]
0x0F12FFE5,  //TVAR_wbt_pBaseCcms[38]
0x0F12FF4A,  //TVAR_wbt_pBaseCcms[39]
0x0F12019B,  //TVAR_wbt_pBaseCcms[40]
0x0F12FF78,  //TVAR_wbt_pBaseCcms[41]
0x0F12FFF5,  //TVAR_wbt_pBaseCcms[42]
0x0F12FFE3,  //TVAR_wbt_pBaseCcms[43]
0x0F1201AF,  //TVAR_wbt_pBaseCcms[44]
0x0F120124,  //TVAR_wbt_pBaseCcms[45]
0x0F120103,  //TVAR_wbt_pBaseCcms[46]
0x0F12FF71,  //TVAR_wbt_pBaseCcms[47]
0x0F120134,  //TVAR_wbt_pBaseCcms[48]
0x0F12FF4C,  //TVAR_wbt_pBaseCcms[49]
0x0F12011C,  //TVAR_wbt_pBaseCcms[50]
0x0F12FEE6,  //TVAR_wbt_pBaseCcms[51]
0x0F120162,  //TVAR_wbt_pBaseCcms[52]
0x0F1200F7,  //TVAR_wbt_pBaseCcms[53]  
//5000K
0x0F1201EB,	//TVAR_wbt_pBaseCcms[72] 
0x0F12FFCA,	//TVAR_wbt_pBaseCcms[73]
0x0F12000E,	//TVAR_wbt_pBaseCcms[74]
0x0F12FF02,	//TVAR_wbt_pBaseCcms[75]
0x0F120145,	//TVAR_wbt_pBaseCcms[76]
0x0F12FF5D,	//TVAR_wbt_pBaseCcms[77]
0x0F12FFE7,	//TVAR_wbt_pBaseCcms[78]
0x0F12FFD4,	//TVAR_wbt_pBaseCcms[79]
0x0F1201BA,	//TVAR_wbt_pBaseCcms[80]
0x0F12013D,	//TVAR_wbt_pBaseCcms[81]
0x0F120106,	//TVAR_wbt_pBaseCcms[82]
0x0F12FF80,	//TVAR_wbt_pBaseCcms[83]
0x0F120125,	//TVAR_wbt_pBaseCcms[84]
0x0F12FF7D,	//TVAR_wbt_pBaseCcms[85]
0x0F120110,	//TVAR_wbt_pBaseCcms[86]
0x0F12FF73,	//TVAR_wbt_pBaseCcms[87]
0x0F12016C,	//TVAR_wbt_pBaseCcms[88]
0x0F120158,	//TVAR_wbt_pBaseCcms[89]   

//6500K
0x0F1201EB,	//TVAR_wbt_pBaseCcms[72] 
0x0F12FFCA,	//TVAR_wbt_pBaseCcms[73]
0x0F12000E,	//TVAR_wbt_pBaseCcms[74]
0x0F12FF02,	//TVAR_wbt_pBaseCcms[75]
0x0F120145,	//TVAR_wbt_pBaseCcms[76]
0x0F12FF5D,	//TVAR_wbt_pBaseCcms[77]
0x0F12FFE7,	//TVAR_wbt_pBaseCcms[78]
0x0F12FFD4,	//TVAR_wbt_pBaseCcms[79]
0x0F1201BA,	//TVAR_wbt_pBaseCcms[80]
0x0F12013D,	//TVAR_wbt_pBaseCcms[81]
0x0F120106,	//TVAR_wbt_pBaseCcms[82]
0x0F12FF80,	//TVAR_wbt_pBaseCcms[83]
0x0F120125,	//TVAR_wbt_pBaseCcms[84]
0x0F12FF7D,	//TVAR_wbt_pBaseCcms[85]
0x0F120110,	//TVAR_wbt_pBaseCcms[86]
0x0F12FF73,	//TVAR_wbt_pBaseCcms[87]
0x0F12016C,	//TVAR_wbt_pBaseCcms[88]
0x0F120158,	//TVAR_wbt_pBaseCcms[89]  
//	param_end	TVAR_wbt_pBaseCcms

//	param_start	TVAR_wbt_pOutdoorCcm

0x002A05CC,
0x0F1238D8,	//#TVAR_wbt_pOutdoorCcm
0x0F127000,

0x002A38D8,
0x0F1201E4,	//01F4	//TVAR_wbt_pOutdoorCcm[0]
0x0F12FFBF,	//FFB0	//TVAR_wbt_pOutdoorCcm[1]
0x0F12000E,	//000E	//TVAR_wbt_pOutdoorCcm[2]
0x0F12FF09,	//FF16	//TVAR_wbt_pOutdoorCcm[3]
0x0F120153,	//0147	//TVAR_wbt_pOutdoorCcm[4]
0x0F12FF46,	//FF46	//TVAR_wbt_pOutdoorCcm[5]
0x0F12FFB9,	//FFB9	//TVAR_wbt_pOutdoorCcm[6]
0x0F120007,	//0007	//TVAR_wbt_pOutdoorCcm[7]
0x0F1201AB,	//01AB	//TVAR_wbt_pOutdoorCcm[8]
0x0F1200B1,	//00B1	//TVAR_wbt_pOutdoorCcm[9]
0x0F120101,	//0101	//TVAR_wbt_pOutdoorCcm[10]
0x0F12FF61,	//FF61	//TVAR_wbt_pOutdoorCcm[11]
0x0F120148,	//0148	//TVAR_wbt_pOutdoorCcm[12]
0x0F12FF78,	//FF78	//TVAR_wbt_pOutdoorCcm[13]
0x0F12011F,	//011F	//TVAR_wbt_pOutdoorCcm[14]
0x0F12FF36,	//FF36	//TVAR_wbt_pOutdoorCcm[15]
0x0F12013C,	//013C	//TVAR_wbt_pOutdoorCcm[16]
0x0F120126,	//0126	//TVAR_wbt_pOutdoorCcm[17]
//	param_end	TVAR_wbt_pOutdoorCcm
0x002A2404,
0x0F120001, //#MVAR_AAIO_bFIT
0x002A2408,
0x0F120001, //#MVAR_AAIO_bAutoCCMandASH
0x002A23DC,
0x0F1201DD, //#Mon_AAIO_PrevFrmData_NormBr
//===================================================================
//AFIT
//===================================================================
//	param_start afit_uNoiseIndInDoor
0x002A065C,
0x0F12003F, //#afit_uNoiseIndInDoor_0_
0x0F12004A, //--#afit_uNoiseIndInDoor_1_
0x0F1200CB, //#afit_uNoiseIndInDoor_2_
0x0F1201E0, //#afit_uNoiseIndInDoor_3_
0x0F120220, //#afit_uNoiseIndInDoor_4_
//	param_end	afit_uNoiseIndInDoor
//	param_start TVAR_afit_pBaseVals
0x002A06BC,
0x0F120000, //#AfitBaseVals 	 AFIT16_BRIGHTNESS
0x0F120000, //#AfitBaseVals_0__1_	 AFIT16_CONTRAST
0x0F120014, //#AfitBaseVals_0__2_	 AFIT16_SATURATION
0x0F120000,	//#AfitBaseVals_0__3_	 AFIT16_N H
0x0F120000, //#AfitBaseVals_0__4_	 AFIT16_GLAMOUR
0x0F1200C1, //#AfitBaseVals_0__5_	 AFIT16_0xddd8a_ed,ge_high
0x0F1203FF, //#AfitBaseVals_0__6_	 AFIT16_Demosaicing_iSatVal
0x0F12009C, //#AfitBaseVals_0__7_	 AFIT16_Sharpening_iReduceEdgeThresh
0x0F12017C, //#AfitBaseVals_0__8_	 AFIT16_demsharpmix1_iRGBOffset
0x0F1203FF, //#AfitBaseVals_0__9_	 AFIT16_demsharpmix1_iDemClamp
0x0F12000C, //#AfitBaseVals_0__10_	 AFIT16_demsharpmix1_iLowThreshold
0x0F120010, //#AfitBaseVals_0__11_	 AFIT16_demsharpmix1_iHighThreshold
0x0F12012C, //#AfitBaseVals_0__12_	 AFIT16_demsharpmix1_iLowBright
0x0F1203E8, //#AfitBaseVals_0__13_	 AFIT16_demsharpmix1_iHighBright
0x0F120046, //#AfitBaseVals_0__14_	 AFIT16_demsharpmix1_iLowSat
0x0F12005A, //#AfitBaseVals_0__15_	 AFIT16_demsharpmix1_iHighSat
0x0F120070, //#AfitBaseVals_0__16_	 AFIT16_demsharpmix1_iTune
0x0F12001E, //#AfitBaseVals_0__17_	 AFIT16_demsharpmix1_iHystThLow
0x0F12001E, //#AfitBaseVals_0__18_	 AFIT16_demsharpmix1_iHystThHigh
0x0F1201F4, //#AfitBaseVals_0__19_	 AFIT16_demsharpmix1_iHystCenter
0x0F120046, //#AfitBaseVals_0__20_	 AFIT16_YUV422_DENOISE_iUVLowThresh
0x0F120046, //#AfitBaseVals_0__21_	 AFIT16_YUV422_DENOISE_iUVHighThresh
0x0F120005, //#AfitBaseVals_0__22_	 AFIT16_YUV422_DENOISE_iYLowThresh
0x0F120005, //#AfitBaseVals_0__23_	 AFIT16_YUV422_DENOISE_iYHighThresh
0x0F12003C, //#AfitBaseVals_0__24_	 AFIT16_Sharpening_iLowSharpClamp
0x0F120014, //#AfitBaseVals_0__25_	 AFIT16_Sharpening_iHighSharpClamp
0x0F12003C, //#AfitBaseVals_0__26_	 AFIT16_Sharpening_iLowSharpClamp_Bin
0x0F120014, //#AfitBaseVals_0__27_	 AFIT16_Sharpening_iHighSharpClamp_Bin
0x0F12003C, //#AfitBaseVals_0__28_	 AFIT16_Sharpening_iLowSharpClamp_sBin
0x0F12001E, //#AfitBaseVals_0__29_	 AFIT16_Sharpening_iHighSharpClamp_sBin
0x0F120A24, //#AfitBaseVals_0__30_	 AFIT8_0xddd8a_ed,ge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]
0x0F121701, //#AfitBaseVals_0__31_	 AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]
0x0F120229, //#AfitBaseVals_0__32_	 AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]
0x0F121403, //#AfitBaseVals_0__33_	 AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]
0x0F120000, //#AfitBaseVals_0__34_	 AFIT8_sddd8a_iHotThreshHigh[7:0],	AFIT8_sddd8a_iHotThreshLow [15:8]
0x0F120000, //#AfitBaseVals_0__35_	 AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]
0x0F120000, //#AfitBaseVals_0__36_	 AFIT8_0xddd8a_Ad,dNoisePower1[7:0],  AFIT8_0xddd8a_Ad,dNoisePower2 [15:8]
0x0F1200FF, //#AfitBaseVals_0__37_	 AFIT8_sddd8a_iSatSat[7:0],   AFIT8_sddd8a_iRadialTune [15:8]
0x0F120A3B, //#AfitBaseVals_0__38_	 AFIT8_sddd8a_iRadialLimit [7:0],	AFIT8_sddd8a_iRadialPower [15:8]
0x0F121414, //#AfitBaseVals_0__39_	 AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],  AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]
0x0F120301, //#AfitBaseVals_0__40_	 AFIT8_sddd8a_iLowSlopeThresh[7:0],   AFIT8_sddd8a_iHighSlopeThresh [15:8]
0x0F12FF07, //#AfitBaseVals_0__41_	 AFIT8_sddd8a_iSquaresRounding [7:0],	AFIT8_Demosaicing_iCentGrad [15:8]
0x0F12081E, //#AfitBaseVals_0__42_	 AFIT8_Demosaicing_iMonochrom [7:0],   AFIT8_Demosaicing_iDecisionThresh [15:8]
0x0F120A1E, //#AfitBaseVals_0__43_	 AFIT8_Demosaicing_iDesatThresh [7:0],	 AFIT8_Demosaicing_iEnhThresh [15:8]
0x0F120F0F, //#AfitBaseVals_0__44_	 AFIT8_Demosaicing_iGRDenoiseVal [7:0],   AFIT8_Demosaicing_iGBDenoiseVal [15:8]
0x0F120A00, //#AfitBaseVals_0__45_	 AFIT8_Demosaicing_iNearGrayDesat[7:0],   AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]
0x0F120012, //#AfitBaseVals_0__46_	 AFIT8_Sharpening_iMSharpen [7:0],	 AFIT8_Sharpening_iMShThresh [15:8]
0x0F12001E, //#AfitBaseVals_0__47_	 AFIT8_Sharpening_iWSharpen [7:0],	 AFIT8_Sharpening_iWShThresh [15:8]
0x0F120002, //#AfitBaseVals_0__48_	 AFIT8_Sharpening_nSharpWidth [7:0],   AFIT8_Sharpening_iReduceNegative [15:8]
0x0F1200FF, //#AfitBaseVals_0__49_	 AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]
0x0F121102, //#AfitBaseVals_0__50_	 AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]
0x0F12001B, //#AfitBaseVals_0__51_	 AFIT8_demsharpmix1_iGCoeff [7:0],	 AFIT8_demsharpmix1_iWideMult [15:8]
0x0F120900, //#AfitBaseVals_0__52_	 AFIT8_demsharpmix1_iNarrMult [7:0],   AFIT8_demsharpmix1_iHystFalloff [15:8]
0x0F120600, //#AfitBaseVals_0__53_	 AFIT8_demsharpmix1_iHystMinMult [7:0],   AFIT8_demsharpmix1_iHystWidth [15:8]
0x0F120504, //#AfitBaseVals_0__54_	 AFIT8_demsharpmix1_iHystFallLow [7:0],   AFIT8_demsharpmix1_iHystFallHigh [15:8]
0x0F120306, //#AfitBaseVals_0__55_	 AFIT8_demsharpmix1_iHystTune [7:0],  * AFIT8_YUV422_DENOISE_iUVSupport [15:8]
0x0F128003, //#AfitBaseVals_0__56_	 AFIT8_YUV422_DENOISE_iYSupport [7:0],	 AFIT8_byr_cgras_iShadingPower [15:8]
0x0F121982, //#AfitBaseVals_0__57_	 AFIT8_RGBGamma2_iLinearity [7:0],	AFIT8_RGBGamma2_iDarkReduce [15:8]
0x0F120480, //0A80	//#AfitBaseVals_0__58_	 AFIT8_ccm_oscar_iSaturation[7:0],	 AFIT8_RGB2YUV_iYOffset [15:8]
0x0F120080, //#AfitBaseVals_0__59_	 AFIT8_RGB2YUV_iRGBGain [7:0],	 AFIT8_RGB2YUV_iSaturation [15:8]
0x0F121414, //#AfitBaseVals_0__60_	 AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]
0x0F120101, //#AfitBaseVals_0__61_	 AFIT8_sddd8a_iClustMulT_H [7:0],	AFIT8_sddd8a_iClustMulT_C [15:8]
0x0F124601, //#AfitBaseVals_0__62_	 AFIT8_sddd8a_nClustLevel_H [7:0],	 AFIT8_sddd8a_DispTH_Low [15:8]
0x0F126444, //#AfitBaseVals_0__63_	 AFIT8_sddd8a_DispTH_High [7:0],   AFIT8_sddd8a_iDenThreshLow [15:8]
0x0F129650, //#AfitBaseVals_0__64_	 AFIT8_sddd8a_iDenThreshHigh[7:0],	 AFIT8_Demosaicing_iEdgeDesat [15:8]
0x0F120000, //#AfitBaseVals_0__65_	 AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],	 AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]
0x0F120003, //#AfitBaseVals_0__66_	 AFIT8_Demosaicing_iEdgeDesatLimit[7:0],  AFIT8_Demosaicing_iDemSharpenLow [15:8]
0x0F121E00, //#AfitBaseVals_0__67_	 AFIT8_Demosaicing_iDemSharpenHigh[7:0],   AFIT8_Demosaicing_iDemSharpThresh [15:8]
0x0F120714, //#AfitBaseVals_0__68_	 AFIT8_Demosaicing_iDemShLowLimit [7:0],   AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]
0x0F121464, //#AfitBaseVals_0__69_	 AFIT8_Demosaicing_iDemBlurLow[7:0],   AFIT8_Demosaicing_iDemBlurHigh [15:8]
0x0F121404, //#AfitBaseVals_0__70_	 AFIT8_Demosaicing_iDemBlurRange[7:0],	 AFIT8_Sharpening_iLowSharpPower [15:8]
0x0F120F14, //#AfitBaseVals_0__71_	 AFIT8_Sharpening_iHighSharpPower[7:0],   AFIT8_Sharpening_iLowShDenoise [15:8]
0x0F12400F, //#AfitBaseVals_0__72_	 AFIT8_Sharpening_iHighShDenoise [7:0],   AFIT8_Sharpening_iReduceEdgeMinMult [15:8]
0x0F120204, //#AfitBaseVals_0__73_	 AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]
0x0F121403, //#AfitBaseVals_0__74_	 AFIT8_demsharpmix1_iNarrFiltReduce [7:0],	AFIT8_sddd8a_iClustThresh_H_Bin [15:8]
0x0F120114, //#AfitBaseVals_0__75_	 AFIT8_sddd8a_iClustThresh_C_Bin [7:0],   AFIT8_sddd8a_iClustMulT_H_Bin [15:8]
0x0F120101, //#AfitBaseVals_0__76_	 AFIT8_sddd8a_iClustMulT_C_Bin [7:0],	AFIT8_sddd8a_nClustLevel_H_Bin [15:8]
0x0F124446, //#AfitBaseVals_0__77_	 AFIT8_sddd8a_DispTH_Low_Bin [7:0],   AFIT8_sddd8a_DispTH_High_Bin [15:8]
0x0F122832, //5064	//#AfitBaseVals_0__78_	 AFIT8_sddd8a_iDenThreshLow_Bin [7:0],	 AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]
0x0F120028, //#AfitBaseVals_0__79_	 AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],   AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]
0x0F120300, //#AfitBaseVals_0__80_	 AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]
0x0F120000, //#AfitBaseVals_0__81_	 AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],  AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]
0x0F12141E, //#AfitBaseVals_0__82_	 AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]
0x0F126407, //#AfitBaseVals_0__83_	 AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]
0x0F120414, //#AfitBaseVals_0__84_	 AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],	AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]
0x0F120A0A, //1414	//#AfitBaseVals_0__85_	 AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]
0x0F120F0F, //#AfitBaseVals_0__86_	 AFIT8_Sharpening_iLowShDenoise_Bin [7:0],	AFIT8_Sharpening_iHighShDenoise_Bin [15:8]
0x0F120440, //#AfitBaseVals_0__87_	 AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]
0x0F120302, //#AfitBaseVals_0__88_	 AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],	AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]
0x0F121414, //#AfitBaseVals_0__89_	 AFIT8_sddd8a_iClustThresh_H_sBin[7:0],   AFIT8_sddd8a_iClustThresh_C_sBin [15:8]
0x0F120101, //#AfitBaseVals_0__90_	 AFIT8_sddd8a_iClustMulT_H_sBin [7:0],	 AFIT8_sddd8a_iClustMulT_C_sBin [15:8]
0x0F124601, //#AfitBaseVals_0__91_	 AFIT8_sddd8a_nClustLevel_H_sBin [7:0],   AFIT8_sddd8a_DispTH_Low_sBin [15:8]
0x0F126E44, //#AfitBaseVals_0__92_	 AFIT8_sddd8a_DispTH_High_sBin [7:0],	AFIT8_sddd8a_iDenThreshLow_sBin [15:8]
0x0F122864, //#AfitBaseVals_0__93_	 AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],   AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]
0x0F120A00, //#AfitBaseVals_0__94_	 AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]
0x0F120003, //#AfitBaseVals_0__95_	 AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],	AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]
0x0F121E00, //#AfitBaseVals_0__96_	 AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],	AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]
0x0F120714, //#AfitBaseVals_0__97_	 AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]
0x0F1232FF, //#AfitBaseVals_0__98_	 AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],	AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]
0x0F120004, //#AfitBaseVals_0__99_	 AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],  AFIT8_Sharpening_iLowSharpPower_sBin [15:8]
0x0F120F00, //#AfitBaseVal0x_0__100_,	 AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]
0x0F12400F, //#AfitBaseVal0x_0__101_,	 AFIT8_Sharpening_iHighShDenoise_sBin [7:0],  AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]
0x0F120204, //#AfitBaseVal0x_0__102_,	 AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],	AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]
0x0F120003, //#AfitBaseVal0x_0__103_,	 AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]

0x0F120000, //#AfitBaseVals_1__0_	 AFIT16_BRIGHTNESS
0x0F120000, //#AfitBaseVals_1__1_	 AFIT16_CONTRAST
0x0F120008, //--#AfitBaseVals_1__2_	 AFIT16_SATURATION
0x0F120000,	//#AfitBaseVals_1__3_	 AFIT16_SHARP_BLUR
0x0F120000, //#AfitBaseVals_1__4_	 AFIT16_GLAMOUR
0x0F1200C1, //#AfitBaseVals_1__5_	 AFIT16_0xddd8a_ed,ge_high
0x0F1203FF, //#AfitBaseVals_1__6_	 AFIT16_Demosaicing_iSatVal
0x0F12009C, //#AfitBaseVals_1__7_	 AFIT16_Sharpening_iReduceEdgeThresh
0x0F12017C, //#AfitBaseVals_1__8_	 AFIT16_demsharpmix1_iRGBOffset
0x0F1203FF, //#AfitBaseVals_1__9_	 AFIT16_demsharpmix1_iDemClamp
0x0F12000C, //#AfitBaseVals_1__10_	 AFIT16_demsharpmix1_iLowThreshold
0x0F120010, //#AfitBaseVals_1__11_	 AFIT16_demsharpmix1_iHighThreshold
0x0F12012C, //#AfitBaseVals_1__12_	 AFIT16_demsharpmix1_iLowBright
0x0F1203E8, //#AfitBaseVals_1__13_	 AFIT16_demsharpmix1_iHighBright
0x0F120046, //#AfitBaseVals_1__14_	 AFIT16_demsharpmix1_iLowSat
0x0F12005A, //#AfitBaseVals_1__15_	 AFIT16_demsharpmix1_iHighSat
0x0F120070, //#AfitBaseVals_1__16_	 AFIT16_demsharpmix1_iTune
0x0F120008, //#AfitBaseVals_1__17_	 AFIT16_demsharpmix1_iHystThLow
0x0F120000, //#AfitBaseVals_1__18_	 AFIT16_demsharpmix1_iHystThHigh
0x0F120320, //#AfitBaseVals_1__19_	 AFIT16_demsharpmix1_iHystCenter
0x0F120046, //#AfitBaseVals_1__20_	 AFIT16_YUV422_DENOISE_iUVLowThresh
0x0F120046, //#AfitBaseVals_1__21_	 AFIT16_YUV422_DENOISE_iUVHighThresh
0x0F120000, //#AfitBaseVals_1__22_	 AFIT16_YUV422_DENOISE_iYLowThresh
0x0F120000, //#AfitBaseVals_1__23_	 AFIT16_YUV422_DENOISE_iYHighThresh
0x0F120064, //#AfitBaseVals_1__24_	 AFIT16_Sharpening_iLowSharpClamp
0x0F120014, //#AfitBaseVals_1__25_	 AFIT16_Sharpening_iHighSharpClamp
0x0F120064, //#AfitBaseVals_1__26_	 AFIT16_Sharpening_iLowSharpClamp_Bin
0x0F120014, //#AfitBaseVals_1__27_	 AFIT16_Sharpening_iHighSharpClamp_Bin
0x0F12003C, //#AfitBaseVals_1__28_	 AFIT16_Sharpening_iLowSharpClamp_sBin
0x0F12001E, //#AfitBaseVals_1__29_	 AFIT16_Sharpening_iHighSharpClamp_sBin
0x0F120A24, //#AfitBaseVals_1__30_	 AFIT8_0xddd8a_ed,ge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]
0x0F121701, //#AfitBaseVals_1__31_	 AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]
0x0F120229, //#AfitBaseVals_1__32_	 AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]
0x0F121403, //#AfitBaseVals_1__33_	 AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]
0x0F120000, //#AfitBaseVals_1__34_	 AFIT8_sddd8a_iHotThreshHigh[7:0],	AFIT8_sddd8a_iHotThreshLow [15:8]
0x0F120000, //#AfitBaseVals_1__35_	 AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]
0x0F120000, //#AfitBaseVals_1__36_	 AFIT8_0xddd8a_Ad,dNoisePower1[7:0],  AFIT8_0xddd8a_Ad,dNoisePower2 [15:8]
0x0F1200FF, //#AfitBaseVals_1__37_	 AFIT8_sddd8a_iSatSat[7:0],   AFIT8_sddd8a_iRadialTune [15:8]
0x0F12033B, //#AfitBaseVals_1__38_	 AFIT8_sddd8a_iRadialLimit [7:0],	AFIT8_sddd8a_iRadialPower [15:8]
0x0F121414, //#AfitBaseVals_1__39_	 AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],  AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]
0x0F120301, //#AfitBaseVals_1__40_	 AFIT8_sddd8a_iLowSlopeThresh[7:0],   AFIT8_sddd8a_iHighSlopeThresh [15:8]
0x0F12FF07, //#AfitBaseVals_1__41_	 AFIT8_sddd8a_iSquaresRounding [7:0],	AFIT8_Demosaicing_iCentGrad [15:8]
0x0F12081E, //#AfitBaseVals_1__42_	 AFIT8_Demosaicing_iMonochrom [7:0],   AFIT8_Demosaicing_iDecisionThresh [15:8]
0x0F120A1E, //#AfitBaseVals_1__43_	 AFIT8_Demosaicing_iDesatThresh [7:0],	 AFIT8_Demosaicing_iEnhThresh [15:8]
0x0F120F0F, //#AfitBaseVals_1__44_	 AFIT8_Demosaicing_iGRDenoiseVal [7:0],   AFIT8_Demosaicing_iGBDenoiseVal [15:8]
0x0F120A00, //#AfitBaseVals_1__45_	 AFIT8_Demosaicing_iNearGrayDesat[7:0],   AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]
0x0F120012, //#AfitBaseVals_1__46_	 AFIT8_Sharpening_iMSharpen [7:0],	 AFIT8_Sharpening_iMShThresh [15:8]
0x0F120005, //#AfitBaseVals_1__47_	 AFIT8_Sharpening_iWSharpen [7:0],	 AFIT8_Sharpening_iWShThresh [15:8]
0x0F120002, //#AfitBaseVals_1__48_	 AFIT8_Sharpening_nSharpWidth [7:0],   AFIT8_Sharpening_iReduceNegative [15:8]
0x0F1200FF, //#AfitBaseVals_1__49_	 AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]
0x0F121102, //#AfitBaseVals_1__50_	 AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]
0x0F12001B, //#AfitBaseVals_1__51_	 AFIT8_demsharpmix1_iGCoeff [7:0],	 AFIT8_demsharpmix1_iWideMult [15:8]
0x0F120900, //#AfitBaseVals_1__52_	 AFIT8_demsharpmix1_iNarrMult [7:0],   AFIT8_demsharpmix1_iHystFalloff [15:8]
0x0F120600, //#AfitBaseVals_1__53_	 AFIT8_demsharpmix1_iHystMinMult [7:0],   AFIT8_demsharpmix1_iHystWidth [15:8]
0x0F120504, //#AfitBaseVals_1__54_	 AFIT8_demsharpmix1_iHystFallLow [7:0],   AFIT8_demsharpmix1_iHystFallHigh [15:8]
0x0F120306, //#AfitBaseVals_1__55_	 AFIT8_demsharpmix1_iHystTune [7:0],  * AFIT8_YUV422_DENOISE_iUVSupport [15:8]
0x0F128003, //#AfitBaseVals_1__56_	 AFIT8_YUV422_DENOISE_iYSupport [7:0],	 AFIT8_byr_cgras_iShadingPower [15:8]
0x0F120A6E, //#AfitBaseVals_1__57_	 AFIT8_RGBGamma2_iLinearity [7:0],	AFIT8_RGBGamma2_iDarkReduce [15:8]
0x0F120080, //#AfitBaseVals_1__58_	 AFIT8_ccm_oscar_iSaturation[7:0],	 AFIT8_RGB2YUV_iYOffset [15:8]
0x0F120080, //#AfitBaseVals_1__59_	 AFIT8_RGB2YUV_iRGBGain [7:0],	 AFIT8_RGB2YUV_iSaturation [15:8]
0x0F125050, //#AfitBaseVals_1__60_	 AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]
0x0F120101, //#AfitBaseVals_1__61_	 AFIT8_sddd8a_iClustMulT_H [7:0],	AFIT8_sddd8a_iClustMulT_C [15:8]
0x0F122801, //#AfitBaseVals_1__62_	 AFIT8_sddd8a_nClustLevel_H [7:0],	 AFIT8_sddd8a_DispTH_Low [15:8]
0x0F12501E, //231E-- //#AfitBaseVals_1__63_	 AFIT8_sddd8a_DispTH_High [7:0],   AFIT8_sddd8a_iDenThreshLow [15:8]
0x0F12963C, //961E-- //#AfitBaseVals_1__64_	 AFIT8_sddd8a_iDenThreshHigh[7:0],	 AFIT8_Demosaicing_iEdgeDesat [15:8]
0x0F120000, //#AfitBaseVals_1__65_	 AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],	 AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]
0x0F120003, //#AfitBaseVals_1__66_	 AFIT8_Demosaicing_iEdgeDesatLimit[7:0],  AFIT8_Demosaicing_iDemSharpenLow [15:8]
0x0F120A02, //#AfitBaseVals_1__67_	 AFIT8_Demosaicing_iDemSharpenHigh[7:0],   AFIT8_Demosaicing_iDemSharpThresh [15:8]
0x0F120764, //#AfitBaseVals_1__68_	 AFIT8_Demosaicing_iDemShLowLimit [7:0],   AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]
0x0F12143C, //#AfitBaseVals_1__69_	 AFIT8_Demosaicing_iDemBlurLow[7:0],   AFIT8_Demosaicing_iDemBlurHigh [15:8]
0x0F121401, //#AfitBaseVals_1__70_	 AFIT8_Demosaicing_iDemBlurRange[7:0],	 AFIT8_Sharpening_iLowSharpPower [15:8]
0x0F120F14, //#AfitBaseVals_1__71_	 AFIT8_Sharpening_iHighSharpPower[7:0],   AFIT8_Sharpening_iLowShDenoise [15:8]
0x0F12400F, //#AfitBaseVals_1__72_	 AFIT8_Sharpening_iHighShDenoise [7:0],   AFIT8_Sharpening_iReduceEdgeMinMult [15:8]
0x0F120204, //#AfitBaseVals_1__73_	 AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]
0x0F125003, //#AfitBaseVals_1__74_	 AFIT8_demsharpmix1_iNarrFiltReduce [7:0],	AFIT8_sddd8a_iClustThresh_H_Bin [15:8]
0x0F120150, //#AfitBaseVals_1__75_	 AFIT8_sddd8a_iClustThresh_C_Bin [7:0],   AFIT8_sddd8a_iClustMulT_H_Bin [15:8]
0x0F120101, //#AfitBaseVals_1__76_	 AFIT8_sddd8a_iClustMulT_C_Bin [7:0],	AFIT8_sddd8a_nClustLevel_H_Bin [15:8]
0x0F121E28, //#AfitBaseVals_1__77_	 AFIT8_sddd8a_DispTH_Low_Bin [7:0],   AFIT8_sddd8a_DispTH_High_Bin [15:8]
0x0F120A0C, //1419	//#AfitBaseVals_1__78_	 AFIT8_sddd8a_iDenThreshLow_Bin [7:0],	 AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]
0x0F120028, //#AfitBaseVals_1__79_	 AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],   AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]
0x0F120300, //#AfitBaseVals_1__80_	 AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]
0x0F120200, //#AfitBaseVals_1__81_	 AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],  AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]
0x0F121E0A, //#AfitBaseVals_1__82_	 AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]
0x0F123C07, //#AfitBaseVals_1__83_	 AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]
0x0F120114, //#AfitBaseVals_1__84_	 AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],	AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]
0x0F120A0A, //1414	//#AfitBaseVals_1__85_	 AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]
0x0F120F0F, //#AfitBaseVals_1__86_	 AFIT8_Sharpening_iLowShDenoise_Bin [7:0],	AFIT8_Sharpening_iHighShDenoise_Bin [15:8]
0x0F120440, //#AfitBaseVals_1__87_	 AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]
0x0F120302, //#AfitBaseVals_1__88_	 AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],	AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]
0x0F121E1E, //#AfitBaseVals_1__89_	 AFIT8_sddd8a_iClustThresh_H_sBin[7:0],   AFIT8_sddd8a_iClustThresh_C_sBin [15:8]
0x0F120101, //#AfitBaseVals_1__90_	 AFIT8_sddd8a_iClustMulT_H_sBin [7:0],	 AFIT8_sddd8a_iClustMulT_C_sBin [15:8]
0x0F123C01, //#AfitBaseVals_1__91_	 AFIT8_sddd8a_nClustLevel_H_sBin [7:0],   AFIT8_sddd8a_DispTH_Low_sBin [15:8]
0x0F125A3A, //#AfitBaseVals_1__92_	 AFIT8_sddd8a_DispTH_High_sBin [7:0],	AFIT8_sddd8a_iDenThreshLow_sBin [15:8]
0x0F122858, //#AfitBaseVals_1__93_	 AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],   AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]
0x0F120A00, //#AfitBaseVals_1__94_	 AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]
0x0F120003, //#AfitBaseVals_1__95_	 AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],	AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]
0x0F121E00, //#AfitBaseVals_1__96_	 AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],	AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]
0x0F120714, //#AfitBaseVals_1__97_	 AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]
0x0F1232FF, //#AfitBaseVals_1__98_	 AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],	AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]
0x0F120004, //#AfitBaseVals_1__99_	 AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],  AFIT8_Sharpening_iLowSharpPower_sBin [15:8]
0x0F120F00, //#AfitBaseVal0x_1__100_,	 AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]
0x0F12400F, //#AfitBaseVal0x_1__101_,	 AFIT8_Sharpening_iHighShDenoise_sBin [7:0],  AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]
0x0F120204, //#AfitBaseVal0x_1__102_,	 AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],	AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]
0x0F120003, //#AfitBaseVal0x_1__103_,	 AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]

0x0F120000, //#AfitBaseVals_2__0_	 AFIT16_BRIGHTNESS
0x0F120000, //#AfitBaseVals_2__1_	 AFIT16_CONTRAST
0x0F120000, //#AfitBaseVals_2__2_	 AFIT16_SATURATION
0x0F120000,	//#AfitBaseVals_2__3_	 AFIT16_SHARP_BLUR
0x0F120000, //#AfitBaseVals_2__4_	 AFIT16_GLAMOUR
0x0F1200C1, //#AfitBaseVals_2__5_	 AFIT16_0xddd8a_ed,ge_high
0x0F1203FF, //#AfitBaseVals_2__6_	 AFIT16_Demosaicing_iSatVal
0x0F12009C, //#AfitBaseVals_2__7_	 AFIT16_Sharpening_iReduceEdgeThresh
0x0F12017C, //#AfitBaseVals_2__8_	 AFIT16_demsharpmix1_iRGBOffset
0x0F1203FF, //#AfitBaseVals_2__9_	 AFIT16_demsharpmix1_iDemClamp
0x0F12000C, //#AfitBaseVals_2__10_	 AFIT16_demsharpmix1_iLowThreshold
0x0F120010, //#AfitBaseVals_2__11_	 AFIT16_demsharpmix1_iHighThreshold
0x0F12012C, //#AfitBaseVals_2__12_	 AFIT16_demsharpmix1_iLowBright
0x0F1203E8, //#AfitBaseVals_2__13_	 AFIT16_demsharpmix1_iHighBright
0x0F120046, //#AfitBaseVals_2__14_	 AFIT16_demsharpmix1_iLowSat
0x0F12005A, //#AfitBaseVals_2__15_	 AFIT16_demsharpmix1_iHighSat
0x0F120070, //#AfitBaseVals_2__16_	 AFIT16_demsharpmix1_iTune
0x0F120008, //#AfitBaseVals_2__17_	 AFIT16_demsharpmix1_iHystThLow
0x0F120000, //#AfitBaseVals_2__18_	 AFIT16_demsharpmix1_iHystThHigh
0x0F120320, //#AfitBaseVals_2__19_	 AFIT16_demsharpmix1_iHystCenter
0x0F120032, //#AfitBaseVals_2__20_	 AFIT16_YUV422_DENOISE_iUVLowThresh
0x0F120032, //#AfitBaseVals_2__21_	 AFIT16_YUV422_DENOISE_iUVHighThresh
0x0F120000, //#AfitBaseVals_2__22_	 AFIT16_YUV422_DENOISE_iYLowThresh
0x0F120000, //#AfitBaseVals_2__23_	 AFIT16_YUV422_DENOISE_iYHighThresh
0x0F1200B4, //#AfitBaseVals_2__24_	 AFIT16_Sharpening_iLowSharpClamp
0x0F120014, //#AfitBaseVals_2__25_	 AFIT16_Sharpening_iHighSharpClamp
0x0F1200B4, //#AfitBaseVals_2__26_	 AFIT16_Sharpening_iLowSharpClamp_Bin
0x0F120014, //#AfitBaseVals_2__27_	 AFIT16_Sharpening_iHighSharpClamp_Bin
0x0F12003C, //#AfitBaseVals_2__28_	 AFIT16_Sharpening_iLowSharpClamp_sBin
0x0F12001E, //#AfitBaseVals_2__29_	 AFIT16_Sharpening_iHighSharpClamp_sBin
0x0F120A24, //#AfitBaseVals_2__30_	 AFIT8_0xddd8a_ed,ge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]
0x0F121701, //#AfitBaseVals_2__31_	 AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]
0x0F120229, //#AfitBaseVals_2__32_	 AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]
0x0F121403, //#AfitBaseVals_2__33_	 AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]
0x0F120000, //#AfitBaseVals_2__34_	 AFIT8_sddd8a_iHotThreshHigh[7:0],	AFIT8_sddd8a_iHotThreshLow [15:8]
0x0F120000, //#AfitBaseVals_2__35_	 AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]
0x0F120000, //#AfitBaseVals_2__36_	 AFIT8_0xddd8a_Ad,dNoisePower1[7:0],  AFIT8_0xddd8a_Ad,dNoisePower2 [15:8]
0x0F1200FF, //#AfitBaseVals_2__37_	 AFIT8_sddd8a_iSatSat[7:0],   AFIT8_sddd8a_iRadialTune [15:8]
0x0F12033B, //#AfitBaseVals_2__38_	 AFIT8_sddd8a_iRadialLimit [7:0],	AFIT8_sddd8a_iRadialPower [15:8]
0x0F121414, //#AfitBaseVals_2__39_	 AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],  AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]
0x0F120301, //#AfitBaseVals_2__40_	 AFIT8_sddd8a_iLowSlopeThresh[7:0],   AFIT8_sddd8a_iHighSlopeThresh [15:8]
0x0F12FF07, //#AfitBaseVals_2__41_	 AFIT8_sddd8a_iSquaresRounding [7:0],	AFIT8_Demosaicing_iCentGrad [15:8]
0x0F12081E, //#AfitBaseVals_2__42_	 AFIT8_Demosaicing_iMonochrom [7:0],   AFIT8_Demosaicing_iDecisionThresh [15:8]
0x0F120A1E, //#AfitBaseVals_2__43_	 AFIT8_Demosaicing_iDesatThresh [7:0],	 AFIT8_Demosaicing_iEnhThresh [15:8]
0x0F120F0F, //#AfitBaseVals_2__44_	 AFIT8_Demosaicing_iGRDenoiseVal [7:0],   AFIT8_Demosaicing_iGBDenoiseVal [15:8]
0x0F120A01, //#AfitBaseVals_2__45_	 AFIT8_Demosaicing_iNearGrayDesat[7:0],   AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]
0x0F120012, //#AfitBaseVals_2__46_	 AFIT8_Sharpening_iMSharpen [7:0],	 AFIT8_Sharpening_iMShThresh [15:8]
0x0F120005, //#AfitBaseVals_2__47_	 AFIT8_Sharpening_iWSharpen [7:0],	 AFIT8_Sharpening_iWShThresh [15:8]
0x0F120001, //#AfitBaseVals_2__48_	 AFIT8_Sharpening_nSharpWidth [7:0],   AFIT8_Sharpening_iReduceNegative [15:8]
0x0F1200FF, //#AfitBaseVals_2__49_	 AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]
0x0F121102, //#AfitBaseVals_2__50_	 AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]
0x0F12001B, //#AfitBaseVals_2__51_	 AFIT8_demsharpmix1_iGCoeff [7:0],	 AFIT8_demsharpmix1_iWideMult [15:8]
0x0F120900, //#AfitBaseVals_2__52_	 AFIT8_demsharpmix1_iNarrMult [7:0],   AFIT8_demsharpmix1_iHystFalloff [15:8]
0x0F120600, //#AfitBaseVals_2__53_	 AFIT8_demsharpmix1_iHystMinMult [7:0],   AFIT8_demsharpmix1_iHystWidth [15:8]
0x0F120504, //#AfitBaseVals_2__54_	 AFIT8_demsharpmix1_iHystFallLow [7:0],   AFIT8_demsharpmix1_iHystFallHigh [15:8]
0x0F120306, //#AfitBaseVals_2__55_	 AFIT8_demsharpmix1_iHystTune [7:0],  * AFIT8_YUV422_DENOISE_iUVSupport [15:8]
0x0F128002, //#AfitBaseVals_2__56_	 AFIT8_YUV422_DENOISE_iYSupport [7:0],	 AFIT8_byr_cgras_iShadingPower [15:8]
0x0F120080, //#AfitBaseVals_2__57_	 AFIT8_RGBGamma2_iLinearity [7:0],	AFIT8_RGBGamma2_iDarkReduce [15:8]
0x0F120080, //#AfitBaseVals_2__58_	 AFIT8_ccm_oscar_iSaturation[7:0],	 AFIT8_RGB2YUV_iYOffset [15:8]
0x0F120080, //#AfitBaseVals_2__59_	 AFIT8_RGB2YUV_iRGBGain [7:0],	 AFIT8_RGB2YUV_iSaturation [15:8]
0x0F125050, //#AfitBaseVals_2__60_	 AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]
0x0F120101, //#AfitBaseVals_2__61_	 AFIT8_sddd8a_iClustMulT_H [7:0],	AFIT8_sddd8a_iClustMulT_C [15:8]
0x0F121B01, //#AfitBaseVals_2__62_	 AFIT8_sddd8a_nClustLevel_H [7:0],	 AFIT8_sddd8a_DispTH_Low [15:8]
0x0F122319, //#AfitBaseVals_2__63_	 AFIT8_sddd8a_DispTH_High [7:0],   AFIT8_sddd8a_iDenThreshLow [15:8]
0x0F12960F, //#AfitBaseVals_2__64_	 AFIT8_sddd8a_iDenThreshHigh[7:0],	 AFIT8_Demosaicing_iEdgeDesat [15:8]
0x0F120000, //#AfitBaseVals_2__65_	 AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],	 AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]
0x0F122A03, //#AfitBaseVals_2__66_	 AFIT8_Demosaicing_iEdgeDesatLimit[7:0],  AFIT8_Demosaicing_iDemSharpenLow [15:8]
0x0F120A02, //#AfitBaseVals_2__67_	 AFIT8_Demosaicing_iDemSharpenHigh[7:0],   AFIT8_Demosaicing_iDemSharpThresh [15:8]
0x0F120864, //#AfitBaseVals_2__68_	 AFIT8_Demosaicing_iDemShLowLimit [7:0],   AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]
0x0F121432, //#AfitBaseVals_2__69_	 AFIT8_Demosaicing_iDemBlurLow[7:0],   AFIT8_Demosaicing_iDemBlurHigh [15:8]
0x0F129601, //#AfitBaseVals_2__70_	 AFIT8_Demosaicing_iDemBlurRange[7:0],	 AFIT8_Sharpening_iLowSharpPower [15:8]
0x0F122814, //#AfitBaseVals_2__71_	 AFIT8_Sharpening_iHighSharpPower[7:0],   AFIT8_Sharpening_iLowShDenoise [15:8]
0x0F12400A, //#AfitBaseVals_2__72_	 AFIT8_Sharpening_iHighShDenoise [7:0],   AFIT8_Sharpening_iReduceEdgeMinMult [15:8]
0x0F120204, //#AfitBaseVals_2__73_	 AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]
0x0F125003, //#AfitBaseVals_2__74_	 AFIT8_demsharpmix1_iNarrFiltReduce [7:0],	AFIT8_sddd8a_iClustThresh_H_Bin [15:8]
0x0F120150, //#AfitBaseVals_2__75_	 AFIT8_sddd8a_iClustThresh_C_Bin [7:0],   AFIT8_sddd8a_iClustMulT_H_Bin [15:8]
0x0F120101, //#AfitBaseVals_2__76_	 AFIT8_sddd8a_iClustMulT_C_Bin [7:0],	AFIT8_sddd8a_nClustLevel_H_Bin [15:8]
0x0F12191B, //#AfitBaseVals_2__77_	 AFIT8_sddd8a_DispTH_Low_Bin [7:0],   AFIT8_sddd8a_DispTH_High_Bin [15:8]
0x0F12070C, //0F19	//#AfitBaseVals_2__78_	 AFIT8_sddd8a_iDenThreshLow_Bin [7:0],	 AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]
0x0F120028, //#AfitBaseVals_2__79_	 AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],   AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]
0x0F120300, //#AfitBaseVals_2__80_	 AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]
0x0F12021E, //#AfitBaseVals_2__81_	 AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],  AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]
0x0F121E0A, //#AfitBaseVals_2__82_	 AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]
0x0F123208, //#AfitBaseVals_2__83_	 AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]
0x0F120114, //#AfitBaseVals_2__84_	 AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],	AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]
0x0F120A28, //1450	//#AfitBaseVals_2__85_	 AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]
0x0F120A28, //#AfitBaseVals_2__86_	 AFIT8_Sharpening_iLowShDenoise_Bin [7:0],	AFIT8_Sharpening_iHighShDenoise_Bin [15:8]
0x0F120440, //#AfitBaseVals_2__87_	 AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]
0x0F120302, //#AfitBaseVals_2__88_	 AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],	AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]
0x0F122828, //#AfitBaseVals_2__89_	 AFIT8_sddd8a_iClustThresh_H_sBin[7:0],   AFIT8_sddd8a_iClustThresh_C_sBin [15:8]
0x0F120101, //#AfitBaseVals_2__90_	 AFIT8_sddd8a_iClustMulT_H_sBin [7:0],	 AFIT8_sddd8a_iClustMulT_C_sBin [15:8]
0x0F122401, //#AfitBaseVals_2__91_	 AFIT8_sddd8a_nClustLevel_H_sBin [7:0],   AFIT8_sddd8a_DispTH_Low_sBin [15:8]
0x0F123622, //#AfitBaseVals_2__92_	 AFIT8_sddd8a_DispTH_High_sBin [7:0],	AFIT8_sddd8a_iDenThreshLow_sBin [15:8]
0x0F122832, //#AfitBaseVals_2__93_	 AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],   AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]
0x0F120A00, //#AfitBaseVals_2__94_	 AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]
0x0F121003, //#AfitBaseVals_2__95_	 AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],	AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]
0x0F121E04, //#AfitBaseVals_2__96_	 AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],	AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]
0x0F120714, //#AfitBaseVals_2__97_	 AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]
0x0F1232FF, //#AfitBaseVals_2__98_	 AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],	AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]
0x0F125004, //#AfitBaseVals_2__99_	 AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],  AFIT8_Sharpening_iLowSharpPower_sBin [15:8]
0x0F120F40, //#AfitBaseVal0x_2__100_,	 AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]
0x0F12400F, //#AfitBaseVal0x_2__101_,	 AFIT8_Sharpening_iHighShDenoise_sBin [7:0],  AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]
0x0F120204, //#AfitBaseVal0x_2__102_,	 AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],	AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]
0x0F120003, //#AfitBaseVal0x_2__103_,	 AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]

0x0F120000, //#AfitBaseVals_3__0_	 AFIT16_BRIGHTNESS
0x0F120000, //#AfitBaseVals_3__1_	 AFIT16_CONTRAST
0x0F120000, //#AfitBaseVals_3__2_	 AFIT16_SATURATION
0x0F120000, //0000	//#AfitBaseVals_3__3_	 AFIT16_SHARP_BLUR
0x0F120000, //#AfitBaseVals_3__4_	 AFIT16_GLAMOUR
0x0F1200C1, //#AfitBaseVals_3__5_	 AFIT16_0xddd8a_ed,ge_high
0x0F1203FF, //#AfitBaseVals_3__6_	 AFIT16_Demosaicing_iSatVal
0x0F12009C, //#AfitBaseVals_3__7_	 AFIT16_Sharpening_iReduceEdgeThresh
0x0F12017C, //#AfitBaseVals_3__8_	 AFIT16_demsharpmix1_iRGBOffset
0x0F1203FF, //#AfitBaseVals_3__9_	 AFIT16_demsharpmix1_iDemClamp
0x0F12000C, //#AfitBaseVals_3__10_	 AFIT16_demsharpmix1_iLowThreshold
0x0F120010, //#AfitBaseVals_3__11_	 AFIT16_demsharpmix1_iHighThreshold
0x0F1200C8, //#AfitBaseVals_3__12_	 AFIT16_demsharpmix1_iLowBright
0x0F1203E8, //#AfitBaseVals_3__13_	 AFIT16_demsharpmix1_iHighBright
0x0F120046, //#AfitBaseVals_3__14_	 AFIT16_demsharpmix1_iLowSat
0x0F120050, //#AfitBaseVals_3__15_	 AFIT16_demsharpmix1_iHighSat
0x0F120070, //#AfitBaseVals_3__16_	 AFIT16_demsharpmix1_iTune
0x0F120008, //#AfitBaseVals_3__17_	 AFIT16_demsharpmix1_iHystThLow
0x0F120000, //#AfitBaseVals_3__18_	 AFIT16_demsharpmix1_iHystThHigh
0x0F120320, //#AfitBaseVals_3__19_	 AFIT16_demsharpmix1_iHystCenter
0x0F120032, //#AfitBaseVals_3__20_	 AFIT16_YUV422_DENOISE_iUVLowThresh
0x0F120032, //#AfitBaseVals_3__21_	 AFIT16_YUV422_DENOISE_iUVHighThresh
0x0F120000, //#AfitBaseVals_3__22_	 AFIT16_YUV422_DENOISE_iYLowThresh
0x0F120000, //#AfitBaseVals_3__23_	 AFIT16_YUV422_DENOISE_iYHighThresh
0x0F1200B4, //#AfitBaseVals_3__24_	 AFIT16_Sharpening_iLowSharpClamp
0x0F120014, //#AfitBaseVals_3__25_	 AFIT16_Sharpening_iHighSharpClamp
0x0F1200B4, //#AfitBaseVals_3__26_	 AFIT16_Sharpening_iLowSharpClamp_Bin
0x0F120014, //#AfitBaseVals_3__27_	 AFIT16_Sharpening_iHighSharpClamp_Bin
0x0F12002D, //#AfitBaseVals_3__28_	 AFIT16_Sharpening_iLowSharpClamp_sBin
0x0F120019, //#AfitBaseVals_3__29_	 AFIT16_Sharpening_iHighSharpClamp_sBin
0x0F120A24, //#AfitBaseVals_3__30_	 AFIT8_0xddd8a_ed,ge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]
0x0F121701, //#AfitBaseVals_3__31_	 AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]
0x0F120229, //#AfitBaseVals_3__32_	 AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]
0x0F121403, //#AfitBaseVals_3__33_	 AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]
0x0F120000, //#AfitBaseVals_3__34_	 AFIT8_sddd8a_iHotThreshHigh[7:0],	AFIT8_sddd8a_iHotThreshLow [15:8]
0x0F120000, //#AfitBaseVals_3__35_	 AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]
0x0F120000, //#AfitBaseVals_3__36_	 AFIT8_0xddd8a_Ad,dNoisePower1[7:0],  AFIT8_0xddd8a_Ad,dNoisePower2 [15:8]
0x0F1200FF, //#AfitBaseVals_3__37_	 AFIT8_sddd8a_iSatSat[7:0],   AFIT8_sddd8a_iRadialTune [15:8]
0x0F12033B, //#AfitBaseVals_3__38_	 AFIT8_sddd8a_iRadialLimit [7:0],	AFIT8_sddd8a_iRadialPower [15:8]
0x0F121414, //#AfitBaseVals_3__39_	 AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],  AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]
0x0F120301, //#AfitBaseVals_3__40_	 AFIT8_sddd8a_iLowSlopeThresh[7:0],   AFIT8_sddd8a_iHighSlopeThresh [15:8]
0x0F12FF07, //#AfitBaseVals_3__41_	 AFIT8_sddd8a_iSquaresRounding [7:0],	AFIT8_Demosaicing_iCentGrad [15:8]
0x0F12081E, //#AfitBaseVals_3__42_	 AFIT8_Demosaicing_iMonochrom [7:0],   AFIT8_Demosaicing_iDecisionThresh [15:8]
0x0F120A1E, //#AfitBaseVals_3__43_	 AFIT8_Demosaicing_iDesatThresh [7:0],	 AFIT8_Demosaicing_iEnhThresh [15:8]
0x0F120F0F, //#AfitBaseVals_3__44_	 AFIT8_Demosaicing_iGRDenoiseVal [7:0],   AFIT8_Demosaicing_iGBDenoiseVal [15:8]
0x0F120A01, //#AfitBaseVals_3__45_	 AFIT8_Demosaicing_iNearGrayDesat[7:0],   AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]
0x0F120012, //#AfitBaseVals_3__46_	 AFIT8_Sharpening_iMSharpen [7:0],	 AFIT8_Sharpening_iMShThresh [15:8]
0x0F120005, //#AfitBaseVals_3__47_	 AFIT8_Sharpening_iWSharpen [7:0],	 AFIT8_Sharpening_iWShThresh [15:8]
0x0F120001, //#AfitBaseVals_3__48_	 AFIT8_Sharpening_nSharpWidth [7:0],   AFIT8_Sharpening_iReduceNegative [15:8]
0x0F1200FF, //#AfitBaseVals_3__49_	 AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]
0x0F121002, //#AfitBaseVals_3__50_	 AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]
0x0F12001E, //#AfitBaseVals_3__51_	 AFIT8_demsharpmix1_iGCoeff [7:0],	 AFIT8_demsharpmix1_iWideMult [15:8]
0x0F120900, //#AfitBaseVals_3__52_	 AFIT8_demsharpmix1_iNarrMult [7:0],   AFIT8_demsharpmix1_iHystFalloff [15:8]
0x0F120600, //#AfitBaseVals_3__53_	 AFIT8_demsharpmix1_iHystMinMult [7:0],   AFIT8_demsharpmix1_iHystWidth [15:8]
0x0F120504, //#AfitBaseVals_3__54_	 AFIT8_demsharpmix1_iHystFallLow [7:0],   AFIT8_demsharpmix1_iHystFallHigh [15:8]
0x0F120307, //#AfitBaseVals_3__55_	 AFIT8_demsharpmix1_iHystTune [7:0],  * AFIT8_YUV422_DENOISE_iUVSupport [15:8]
0x0F128002, //#AfitBaseVals_3__56_	 AFIT8_YUV422_DENOISE_iYSupport [7:0],	 AFIT8_byr_cgras_iShadingPower [15:8]
0x0F120080, //#AfitBaseVals_3__57_	 AFIT8_RGBGamma2_iLinearity [7:0],	AFIT8_RGBGamma2_iDarkReduce [15:8]
0x0F120080, //#AfitBaseVals_3__58_	 AFIT8_ccm_oscar_iSaturation[7:0],	 AFIT8_RGB2YUV_iYOffset [15:8]
0x0F120080, //#AfitBaseVals_3__59_	 AFIT8_RGB2YUV_iRGBGain [7:0],	 AFIT8_RGB2YUV_iSaturation [15:8]
0x0F125050, //#AfitBaseVals_3__60_	 AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]
0x0F120101, //#AfitBaseVals_3__61_	 AFIT8_sddd8a_iClustMulT_H [7:0],	AFIT8_sddd8a_iClustMulT_C [15:8]
0x0F121B01, //#AfitBaseVals_3__62_	 AFIT8_sddd8a_nClustLevel_H [7:0],	 AFIT8_sddd8a_DispTH_Low [15:8]
0x0F122319, //#AfitBaseVals_3__63_	 AFIT8_sddd8a_DispTH_High [7:0],   AFIT8_sddd8a_iDenThreshLow [15:8]
0x0F12960F, //#AfitBaseVals_3__64_	 AFIT8_sddd8a_iDenThreshHigh[7:0],	 AFIT8_Demosaicing_iEdgeDesat [15:8]
0x0F120000, //#AfitBaseVals_3__65_	 AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],	 AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]
0x0F122003, //#AfitBaseVals_3__66_	 AFIT8_Demosaicing_iEdgeDesatLimit[7:0],  AFIT8_Demosaicing_iDemSharpenLow [15:8]
0x0F120A02, //#AfitBaseVals_3__67_	 AFIT8_Demosaicing_iDemSharpenHigh[7:0],   AFIT8_Demosaicing_iDemSharpThresh [15:8]
0x0F120864, //#AfitBaseVals_3__68_	 AFIT8_Demosaicing_iDemShLowLimit [7:0],   AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]
0x0F121432, //#AfitBaseVals_3__69_	 AFIT8_Demosaicing_iDemBlurLow[7:0],   AFIT8_Demosaicing_iDemBlurHigh [15:8]
0x0F12A001, //#AfitBaseVals_3__70_	 AFIT8_Demosaicing_iDemBlurRange[7:0],	 AFIT8_Sharpening_iLowSharpPower [15:8]
0x0F122814, //#AfitBaseVals_3__71_	 AFIT8_Sharpening_iHighSharpPower[7:0],   AFIT8_Sharpening_iLowShDenoise [15:8]
0x0F12400A, //#AfitBaseVals_3__72_	 AFIT8_Sharpening_iHighShDenoise [7:0],   AFIT8_Sharpening_iReduceEdgeMinMult [15:8]
0x0F120204, //#AfitBaseVals_3__73_	 AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]
0x0F125003, //#AfitBaseVals_3__74_	 AFIT8_demsharpmix1_iNarrFiltReduce [7:0],	AFIT8_sddd8a_iClustThresh_H_Bin [15:8]
0x0F120150, //#AfitBaseVals_3__75_	 AFIT8_sddd8a_iClustThresh_C_Bin [7:0],   AFIT8_sddd8a_iClustMulT_H_Bin [15:8]
0x0F120101, //#AfitBaseVals_3__76_	 AFIT8_sddd8a_iClustMulT_C_Bin [7:0],	AFIT8_sddd8a_nClustLevel_H_Bin [15:8]
0x0F12191B, //#AfitBaseVals_3__77_	 AFIT8_sddd8a_DispTH_Low_Bin [7:0],   AFIT8_sddd8a_DispTH_High_Bin [15:8]
0x0F12070C, //0F19	//#AfitBaseVals_3__78_	 AFIT8_sddd8a_iDenThreshLow_Bin [7:0],	 AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]
0x0F120028, //#AfitBaseVals_3__79_	 AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],   AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]
0x0F120300, //#AfitBaseVals_3__80_	 AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]
0x0F12021E, //#AfitBaseVals_3__81_	 AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],  AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]
0x0F121E0A, //#AfitBaseVals_3__82_	 AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]
0x0F123208, //#AfitBaseVals_3__83_	 AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]
0x0F120114, //#AfitBaseVals_3__84_	 AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],	AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]
0x0F120A28, //1450	//#AfitBaseVals_3__85_	 AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]
0x0F120A28, //#AfitBaseVals_3__86_	 AFIT8_Sharpening_iLowShDenoise_Bin [7:0],	AFIT8_Sharpening_iHighShDenoise_Bin [15:8]
0x0F120440, //#AfitBaseVals_3__87_	 AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]
0x0F120302, //#AfitBaseVals_3__88_	 AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],	AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]
0x0F123C3C, //#AfitBaseVals_3__89_	 AFIT8_sddd8a_iClustThresh_H_sBin[7:0],   AFIT8_sddd8a_iClustThresh_C_sBin [15:8]
0x0F120101, //#AfitBaseVals_3__90_	 AFIT8_sddd8a_iClustMulT_H_sBin [7:0],	 AFIT8_sddd8a_iClustMulT_C_sBin [15:8]
0x0F121E01, //#AfitBaseVals_3__91_	 AFIT8_sddd8a_nClustLevel_H_sBin [7:0],   AFIT8_sddd8a_DispTH_Low_sBin [15:8]
0x0F12221C, //#AfitBaseVals_3__92_	 AFIT8_sddd8a_DispTH_High_sBin [7:0],	AFIT8_sddd8a_iDenThreshLow_sBin [15:8]
0x0F12281E, //#AfitBaseVals_3__93_	 AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],   AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]
0x0F120A00, //#AfitBaseVals_3__94_	 AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]
0x0F121403, //#AfitBaseVals_3__95_	 AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],	AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]
0x0F121402, //#AfitBaseVals_3__96_	 AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],	AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]
0x0F12060E, //#AfitBaseVals_3__97_	 AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]
0x0F1232FF, //#AfitBaseVals_3__98_	 AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],	AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]
0x0F125204, //#AfitBaseVals_3__99_	 AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],  AFIT8_Sharpening_iLowSharpPower_sBin [15:8]
0x0F120C40, //#AfitBaseVal0x_3__100_,	 AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]
0x0F124015, //#AfitBaseVal0x_3__101_,	 AFIT8_Sharpening_iHighShDenoise_sBin [7:0],  AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]
0x0F120204, //#AfitBaseVal0x_3__102_,	 AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],	AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]
0x0F120003, //#AfitBaseVal0x_3__103_,	 AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]

0x0F120000, //#AfitBaseVals_4__0_	 AFIT16_BRIGHTNESS
0x0F120000, //#AfitBaseVals_4__1_	 AFIT16_CONTRAST
0x0F120000, //#AfitBaseVals_4__2_	 AFIT16_SATURATION
0x0F120000, //0000	//#AfitBaseVals_4__3_	 AFIT16_SHARP_BLUR
0x0F120000, //#AfitBaseVals_4__4_	 AFIT16_GLAMOUR
0x0F1200C1, //#AfitBaseVals_4__5_	 AFIT16_0xddd8a_ed,ge_high
0x0F1203FF, //#AfitBaseVals_4__6_	 AFIT16_Demosaicing_iSatVal
0x0F12009C, //#AfitBaseVals_4__7_	 AFIT16_Sharpening_iReduceEdgeThresh
0x0F12017C, //#AfitBaseVals_4__8_	 AFIT16_demsharpmix1_iRGBOffset
0x0F1203FF, //#AfitBaseVals_4__9_	 AFIT16_demsharpmix1_iDemClamp
0x0F12000C, //#AfitBaseVals_4__10_	 AFIT16_demsharpmix1_iLowThreshold
0x0F120010, //#AfitBaseVals_4__11_	 AFIT16_demsharpmix1_iHighThreshold
0x0F120032, //#AfitBaseVals_4__12_	 AFIT16_demsharpmix1_iLowBright
0x0F12028A, //#AfitBaseVals_4__13_	 AFIT16_demsharpmix1_iHighBright
0x0F120032, //#AfitBaseVals_4__14_	 AFIT16_demsharpmix1_iLowSat
0x0F1201F4, //#AfitBaseVals_4__15_	 AFIT16_demsharpmix1_iHighSat
0x0F120070, //#AfitBaseVals_4__16_	 AFIT16_demsharpmix1_iTune
0x0F120002, //#AfitBaseVals_4__17_	 AFIT16_demsharpmix1_iHystThLow
0x0F120000, //#AfitBaseVals_4__18_	 AFIT16_demsharpmix1_iHystThHigh
0x0F1201AA, //#AfitBaseVals_4__19_	 AFIT16_demsharpmix1_iHystCenter
0x0F12003C, //#AfitBaseVals_4__20_	 AFIT16_YUV422_DENOISE_iUVLowThresh
0x0F120050, //#AfitBaseVals_4__21_	 AFIT16_YUV422_DENOISE_iUVHighThresh
0x0F120000, //#AfitBaseVals_4__22_	 AFIT16_YUV422_DENOISE_iYLowThresh
0x0F120000, //#AfitBaseVals_4__23_	 AFIT16_YUV422_DENOISE_iYHighThresh
0x0F1200B4, //#AfitBaseVals_4__24_	 AFIT16_Sharpening_iLowSharpClamp
0x0F120014, //#AfitBaseVals_4__25_	 AFIT16_Sharpening_iHighSharpClamp
0x0F1200B4, //#AfitBaseVals_4__26_	 AFIT16_Sharpening_iLowSharpClamp_Bin
0x0F120014, //#AfitBaseVals_4__27_	 AFIT16_Sharpening_iHighSharpClamp_Bin
0x0F120046, //#AfitBaseVals_4__28_	 AFIT16_Sharpening_iLowSharpClamp_sBin
0x0F120019, //#AfitBaseVals_4__29_	 AFIT16_Sharpening_iHighSharpClamp_sBin
0x0F120A24, //#AfitBaseVals_4__30_	 AFIT8_0xddd8a_ed,ge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]
0x0F121701, //#AfitBaseVals_4__31_	 AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]
0x0F120229, //#AfitBaseVals_4__32_	 AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]
0x0F120503, //#AfitBaseVals_4__33_	 AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]
0x0F12080F, //#AfitBaseVals_4__34_	 AFIT8_sddd8a_iHotThreshHigh[7:0],	AFIT8_sddd8a_iHotThreshLow [15:8]
0x0F120808, //#AfitBaseVals_4__35_	 AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]
0x0F120000, //#AfitBaseVals_4__36_	 AFIT8_0xddd8a_Ad,dNoisePower1[7:0],  AFIT8_0xddd8a_Ad,dNoisePower2 [15:8]
0x0F1200FF, //#AfitBaseVals_4__37_	 AFIT8_sddd8a_iSatSat[7:0],   AFIT8_sddd8a_iRadialTune [15:8]
0x0F12022D, //#AfitBaseVals_4__38_	 AFIT8_sddd8a_iRadialLimit [7:0],	AFIT8_sddd8a_iRadialPower [15:8]
0x0F121414, //#AfitBaseVals_4__39_	 AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],  AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]
0x0F120301, //#AfitBaseVals_4__40_	 AFIT8_sddd8a_iLowSlopeThresh[7:0],   AFIT8_sddd8a_iHighSlopeThresh [15:8]
0x0F12FF07, //#AfitBaseVals_4__41_	 AFIT8_sddd8a_iSquaresRounding [7:0],	AFIT8_Demosaicing_iCentGrad [15:8]
0x0F12061E, //#AfitBaseVals_4__42_	 AFIT8_Demosaicing_iMonochrom [7:0],   AFIT8_Demosaicing_iDecisionThresh [15:8]
0x0F120A1E, //#AfitBaseVals_4__43_	 AFIT8_Demosaicing_iDesatThresh [7:0],	 AFIT8_Demosaicing_iEnhThresh [15:8]
0x0F120606, //#AfitBaseVals_4__44_	 AFIT8_Demosaicing_iGRDenoiseVal [7:0],   AFIT8_Demosaicing_iGBDenoiseVal [15:8]
0x0F120A03, //#AfitBaseVals_4__45_	 AFIT8_Demosaicing_iNearGrayDesat[7:0],   AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]
0x0F120028, //#AfitBaseVals_4__46_	 AFIT8_Sharpening_iMSharpen [7:0],	 AFIT8_Sharpening_iMShThresh [15:8]
0x0F120002, //#AfitBaseVals_4__47_	 AFIT8_Sharpening_iWSharpen [7:0],	 AFIT8_Sharpening_iWShThresh [15:8]
0x0F120001, //#AfitBaseVals_4__48_	 AFIT8_Sharpening_nSharpWidth [7:0],   AFIT8_Sharpening_iReduceNegative [15:8]
0x0F1200FF, //#AfitBaseVals_4__49_	 AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]
0x0F121002, //#AfitBaseVals_4__50_	 AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]
0x0F12001E, //#AfitBaseVals_4__51_	 AFIT8_demsharpmix1_iGCoeff [7:0],	 AFIT8_demsharpmix1_iWideMult [15:8]
0x0F120900, //#AfitBaseVals_4__52_	 AFIT8_demsharpmix1_iNarrMult [7:0],   AFIT8_demsharpmix1_iHystFalloff [15:8]
0x0F120600, //#AfitBaseVals_4__53_	 AFIT8_demsharpmix1_iHystMinMult [7:0],   AFIT8_demsharpmix1_iHystWidth [15:8]
0x0F120504, //#AfitBaseVals_4__54_	 AFIT8_demsharpmix1_iHystFallLow [7:0],   AFIT8_demsharpmix1_iHystFallHigh [15:8]
0x0F120307, //#AfitBaseVals_4__55_	 AFIT8_demsharpmix1_iHystTune [7:0],  * AFIT8_YUV422_DENOISE_iUVSupport [15:8]
0x0F128001, //#AfitBaseVals_4__56_	 AFIT8_YUV422_DENOISE_iYSupport [7:0],	 AFIT8_byr_cgras_iShadingPower [15:8]
0x0F120080, //#AfitBaseVals_4__57_	 AFIT8_RGBGamma2_iLinearity [7:0],	AFIT8_RGBGamma2_iDarkReduce [15:8]
0x0F120080, //#AfitBaseVals_4__58_	 AFIT8_ccm_oscar_iSaturation[7:0],	 AFIT8_RGB2YUV_iYOffset [15:8]
0x0F120080, //#AfitBaseVals_4__59_	 AFIT8_RGB2YUV_iRGBGain [7:0],	 AFIT8_RGB2YUV_iSaturation [15:8]
0x0F125050, //#AfitBaseVals_4__60_	 AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]
0x0F120101, //#AfitBaseVals_4__61_	 AFIT8_sddd8a_iClustMulT_H [7:0],	AFIT8_sddd8a_iClustMulT_C [15:8]
0x0F121B01, //#AfitBaseVals_4__62_	 AFIT8_sddd8a_nClustLevel_H [7:0],	 AFIT8_sddd8a_DispTH_Low [15:8]
0x0F121219, //#AfitBaseVals_4__63_	 AFIT8_sddd8a_DispTH_High [7:0],   AFIT8_sddd8a_iDenThreshLow [15:8]
0x0F12320D, //#AfitBaseVals_4__64_	 AFIT8_sddd8a_iDenThreshHigh[7:0],	 AFIT8_Demosaicing_iEdgeDesat [15:8]
0x0F120A0A, //#AfitBaseVals_4__65_	 AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],	 AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]
0x0F122304, //#AfitBaseVals_4__66_	 AFIT8_Demosaicing_iEdgeDesatLimit[7:0],  AFIT8_Demosaicing_iDemSharpenLow [15:8]
0x0F120A08, //#AfitBaseVals_4__67_	 AFIT8_Demosaicing_iDemSharpenHigh[7:0],   AFIT8_Demosaicing_iDemSharpThresh [15:8]
0x0F120832, //#AfitBaseVals_4__68_	 AFIT8_Demosaicing_iDemShLowLimit [7:0],   AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]
0x0F121432, //#AfitBaseVals_4__69_	 AFIT8_Demosaicing_iDemBlurLow[7:0],   AFIT8_Demosaicing_iDemBlurHigh [15:8]
0x0F12A001, //#AfitBaseVals_4__70_	 AFIT8_Demosaicing_iDemBlurRange[7:0],	 AFIT8_Sharpening_iLowSharpPower [15:8]
0x0F122A0A, //#AfitBaseVals_4__71_	 AFIT8_Sharpening_iHighSharpPower[7:0],   AFIT8_Sharpening_iLowShDenoise [15:8]
0x0F124006, //#AfitBaseVals_4__72_	 AFIT8_Sharpening_iHighShDenoise [7:0],   AFIT8_Sharpening_iReduceEdgeMinMult [15:8]
0x0F120604, //#AfitBaseVals_4__73_	 AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]
0x0F125006, //#AfitBaseVals_4__74_	 AFIT8_demsharpmix1_iNarrFiltReduce [7:0],	AFIT8_sddd8a_iClustThresh_H_Bin [15:8]
0x0F120150, //#AfitBaseVals_4__75_	 AFIT8_sddd8a_iClustThresh_C_Bin [7:0],   AFIT8_sddd8a_iClustMulT_H_Bin [15:8]
0x0F120101, //#AfitBaseVals_4__76_	 AFIT8_sddd8a_iClustMulT_C_Bin [7:0],	AFIT8_sddd8a_nClustLevel_H_Bin [15:8]
0x0F12191B, //#AfitBaseVals_4__77_	 AFIT8_sddd8a_DispTH_Low_Bin [7:0],   AFIT8_sddd8a_DispTH_High_Bin [15:8]
0x0F12070C, //0F19	//#AfitBaseVals_4__78_	 AFIT8_sddd8a_iDenThreshLow_Bin [7:0],	 AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]
0x0F120A28, //#AfitBaseVals_4__79_	 AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],   AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]
0x0F12040A, //#AfitBaseVals_4__80_	 AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]
0x0F120820, //#AfitBaseVals_4__81_	 AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],  AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]
0x0F12280A, //#AfitBaseVals_4__82_	 AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]
0x0F123208, //#AfitBaseVals_4__83_	 AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]
0x0F120114, //#AfitBaseVals_4__84_	 AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],	AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]
0x0F120532, //0A64	//#AfitBaseVals_4__85_	 AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]
0x0F12062A, //#AfitBaseVals_4__86_	 AFIT8_Sharpening_iLowShDenoise_Bin [7:0],	AFIT8_Sharpening_iHighShDenoise_Bin [15:8]
0x0F120440, //#AfitBaseVals_4__87_	 AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]
0x0F120606, //#AfitBaseVals_4__88_	 AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],	AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]
0x0F124646, //#AfitBaseVals_4__89_	 AFIT8_sddd8a_iClustThresh_H_sBin[7:0],   AFIT8_sddd8a_iClustThresh_C_sBin [15:8]
0x0F120101, //#AfitBaseVals_4__90_	 AFIT8_sddd8a_iClustMulT_H_sBin [7:0],	 AFIT8_sddd8a_iClustMulT_C_sBin [15:8]
0x0F121801, //#AfitBaseVals_4__91_	 AFIT8_sddd8a_nClustLevel_H_sBin [7:0],   AFIT8_sddd8a_DispTH_Low_sBin [15:8]
0x0F12191C, //#AfitBaseVals_4__92_	 AFIT8_sddd8a_DispTH_High_sBin [7:0],	AFIT8_sddd8a_iDenThreshLow_sBin [15:8]
0x0F122818, //#AfitBaseVals_4__93_	 AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],   AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]
0x0F120A00, //#AfitBaseVals_4__94_	 AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]
0x0F121403, //#AfitBaseVals_4__95_	 AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],	AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]
0x0F121405, //#AfitBaseVals_4__96_	 AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],	AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]
0x0F12050C, //#AfitBaseVals_4__97_	 AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]
0x0F1232FF, //#AfitBaseVals_4__98_	 AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],	AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]
0x0F125204, //#AfitBaseVals_4__99_	 AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],  AFIT8_Sharpening_iLowSharpPower_sBin [15:8]
0x0F121440, //#AfitBaseVal0x_4__100_,	 AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]
0x0F124015, //#AfitBaseVal0x_4__101_,	 AFIT8_Sharpening_iHighShDenoise_sBin [7:0],  AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]
0x0F120204, //#AfitBaseVal0x_4__102_,	 AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],	AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]
0x0F120003, //#AfitBaseVal0x_4__103_,	 AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]
//	param_end	TVAR_afit_pBaseVals
//	param_start afit_pConstBaseVals
0x0F127DFA, //#ConstAfitBaseVals	Because of Edge, disable the iGradientWide
0x0F12FFBD, //#ConstAfitBaseVals_1_
0x0F1226FE, //#ConstAfitBaseVals_2_
0x0F12F7BC, //#ConstAfitBaseVals_3_
0x0F127E06, //#ConstAfitBaseVals_4_
0x0F1200D3, //#ConstAfitBaseVals_5_
//	param_end	afit_pConstBaseVals
//===================================================================
// Brightness setting
//===================================================================
0x00287000,
0x002A10B8, 
0x0F120300,    //bp_uMaxBrightnessFactor	(Max 밝기 증가)
0x002A10BE,
0x0F120300,    //bp_uMinBrightnessFactor (Min 밝기 증가)
// AFIT by Normalized Brightness Tuning parameter
0x002A3780,
0x0F120000, // on/off AFIT by NB option 0000 : Ni  0001:Nb
0x0F120014, // NormBR[0]	//70003782
0x002A3782,
0x0F1200D2, // NormBR[1]	//70003784
0x002A3782,
0x0F120384, // NormBR[2]	//70003786
0x002A3782,
0x0F1207D0, // NormBR[3]	//70003788
0x002A3782,
0x0F121388, // NormBR[4]	//7000378A
0xFFFFFFFF,
};

static const u32 mode_preview_800x600[] = {
0xFCFCD000,
0x00287000,

0x002A0170,                                                                    
0x0F120320, //0500	//0320	//REG_0TC_PCFG_usWidth							2	70000170  800
0x0F120258, //03C0	//0258	//REG_0TC_PCFG_usHeight 						2	70000172  600

0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x002A013C,
0x0F120001,
0xFFFFFFFF,
};

static const  u32 mode_preview_640x480[] = {
0xFCFCD000,
0x00287000,

0x002A0170,                                                            
0x0F120280, //0280	//REG_1TC_PCFG_usWidth							2	70000198  640
0x0F1201E6, //01E0	//REG_1TC_PCFG_usHeight 						2	7000019A  480

0x002A0156,
0x0F120000, //0F120001 //lyg
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x002A013C,
0x0F120001,
0xFFFFFFFF,
};


//add Aalto
static const  u32 mode_preview_320x240[] = {
0xFCFCD000,
0x00287000,

0x002A0170,                                                                    
0x0F120140, //0500	//0320	//REG_0TC_PCFG_usWidth							2	70000170  800
0x0F1200F2, //03C0	//0258	//REG_0TC_PCFG_usHeight 						2	70000172  600

0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x002A013C,
0x0F120001,
0xFFFFFFFF,
};


//add Aalto
static const  u32 mode_preview_176x144[] = {
0xFCFCD000,
0x00287000,

0x002A0170,                                                                    
0x0F1200B0, //0500	//0320	//REG_0TC_PCFG_usWidth							2	70000170  800
0x0F120090, //03C0	//0258	//REG_0TC_PCFG_usHeight 						2	70000172  600

0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x002A013C,
0x0F120001,
0xFFFFFFFF,
};



static const u32 mode_return_camera_preview[] = {
0xFCFCD000,
0x00287000,

0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x002A013C,
0x0F120001,

0xFFFFFFFF,
};

static const u32 mode_preview_800x600_fixframe[] = {
0xFCFCD000,
0x00287000,

0x002A0170,                                                                    
0x0F120320, //0500	//0320	//REG_0TC_PCFG_usWidth							2	70000170  800
0x0F120258, //03C0	//0258	//REG_0TC_PCFG_usHeight 						2	70000172  600

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F1202BC, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x002A013C,
0x0F120001,

0xFFFFFFFF,
};

static const u32 mode_capture_1600x1200[] = {
0xFCFCD000,
0x00287000,

0x002A0238, //LYG
0x0F120001, //REG_0TC_CCFG_uCaptureMode 					2	70000238
0x0F120640, //REG_0TC_CCFG_usWidth							2	7000023A
0x0F1204B0, //REG_0TC_CCFG_usHeight 						2	7000023C

0x002A015E,
0x0F120000,
0x002A0142,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013E,
0x0F120001,
0x002A0140,
0x0F120001,

0xFFFFFFFF,
};


// add Aalto
static const  u32 mode_capture_1280x960[] = {
0xFCFCD000,
0x00287000,
0x002A0238,
0x0F120001, //REG_0TC_CCFG_uCaptureMode 					2	70000238
0x0F120500, //REG_0TC_CCFG_usWidth							2	7000023A
0x0F1203C0, //REG_0TC_CCFG_usHeight 						2	7000023C
0x002A015E,
0x0F120000,
0x002A0142,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013E,
0x0F120001,
0x002A0140,
0x0F120001,
0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_capture_800x600[] = {
0xFCFCD000,
0x00287000,

0x002A0238,
0x0F120001, //REG_0TC_CCFG_uCaptureMode 					2	70000238
0x0F120320, //REG_0TC_CCFG_usWidth							2	7000023A
0x0F120258, //REG_0TC_CCFG_usHeight 						2	7000023C

0x002A015E,
0x0F120000,
0x002A0142,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013E,
0x0F120001,
0x002A0140,
0x0F120001,

0xFFFFFFFF,
};


// add Aalto
static const  u32 mode_capture_640x480[] = {
0xFCFCD000,
0x00287000,

0x002A0238,
0x0F120001, //REG_0TC_CCFG_uCaptureMode 					2	70000238
0x0F120280, //REG_0TC_CCFG_usWidth							2	7000023A
0x0F1201E6, //REG_0TC_CCFG_usHeight 						2	7000023C

0x002A015E,
0x0F120000,
0x002A0142,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013E,
0x0F120001,
0x002A0140,
0x0F120001,

0xFFFFFFFF,
};


// add Aalto
static const  u32 mode_capture_320x240[] = {
0xFCFCD000,
0x00287000,

0x002A0238,
0x0F120001, //REG_0TC_CCFG_uCaptureMode 					2	70000238
0x0F120140, //REG_0TC_CCFG_usWidth							2	7000023A
0x0F1200F2, //REG_0TC_CCFG_usHeight 						2	7000023C

0x002A015E,
0x0F120000,
0x002A0142,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013E,
0x0F120001,
0x002A0140,
0x0F120001,

0xFFFFFFFF,
};



// add Aalto
static const  u32 mode_scene_normal[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C


//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,


0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_scene_night[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C


//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//======================================
//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F1209C4, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F121388, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F121388,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03D0,
0x0F12D090, //#lt_uMaxExp4
0x0F120003, //#lt_uMaxExp4

0x002A03E0,
0x0F12D090, //#lt_uCapMaxExp4
0x0F120003, //#lt_uCapMaxExp4

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_scene_OFF[] = {
// ==========================================================
// 	CAMERA_CENE_OFF
// ==========================================================
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C



//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_scene_PORTRAIT[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C


//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F12FFF8,		//REG_TC_UserSharpBlur


//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,


0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_scene_LANDSCAPE[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C

//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
//Matrix
0x002A10CE,
0x0F120101, //ae_WeightTbl_16[0]	
0x0F120101, //ae_WeightTbl_16[1]	
0x0F120101, //ae_WeightTbl_16[2]	
0x0F120101, //ae_WeightTbl_16[3]
          
0x0F120101, //ae_WeightTbl_16[4]	
0x0F120101, //ae_WeightTbl_16[5]	
0x0F120101, //ae_WeightTbl_16[6]	
0x0F120101, //ae_WeightTbl_16[7]	
          
0x0F120101, //ae_WeightTbl_16[8]	
0x0F120101, //ae_WeightTbl_16[9]	
0x0F120101, //ae_WeightTbl_16[10]	
0x0F120101, //ae_WeightTbl_16[11]	
          
0x0F120101, //ae_WeightTbl_16[12]	
0x0F120101, //ae_WeightTbl_16[13]	
0x0F120101, //ae_WeightTbl_16[14]	
0x0F120101, //ae_WeightTbl_16[15]	
          
0x0F120101, //ae_WeightTbl_16[16]	
0x0F120101, //ae_WeightTbl_16[17]	
0x0F120101, //ae_WeightTbl_16[18]	
0x0F120101, //ae_WeightTbl_16[19]	
          
0x0F120101, //ae_WeightTbl_16[20]	
0x0F120101, //ae_WeightTbl_16[21]	
0x0F120101, //ae_WeightTbl_16[22]	
0x0F120101, //ae_WeightTbl_16[23]	
          
0x0F120101, //ae_WeightTbl_16[24]	
0x0F120101, //ae_WeightTbl_16[25]	
0x0F120101, //ae_WeightTbl_16[26]	
0x0F120101, //ae_WeightTbl_16[27]	
          
0x0F120101, //ae_WeightTbl_16[28]	
0x0F120101, //ae_WeightTbl_16[29]	
0x0F120101, //ae_WeightTbl_16[30]	
0x0F120101, //ae_WeightTbl_16[31]	

0x002A012E,
0x0F120010,		//REG_TC_UserSaturation
0x0F120008,		//REG_TC_UserSharpBlur

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_scene_SPORTS[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C


//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
//Shutter
0x002A03C4,
0x0F121A04, //#lt_uMaxExp1
0x002A03C8,
0x0F123415, //#lt_uMaxExp2
0x002A03CC,
0x0F123415, //#lt_uMaxExp3
0x002A03D0,
0x0F123415, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F121A04, //#lt_uCapMaxExp1
0x002A03D8,
0x0F123415, //#lt_uCapMaxExp2
0x002A03DC,
0x0F123415, //#lt_uCapMaxExp3
0x002A03E0,
0x0F123415, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120260, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120300, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120400, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120460, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F1204C0, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120700, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_scene_PARTY[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C



//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
0x002A035A,
0x0F120001,		//REG_SF_USER_IsoType
0x0F120280,		//REG_SF_USER_IsoVal
0x0F120001,		//REG_SF_USER_IsoChanged

//Saturation - Sharp
0x002A012E,
0x0F120010,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_scene_BEACH[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C



//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
0x002A035A,
0x0F120001,		//REG_SF_USER_IsoType
0x0F120100,		//REG_SF_USER_IsoVal
0x0F120001,		//REG_SF_USER_IsoChanged

0x002A012A,
0x0F120008,

0x002A012E,
0x0F120010,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,

};

// add Aalto
static const  u32 mode_scene_SUNSET[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C


//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
0x002A2402,
0x0F120000,		//Mon_AAIO_bAWB		AWB OFF
0x002A0344,
0x0F120500,		//REG_SF_USER_Rgain
0x0F120001,		//REG_SF_USER_RgainChanged
0x0F120400,		//REG_SF_USER_Ggain
0x0F120001,		//REG_SF_USER_GgainChanged
0x0F120500,		//REG_SF_USER_Bgain
0x0F120001,		//REG_SF_USER_BgainChaged

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_scene_DAWN[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C

//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur



//=====================================
0x002A2402,
0x0F120000,		//Mon_AAIO_bAWB		AWB OFF
0x002A0344,
0x0F1204B0,		//REG_SF_USER_Rgain
0x0F120001,		//REG_SF_USER_RgainChanged
0x0F120400,		//REG_SF_USER_Ggain
0x0F120001,		//REG_SF_USER_GgainChanged
0x0F1208E8,		//REG_SF_USER_Bgain
0x0F120001,		//REG_SF_USER_BgainChaged

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_scene_FALL[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C


//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
0x002A012E,
0x0F120020,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};


// add Aalto
static const  u32 mode_scene_TEXT[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C


//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120010,		//REG_TC_UserSharpBlur

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};
// add Aalto
static const  u32 mode_scene_FIREWORK[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C


//EV 0
0x002A012A,
0x0F120000,


//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
0x002A035A,
0x0F120001,	//REG_SF_USER_IsoType
0x0F120100,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F122710, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F122710,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03D4,
0x0F121A80, //#lt_uCapMaxExp1
0x0F120006, //#lt_uCapMaxExp1
0x002A03D8,
0x0F121A80, //#lt_uCapMaxExp2
0x0F120006, //#lt_uCapMaxExp2
0x002A03DC,
0x0F121A80, //#lt_uCapMaxExp3
0x0F120006, //#lt_uCapMaxExp3
0x002A03E0,
0x0F121A80, //#lt_uCapMaxExp4
0x0F120006, //#lt_uCapMaxExp4

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};


// add Aalto
static const  u32 mode_scene_CANDLE[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C

//EV 0
0x002A012A,
0x0F120000,
0x002A012E,
0x0F120000,

//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
0x002A2402,
0x0F120000,		//Mon_AAIO_bAWB		AWB OFF
0x002A0344,
0x0F120500,		//REG_SF_USER_Rgain
0x0F120001,		//REG_SF_USER_RgainChanged
0x0F120400,		//REG_SF_USER_Ggain
0x0F120001,		//REG_SF_USER_GgainChanged
0x0F120500,		//REG_SF_USER_Bgain
0x0F120001,		//REG_SF_USER_BgainChaged

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_scene_BACKLIGHT[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

0x002A024E, //REG_0TC_PCFG_FrRateQualityType
0x0F120514, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120514,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//Shutter
0x002A03C4,
0x0F123415, //#lt_uMaxExp1
0x002A03C8,
0x0F12681F, //#lt_uMaxExp2
0x002A03CC,
0x0F128227, //#lt_uMaxExp3
0x002A03D0,
0x0F12C350, //#lt_uMaxExp4
0x0F120000, //#lt_uMaxExp4

0x002A03D4,
0x0F123415, //#lt_uCapMaxExp1
0x0F120000, //#lt_uCapMaxExp1
0x002A03D8,
0x0F12681F, //#lt_uCapMaxExp2
0x0F120000, //#lt_uCapMaxExp2
0x002A03DC,
0x0F128227, //#lt_uCapMaxExp3
0x0F120000, //#lt_uCapMaxExp3
0x002A03E0,
0x0F12C350, //#lt_uCapMaxExp4
0x0F120000, //#lt_uCapMaxExp4

0x002A03E4,
0x0F120230, //#lt_uMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120370, //#lt_uMaxAnGain3
0x0F120880, //#lt_uMaxAnGain4
0x0F120100, //#lt_uMaxDigGain
0x0F128000, //#lt_uMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

0x0F120230, //#lt_uCapMaxAnGain1  //01C0:14ea 0230:10ea
0x0F120260, //#lt_uCapMaxAnGain2  //01D0:21ea 0260:18ea
0x0F120380, //#lt_uCapMaxAnGain3
0x0F120880, //#lt_uCapMaxAnGain4
0x0F120100, //#lt_uCapMaxDigGain
0x0F128000, //#lt_uCapMaxTotGain  Total-gain is limited by #lt_uMaxTotGain

//ISO mode off
0x002A035A,
0x0F120000,	//REG_SF_USER_IsoType
0x0F120000,	//REG_SF_USER_IsoVal
0x0F120001,	//REG_SF_USER_IsoChanged

//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C


//EV 0
0x002A012A,
0x0F120000,
0x002A012E,
0x0F120000,

//WB Auto
0x002A2402,
0x0F120001,

//Saturation - Sharp
0x002A012E,
0x0F120000,		//REG_TC_UserSaturation
0x0F120000,		//REG_TC_UserSharpBlur


//=====================================
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]

0x0F120000, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120000, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120000, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120000, //0102, //ae_WeightTbl_16[7]							2	7000105C

0x0F120000, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120101, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120101, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120000, //0102, //ae_WeightTbl_16[11]							2	70001064

0x0F120000, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120F01, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F12010F, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120000, //0102, //ae_WeightTbl_16[15]							2	7000106C

0x0F120000, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120F01, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F12010F, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120000, //0102, //ae_WeightTbl_16[19]							2	70001074

0x0F120000, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120101, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120101, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120000, //0102, //ae_WeightTbl_16[23]							2	7000107C

0x0F120000, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120000, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120000, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120000, //0102, //ae_WeightTbl_16[27]							2	70001084

0x0F120000, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120000, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120000, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120000, //0001, //ae_WeightTbl_16[31]							2	7000108C

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};


// add Aalto
static const  u32 mode_7_FPS[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120594, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120594,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_10_FPS[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F1203E8, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1203E8,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_15_FPS[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F1202BC, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_20_FPS[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F1201F4, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1201F4,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,

0xFFFFFFFF,
};

// add Aalto
static const  u32 mode_25_FPS[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184, //REG_0TC_PCFG_FrRateQualityType
0x0F120190, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F120190,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,


0xFFFFFFFF,
};

static const  u32 mode_Auto_FPS[] = {
0xFCFCD000,
0x00287000,

//frame
0x002A0184,
0x0F120535, //REG_0TC_PCFG_usMaxFrTimeMsecMult10
0x0F1202BC,	//REG_0TC_PCFG_usMinFrTimeMsecMult10

//PREVIEW
0x002A0156,
0x0F120000,
0x002A015E,
0x0F120000,
0x002A015A,
0x0F120001,
0x002A0142,
0x0F120001,
0x002A0158,
0x0F120001,
0x002A0160,
0x0F120001,
0x002A013A,
0x0F120001,
0x0F120001,
0xFFFFFFFF,

};

static const u32 mode_test_pattern[] = {
/*MOTO Test Pattern*/
0x0028D000,
0x002AB054,
0x0F120005, /*5:8bit Mode, 1:10Bit Mode*/
0xFFFFFFFF,
};

static const u32 mode_test_pattern_off[] = {
/*MOTO Test Pattern*/
0x0028D000,
0x002AB054,
0x0F120000, /*5:8bit Mode, 1:10Bit Mode*/
0xFFFFFFFF,
};

static const u32 mode_check_capture_staus[] = {
0x002C7000,
0x002E0142,
0xFFFFFFFF,
};

static const u32 mode_exif_shutterspeed[] = {
0x002C7000,
0x002E238C,
0xFFFFFFFF,
};

static const u32 mode_exif_iso[] = {
0x002C7000,
0x002E2390,
0xFFFFFFFF,
};

static const u32 mode_coloreffect_none[] = {
0xFCFCD000,
0x00287000,
0x002A0138,
0x0F120000,
0xFFFFFFFF,
};

static const u32 mode_coloreffect_mono[] = {
0xFCFCD000,
0x00287000,
0x002A0138,
0x0F120001,
0xFFFFFFFF,
};

static const u32 mode_coloreffect_sepia[] = {
0xFCFCD000,
0x00287000,
0x002A0138,
0x0F120004,
0xFFFFFFFF,
};

static const u32 mode_coloreffect_negative[] = {
0xFCFCD000,
0x00287000,
0x002A0138,
0x0F120003,
0xFFFFFFFF,
};

static const u32 mode_WB_auto[] = {
0xFCFCD000,
0x00287000,
0x002A2402,
0x0F120001,
0xFFFFFFFF,
};


//add Aalto
static const  u32 mode_exposure_centerweighted[] = {
//center Weight
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]
          
0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C
          
0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120202, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120202, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064
          
0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120203, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120302, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C
          
0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120203, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120302, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074
          
0x0F120102, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120202, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120202, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120201, //0102, //ae_WeightTbl_16[23]							2	7000107C
          
0x0F120102, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120202, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120202, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120201, //0102, //ae_WeightTbl_16[27]							2	70001084
          
0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C


0xFFFFFFFF,
};
//add Aalto
static const  u32 mode_exposure_matrix[] = {
0x002A10CE,
0x0F120101, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120101, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120101, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120101, //0001, //ae_WeightTbl_16[3]

0x0F120101, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120101, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120101, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120101, //0102, //ae_WeightTbl_16[7]							2	7000105C

0x0F120101, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120101, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120101, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120101, //0102, //ae_WeightTbl_16[11]							2	70001064

0x0F120101, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120101, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F120101, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120101, //0102, //ae_WeightTbl_16[15]							2	7000106C

0x0F120101, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120101, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F120101, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120101, //0102, //ae_WeightTbl_16[19]							2	70001074

0x0F120101, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120101, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120101, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120101, //0102, //ae_WeightTbl_16[23]							2	7000107C

0x0F120101, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120101, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120101, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120101, //0102, //ae_WeightTbl_16[27]							2	70001084

0x0F120101, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120101, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120101, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120101, //0001, //ae_WeightTbl_16[31]							2	7000108C

0xFFFFFFFF,
};

//add Aalto
static const  u32 mode_exposure_spot[] = {
0x002A10CE,
0x0F120000, //0100, //ae_WeightTbl_16[0]							2	7000104E
0x0F120000, //0101, //ae_WeightTbl_16[1]							2	70001050
0x0F120000, //0101, //ae_WeightTbl_16[2]							2	70001052
0x0F120000, //0001, //ae_WeightTbl_16[3]

0x0F120000, //0201, //ae_WeightTbl_16[4]							2	70001056
0x0F120000, //0303, //ae_WeightTbl_16[5]							2	70001058
0x0F120000, //0303, //ae_WeightTbl_16[6]							2	7000105A
0x0F120000, //0102, //ae_WeightTbl_16[7]							2	7000105C

0x0F120000, //0201, //ae_WeightTbl_16[8]							2	7000105E
0x0F120101, //0403, //ae_WeightTbl_16[9]							2	70001060
0x0F120101, //0304, //ae_WeightTbl_16[10]							2	70001062
0x0F120000, //0102, //ae_WeightTbl_16[11]							2	70001064

0x0F120000, //0201, //ae_WeightTbl_16[12]							2	70001066
0x0F120F01, //0403, //ae_WeightTbl_16[13]							2	70001068
0x0F12010F, //0304, //ae_WeightTbl_16[14]							2	7000106A
0x0F120000, //0102, //ae_WeightTbl_16[15]							2	7000106C

0x0F120000, //0201, //ae_WeightTbl_16[16]							2	7000106E
0x0F120F01, //0403, //ae_WeightTbl_16[17]							2	70001070
0x0F12010F, //0304, //ae_WeightTbl_16[18]							2	70001072
0x0F120000, //0102, //ae_WeightTbl_16[19]							2	70001074

0x0F120000, //0201, //ae_WeightTbl_16[20]							2	70001076
0x0F120101, //0403, //ae_WeightTbl_16[21]							2	70001078
0x0F120101, //0304, //ae_WeightTbl_16[22]							2	7000107A
0x0F120000, //0102, //ae_WeightTbl_16[23]							2	7000107C

0x0F120000, //0201, //ae_WeightTbl_16[24]							2	7000107E
0x0F120000, //0303, //ae_WeightTbl_16[25]							2	70001080
0x0F120000, //0303, //ae_WeightTbl_16[26]							2	70001082
0x0F120000, //0102, //ae_WeightTbl_16[27]							2	70001084

0x0F120000, //0100, //ae_WeightTbl_16[28]							2	70001086
0x0F120000, //0101, //ae_WeightTbl_16[29]							2	70001088
0x0F120000, //0101, //ae_WeightTbl_16[30]							2	7000108A
0x0F120000, //0001, //ae_WeightTbl_16[31]							2	7000108C

0xFFFFFFFF,
};



static const u32 mode_exposure_p4[] = {
0xFCFCD000,
0x00287000,
0x002A012A,
0x0F120024, // ev 24
0x002A012E,
0x0F120024,
0xFFFFFFFF,
};

static const u32 mode_exposure_p3[] = {
0xFCFCD000,
0x00287000,
0x002A012A,
0x0F12001A,
0x002A012E,
0x0F12001A,
0xFFFFFFFF,
};

static const u32 mode_exposure_p2[] = {
0xFCFCD000,
0x00287000,
0x002A012A,
0x0F120010,
0x002A012E,
0x0F120010,
0xFFFFFFFF,
};

static const u32 mode_exposure_p1[] = {
0xFCFCD000,
0x00287000,
0x002A012A,
0x0F120008,
0x002A012E,
0x0F120008,
0xFFFFFFFF,
};

static const u32 mode_exposure_0[] = {
0xFCFCD000,
0x00287000,
0x002A012A,
0x0F120000,
0x002A012E,
0x0F120000,
0xFFFFFFFF,
};

static const u32 mode_exposure_m1[] = {
0xFCFCD000,
0x00287000,
0x002A012A,
0x0F12FFF0,
0x002A012E,
0x0F12FFF0,
0xFFFFFFFF,
};

static const u32 mode_exposure_m2[] = {
0xFCFCD000,
0x00287000,
0x002A012A,
0x0F12FFE0,
0x002A012E,
0x0F12FFE0,
0xFFFFFFFF,
};

static const u32 mode_exposure_m3[] = {
0xFCFCD000,
0x00287000,
0x002A012A,
0x0F12FFD8,
0x002A012E,
0x0F12FFD8,
0xFFFFFFFF,
};

static const u32 mode_exposure_m4[] = {
0xFCFCD000,
0x00287000,
0x002A012A,
0x0F12FFB0,
0x002A012E,
0x0F12FFB0,
0xFFFFFFFF,
};


static const u32 mode_WB_daylight[] = {
0xFCFCD000,
0x00287000,
0x002A2402,
0x0F120000,		//Mon_AAIO_bAWB		AWB OFF
0x002A0344,
0x0F120500,		//REG_SF_USER_Rgain
0x0F120001,		//REG_SF_USER_RgainChanged
0x0F120400,		//REG_SF_USER_Ggain
0x0F120001,		//REG_SF_USER_GgainChanged
0x0F120508,		//REG_SF_USER_Bgain
0x0F120001,		//REG_SF_USER_BgainChaged
0xFFFFFFFF,
};

static const u32 mode_WB_incandescent[] = {
0xFCFCD000,
0x00287000,
0x002A2402,
0x0F120000,		//Mon_AAIO_bAWB		AWB OFF
0x002A0344,
0x0F120310,		//REG_SF_USER_Rgain
0x0F120001,		//REG_SF_USER_RgainChanged
0x0F120400,		//REG_SF_USER_Ggain
0x0F120001,		//REG_SF_USER_GgainChanged
0x0F1209D0,		//REG_SF_USER_Bgain
0x0F120001,		//REG_SF_USER_BgainChaged
0xFFFFFFFF,
};

static const u32 mode_WB_fluorescent[] = {
0xFCFCD000,
0x00287000,
0x002A2402,
0x0F120000,		//Mon_AAIO_bAWB		AWB OFF
0x002A0344,
0x0F1204B8,		//REG_SF_USER_Rgain
0x0F120001,		//REG_SF_USER_RgainChanged
0x0F120400,		//REG_SF_USER_Ggain
0x0F120001,		//REG_SF_USER_GgainChanged
0x0F1208F8,		//REG_SF_USER_Bgain
0x0F120001,		//REG_SF_USER_BgainChaged
0xFFFFFFFF,
};

static const u32 mode_WB_cloudy[] = {
0xFCFCD000,
0x00287000,
0x002A2402,
0x0F120000,		//Mon_AAIO_bAWB		AWB OFF
0x002A0344,
0x0F120638, //REG_SF_USER_Rgain
0x0F120001,	//REG_SF_USER_RgainChanged
0x0F120400, //REG_SF_USER_Ggain  
0x0F120001,	//REG_SF_USER_GgainChanged
0x0F120468, //REG_SF_USER_Bgain
0x0F120001,	//REG_SF_USER_BgainChaged
0xFFFFFFFF,
};

#endif //p5-last





enum {
	S5K5BBGX_MODE_SENSOR_INIT,
	S5K5BBGX_MODE_PREVIEW_640x480,
	S5K5BBGX_MODE_PREVIEW_800x600,
	S5K5BBGX_MODE_CAPTURE_1600x1200,
};

#endif /* ifndef CE147_H */
