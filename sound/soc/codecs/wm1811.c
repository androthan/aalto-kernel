/*
 * wm1811.c  --  Simple register write WM1811 driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <linux/delay.h>
#include <linux/i2c/twl.h>

#include <asm/gpio.h> 
#include <mach/gpio.h> 

#include <linux/mfd/wm1811/core.h>
#include <linux/mfd/wm1811/pdata.h>
#include <linux/mfd/wm1811/gpio.h>

#include "wm1811.h"

/* C2 State Patch */
#include <plat/omap-pm.h> 


//------------------------------------------------
//		Debug Feature
//------------------------------------------------
#define SUBJECT "wm1811.c"
#define TUNING "Audio Tuning"

//#define GPS_TEST_FW		// CODEC LDO Off

//#define WM1811_USE_LINEOUT
#define WM1811_SPK_EQ_EN


struct snd_soc_codec *gCodec;

u16 wm1811_codec_probe_enable = STATUS_OFF;
u16 wm1811_codec_hp_optimized_check = STATUS_OFF;
u16 wm1811_codec_hp_optimized_value = 0;
int wm1811_mode = 0;
int wm1811_old_mode = 0;
int wm1811_call_device = 0;
int wm1811_playback_device = 0;
int wm1811_recording_device = 0;
int wm1811_voip_device = 0;
int wm1811_fm_device = 0;
int wm1811_hf_device = 0;
int wm1811_mic_mute_enable = 0;
int wm1811_rec_8k_enable = 0;
int wm1811_fm_radio_mute_enable = 0;
int wm1811_vr_mode = false;
WM1811_HEADSET_STATUS  wm1811_headset_status;

u16 wm1811_voip_earkey_check = 1;

//retype the path name, sejong
FMRADIO_OWNED_PATH wm1811_radio_path = FM_OFF;
FMRADIO_MUTE_STATUS wm1811_radio_mute = FM_MUTE_OFF;

static const char *audio_path[] = 		{"Playback Path", "Voice Call Path", "Memo Path", "VT Call Path", "VOIP Call Path",  "FM Radio Path",
									"Idle Mode","Mic Mute", "Loopback Path", "Handsfree Path"};
static const char *playback_path[] = 	{"off","RCV","SPK","HP","SPK_HP", "EXTRA_DOCK_SPEAKER"};
static const char *voicecall_path[] =     	{"off","RCV","SPK","3HP","4HP", "BT"};
static const char *voicememo_path[] =	{"off","MAIN","SUB", "HP", "BT"};
static const char *vtcall_path[]=     		{"off","RCV","SPK","3HP","4HP", "BT"};
static const char *voip_path[]=             	{"off","RCV","SPK","3HP","4HP", "BT"};
static const char *fmradio_path[]=        	{"off","RCV","SPK","HP3P","HP4P","BT","SPK_HP"};
static const char *loopback_path[]=      	{"off","RCV","SPK", "HP"};
static const char *idle_mode[] = 		{"on","off"};
static const char *mic_mute[] = 			{"off", "on"};   // hskwon-ss-db05, to support mic mute/unmute for CTS test
static const char *vr_mode[] = 			{"off", "on"};
static const char *bt_dtmf_enum[] =      	{"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
static const char *handsfree_path[] =     	{"off","RCV","SPK","3HP","4HP", "BT"};
/* add by park dong yun for BT Volume Control */
static const char *hands_volume_control[] = {"BT UP","BT DOWN"};

enum power_state		{CODEC_OFF, CODEC_ON };
enum mute_state		{MUTE_SWITCH_ON, MUTE_SWITCH_OFF};
/* add by park dongy un for Digital Analog Mixing table */
static const char *music_stream_on_off[] = {"ON","OFF"};
static const char *music_index_volume[] = {"up","down"};
static const char *codec_status[] = {"on","off"};
static const char *voip_status[] = {"on","off"};


static u16 wm1811_reg_buf[WM1811_REGISTER_COUNT] = {0};


// ---------------------------------------------------
// --------------------Volume Table--------------------
#define VOLUME_TABLE_NUM	31

// Speaker Volume
static u16 uIsMusicStreamOn = 0;
static u16 curMusicStreamOn = 0;
static u16 uMusicStreamVol = 0;


int voipStatus=0;

#define VOL_EUR								//  0     1         2       3       4        5       6        7       8       9
static const int HP_Vol_WW[VOLUME_TABLE_NUM] = {0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29,
					     				       	//   10     11      12     13     14      15     16     17      18      19	
				       						      0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29,
											//   20     21      22,    23      24,     25,     26,     27,     28,    29,     30
				            						      0x29, 0x29, 0x29, 0x29, 0x29,  0x29,  0x2B, 0x2D, 0x30, 0x33, 0x36};

											  //  0     1        2       3       4        5       6       7       8       9
static const int HP_Vol_EUR[VOLUME_TABLE_NUM] = {0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
					     				       	//   10     11      12     13     14      15      16     17     18      19	
				       						      0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
											//   20     21      22,    23     24,     25,     26,    27,    28,     29,     30
				            						      0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2d, 0x2e};


									        	     //  0       1       2        3       4       5       6        7       8       9
static const int FM_HP_Vol_WW[VOLUME_TABLE_NUM] = {0x00, 0x03, 0x06, 0x09, 0x0B, 0x0D, 0x0F, 0x11, 0x13, 0x15,
     											//   10     11      12     13      14      15     16      17      18     19	
											     0x17, 0x19, 0x1B, 0x1D, 0x1F, 0x21, 0x23, 0x25, 0x27, 0x29,
											//   20     21      22,     23     24      25     26      27      28     29     30
											      0x2B, 0x2D, 0x2F, 0x31, 0x31, 0x35, 0x37, 0x39, 0x3B, 0x3D, 0x3E};

								     	        	      //  0       1       2        3       4       5       6        7       8       9
static const int FM_HP_Vol_EUR[VOLUME_TABLE_NUM] = {0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x17, 0x19, 0x1B, 0x1D,
     											//   10     11      12     13      14      15     16      17      18     19	
											     0x1F, 0x21, 0x23, 0x25, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
											//   20     21      22,     23     24      25     26      27      28     29     30
											     0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
/*
								     	        	      //  0       1       2        3       4       5       6        7       8       9
static const int FM_SPK_VOL[VOLUME_TABLE_NUM] = {0x00, 0x03, 0x07, 0x0B, 0x0E, 0x11, 0x14, 0x16, 0x18, 0x1A,
     											//   10     11      12     13      14      15     16      17      18     19	
											     0x1C, 0x1E, 0x20, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
											//   20     21      22,     23     24      25     26      27      28     29     30
											     0x2A, 0x2C, 0x2E, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3D};
*/

           	     	        	      //  0       1    2     3    4      5     6    7     8     9
static const int FM_SPK_VOL[VOLUME_TABLE_NUM] = {0x00, 0x1C, 0x1F, 0x21, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
     	         			     //   10     11   12     13   14     15   16   17    18     19	
		          			 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32,
					     //   20     21   22,   23    24     25   26   27    28    29     30
				                 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D};


#ifdef WM1811_USE_LINEOUT
								     	        	      //  0       1       2        3       4       5       6        7       8       9
static const int LINEOUT_VOL[VOLUME_TABLE_NUM] = {0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29,
					     				       	//   10     11      12     13     14      15     16     17      18      19	
				       						      0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29,
											//   20     21      22,    23      24,     25,     26,     27,     28,    29,     30
				            						      0x29, 0x29, 0x29, 0x29, 0x29,  0x29,  0x2B, 0x2D, 0x30, 0x33, 0x36};
#endif


/*------------------------- Audio Tuning -------------------------*/
int gSpkVolume = 0x3b;		// 0dB->Speaker Volume test	// range:-57dB to 6dB(0x00 to 0x3F), 1dB step
int gHpVolumeEUR = 0x2e;
int gHpTempVolumeEUR = 0x20;
int gHpVolumeWW = 0x36;		// range:-57dB to 6dB(0x00 to 0x3F), 1dB step
int gRcvVolume = 0x00;		// 0x00:0dB, 0x10:-6dB
//int gMainMic = 0x1C;			// 0x0B;->voice Recorder		// range:-16.5dB to 30dB, 1.5dB step
//int gMainMic = 0x1E;			//+28.5dB
int gMainMic = 0x16;			//+16.5dB
int gEarMic = 0x17;			// range:-16.5dB to 30dB, 1.5dB step
int gSubMic = 0x0B;			// range:-16.5dB to 30dB, 1.5dB step
int gVR_MainMic = 0x0B;		// range:-16.5dB to 30dB, 1.5dB step - Voice Recgnition
int gVR_MainMicADC = 0xC0;
int gVR_EarMic = 0x17;		// range:-16.5dB to 30dB, 1.5dB step - Voice Recgnition
int gVR_SubMic = 0x0B;		// range:-16.5dB to 30dB, 1.5dB step - Voice Recgnition
int gLineOutVolume = 0x36;

/* added by jinho.lim : set codec status such as locale version */
#define MID_LOCALE_VOLUME
#ifdef MID_LOCALE_VOLUME
#define CMD_LOCALE_EUR 17
enum {
	LOCALE_WW=0,
	LOCALE_EUR = 1
};
#endif
#define FM_SWITCH_MUTE 0xFF

/* codec private data */
struct wm1811_priv {
	struct snd_soc_codec codec;
	unsigned int codec_powered;

	/* reference counts of AIF/APLL users */
	unsigned int apll_enabled;

	struct snd_pcm_substream *master_substream;
	struct snd_pcm_substream *slave_substream;

	unsigned int configured;
	unsigned int rate;
	unsigned int sample_bits;
	unsigned int channels;
	unsigned int sysclk;

#ifdef MID_LOCALE_VOLUME
	/* added by jinho.lim : set locale to make different volume table as locale (EUR or WW) */
	int locale;
#endif
};


//#define TUNING_ANALOG

#ifdef TUNING_ANALOG // Aalto PROJECT  UPDATE BY LEE SEUNG WOOK FOR SOUND TUNNING 

#include <linux/string.h>
#include <linux/kernel.h>
int isLoadSoundConfig = STATUS_ON;
char *token;
char *last;

int ReadSoundConfigFile(char *Filename, int StartPos)
{
	struct file *filp;
	char * Buffer;
	mm_segment_t oldfs;
	int BytesRead;

	Buffer = kmalloc(256,GFP_KERNEL);
	if (Buffer==NULL) 
		return-1;
	
	filp = filp_open(Filename,00,O_RDONLY);

	if (IS_ERR(filp)||(filp==NULL))	{
		DEBUG_LOG("ReadSoundconfig open errrrrrrr");

		return -1;  /* Or do something else */
       }

	if (filp->f_op->read==NULL)	{
		DEBUG_LOG("ReadSoundconfig open read error");

		return-1;  /* File(system) doesn't allow reads */
       }

	/* Now read 4096 bytes from postion "StartPos" */
	filp->f_pos = StartPos;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	BytesRead = filp->f_op->read(filp,Buffer,256,&filp->f_pos);
	set_fs(oldfs);

	last=Buffer;
	token = strsep(&last, ",");
	gVR_MainMic = simple_strtoul(token, NULL, 16);

	token = strsep(&last, ",");
	gVR_MainMicADC = simple_strtoul(token, NULL, 16);

	DEBUG_LOG("[TUNING_ANALOG]gVR_MainMic=%x\n", gVR_MainMic);
	DEBUG_LOG("[TUNING_ANALOG]gVR_MainMicADC=%x\n", gVR_MainMicADC);

	/* Close the file */
	//fput(filp);

	/* release allocate memeory */
	kfree(Buffer);

	return 0;
}
#endif


int wm1811_read_cacheable(unsigned int reg)
{
	switch (reg) {
	case WM1811_GPIO_1:
	case WM1811_GPIO_2:
	case WM1811_GPIO_3:
	case WM1811_GPIO_4:
	case WM1811_GPIO_5:
	case WM1811_GPIO_6:
	case WM1811_GPIO_7:
	case WM1811_GPIO_8:
	case WM1811_GPIO_9:
	case WM1811_GPIO_10:
	case WM1811_GPIO_11:
	case WM1811_INTERRUPT_STATUS_1:
	case WM1811_INTERRUPT_STATUS_2:
		return 1;
	default:
		break;
	}

	if (reg >= ARRAY_SIZE(access_masks))
		return 0;
	return access_masks[reg].readable != 0;
}


#ifdef WM1811_TUNING_ENABLE
/* write to the wm1811 register space */
int wm1811_write(struct snd_soc_codec *codec, unsigned int reg, unsigned int value)
{
	if (reg >= WM1811_REGISTER_COUNT)	{
		DEBUG_LOG_ERR("reg is over the maximum reg\n");
		return 0;
	}
	
	wm1811_reg_buf[reg] = value;
		
	//dev_dbg(codec->dev, "0x%x = 0x%x\n", reg, value);
	//dev_err(codec->dev, "0x%x = 0x%x\n", reg, value);

	return wm1811_reg_write(codec->control_data, reg, value);
}

static inline unsigned int wm1811_read(struct snd_soc_codec *codec, unsigned int reg)
{
	return wm1811_reg_read(codec->control_data, reg);
}

static inline unsigned int wm1811_read_cache(unsigned int reg)
{
	if (reg >= WM1811_REGISTER_COUNT)	{
		DEBUG_LOG_ERR("reg is over the maximum reg\n");
		return -EIO;
	}

	return wm1811_reg_buf[reg];
}
#endif


int codec_LDO_power_check = 0;
static void wm1811_codec_enable(int enable)
{
	u16 reg = 0;

	reg = omap_readb(GPIO_154_ADDR);

	if (STATUS_OFF == enable)	{
		reg &= ~(0x1 << 4);
		gpio_set_value(CODEC_LDO_ENA, GPIO_LEVEL_LOW);
		codec_LDO_power_check = 1;
	}
	else		{
		reg |= (0x1 << 4);
		gpio_set_value(CODEC_LDO_ENA, GPIO_LEVEL_HIGH);
	}

	omap_writeb(reg, GPIO_154_ADDR);

	DEBUG_LOG("CODEC_LDO_ENA=%d\n", gpio_get_value(CODEC_LDO_ENA));
}


static inline void wm1811_reset_registers(struct snd_soc_codec *codec)
{
	u16 i;
	
	wm1811_write(codec, WM1811_SOFTWARE_RESET, 0x1811);
	msleep(50);
	
	/* set all audio section registers to reasonable defaults */
	for (i=0; i<WM1811_CACHEREGNUM; i++)	{
		if (access_masks[i].readable != 0)
			wm1811_reg_buf[i] = wm1811_reg[i];
	}
}

static void wm1811_init_chip(struct snd_soc_codec *codec)
{
	//struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);

	wm1811_write(codec, WM1811_SOFTWARE_RESET, 0x1811);
	DEBUG_LOG_ERR("WM1811 Audio Codec init \n");
	memcpy (wm1811_reg_buf, wm1811_reg, sizeof(u16) * WM1811_REGISTER_COUNT);

	DEBUG_LOG_ERR("WM1811 Audio Codec init \n");
}

static int wm1811_mic_enable(struct snd_soc_codec *codec, int mode, int enable)
{
	u16 val=0;
	
	DEBUG_LOG_ERR("mic_enable mode : %d, enable : %d\n", mode, enable);

#if 0
	if(wm1811_mic_mute_enable)
	{
		DEBUG_LOG_ERR("wm1811.c mic_enable() : mic muted, do not power on");
		return 0;
	}
#endif

	if (STATUS_ON == enable)	{
		switch (mode)
		{
			case MAIN_MIC:
				// Main MIC ON
				val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM1811_IN1L_VU_MASK | WM1811_IN1L_MUTE_MASK);
				val |= WM1811_IN1L_VU;
				wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, val);

				// HP MIC OFF
				val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_3_4_VOLUME);
				val &= ~(WM1811_IN2L_VU_MASK | WM1811_IN2L_MUTE_MASK);
				val |= WM1811_IN2L_VU | WM1811_IN2L_MUTE;
				wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, val);

				// SUB MIC OFF
				val = wm1811_read_cache(WM1811_RIGHT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM1811_IN1R_VU_MASK | WM1811_IN1R_MUTE_MASK);
				val |= WM1811_IN1R_VU | WM1811_IN1R_MUTE;
				wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, val);

				// MIC BIAS
				wm1811_mic_bias_on_off(codec, MAIN_MIC_ON);
				break;

			case HP_MIC:
				// Main MIC OFF
				val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM1811_IN1L_VU_MASK | WM1811_IN1L_MUTE_MASK);
				val |= WM1811_IN1L_VU | WM1811_IN1L_MUTE;
				wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, val);

				// HP MIC ON
				val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_3_4_VOLUME);
				val &= ~(WM1811_IN2L_VU_MASK | WM1811_IN2L_MUTE_MASK);
				val |= WM1811_IN2L_VU;
				wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, val);

				// SUB MIC OFF
				val = wm1811_read_cache(WM1811_RIGHT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM1811_IN1R_VU_MASK | WM1811_IN1R_MUTE_MASK);
				val |= WM1811_IN1R_VU | WM1811_IN1R_MUTE;
				wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, val);

				// MIC BIAS
				wm1811_mic_bias_on_off(codec, HP_MIC_ON);
				break;

			case SUB_MIC:				
				// Main MIC OFF
				val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM1811_IN1L_VU_MASK | WM1811_IN1L_MUTE_MASK);
				val |= WM1811_IN1L_VU | WM1811_IN1L_MUTE;
				wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, val);

				// HP MIC OFF
				val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_3_4_VOLUME);
				val &= ~(WM1811_IN2L_VU_MASK | WM1811_IN2L_MUTE_MASK);
				val |= WM1811_IN2L_VU | WM1811_IN2L_MUTE;
				wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, val);

				// SUB MIC ON
				val = wm1811_read_cache(WM1811_RIGHT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM1811_IN1R_VU_MASK | WM1811_IN1R_MUTE_MASK);
				val |= WM1811_IN1R_VU;
				wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, val);

				// MIC BIAS
				wm1811_mic_bias_on_off(codec, SUB_MIC_ON);
				break;

			default:
				break;
		}
	}
	else		{
		switch (mode)
		{
			case MAIN_MIC:
				val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM1811_IN1L_VU_MASK | WM1811_IN1L_MUTE_MASK);
				val |= WM1811_IN1L_VU | WM1811_IN1L_MUTE;
				wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, val);

				// MAIN MIC BIAS OFF
				wm1811_mic_bias_on_off(codec, MAIN_MIC_OFF);
				break;

			case HP_MIC:	
				val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_3_4_VOLUME);
				val &= ~(WM1811_IN2L_VU_MASK | WM1811_IN2L_MUTE_MASK);
				val |= WM1811_IN2L_VU | WM1811_IN2L_MUTE;
				wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, val);

				// Keep the ear micbias, when disabled ear recording
				//wm1811_mic_bias_on_off(codec, HP_MIC_OFF);	// HP MIC BIAS OFF
				break;

			case SUB_MIC:				
				val = wm1811_read_cache(WM1811_RIGHT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM1811_IN1R_VU_MASK | WM1811_IN1R_MUTE_MASK);
				val |= WM1811_IN1R_VU | WM1811_IN1R_MUTE;
				wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, val);

				// SUB MIC BIAS
				wm1811_mic_bias_on_off(codec, SUB_MIC_OFF);
				break;

			case MIC_OFF:	
				// Main MIC OFF
				val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM1811_IN1L_VU_MASK | WM1811_IN1L_MUTE_MASK);
				val |= WM1811_IN1L_VU | WM1811_IN1L_MUTE;
				wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, val);

				// HP MIC OFF
				val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_3_4_VOLUME);
				val &= ~(WM1811_IN2L_VU_MASK | WM1811_IN2L_MUTE_MASK);
				val |= WM1811_IN2L_VU | WM1811_IN2L_MUTE;
				wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, val);

				// Sub MIC OFF
				val = wm1811_read_cache(WM1811_RIGHT_LINE_INPUT_1_2_VOLUME);
				val &= ~(WM1811_IN1R_VU_MASK | WM1811_IN1R_MUTE_MASK);
				val |= WM1811_IN1R_VU | WM1811_IN1R_MUTE;
				wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, val);

				// ALL MIC BIAS OFF
				wm1811_mic_bias_on_off(codec, ALL_MIC_OFF);
				break;

			default:
				break;
		}
	}

	return 0;
}


int wm1811_get_codec_mode(void)
{
    //printk("wm1811_get_codec_mode = %d !!!\n", wm1811_mode);

    return wm1811_mode;
}
EXPORT_SYMBOL(wm1811_get_codec_mode);


int wm1811_is_vr_mode(void)
{
	return wm1811_vr_mode;
}
EXPORT_SYMBOL(wm1811_is_vr_mode);


static int wm1811_set_playback_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol,unsigned int codec_mode)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	u16 pre_playback_device=0;
	u16 val=0;
	
	DEBUG_LOG_ERR("wm1811_set_playback_path value = %ld device= %d mode = %d codec_mode = %d \n",ucontrol->value.integer.value[0], wm1811_playback_device, wm1811_mode, codec_mode);

	if(voipStatus==STATUS_ON)
	{
		wm1811_set_voipcall_path(kcontrol,ucontrol,codec_mode,0);
		return 0;
	}
	

	if (HP3P == ucontrol->value.integer.value[0])
		ucontrol->value.integer.value[0] = HP4P;

	DEBUG_LOG("wm1811_recording_device=%d", wm1811_recording_device);

//move to spk setting, sejong
/* 
	// Temporary code for estimated codec bug that speaker output level decreased by AIF1ADCL_ENA
	if (((STATUS_OFF==wm1811_recording_device) || (VOICE_MEMO==wm1811_old_mode)) 
		&& (wm1811_vr_mode == STATUS_OFF))
	{
		DEBUG_LOG("WM1811_AIF1ADCL_ENA Disalbed");
		// Power Management (4)
		val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
		val &= ~(WM1811_AIF1ADCL_ENA_MASK | WM1811_AIF1ADCR_ENA_MASK);
		wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);

		WM1811_Set_Playback_SPK_DRC(codec, STATUS_ON);
	}
*/

	/* restore digital volume for handsfree */
	if (wm1811_hf_device!=0)
	{
		WM1811_Set_Handsfee_Digital_Volume (codec,-1);
	}

	// Prevent howling what floating SPK to Main MIC
#if 0
	if (STATUS_OFF == wm1811_vr_mode)
		wm1811_record_off(codec);
#else
		WM1811_Set_ADC_To_DAC_Sidetone(codec, STATUS_OFF);
#endif

#if 0
	wm1811_call_device = 0;
	wm1811_fm_device = 0;
	wm1811_recording_device = 0;
#endif

	if ( wm1811_playback_device == ucontrol->value.integer.value[0] ) //DB24 sec_lilkan
	{
		DEBUG_LOG_ERR("wm1811_set_playback_device same device d=%d  \n",wm1811_playback_device);
/* ensure the codec setting, sejong
		return WM1811_SAME_DEVICE;
*/
	}
	else
	{
		switch (wm1811_playback_device)
		{
			case OFF:
				break;

			case RCV:
				WM1811_Set_Playback_RCV_Off(codec);
				break;

			case SPK:
				WM1811_Set_Playback_SPK_Off(codec);
				break;
				
			case EXTRA_SPEAKER:
				WM1811_Set_Playback_Dock_Off(codec);
				break;

			case HP3P:
			case HP4P:
				if(ucontrol->value.integer.value[0] != SPK_HP)
					WM1811_Set_Playback_EAR_Off(codec);
				break;

			case SPK_HP:
				break;

			default:
				DEBUG_LOG_ERR("!!!!playback path setting failed!!!\n");
				break;
		}
	}

	pre_playback_device = wm1811_playback_device;
	wm1811_playback_device = ucontrol->value.integer.value[0];

	WM1811_Set_Playback_SPK_DRC(codec, STATUS_OFF);

	switch (wm1811_playback_device)
	{
		case OFF:
			break;

		case RCV:
			WM1811_Set_Playback_RCV(codec);
			break;

		case SPK:
		#if 1
			if (SPK_HP==pre_playback_device)	{
				WM1811_Set_Playback_EAR_Off(codec);
				return 0;
			}
			
			WM1811_Set_Playback_SPK(codec);
		#else
			WM1811_Set_Playback_RCV(codec);
		#endif
			break;
			
		case EXTRA_SPEAKER:
			WM1811_Set_Playback_Dock(codec);
			break;

		case HP3P:
		case HP4P:
			/* remove condition because we down gain for SPK_HP */
			//if (SPK_HP==pre_playback_device)	{
				WM1811_Set_Playback_SPK_Off(codec);
			//	return 0;
			//}
			//if(pre_playback_device != SPK_HP)
				WM1811_Set_Playback_EAR(codec);
			break;

		case SPK_HP:
			WM1811_Set_Playback_SPK(codec);
			//WM1811_Set_Playback_EAR(codec);

#if 1
			// add by park dong yun down gain for Seapker ear phone Path 
			{
				u16 val=0;

				// Left output volume
				val = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
				val &= ~(WM1811_HPOUT1L_VOL_MASK | WM1811_HPOUT1L_VU_MASK |
						WM1811_HPOUT1L_MUTE_N_MASK | WM1811_HPOUT1L_ZC_MASK);
				val |= (WM1811_HPOUT1L_VU | WM1811_HPOUT1L_ZC | WM1811_HPOUT1L_MUTE_N);
				val |= 0x20;
				wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val);

				// Left output volume
				val = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
				val &= ~(WM1811_HPOUT1R_VOL_MASK | WM1811_HPOUT1R_VU_MASK |
						WM1811_HPOUT1R_MUTE_N_MASK | WM1811_HPOUT1R_ZC_MASK);
				val |= (WM1811_HPOUT1R_VU | WM1811_HPOUT1R_ZC | WM1811_HPOUT1R_MUTE_N);
				val |= 0x20;
				wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val);

				gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_OFF);
			}
#endif
			break;

		default:
			DEBUG_LOG_ERR("!!!!playback path setting failed!!!\n");
			break;
	}
     wm1811_hf_device=0;
	wm1811_voip_device=0;
	 
	return 0;
}


static int wm1811_set_voicecall_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol,unsigned int codec_mode)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	
	DEBUG_LOG_ERR("wm1811_set_voicecall_path value = %ld call_device = %d, mode = %d \n",ucontrol->value.integer.value[0], wm1811_call_device, codec_mode);

	if((wm1811_call_device == ucontrol->value.integer.value[0])) //DB24 sec_lilkan
	{
		DEBUG_LOG_ERR("wm1811_set_voicecall_path same device d=%d  \n",wm1811_call_device);

/* for aparent codec setting, sejong
		return WM1811_SAME_DEVICE;
*/
	}
	wm1811_hf_device=0;
	wm1811_fm_device = 0;
	//wm1811_recording_device = 0;
	wm1811_playback_device = 0;
	wm1811_call_device = ucontrol->value.integer.value[0];

	wm1811_record_off(codec);

	switch (wm1811_voip_device)
	{
		case OFF:	
			break;

		case RCV:
			// MIC : Main
			wm1811_mic_enable(codec, MAIN_MIC, STATUS_ON);
			wm1811_main_mic_path_enable(codec);

			// Speaker
			WM1811_Set_Playback_SPK_Off(codec);

			// Earphone
			WM1811_Set_Playback_EAR_Off(codec);

			// Receiver
			WM1811_Set_Playback_RCV(codec);
			break;

		case SPK:
			// MIC : Sub
			wm1811_mic_enable(codec, SUB_MIC, STATUS_ON);
			wm1811_sub_mic_path_enable(codec);

			// Speaker
			WM1811_Set_Playback_SPK(codec);

			// Volume
			WM1811_Set_Playback_EAR_Volume(codec, STATUS_OFF);
			WM1811_Set_Playback_RCV_Volume(codec, STATUS_OFF);
			break;

		case HP3P:
			// MIC : Main
			wm1811_mic_enable(codec, MAIN_MIC, STATUS_ON);
			wm1811_main_mic_path_enable(codec);

			// DRC
			WM1811_Set_Playback_SPK_DRC(codec, STATUS_OFF);

			// Earphone
			WM1811_Set_Playback_EAR(codec);

			// Volume
			WM1811_Set_Playback_SPK_Volume(codec, STATUS_OFF);
			WM1811_Set_Playback_EAR_Volume(codec, STATUS_ON);
			WM1811_Set_Playback_RCV_Volume(codec, STATUS_OFF);
			break;
			
		case HP4P:
			// MIC : HP
			wm1811_mic_enable(codec, HP_MIC, STATUS_ON);
			wm1811_ear_mic_path_enable(codec);

			// DRC
			WM1811_Set_Playback_SPK_DRC(codec, STATUS_OFF);

			// Earphone
			WM1811_Set_Playback_EAR(codec);

			// Volume
			WM1811_Set_Playback_SPK_Volume(codec, STATUS_OFF);
			WM1811_Set_Playback_EAR_Volume(codec, STATUS_ON);
			WM1811_Set_Playback_RCV_Volume(codec, STATUS_OFF);
			break;

		case BT:
			break;

		default:
			DEBUG_LOG_ERR("!!!!voicecall path setting failed!!!\n");
			break;
	}

	return 0;
}

static int wm1811_set_handsfree_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol,unsigned int codec_mode)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	
	DEBUG_LOG_ERR("wm1811_set_handsfree_path value = %ld wm1811_hf_device = %d, mode = %d \n",ucontrol->value.integer.value[0], wm1811_hf_device, codec_mode);

	if((wm1811_hf_device == ucontrol->value.integer.value[0])) //DB24 sec_lilkan
	{
		DEBUG_LOG_ERR("wm1811_set_handsfree_path same device d=%d  \n",wm1811_hf_device);
		//return WM1811_SAME_DEVICE;
	}
	wm1811_hf_device = ucontrol->value.integer.value[0];
	wm1811_fm_device = 0;
	wm1811_recording_device = 0;
	wm1811_playback_device = 0; 
	wm1811_call_device=0;

	wm1811_record_off(codec);
	wm1811_mic_enable(codec, MIC_OFF, STATUS_OFF);
       wm1811_set_playback_hf_clocking(codec);	   

	switch(ucontrol->value.integer.value[0])
	{
		case OFF:	
			break;

		case RCV:
			// DRC
			WM1811_Set_Playback_SPK_DRC(codec, STATUS_ON);
			WM1811_Set_Playback_SPK_Off(codec);
			
			wm1811_mic_enable(codec, MAIN_MIC, STATUS_ON);
			WM1811_Set_Playback_RCV(codec);
			wm1811_set_playback_hf_rcv(codec);
			break;

		case SPK:
			// DRC
			WM1811_Set_Playback_SPK_DRC(codec, STATUS_ON);
			
			wm1811_mic_enable(codec, SUB_MIC, STATUS_ON);
			wm1811_set_playback_hf_spk(codec);
			break;

		case HP3P:
		case HP4P:
			// DRC
			WM1811_Set_Playback_SPK_DRC(codec, STATUS_OFF);
			WM1811_Set_Playback_SPK_Off(codec);
			WM1811_Set_Playback_RCV_Off(codec);
			
			wm1811_mic_enable(codec, HP_MIC, STATUS_ON);
			wm1811_set_playback_hf_hp(codec);
			break;

		default:
			DEBUG_LOG_ERR("!!!!voicecall path setting failed!!!\n");
			break;
	}

	return 0;
}

static void wm1811_set_pcm_sel(int mode)
{
	DEBUG_LOG_ERR("wm1811_set_pcm_sel mode %d\n",  mode);
	if(mode == BT_SEL_PCM_MODE){

	}else if(mode == BT_SEL_I2S_MODE){

	}else if(mode == BT_SEL_LOW_MODE){

	}else
		DEBUG_LOG_ERR("ERR!! wm1811_set_pcm_sel() %d\n", mode);
}

static int wm1811_set_voicememo_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol,unsigned int codec_mode)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

#if 0
	wm1811_playback_device = 0;
	wm1811_call_device = 0;
	wm1811_fm_device = 0;
#endif
	
	DEBUG_LOG("cur path = %ld, prev path = %d\n", ucontrol->value.integer.value[0], wm1811_recording_device);

	if (wm1811_recording_device == ucontrol->value.integer.value[0])	{
		DEBUG_LOG("Same to mic device, old=%d, cur=%ld", wm1811_recording_device, ucontrol->value.integer.value[0]);
/* for aparent codec setting, sejong
		return 0;
*/
	}

	//wm1811_record_off(codec);
	wm1811_voip_device = 0;

	// Prevent howling what floating SPK to Main MIC
	WM1811_Set_ADC_To_DAC_Sidetone(codec, STATUS_OFF);

	//WM1811_Set_Playback_EAR_Off(codec);
	//WM1811_Set_Playback_SPK_Off(codec);

	// DRC
	WM1811_Set_Playback_SPK_DRC(codec, STATUS_OFF);

#if 0
	switch (wm1811_recording_device)
	{
		case OFF:
			break;

		case MAIN_MIC:
			wm1811_mic_enable(codec, MAIN_MIC, STATUS_OFF);
			wm1811_record_main_mic(codec, STATUS_OFF);
			break;

		case HP_MIC:
			wm1811_mic_enable(codec, HP_MIC, STATUS_OFF);
			wm1811_record_ear_mic(codec, STATUS_OFF);
       		break;

		case SUB_MIC:
			wm1811_mic_enable(codec, SUB_MIC, STATUS_OFF);
			wm1811_record_sub_mic(codec, STATUS_OFF);
			break;

		case BT_MIC:
			wm1811_set_pcm_sel(BT_SEL_I2S_MODE);
			break;
	}
#endif

	wm1811_recording_device =ucontrol->value.integer.value[0];

	switch (wm1811_recording_device)
	{
		case OFF:
			break;

		case MAIN_MIC:
			wm1811_mic_enable(codec, MAIN_MIC, STATUS_ON);
			wm1811_record_main_mic(codec, STATUS_ON);
			break;

		case HP_MIC:
			wm1811_mic_enable(codec, HP_MIC, STATUS_ON);
			wm1811_record_ear_mic(codec, STATUS_ON);
       		break;

		case SUB_MIC:
			wm1811_mic_enable(codec, SUB_MIC, STATUS_ON);
			wm1811_record_sub_mic(codec, STATUS_ON);
			break;

		case BT_MIC:
			wm1811_set_pcm_sel(BT_SEL_I2S_MODE);
			break;
	}

	return 0;
}

// hskwon-ss-cl31, added for FMC(VoIP) call path
static int wm1811_set_voipcall_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol,unsigned int codec_mode, int forceDevice )
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	int newDevice=0;
	//unsigned int i=0;

	/* force Control for voip start */
	if (0 ==forceDevice)
	{
		newDevice= ucontrol->value.integer.value[0];
	}else 
	{
		newDevice=forceDevice;
	}
	 DEBUG_LOG_ERR("wm1811_set_voipcall_path newDevice = %d wm1811_voip_device = %d forceDevice = %d \n",newDevice, wm1811_voip_device, forceDevice );

  	if( wm1811_voip_device == newDevice)
	 {
	 	DEBUG_LOG_ERR("wm1811_set_voipcall_path same device\n ");	  	
		//return 0;
	} 


#if 0
	switch (wm1811_voip_device)
	{
		case OFF:	
			break;

		case RCV:
			// MIC : Main
		
			wm1811_main_mic_path_disable(codec);

			// RCV
			WM1811_Set_Playback_RCV_Off(codec);
			break;

		case SPK:
			// MIC : Sub
			wm1811_mic_enable(codec, SUB_MIC, STATUS_OFF);
			wm1811_sub_mic_path_disable(codec);

			// SPK
			WM1811_Set_Playback_SPK_Off(codec);
			break;

		case HP3P:
			// MIC : Main
			wm1811_mic_enable(codec, MAIN_MIC, STATUS_OFF);
			wm1811_main_mic_path_disable(codec);

			// HP3P
			WM1811_Set_Playback_EAR_Off(codec);
			break;
			
		case HP4P:
			// MIC : HP
			wm1811_mic_enable(codec, HP_MIC, STATUS_OFF);
			wm1811_ear_mic_path_disable(codec);

			// HP4P
			WM1811_Set_Playback_EAR_Off(codec);
			break;

		case BT:
			break;

		default:
			DEBUG_LOG_ERR("!!!!voipcall path setting failed!!!\n");

		break;
	}
#endif 

	// shut up all device 
	//wm1811_record_off(codec); 
	WM1811_Set_Playback_SPK_DRC(codec, STATUS_OFF);
	//wm1811_mic_enable(codec, MIC_OFF, STATUS_OFF);
	WM1811_Set_Playback_SPK_Off(codec);
	WM1811_Set_Playback_RCV_Off(codec);
	WM1811_Set_Playback_EAR_Off(codec);
	gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_ON);

	// Left line input 1&2 volume
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, WM1811_IN1L_RESET);
	// Left line input 3&4 volume : MIC gain control
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, WM1811_IN2L_RESET);
	// Right line input 1&2 volume
	wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, WM1811_INR1_RESET);
	// Input mixer 2
	wm1811_write(codec, WM1811_INPUT_MIXER_2, WM1811_INMIX2_RESET);
	// Input mixer 3
	wm1811_write(codec, WM1811_INPUT_MIXER_3, WM1811_IN2LMIX_RESET);	
	// Input mixer 4
	wm1811_write(codec, WM1811_INPUT_MIXER_4, WM1811_INMIX4_RESET);
	switch (newDevice)
	{
		case OFF:	
			break;

		case RCV:
			// MIC : Main
			wm1811_mic_enable(codec, MAIN_MIC, STATUS_ON);
			wm1811_main_mic_path_enable(codec);

			// RCV
			WM1811_Set_Playback_RCV(codec);
			break;

		case SPK:
			// SPK
			WM1811_Set_Playback_SPK(codec);

			wm1811_sub_mic_path_enable(codec);
			// MIC : Sub
			wm1811_mic_enable(codec, SUB_MIC, STATUS_ON);
			break;

		case HP3P:			
			// MIC : Main
			wm1811_mic_enable(codec, MAIN_MIC, STATUS_ON);
			wm1811_main_mic_path_enable(codec);

			// HP3P
			WM1811_Set_Playback_EAR(codec);
			gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_OFF);
			break;
			
		case HP4P:
			// MIC : HP
			wm1811_mic_enable(codec, HP_MIC, STATUS_ON);
			wm1811_ear_mic_path_enable(codec);
			/* this is temporary code for check recording */
//			wm1811_mic_enable(codec, MAIN_MIC, STATUS_ON);
//			wm1811_main_mic_path_enable(codec);

			// HP4P
			WM1811_Set_Playback_EAR(codec);
			gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_OFF);
			break;

		case BT:
			break;

		default:
			DEBUG_LOG_ERR("!!!!voipcall path setting failed!!!\n");

		break;
	}

	wm1811_voip_device = newDevice;
	wm1811_call_device = 0;
	wm1811_fm_device = 0;
	wm1811_recording_device = 0;
	wm1811_playback_device=0;
	
	return 0;
}

static int wm1811_set_fmradio_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol,unsigned int codec_mode)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	if (HEADSET_DISCONNECT == wm1811_headset_status)	{
		DEBUG_LOG_ERR("HEADSET_DISCONNET");
/* for aparent codec setting, sejong
		return 0;
*/		
	}

	if((wm1811_fm_device == ucontrol->value.integer.value[0])) //DB24 sec_lilkan
	{
		DEBUG_LOG_ERR("wm1811_set_fmradio_path same device d=%d mute = %d \n",wm1811_fm_device, wm1811_fm_radio_mute_enable);

		//return WM1811_SAME_DEVICE;
	}
	
	//fm and playback don't occur at the same time. sejong.
	wm1811_playback_device = 0;
	// wm1811_call_device = 0;
	 //wm1811_recording_device = 0;
/*
	 if (STATUS_OFF==wm1811_fm_device)
	 {
		wm1811_Set_FM_Radio(codec);
	 }
*/
 	//Mute Off when the FM Path Opens, sejong
 	/*
	if(wm1811_radio_mute == FM_MUTE_OFF)
	{
		DEBUG_LOG_ERR("FM First enabled.");
		wm1811_Set_FM_Mute_Switch(codec, FM_MUTE_OFF);
	}*/


	 DEBUG_LOG_ERR("wm1811_set_fmradio_path value = %ld output_device = %d, mute = %d \n",
					ucontrol->value.integer.value[0], wm1811_fm_device, wm1811_fm_radio_mute_enable);

#if 0
	 switch (wm1811_fm_device)
	 {
		case OFF:
			wm1811_radio_path = 0;
			break;

		case SPK:
			wm1811_radio_path = 1;

			WM1811_Set_Playback_SPK_Off(codec);
			break;

		case HP3P:
		case HP4P:
			wm1811_radio_path = 0;

			WM1811_Set_Playback_EAR_Off(codec);
			break;

		 case SPK_HP:
			WM1811_Set_Playback_SPK_Off(codec);
			WM1811_Set_Playback_EAR_Off(codec);
			break;

		default:
			DEBUG_LOG_ERR("!!!!fmradio path setting failed!!!\n");
			break;
 	}
#endif
	 
	 wm1811_fm_device = ucontrol->value.integer.value[0];

	 WM1811_Set_Playback_SPK_DRC(codec, STATUS_OFF);

	 switch (wm1811_fm_device)
	 {
		case OFF:
			DEBUG_LOG_ERR("!!!!!!!!!!!!!!!!!!!!FM_OFF");
			wm1811_radio_path = FM_OFF;
			break;

		case SPK:
			wm1811_radio_path = FM_SPK;
			WM1811_Set_Playback_EAR_Off(codec);
			WM1811_Set_Playback_SPK(codec);
			break;

		case HP3P:
		case HP4P:
			wm1811_radio_path = FM_HP;
			WM1811_Set_Playback_SPK_Off(codec);
			WM1811_Set_Playback_EAR(codec);
			break;

		 case SPK_HP:
			wm1811_radio_path = FM_DUAL;
			WM1811_Set_Playback_SPK(codec);
			WM1811_Set_Playback_EAR(codec);
			break;

		default:
			DEBUG_LOG_ERR("!!!!fmradio path setting failed!!!\n");
			break;
	}
	DEBUG_LOG_ERR("FM First enabled.");

	 //if (STATUS_OFF==wm1811_fm_device)
	 {
		wm1811_Set_FM_Radio(codec);
	 }
	
	 wm1811_Set_FM_Mute_Switch(codec, FM_MUTE_OFF);

	 return 0;
}


static int wm1811_set_idle_mode(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol,unsigned int codec_mode)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	
	DEBUG_LOG_ERR("wm1811_set_idle_mode value = %ld \n",ucontrol->value.integer.value[0]);
	DEBUG_LOG_ERR("wm1811_set  idle_mode=%d cur_mode=%d\n", wm1811_old_mode, wm1811_mode);

	DEBUG_LOG_ERR("voip_status=%d wm1811_voip_device=%d\n", voipStatus, wm1811_voip_device);
	/* do not set idle mode for voip mode */
//	if((voipStatus!=0)||(wm1811_voip_device!=0)) return 0;
		
//	wm1811_write(codec, WM1811_SOFTWARE_RESET, 0x1811);
//	memcpy (wm1811_reg_buf, wm1811_reg, sizeof(u16) * WM1811_REGISTER_COUNT);
//	wm1811_set_clocking();

	if(ucontrol->value.integer.value[0])  //off
	{
#if 0
		wm1811_mic_mute_enable = 0;
		wm1811_playback_device = 0;
		wm1811_call_device = 0;
		wm1811_voip_device = 0;
		wm1811_recording_device = 0;
		wm1811_fm_device = 0;
		wm1811_rec_8k_enable = 0;
		wm1811_vr_mode = false;

		wm1811_set_pcm_sel(BT_SEL_LOW_MODE);
#endif
		switch (wm1811_old_mode)
		{
			case PLAY_BACK:
				WM1811_Set_Playback_Off(codec);
				wm1811_playback_device = 0;
				wm1811_voip_device = 0;
				
				/* restore digital volume for handsfree */
				if (wm1811_hf_device!=0)
				{
					WM1811_Set_Handsfee_Digital_Volume (codec,-1);
					wm1811_hf_device = 0;
				}
				break;
				
			case VOICE_MEMO:
				wm1811_record_off(codec);
				wm1811_recording_device = 0;
				break;
		}
	}

	return 0;
}
// hskwon-ss-db05, to support mic mute/unmute for CTS test
static int wm1811_set_mic_mute(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol,unsigned int codec_mode)
{
//	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	DEBUG_LOG_ERR("wm1811_set_mic_mute value = %ld \n",ucontrol->value.integer.value[0]);

	if(ucontrol->value.integer.value[0])	{
		if (wm1811_call_device == BT)
		{
               // do something 
           }
		else if(wm1811_call_device == HP4P)
			;//mic_enable(codec, HP_MIC, 0);
		else
			;//mic_enable(codec, MAIN_MIC, 0);
		wm1811_mic_mute_enable = 1;
	}
    else
    {
    	wm1811_mic_mute_enable = 0;

		if(wm1811_call_device == BT)\
		{
		    // do some thing 
          }
		else if(wm1811_call_device == HP4P)
			;//mic_enable(codec, HP_MIC, 1);
		else
			;//mic_enable(codec, MAIN_MIC, 1);
    }
	return 0;
}
#ifdef VOICE_RECOGNITION
static int wm1811_set_vr_mode(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	DEBUG_LOG_ERR("wm1811_set_vr_mode value = %ld \n", ucontrol->value.integer.value[0]);

	if (ucontrol->value.integer.value[0])		{	// on, mute
		DEBUG_LOG("wm1811_vr_mode = STATUS_ON");
		wm1811_vr_mode = STATUS_ON;

#if 0
		WM1811_Set_Playback_From_Headset_Status(codec, STATUS_ON);
#endif
	}
	else		{	// off, unmute
		DEBUG_LOG("wm1811_vr_mode = STATUS_OFF");
		wm1811_vr_mode = STATUS_OFF;
	}
	return 0;
}
static int wm1811_get_vr_mode(struct snd_kcontrol *kcontrol,struct snd_ctl_elem_value *ucontrol)
{
	return wm1811_vr_mode;
}
#endif


#ifdef WM1811_RECORDING_ENABLE
void wm1811_record_off(struct snd_soc_codec *codec)
{
#if 1
	// Left line input 1&2 volume
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, WM1811_IN1L_RESET);

	// Left line input 3&4 volume
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, WM1811_IN2L_RESET);

	// Right line input 1&2 volume
	wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, WM1811_INR1_RESET);

	// Input mixer 2
	wm1811_write(codec, WM1811_INPUT_MIXER_2, WM1811_INMIX2_RESET);

	// Input mixer 3
	wm1811_write(codec, WM1811_INPUT_MIXER_3, WM1811_IN2LMIX_RESET);

	// Input mixer 4
	wm1811_write(codec, WM1811_INPUT_MIXER_4, WM1811_INMIX4_RESET);

	// AIF1 ADC1 left mixer routing
	wm1811_write(codec, WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING, WM1811_AIF1ADC1L_MIX_RESET);

	// AIF1 ADC1 right mixer routing
	wm1811_write(codec, WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING, WM1811_AIF1ADC1R_MIX_RESET);

	// Power management (2)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, WM1811_PM2_RESET);

	// Power Management (4)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, WM1811_PM4_RESET);
#endif

	DEBUG_LOG("MIC OFF=%d", wm1811_recording_device);

	switch (wm1811_recording_device)	{
		case OFF:
			break;

		case MAIN_MIC:
			wm1811_mic_enable(codec, MAIN_MIC, STATUS_OFF);
			wm1811_record_main_mic(codec, STATUS_OFF);
			break;

		case HP_MIC:
			wm1811_mic_enable(codec, HP_MIC, STATUS_OFF);
			wm1811_record_ear_mic(codec, STATUS_OFF);
       		break;

		case SUB_MIC:
			wm1811_mic_enable(codec, SUB_MIC, STATUS_OFF);
			wm1811_record_sub_mic(codec, STATUS_OFF);
			break;

		case BT_MIC:
			wm1811_set_pcm_sel(BT_SEL_I2S_MODE);
			break;

		default:
			DEBUG_LOG("ERROR");
			break;
	}
}

void wm1811_main_mic_path_enable(struct snd_soc_codec *codec)
{
	u16 val = 0;
	
	DEBUG_LOG("");

	// AIF1 Control (1)
	wm1811_write(codec, WM1811_AIF1_CONTROL_1, 0x0010);
	
	// Power Management (2)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_2);
	val &= ~(WM1811_PM2_ALL_MASK);
	val |= (WM1811_TSHUT_ENA | WM1811_TSHUT_OPDIS |
			WM1811_MIXINL_ENA | WM1811_MIXINR_ENA |
			WM1811_IN1L_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, val);

	// Power Management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK | 
			WM1811_AIF1ADCL_ENA_MASK | WM1811_AIF1ADCR_ENA_MASK);
	val |= (WM1811_ADCL_ENA | WM1811_ADCR_ENA |
			WM1811_AIF1ADCL_ENA | WM1811_AIF1ADCR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);

	// Left line input 1&2 volume
	val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM1811_IN1L_VU_MASK | WM1811_IN1L_MUTE_MASK | WM1811_IN1L_VOL_MASK);
	if (wm1811_vr_mode)
		val |= gVR_MainMic | WM1811_IN1L_VU;
	else
		val |= gMainMic | WM1811_IN1L_VU;
	DEBUG_LOG("[Left line input 1&2 Volume] VR Mode=%d, Value=%x\n", wm1811_vr_mode, val);
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, val);

// used in voice recgnition mode
#if 0
	// AIF1 ADC1 Left Volume
	val = wm1811_read_cache(WM1811_AIF1_ADC1_LEFT_VOLUME);
	val &= ~(WM1811_AIF1ADC1L_VU_MASK | WM1811_AIF1ADC1L_VOL_MASK);
	if (wm1811_vr_mode)
		val |= gVR_MainMicADC | WM1811_AIF1ADC1L_VU;
	else
		val |= WM1811_AIF1ADC1L_VOL_DEFAULT | WM1811_AIF1ADC1L_VU;
	DEBUG_LOG("[AIF1 ADC1 Left Volume] VR Mode=%d, Value=%x\n", wm1811_vr_mode, val);
	wm1811_write(codec, WM1811_AIF1_ADC1_LEFT_VOLUME, val);
#endif

	// Input mixer 2
	val = wm1811_read_cache(WM1811_INPUT_MIXER_2);
	val &= ~(WM1811_IN1LN_TO_IN1L_MASK);
	val |= WM1811_IN1LN_TO_IN1L;
	wm1811_write(codec, WM1811_INPUT_MIXER_2, val);

	// Input mixer 3
	val = wm1811_read_cache(WM1811_INPUT_MIXER_3);
	val &= ~(WM1811_IN1L_TO_MIXINL_MASK | WM1811_IN1L_MIXINL_VOL_MASK);
	val |= WM1811_IN1L_TO_MIXINL | WM1811_IN1L_MIXINL_VOL;
	wm1811_write(codec, WM1811_INPUT_MIXER_3, val);

	// AIF1 ADC1 left mixer routing
	val = wm1811_read_cache(WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_ADC1L_TO_AIF1ADC1L_MASK);
	val |= WM1811_ADC1L_TO_AIF1ADC1L;
	wm1811_write(codec, WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

	// AIF1 ADC1 right mixer routing
	val = wm1811_read_cache(WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_ADC1R_TO_AIF1ADC1R_MASK);
	val |= WM1811_ADC1R_TO_AIF1ADC1R;
	wm1811_write(codec, WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

	// AIF1 ADC1 Filters
	val = wm1811_read_cache(WM1811_AIF1_ADC1_FILTERS);
	val &= ~(WM1811_AIF1ADC1L_HPF_MASK | WM1811_AIF1ADC1R_HPF_MASK);
	val |= (WM1811_AIF1ADC1L_HPF | WM1811_AIF1ADC1R_HPF);
	wm1811_write(codec, WM1811_AIF1_ADC1_FILTERS, val);

	msleep(50);
}

void wm1811_main_mic_path_disable(struct snd_soc_codec *codec)
{
	u16 val=0;
	
	DEBUG_LOG("");

	// AIF1 Control (1)
	wm1811_write(codec, WM1811_AIF1_CONTROL_1, 0x4010);

	// Power management (2)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, WM1811_PM2_RESET);

	// Power management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK | 
			WM1811_AIF1ADCL_ENA_MASK | WM1811_AIF1ADCR_ENA_MASK);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);

	// Left line input 1&2 volume
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, WM1811_IN1L_RESET);

	// Input mixer 2
	wm1811_write(codec, WM1811_INPUT_MIXER_2, WM1811_INMIX2_RESET);

	// Input mixer 3
	wm1811_write(codec, WM1811_INPUT_MIXER_3, WM1811_IN2LMIX_RESET);

	// AIF1 ADC1 left mixer routing
	wm1811_write(codec, WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING, WM1811_AIF1ADC1L_MIX_RESET);

	// AIF1 ADC1 right mixer routing
	wm1811_write(codec, WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING, WM1811_AIF1ADC1R_MIX_RESET);

	// AIF1 ADC1 Filters
	wm1811_write(codec, WM1811_AIF1_ADC1_FILTERS, WM1811_AIF1ADC1_HPF_DEFAULT);
}

void wm1811_sub_mic_path_enable(struct snd_soc_codec *codec)
{
	u16 val = 0;
	DEBUG_LOG("");
	// AIF1 Control (1)
	wm1811_write(codec, WM1811_AIF1_CONTROL_1, 0x4010);
	
	// Power Management (2)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_2);
	val &= ~(WM1811_PM2_ALL_MASK);
	val |= (WM1811_TSHUT_ENA | WM1811_TSHUT_OPDIS |
			WM1811_MIXINL_ENA | WM1811_MIXINR_ENA |
			WM1811_IN1R_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, val);

	// Power Management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK | 
			WM1811_AIF1ADCL_ENA_MASK | WM1811_AIF1ADCR_ENA_MASK);
	val |= (WM1811_ADCL_ENA | WM1811_ADCR_ENA |
			WM1811_AIF1ADCL_ENA | WM1811_AIF1ADCR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);

	// Right line input 1&2 volume
	val = wm1811_read_cache(WM1811_RIGHT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM1811_IN1R_VU_MASK | WM1811_IN1R_MUTE_MASK |
			WM1811_IN1R_VOL_MASK);
	//val |= WM1811_IN1R_VU | WM1811_IN1R_VOL_30dB;
	val |= WM1811_IN1R_VU | 0x0012;
	wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, val);

	// Input mixer 2
	val = wm1811_read_cache(WM1811_INPUT_MIXER_2);
	val &= ~(WM1811_IN1RN_TO_IN1R_MASK);
	val |= WM1811_IN1RN_TO_IN1R;
	wm1811_write(codec, WM1811_INPUT_MIXER_2, 0x0001);

	// Input mixer 4
	val = wm1811_read_cache(WM1811_INPUT_MIXER_4);
	val &= ~(WM1811_IN1R_TO_MIXINR_MASK | WM1811_IN1R_MIXINR_VOL_MASK);
	val |= WM1811_IN1R_TO_MIXINR | WM1811_IN1R_MIXINR_VOL;
	//val |= WM1811_IN1R_TO_MIXINR;
	wm1811_write(codec, WM1811_INPUT_MIXER_4, val);

	// AIF1ADC Left Mixer Routing
	val = wm1811_read_cache(WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_ADC1L_TO_AIF1ADC1L_MASK);
	val |= WM1811_ADC1L_TO_AIF1ADC1L;
	wm1811_write(codec, WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

	// AIF1ADC Right Mixer Routing
	val = wm1811_read_cache(WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_ADC1R_TO_AIF1ADC1R_MASK);
	val |= WM1811_ADC1R_TO_AIF1ADC1R;
	wm1811_write(codec, WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);
}

void wm1811_sub_mic_path_disable(struct snd_soc_codec *codec)
{
	u16 val=0;
	
	DEBUG_LOG("");

	// Power management (2)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, WM1811_PM2_RESET);

	// Power management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK | 
			WM1811_AIF1ADCL_ENA_MASK | WM1811_AIF1ADCR_ENA_MASK);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);

	// Right line input 1&2 volume
	wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, WM1811_INR1_RESET);

	// Input mixer 2
	wm1811_write(codec, WM1811_INPUT_MIXER_2, WM1811_INMIX2_RESET);

	// Input mixer 4
	wm1811_write(codec, WM1811_INPUT_MIXER_4, WM1811_INMIX4_RESET);

	// AIF1 ADC1 left mixer routing
	wm1811_write(codec, WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING, WM1811_AIF1ADC1L_MIX_RESET);

	// AIF1 ADC1 right mixer routing
	wm1811_write(codec, WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING, WM1811_AIF1ADC1R_MIX_RESET);
}

void wm1811_ear_mic_path_enable(struct snd_soc_codec *codec)
{
	u16 val = 0;
	
	DEBUG_LOG("");

	// Power Management (2)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_2);
	val &= ~(WM1811_PM2_ALL_MASK);
	val |= (WM1811_TSHUT_ENA | WM1811_TSHUT_OPDIS |
			WM1811_MIXINL_ENA | WM1811_MIXINR_ENA |
			WM1811_IN2L_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, val);

	// Power Management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK | 
			WM1811_AIF1ADCL_ENA_MASK | WM1811_AIF1ADCR_ENA_MASK);
	val |= (WM1811_ADCL_ENA | WM1811_ADCR_ENA |
			WM1811_AIF1ADCL_ENA | WM1811_AIF1ADCR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);

	// Left line input 3&4 volume
	val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_3_4_VOLUME);
	val &= ~(WM1811_IN2L_VU_MASK | WM1811_IN2L_MUTE_MASK);
	val |= WM1811_IN2L_VU;
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, val);

	// Left line input 3&4 volume : MIC gain control
	val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_3_4_VOLUME);
	val &= ~(WM1811_IN2L_VOL_MASK);
	if (wm1811_vr_mode)	{
		val |= gVR_EarMic;
		DEBUG_LOG("gVR_EarMic=%x\n", gVR_EarMic);
	}
	else	{
		val |= gEarMic;
		DEBUG_LOG("gEarMic=%x\n", gEarMic);
	}
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, val);

	// Input mixer 2
	val = wm1811_read_cache(WM1811_INPUT_MIXER_2);
	val &= ~(WM1811_IN2LP_TO_IN2L_MASK | WM1811_IN2LN_TO_IN2L_MASK);
	val |= WM1811_IN2LP_TO_IN2L | WM1811_IN2LN_TO_IN2L;
	wm1811_write(codec, WM1811_INPUT_MIXER_2, val);

	// Input mixer 3
	val = wm1811_read_cache(WM1811_INPUT_MIXER_3);
	val &= ~(WM1811_IN2L_TO_MIXINL_MASK | WM1811_IN2L_MIXINL_VOL_MASK);
	val |= WM1811_IN2L_TO_MIXINL | WM1811_IN2L_MIXINL_VOL;
	wm1811_write(codec, WM1811_INPUT_MIXER_3, val);

	// AIF1 ADC1 Left Mixer Routing
	val = wm1811_read_cache(WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_ADC1L_TO_AIF1ADC1L_MASK);
	val |= WM1811_ADC1L_TO_AIF1ADC1L;
	wm1811_write(codec, WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

	// AIF1 ADC1 Right Mixer Routing
	val = wm1811_read_cache(WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_ADC1R_TO_AIF1ADC1R_MASK);
	val |= WM1811_ADC1R_TO_AIF1ADC1R;
	wm1811_write(codec, WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);
}

void wm1811_ear_mic_path_disable(struct snd_soc_codec *codec)
{
	u16 val=0;
	
	DEBUG_LOG("");

	// Power management (2)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, WM1811_PM2_RESET);

	// Power management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK | 
			WM1811_AIF1ADCL_ENA_MASK | WM1811_AIF1ADCR_ENA_MASK);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);

	// Left line input 3&4 volume
	val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_3_4_VOLUME);
	val &= ~(WM1811_IN2L_VU_MASK | WM1811_IN2L_MUTE_MASK);
	val |= WM1811_IN2L_VU;
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, val);

	// Left line input 3&4 volume : MIC gain control
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, WM1811_IN2L_RESET);

	// Input mixer 2
	wm1811_write(codec, WM1811_INPUT_MIXER_2, WM1811_INMIX2_RESET);

	// Input mixer 3
	wm1811_write(codec, WM1811_INPUT_MIXER_3, WM1811_IN2LMIX_RESET);

	// AIF1 ADC1 left mixer routing
	wm1811_write(codec, WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING, WM1811_AIF1ADC1L_MIX_RESET);

	// AIF1 ADC1 right mixer routing
	wm1811_write(codec, WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING, WM1811_AIF1ADC1R_MIX_RESET);
}

void wm1811_record_main_mic(struct snd_soc_codec *codec, u16 on_off)
{
	if (STATUS_ON == on_off)
		wm1811_main_mic_path_enable(codec);
	else
		wm1811_main_mic_path_disable(codec);
}

void wm1811_record_sub_mic(struct snd_soc_codec *codec, u16 on_off)
{
	if (STATUS_ON == on_off)
		wm1811_sub_mic_path_enable(codec);
	else
		wm1811_sub_mic_path_disable(codec);
}

void wm1811_record_ear_mic(struct snd_soc_codec *codec, u16 on_off)
{
	if (STATUS_ON == on_off)
		wm1811_ear_mic_path_enable(codec);
	else
		wm1811_ear_mic_path_disable(codec);
}
#endif		// WM1811_RECORDING_ENABLE


#ifdef WM1811_BLUETOOTH_ENABLE
void wm1811_set_playback_hf_clocking(struct snd_soc_codec *codec)
{
	u16 val=0;

	// Prevent what set to clocking of the handsfree two times
	if (STATUS_OFF != wm1811_read_cache(WM1811_AIF2_CLOCKING_1))	{
		DEBUG_LOG("Clocking setting of the handsfree did set previous!");
		return;
	}

	DEBUG_LOG("");

	wm1811_write(codec, WM1811_GPIO_1, 0xA101);
	wm1811_write(codec, WM1811_GPIO_3, 0x0100);
	wm1811_write(codec, WM1811_GPIO_5, 0x8100);
	wm1811_write(codec, WM1811_GPIO_8, 0x8100);
	wm1811_write(codec, WM1811_GPIO_9, 0x0100);
	wm1811_write(codec, WM1811_GPIO_10, 0x0100);
	wm1811_write(codec, WM1811_GPIO_11, 0x0100);

	// Analogue Configuration
	wm1811_write(codec, WM1811_ANTIPOP_2, 0x01E4);

	msleep(50);

	// FLL and Clocking
	wm1811_write(codec, WM1811_FLL2_CONTROL_2, 0x0700);
	wm1811_write(codec, WM1811_FLL2_CONTROL_3, 0x0391);
	wm1811_write(codec, WM1811_FLL2_CONTROL_4, 0x00e0);
	wm1811_write(codec, WM1811_FLL2_CONTROL_5, 0x0c88);
	wm1811_write(codec, WM1811_FLL2_EFS_1, 0x0659);
	wm1811_write(codec, WM1811_FLL2_EFS_2, 0x0007);
	wm1811_write(codec, WM1811_FLL2_CONTROL_1, 0x0005);
	
	wm1811_write(codec, WM1811_CLOCKING_1, 0x000f);
	wm1811_write(codec, WM1811_AIF2_CLOCKING_1, 0x0019);
	wm1811_write(codec, WM1811_AIF2_RATE, 0x0009);
	
	wm1811_write(codec, WM1811_AIF2_CONTROL_1, 0x4118);
	wm1811_write(codec, WM1811_AIF2_MASTER_SLAVE, 0x4000);
	wm1811_write(codec, WM1811_AIF2_BCLK, 0x00c0);

	// Power Management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_AIF2ADCL_ENA_MASK | WM1811_AIF2ADCR_ENA_MASK);
	val |= (WM1811_AIF2ADCL_ENA | WM1811_AIF2ADCR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);

	// Power management (5)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_5);
	val &= ~(WM1811_AIF2DACL_ENA_MASK | WM1811_AIF2DACR_ENA_MASK);
	val |= (WM1811_AIF2DACL_ENA | WM1811_AIF2DACR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_5, val);

	// Power Management (6)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_6);
	val &= ~(WM1811_AIF3_ADCDAT_SRC_MASK | WM1811_AIF2_DACDAT_SRC_MASK);
	val |= 0x0008 | WM1811_AIF2_DACDAT_SRC;
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_6, val);
}


void wm1811_set_playback_hf_rcv(struct snd_soc_codec *codec)
{
	u16 val=0;
	
	DEBUG_LOG("");

	/* ---------------------Main MIC to ADC---------------------*/
	// Input mixer 2
	val = wm1811_read_cache(WM1811_INPUT_MIXER_2);
	val &= ~(WM1811_IN1LN_TO_IN1L_MASK);
	val |= WM1811_IN1LN_TO_IN1L;
	wm1811_write(codec, WM1811_INPUT_MIXER_2, val);

	// Left line input 1&2 volume
	val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM1811_IN1L_VU_MASK | WM1811_IN1L_MUTE_MASK |
		WM1811_IN1L_VOL_MASK);
	val |= WM1811_IN1L_VU | WM1811_IN1L_VOL_30dB;
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, val);

	// Input mixer 3
	val = wm1811_read_cache(WM1811_INPUT_MIXER_3);
	val &= ~(WM1811_IN1L_TO_MIXINL_MASK | WM1811_IN1L_MIXINL_VOL_MASK);
	val |= WM1811_IN1L_TO_MIXINL | WM1811_IN1L_MIXINL_VOL;
	wm1811_write(codec, WM1811_INPUT_MIXER_3, val);

	// Power Management (2)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_2);
	val &= ~(WM1811_PM2_ALL_MASK);
	val |= (WM1811_TSHUT_ENA | WM1811_TSHUT_OPDIS |
			WM1811_MIXINL_ENA | WM1811_MIXINR_ENA |
			WM1811_IN1L_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, val);

	// Power Management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_AIF2ADCL_ENA_MASK | WM1811_AIF2ADCR_ENA_MASK |
			WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK);
	val |= (WM1811_AIF2ADCL_ENA | WM1811_AIF2ADCR_ENA |
			WM1811_ADCL_ENA | WM1811_ADCR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);


	/* ---------------------ADC to AIF2---------------------*/
	// AIF2ADC Mixer Volumes
	val = wm1811_read_cache(WM1811_AIF2ADC_MIXER_VOLUMES);
	val &= ~(WM1811_ADCL_AIF2ADC_VOL_MASK);
	val |= 0xC;
	wm1811_write(codec, WM1811_AIF2ADC_MIXER_VOLUMES, val);

	// AIF2ADC Left Mixer Routing
	val = wm1811_read_cache(WM1811_AIF2ADC_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_ADCL_TO_AIF2ADCL_MASK);
	val |= WM1811_ADCL_TO_AIF2ADCL;
	wm1811_write(codec, WM1811_AIF2ADC_LEFT_MIXER_ROUTING, val);

	// AIF2ADC Right Mixer Routing
	val = wm1811_read_cache(WM1811_AIF2ADC_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_ADCL_TO_AIF2ADCR_MASK);
	val |= WM1811_ADCL_TO_AIF2ADCR;
	wm1811_write(codec, WM1811_AIF2ADC_RIGHT_MIXER_ROUTING, val);

	// AIF2TX Left Volume
	val = wm1811_read_cache(WM1811_AIF2TX_LEFT_VOLUME);
	val &= ~(WM1811_AIF2TXL_MUTE_MASK | WM1811_AIF2TX_VU_MASK |
		WM1811_AIF2TXL_VOL_MASK);
	val |= WM1811_AIF2TX_VU | 0xC0;
	wm1811_write(codec, WM1811_AIF2TX_LEFT_VOLUME, val);


	/* ---------------------AIF2 to DAC---------------------*/
	// DAC1 Left Mixer Routing
	val = wm1811_read_cache(WM1811_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_AIF2DACL_TO_DACL_MASK);
	val |= WM1811_AIF2DACL_TO_DACL;
	wm1811_write(codec, WM1811_DAC1_LEFT_MIXER_ROUTING, val);

	// DAC1 Right Mixer Routing
	val = wm1811_read_cache(WM1811_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_AIF2DACR_TO_DACR_MASK);
	val |= WM1811_AIF2DACR_TO_DACR;
	wm1811_write(codec, WM1811_DAC1_RIGHT_MIXER_ROUTING, val);

	// HPOUT2 Volume
	val = wm1811_read_cache(WM1811_HPOUT2_VOLUME);
	val &= ~(WM1811_HPOUT2_MUTE_MASK | WM1811_HPOUT2_VOL_MASK);
	wm1811_write(codec, WM1811_HPOUT2_VOLUME, val);

	// Output Mixer (1)
	val = wm1811_read_cache(WM1811_OUTPUT_MIXER_1);
	val &= ~(WM1811_DAC1L_TO_MIXOUTL_MASK);
	val |= WM1811_DAC1L_TO_MIXOUTL;
	wm1811_write(codec, WM1811_OUTPUT_MIXER_1, val);

	// Output Mixer (2)
	val = wm1811_read_cache(WM1811_OUTPUT_MIXER_2);
	val &= ~(WM1811_DAC1R_TO_MIXOUTR_MASK);
	val |= WM1811_DAC1R_TO_MIXOUTR;
	wm1811_write(codec, WM1811_OUTPUT_MIXER_2, val);

	// HPOUT2 Mixer
	val = wm1811_read_cache(WM1811_HPOUT2_MIXER);
	val &= ~(WM1811_MIXOUTLVOL_TO_HPOUT2_MASK | WM1811_MIXOUTRVOL_TO_HPOUT2_MASK);
	val |= WM1811_MIXOUTLVOL_TO_HPOUT2 | WM1811_MIXOUTRVOL_TO_HPOUT2;
	wm1811_write(codec, WM1811_HPOUT2_MIXER, val);

	wm1811_write(codec, WM1811_DAC1_LEFT_VOLUME, 0x01E0);
	wm1811_write(codec, WM1811_DAC1_RIGHT_VOLUME, 0x01E0);
	wm1811_write(codec, WM1811_AIF2_DAC_FILTERS_1, 0x0000);

	// AIF2 DAC Filters (1)
	val = wm1811_read_cache(WM1811_AIF2_DAC_FILTERS_1);
	val &= ~(WM1811_AIF2DAC_MUTE_MASK);
	wm1811_write(codec, WM1811_AIF2_DAC_FILTERS_1, val);

	// Power management (1)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_1);
	val &= ~WM1811_HPOUT2_ENA_MASK;
	val |= WM1811_HPOUT2_ENA;
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);

	// Antipop (1)
	val = wm1811_read_cache(WM1811_ANTIPOP_1);
	val &= ~(WM1811_HPOUT2_IN_ENA_MASK);
	val |= (WM1811_HPOUT2_IN_ENA);
	wm1811_write(codec, WM1811_ANTIPOP_1, val);

	// Power Management (3)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_3);
	val &= ~(WM1811_MIXOUTLVOL_ENA_MASK | WM1811_MIXOUTRVOL_ENA_MASK);
	val |= (WM1811_MIXOUTLVOL_ENA | WM1811_MIXOUTRVOL_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_3, val);
}


void wm1811_set_playback_hf_spk(struct snd_soc_codec *codec)
{
	u16 val=0;
	
	DEBUG_LOG("");
	
	/* ---------------------Main MIC to ADC---------------------*/
	// Right line input 1&2 volume
	val = wm1811_read_cache(WM1811_RIGHT_LINE_INPUT_1_2_VOLUME);
	val &= ~(WM1811_IN1R_VU_MASK | WM1811_IN1R_MUTE_MASK |
		WM1811_IN1R_VOL_MASK);
	val |= WM1811_IN1R_VU | WM1811_IN1R_VOL_30dB;
	wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, val);

	// Input mixer 2
	val = wm1811_read_cache(WM1811_INPUT_MIXER_2);
	val &= ~(WM1811_IN1RN_TO_IN1R_MASK);
	val |= WM1811_IN1RN_TO_IN1R;
	wm1811_write(codec, WM1811_INPUT_MIXER_2, val);

	// Input mixer 4
	val = wm1811_read_cache(WM1811_INPUT_MIXER_4);
	val &= ~(WM1811_IN1R_TO_MIXINR_MASK | WM1811_IN1R_MIXINR_VOL_MASK);
	val |= WM1811_IN1R_TO_MIXINR | WM1811_IN1R_MIXINR_VOL;
	wm1811_write(codec, WM1811_INPUT_MIXER_4, val);

	// Power Management (2)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_2);
	val &= ~(WM1811_PM2_ALL_MASK);
	val |= (WM1811_TSHUT_ENA | WM1811_TSHUT_OPDIS |
			WM1811_MIXINL_ENA | WM1811_MIXINR_ENA |
			WM1811_IN1R_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, val);

	// Power Management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_AIF2ADCL_ENA_MASK | WM1811_AIF2ADCR_ENA_MASK |
			WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK);
	val |= (WM1811_AIF2ADCL_ENA | WM1811_AIF2ADCR_ENA |
			WM1811_ADCL_ENA | WM1811_ADCR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);


	/* ---------------------ADC to AIF2---------------------*/
	// AIF2ADC Left Mixer Routing
	val = wm1811_read_cache(WM1811_AIF2ADC_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_ADCR_TO_AIF2ADCL_MASK);
	val |= WM1811_ADCR_TO_AIF2ADCL;
	wm1811_write(codec, WM1811_AIF2ADC_LEFT_MIXER_ROUTING, val);

	// AIF2ADC Right Mixer Routing
	val = wm1811_read_cache(WM1811_AIF2ADC_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_ADCR_TO_AIF2ADCR_MASK);
	val |= WM1811_ADCR_TO_AIF2ADCR;
	wm1811_write(codec, WM1811_AIF2ADC_RIGHT_MIXER_ROUTING, val);

	// AIF2ADC Mixer Volumes
	val = wm1811_read_cache(WM1811_AIF2ADC_MIXER_VOLUMES);
	val &= ~(WM1811_ADCR_AIF2ADC_VOL_MASK);
	val |= 0x018C;
	wm1811_write(codec, WM1811_AIF2ADC_MIXER_VOLUMES, val);

	// AIF2TX Left Volume
	val = wm1811_read_cache(WM1811_AIF2TX_LEFT_VOLUME);
	val &= ~(WM1811_AIF2TXL_MUTE_MASK | WM1811_AIF2TX_VU_MASK |
		WM1811_AIF2TXL_VOL_MASK);
	val |= WM1811_AIF2TX_VU | 0xC0;
	wm1811_write(codec, WM1811_AIF2TX_LEFT_VOLUME, val);

	// AIF2TX Right Volume
	val = wm1811_read_cache(WM1811_AIF2TX_RIGHT_VOLUME);
	val &= ~(WM1811_AIF2TXR_MUTE_MASK | WM1811_AIF2TX_VU_MASK |
		WM1811_AIF2TXR_VOL_MASK);
	val |= WM1811_AIF2TX_VU | 0xC0;
	wm1811_write(codec, WM1811_AIF2TX_RIGHT_VOLUME, val);


	/* ---------------------AIF2 to DAC---------------------*/
	// DAC1 Left Mixer Routing
	val = wm1811_read_cache(WM1811_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_AIF2DACL_TO_DACL_MASK);
	val |= WM1811_AIF2DACL_TO_DACL;
	wm1811_write(codec, WM1811_DAC1_LEFT_MIXER_ROUTING, val);

	// DAC1 Right Mixer Routing
	val = wm1811_read_cache(WM1811_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_AIF2DACR_TO_DACR_MASK);
	val |= WM1811_AIF2DACR_TO_DACR;
	wm1811_write(codec, WM1811_DAC1_RIGHT_MIXER_ROUTING, val);

	// HPOUT2 Volume
	val = wm1811_read_cache(WM1811_HPOUT2_VOLUME);
	val &= ~(WM1811_HPOUT2_MUTE_MASK | WM1811_HPOUT2_VOL_MASK);
	wm1811_write(codec, WM1811_HPOUT2_VOLUME, val);

	wm1811_write(codec, WM1811_DAC1_LEFT_VOLUME, 0x01C0);
	wm1811_write(codec, WM1811_DAC1_RIGHT_VOLUME, 0x01C0);

	// AIF2 DAC Filters (1)
	val = wm1811_read_cache(WM1811_AIF2_DAC_FILTERS_1);
	val &= ~(WM1811_AIF2DAC_MUTE_MASK);
	wm1811_write(codec, WM1811_AIF2_DAC_FILTERS_1, val);
}

void wm1811_set_playback_hf_hp(struct snd_soc_codec *codec)
{
	u16 val=0;
	
	DEBUG_LOG("");

	// DAC Left Volume
	val = wm1811_read_cache(WM1811_DAC1_LEFT_VOLUME);
	val &= ~(WM1811_DAC1L_MUTE_MASK);
	val |= (WM1811_DAC1L_MUTE);
	wm1811_write(codec, WM1811_DAC1_LEFT_VOLUME, val);

	// DAC Right Volume
	val = wm1811_read_cache(WM1811_DAC1_RIGHT_VOLUME);
	val &= ~(WM1811_DAC1R_MUTE_MASK);
	val |= (WM1811_DAC1R_MUTE);
	wm1811_write(codec, WM1811_DAC1_RIGHT_VOLUME, val);
	

	/* ---------------------Main MIC to ADC---------------------*/
	// Input mixer 2
	val = wm1811_read_cache(WM1811_INPUT_MIXER_2);
	val &= ~(WM1811_IN2LP_TO_IN2L_MASK | WM1811_IN2LN_TO_IN2L_MASK);
	val |= WM1811_IN2LP_TO_IN2L | WM1811_IN2LN_TO_IN2L;
	wm1811_write(codec, WM1811_INPUT_MIXER_2, val);

	// Left line input 3&4 volume
	val = wm1811_read_cache(WM1811_LEFT_LINE_INPUT_3_4_VOLUME);
	val &= ~(WM1811_IN2L_VU_MASK | WM1811_IN2L_MUTE_MASK |
		WM1811_IN2L_VOL_MASK);
	val |= WM1811_IN2L_VU | WM1811_IN2L_VOL_30dB;
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, val);

	// Input mixer 3
	val = wm1811_read_cache(WM1811_INPUT_MIXER_3);
	val &= ~(WM1811_IN2L_TO_MIXINL_MASK);
	val |= (WM1811_IN2L_TO_MIXINL);
	wm1811_write(codec, WM1811_INPUT_MIXER_3, val);

	// Power Management (2)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_2);
	val &= ~(WM1811_PM2_ALL_MASK);
	val |= (WM1811_TSHUT_ENA | WM1811_TSHUT_OPDIS |
			WM1811_MIXINL_ENA | WM1811_MIXINR_ENA |
			WM1811_IN2L_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, val);

	// Power Management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_AIF2ADCL_ENA_MASK | WM1811_AIF2ADCR_ENA_MASK |
			WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK);
	val |= (WM1811_AIF2ADCL_ENA | WM1811_AIF2ADCR_ENA |
			WM1811_ADCL_ENA | WM1811_ADCR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);
	

	/* ---------------------ADC to AIF2---------------------*/
	// AIF2ADC Left Mixer Routing
	val = wm1811_read_cache(WM1811_AIF2ADC_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_ADCL_TO_AIF2ADCL_MASK);
	val |= WM1811_ADCL_TO_AIF2ADCL;
	wm1811_write(codec, WM1811_AIF2ADC_LEFT_MIXER_ROUTING, val);

	// AIF2ADC Right Mixer Routing
	val = wm1811_read_cache(WM1811_AIF2ADC_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_ADCR_TO_AIF2ADCR_MASK);
	val |= WM1811_ADCR_TO_AIF2ADCR;
	wm1811_write(codec, WM1811_AIF2ADC_RIGHT_MIXER_ROUTING, val);

	// AIF2ADC Mixer Volumes
	val = wm1811_read_cache(WM1811_AIF2ADC_MIXER_VOLUMES);
	val &= ~(WM1811_ADCR_AIF2ADC_VOL_MASK);
	val |= 0x018C;
	wm1811_write(codec, WM1811_AIF2ADC_MIXER_VOLUMES, val);

	// AIF2TX Left Volume
	val = wm1811_read_cache(WM1811_AIF2TX_LEFT_VOLUME);
	val &= ~(WM1811_AIF2TXL_MUTE_MASK | WM1811_AIF2TX_VU_MASK |
		WM1811_AIF2TXL_VOL_MASK);
	val |= WM1811_AIF2TX_VU | 0xC0;
	wm1811_write(codec, WM1811_AIF2TX_LEFT_VOLUME, val);

	// AIF2TX Right Volume
	val = wm1811_read_cache(WM1811_AIF2TX_RIGHT_VOLUME);
	val &= ~(WM1811_AIF2TXR_MUTE_MASK | WM1811_AIF2TX_VU_MASK |
		WM1811_AIF2TXR_VOL_MASK);
	val |= WM1811_AIF2TX_VU | 0xC0;
	wm1811_write(codec, WM1811_AIF2TX_RIGHT_VOLUME, val);


	/* ---------------------AIF2 to DAC---------------------*/
	// DAC1 Left Mixer Routing
	val = wm1811_read_cache(WM1811_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_AIF2DACL_TO_DACL_MASK);
	val |= WM1811_AIF2DACL_TO_DACL;
	wm1811_write(codec, WM1811_DAC1_LEFT_MIXER_ROUTING, val);

	// DAC1 Right Mixer Routing
	val = wm1811_read_cache(WM1811_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_AIF2DACR_TO_DACR_MASK);
	val |= WM1811_AIF2DACR_TO_DACR;
	wm1811_write(codec, WM1811_DAC1_RIGHT_MIXER_ROUTING, val);

#if 0
	// Class W (1)
	val = wm1811_read_cache(WM1811_CLASS_W_1);
	val &= ~(WM1811_CLASS_W_RESERVED_MASK);
	val |= WM1811_CLASS_W_RESERVED;
	wm1811_write(codec, WM1811_CLASS_W_1, val);

	// Analogue HP (1)
	val = wm1811_read_cache(WM1811_ANALOGUE_HP_1);
	val &= ~(WM1811_HPOUT1L_DLY_MASK | WM1811_HPOUT1R_DLY_MASK);
	val |= WM1811_HPOUT1L_DLY | WM1811_HPOUT1R_DLY;
	wm1811_write(codec, WM1811_ANALOGUE_HP_1, val);

	// Charge Pump (1)
	val = wm1811_read_cache(WM1811_CHARGE_PUMP_1);
	val &= ~(WM1811_CP_ENA_MASK | WM1811_CP_ENA_DEFAULT_MASK);
	val |= WM1811_CP_ENA | WM1811_CP_ENA_DEFAULT;
	wm1811_write(codec, WM1811_CHARGE_PUMP_1, val);

	msleep(15);

	// Output Mixer (1)
	val = wm1811_read_cache(WM1811_OUTPUT_MIXER_1);
	val &= ~(WM1811_DAC1L_TO_HPOUT1L_MASK);
	val |= WM1811_DAC1L_TO_HPOUT1L;
	wm1811_write(codec, WM1811_OUTPUT_MIXER_1, val);

	// Output Mixer (2)
	val = wm1811_read_cache(WM1811_OUTPUT_MIXER_2);
	val &= ~(WM1811_DAC1R_TO_HPOUT1R_MASK);
	val |= WM1811_DAC1R_TO_HPOUT1R;
	wm1811_write(codec, WM1811_OUTPUT_MIXER_2, val);

	// Analogue HP (1)
	val = wm1811_read_cache(WM1811_ANALOGUE_HP_1);
	val &= ~(WM1811_HPOUT1L_RMV_SHORT_MASK | WM1811_HPOUT1L_OUTP_MASK |
		WM1811_HPOUT1L_DLY_MASK | WM1811_HPOUT1R_RMV_SHORT_MASK |
		WM1811_HPOUT1R_OUTP_MASK | WM1811_HPOUT1R_DLY_MASK);
	val |= WM1811_HPOUT1L_RMV_SHORT | WM1811_HPOUT1L_OUTP |
		WM1811_HPOUT1L_DLY | WM1811_HPOUT1R_RMV_SHORT |
		WM1811_HPOUT1R_OUTP | WM1811_HPOUT1R_DLY;
	wm1811_write(codec, WM1811_ANALOGUE_HP_1, val);
	
	// DC Servo (1)
	val = wm1811_read_cache(WM1811_DC_SERVO_1);
	val &= ~(WM1811_DCS_TRIG_STARTUP_1_MASK | WM1811_DCS_TRIG_STARTUP_0_MASK |
		WM1811_DCS_ENA_CHAN_1_MASK | WM1811_DCS_ENA_CHAN_0_MASK);
	val |= WM1811_DCS_TRIG_STARTUP_1 | WM1811_DCS_TRIG_STARTUP_0 |
		WM1811_DCS_ENA_CHAN_1 | WM1811_DCS_ENA_CHAN_0;
	wm1811_write(codec, WM1811_DC_SERVO_1, val);

	msleep(250);
#endif

	// AIF2 DAC Filters (1)
	val = wm1811_read_cache(WM1811_AIF2_DAC_FILTERS_1);
	val &= ~(WM1811_AIF2DAC_MUTE_MASK | WM1811_AIF2DAC_MONO_MASK);
	val |= WM1811_AIF2DAC_MONO;
	wm1811_write(codec, WM1811_AIF2_DAC_FILTERS_1, val);

	// DAC Left Volume
	val = wm1811_read_cache(WM1811_DAC1_LEFT_VOLUME);
	val &= ~(WM1811_DAC1L_MUTE_MASK | WM1811_DAC1_VU_MASK |
		WM1811_DAC1L_VOL_MASK);
	val |= WM1811_DAC1_VU | WM1811_DAC1L_VOL_0dB;
	wm1811_write(codec, WM1811_DAC1_LEFT_VOLUME, val);

	// DAC Right Volume
	val = wm1811_read_cache(WM1811_DAC1_RIGHT_VOLUME);
	val &= ~(WM1811_DAC1R_MUTE_MASK | WM1811_DAC1_VU_MASK |
		WM1811_DAC1R_VOL_MASK);
	val |= WM1811_DAC1_VU | WM1811_DAC1R_VOL_0dB;
	wm1811_write(codec, WM1811_DAC1_RIGHT_VOLUME, val);

#if 0
	wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, 0x0175);
	wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, 0x0175);
#endif

	WM1811_Set_Playback_EAR(codec);

	//gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_OFF);
}
#endif

#ifdef WM1811_PLAYBACK_ENABLE
void WM1811_Set_Playback_Init(struct snd_soc_codec *codec)
{
	u16 val = 0;
	u16 val1 = 0;
	u16 val2 = 0;

#define DELAY_FOR_NOISE 50
	
#ifdef MID_LOCALE_VOLUME
	/* added by jinho.lim to make different volume as locale */
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);
#endif

	DEBUG_LOG("");

	// GPIO (1)
	val = wm1811_read_cache(WM1811_GPIO_1);
	val &= ~(WM1811_GP1_DIR_MASK | WM1811_GP1_DB_MASK | WM1811_GP1_FN_MASK);
	val |= (WM1811_GP1_DIR | WM1811_GP1_DB | WM1811_GP1_FN_LOGIC_INOUT);
	wm1811_write(codec, WM1811_GPIO_1, val);
	
	// DAC1 Left Mixer Routing
	val = wm1811_read_cache(WM1811_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_AIF1DACL_TO_DACL_MASK);
	val |= WM1811_AIF1DACL_TO_DACL;
	wm1811_write(codec, WM1811_DAC1_LEFT_MIXER_ROUTING, val);

	// DAC1 right mixer routing
	val = wm1811_read_cache(WM1811_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_AIF1DACR_TO_DACR_MASK);
	val |= WM1811_AIF1DACR_TO_DACR;
	wm1811_write(codec, WM1811_DAC1_RIGHT_MIXER_ROUTING, val);

	// DAC Left Volume 
	val = wm1811_read_cache(WM1811_DAC1_LEFT_VOLUME);
	val &= ~(WM1811_DAC1L_MUTE_MASK | WM1811_DAC1_VU_MASK | WM1811_DAC1L_VOL_MASK);
	val |= WM1811_DAC1L_MUTE | WM1811_DAC1_VU | WM1811_DAC1L_VOL_0dB;
	wm1811_write(codec, WM1811_DAC1_LEFT_VOLUME, val);

	// DAC Right Volume
	val = wm1811_read_cache(WM1811_DAC1_RIGHT_VOLUME);
	val &= ~(WM1811_DAC1R_MUTE_MASK | WM1811_DAC1_VU_MASK | WM1811_DAC1R_VOL_MASK);
	val |= WM1811_DAC1R_MUTE | WM1811_DAC1_VU | WM1811_DAC1R_VOL_0dB;
	wm1811_write(codec, WM1811_DAC1_RIGHT_VOLUME, val);

	// Hidden
	wm1811_write(codec, 0x102, 0x0003);

	// Class W (1)
	val = wm1811_read_cache(WM1811_CLASS_W_1);
	val &= ~(WM1811_CLASS_W_RESERVED_MASK | WM1811_CP_DYN_PWR_MASK);
	val |= WM1811_CLASS_W_RESERVED | WM1811_CP_DYN_PWR;
	wm1811_write(codec, WM1811_CLASS_W_1, val);

	//WM1811_Set_Playback_EAR_Volume(codec, STATUS_ON);
	// Left output volume
	val = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
	val &= ~(WM1811_HPOUT1L_VOL_MASK | WM1811_HPOUT1L_VU_MASK |
			WM1811_HPOUT1L_MUTE_N_MASK | WM1811_HPOUT1L_ZC_MASK);
	val |= (WM1811_HPOUT1L_VU | WM1811_HPOUT1L_ZC | WM1811_HPOUT1L_MUTE_N);

	/* added by jinho.lim to make different volume as locale */
	#ifdef MID_LOCALE_VOLUME
	if(wm1811->locale == LOCALE_EUR)
	{
	val |= gHpVolumeEUR;
	DEBUG_LOG("[EUR]Headset Volume(not Music)=%x", gHpVolumeEUR);
	}
	else
	{
	val |= gHpVolumeWW;
	DEBUG_LOG("[WW]Headset Volume(not Music)=%x", gHpVolumeWW);
	}
	#endif
	wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val);

	// Right output volume
	val = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
	val&= ~(WM1811_HPOUT1R_VOL_MASK | WM1811_HPOUT1R_VU_MASK |
			WM1811_HPOUT1R_MUTE_N_MASK | WM1811_HPOUT1R_ZC_MASK);
	val |= (WM1811_HPOUT1R_VU | WM1811_HPOUT1R_ZC | WM1811_HPOUT1R_MUTE_N);

	/* added by jinho.lim to make different volume as locale */
	#ifdef MID_LOCALE_VOLUME
	if(wm1811->locale == LOCALE_EUR)
	{
	val |= gHpVolumeEUR;
	DEBUG_LOG("[EUR]Headset Volume(not Music)=%x", gHpVolumeEUR);
	}
	else
	{
	val |= gHpVolumeWW;
	DEBUG_LOG("[WW]Headset Volume(not Music)=%x", gHpVolumeWW);
	}
	#endif
	wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val);
	// Hidden
	wm1811_write(codec, 0x56, 0x0C07);
	//msleep(DELAY_FOR_NOISE);
	wm1811_write(codec, 0x5D, 0x007E);
	//msleep(DELAY_FOR_NOISE);
	wm1811_write(codec, 0x5E, 0x0000);
	//msleep(DELAY_FOR_NOISE);

	// DC Servo (2)
	val = wm1811_read_cache(WM1811_DC_SERVO_2);
	val &= ~(WM1811_DCS_SERIES_NO_01_MASK);
	val |= 0x0540;
	wm1811_write(codec, WM1811_DC_SERVO_2, val);
	//msleep(DELAY_FOR_NOISE);

	// Power management (1)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_1);
	val &= ~(WM1811_PM1_ALL_MASK);
	val |= (WM1811_HPOUT1L_ENA | WM1811_HPOUT1R_ENA |
			WM1811_VMID_SEL_2X40K | WM1811_BIAS_ENA);
	if (HEADSET_4POLE_WITH_MIC == wm1811_get_ear_state())
		val |= (WM1811_MICB1_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
	//msleep(DELAY_FOR_NOISE);

	// Analogue HP (1)
	val = wm1811_read_cache(WM1811_ANALOGUE_HP_1);
	val &= ~(WM1811_HPOUT1L_DLY_MASK | WM1811_HPOUT1R_DLY_MASK);
	val |= WM1811_HPOUT1L_DLY | WM1811_HPOUT1R_DLY;
	wm1811_write(codec, WM1811_ANALOGUE_HP_1, val);

	// Charge Pump (1)
	val = wm1811_read_cache(WM1811_CHARGE_PUMP_1);
	val &= ~(WM1811_CP_ENA_MASK | WM1811_CP_ENA_DEFAULT_MASK);
	val |= WM1811_CP_ENA | WM1811_CP_ENA_DEFAULT;
	wm1811_write(codec, WM1811_CHARGE_PUMP_1, val);
	msleep(20);		// used by wolfson guide(not datasheet)
	//msleep(DELAY_FOR_NOISE);

	// Don't set many times this register
	//if (SND_SOC_BIAS_OFF == codec->dapm->bias_level)
	{
		DEBUG_LOG("SND_SOC_BIAS_OFF");
		// Power management (5)
		val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_5);
		val &= ~(WM1811_PM5_ALL_MASK);
		val |= (WM1811_AIF1DACL_ENA | WM1811_AIF1DACR_ENA |
				WM1811_DAC1L_ENA | WM1811_DAC1R_ENA);
		wm1811_write(codec, WM1811_POWER_MANAGEMENT_5, val);
	}

	// Power Management (3)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_3);
	val &= ~(WM1811_SPKRVOL_ENA_MASK | WM1811_SPKLVOL_ENA_MASK |
			WM1811_MIXOUTLVOL_ENA_MASK | WM1811_MIXOUTRVOL_ENA_MASK |
			WM1811_MIXOUTL_ENA_MASK | WM1811_MIXOUTR_ENA_MASK);
	val |= (WM1811_MIXOUTL_ENA | WM1811_MIXOUTR_ENA |
			WM1811_SPKRVOL_ENA | WM1811_SPKLVOL_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_3, val);

	// Output mixer (1)
	val = wm1811_read_cache( WM1811_OUTPUT_MIXER_1);
	val &= ~(WM1811_DAC1L_TO_MIXOUTL_MASK);
	val |= WM1811_DAC1L_TO_MIXOUTL;
	wm1811_write(codec, WM1811_OUTPUT_MIXER_1, val);

	// Output mixer (2)
	val = wm1811_read_cache( WM1811_OUTPUT_MIXER_2);
	val &= ~(WM1811_DAC1R_TO_MIXOUTR_MASK);
	val |= WM1811_DAC1R_TO_MIXOUTR;
	wm1811_write(codec, WM1811_OUTPUT_MIXER_2, val);

	// DC Servo (1)
	val = wm1811_read_cache(WM1811_DC_SERVO_1);
	val &= ~(WM1811_DCS_TRIG_STARTUP_1_MASK | WM1811_DCS_TRIG_STARTUP_0_MASK |
			WM1811_DCS_ENA_CHAN_1_MASK | WM1811_DCS_ENA_CHAN_0_MASK);

	val |= (WM1811_DCS_TRIG_STARTUP_1 | WM1811_DCS_TRIG_STARTUP_0 |
			WM1811_DCS_ENA_CHAN_1 | WM1811_DCS_ENA_CHAN_0);

	wm1811_write(codec, WM1811_DC_SERVO_1, val);
	msleep(200);		// used by wolfson guide(not datasheet)

	if(wm1811_codec_hp_optimized_check == STATUS_OFF)
	{
		if(wm1811->configured != 1)
		{
			// DC Servo (4)
			DEBUG_LOG("codec->dapm->bias_level=%d\n", codec->dapm->bias_level);
			DEBUG_LOG("!!!!!!!!!!!!!DC Offset CALibration");

			msleep(200);		// used due to popup noise

			val = wm1811_read(codec, WM1811_DC_SERVO_4);
			DEBUG_LOG("BEFORE CALC READ_BACK =%lx\n", val);

			val1 = (((val & 0xFF00) >> 8) -5)&0xFF;
			DEBUG_LOG("BEFORE CALC val1 =%lx\n", val1 );

			val2 = ((val & 0x00FF) -7)&0xFF;
			DEBUG_LOG("BEFORE CALC val2 =%lx\n", val2 );

			val = (val1 << 8) | val2;

			wm1811_write(codec, WM1811_DC_SERVO_4, val);
			
			wm1811_codec_hp_optimized_check = STATUS_ON;
			wm1811_codec_hp_optimized_value = val;

			//msleep(DELAY_FOR_NOISE);
			DEBUG_LOG("AFTER WM1811_DC_SERVO_4=%lx", val);
		}
		else
		{
			wm1811_write(codec, WM1811_DC_SERVO_4, wm1811_codec_hp_optimized_value);
		}

		// DC Servo (1)
		val = wm1811_read_cache(WM1811_DC_SERVO_1);
		val &= ~(WM1811_DCS_TRIG_SINGLE_1_MASK | WM1811_DCS_TRIG_SINGLE_0_MASK |
				WM1811_DCS_TRIG_SERIES_1_MASK | WM1811_DCS_TRIG_SERIES_0_MASK |
				WM1811_DCS_TRIG_STARTUP_1_MASK | WM1811_DCS_TRIG_STARTUP_0_MASK | 
				WM1811_DCS_TRIG_DAC_WR_1_MASK | WM1811_DCS_TRIG_DAC_WR_0_MASK |
				WM1811_DCS_ENA_CHAN_1_MASK | WM1811_DCS_ENA_CHAN_0_MASK);
		val |= (WM1811_DCS_TRIG_DAC_WR_1 | WM1811_DCS_TRIG_DAC_WR_0 |
				WM1811_DCS_ENA_CHAN_1 | WM1811_DCS_ENA_CHAN_0);
		wm1811_write(codec, WM1811_DC_SERVO_1, val);
		//msleep(DELAY_FOR_NOISE);
	}

	msleep(20);		// used by wolfson guide(not datasheet)

	// Left output block
	val = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
	val &= ~(WM1811_HPOUT1L_MUTE_N_MASK);
	wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val);
	
	// Right output block
	val = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
	val &= ~(WM1811_HPOUT1R_MUTE_N_MASK);
	wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val);
	
	// Analogue HP (1)
	val = wm1811_read_cache(WM1811_ANALOGUE_HP_1);
	val &= ~(WM1811_HPOUT1L_RMV_SHORT_MASK | WM1811_HPOUT1L_OUTP_MASK |
		WM1811_HPOUT1L_DLY_MASK | WM1811_HPOUT1R_RMV_SHORT_MASK |
		WM1811_HPOUT1R_OUTP_MASK | WM1811_HPOUT1R_DLY_MASK);
	val |= WM1811_HPOUT1L_RMV_SHORT | WM1811_HPOUT1L_OUTP |
		WM1811_HPOUT1L_DLY | WM1811_HPOUT1R_RMV_SHORT |
		WM1811_HPOUT1R_OUTP | WM1811_HPOUT1R_DLY;
	wm1811_write(codec, WM1811_ANALOGUE_HP_1, val);
	//msleep(DELAY_FOR_NOISE);

	// DAC Left Volume 
	val = wm1811_read_cache(WM1811_DAC1_LEFT_VOLUME);
	val &= ~(WM1811_DAC1L_MUTE_MASK | WM1811_DAC1_VU_MASK |
		WM1811_DAC1L_VOL_MASK);
	val |= WM1811_DAC1_VU | WM1811_DAC1L_VOL_0dB;
	wm1811_write(codec, WM1811_DAC1_LEFT_VOLUME, val);

	// DAC Right Volume
	val = wm1811_read_cache(WM1811_DAC1_RIGHT_VOLUME);
	val &= ~(WM1811_DAC1R_MUTE_MASK | WM1811_DAC1_VU_MASK |
		WM1811_DAC1R_VOL_MASK);
	val |= WM1811_DAC1_VU | WM1811_DAC1R_VOL_0dB;
	wm1811_write(codec, WM1811_DAC1_RIGHT_VOLUME, val);

	// AIF1 DAC1 Filters (1)
	val = wm1811_read_cache(WM1811_AIF1_DAC1_FILTERS_1);
	val &= ~(WM1811_AIF1DAC1_MUTE_MASK);
	wm1811_write(codec, WM1811_AIF1_DAC1_FILTERS_1, val);

#if 0
	// Power Management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_AIF1ADCL_ENA_MASK | WM1811_AIF1ADCR_ENA_MASK);
	val |= (WM1811_AIF1ADCL_ENA | WM1811_AIF1ADCR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);
#endif
}


void WM1811_Set_Playback_Off(struct snd_soc_codec *codec)
{
	DEBUG_LOG("wm1811_playback_device=%d", wm1811_playback_device);
	
	switch (wm1811_playback_device)
	{
		case OFF:
			break;

		case RCV:
			WM1811_Set_Playback_RCV_Off(codec);
			break;

		case SPK:
			WM1811_Set_Playback_SPK_Off(codec);
			break;
			
		case EXTRA_SPEAKER:
			WM1811_Set_Playback_Dock_Off(codec);
			break;

		case HP3P:
		case HP4P:
			WM1811_Set_Playback_EAR_Off(codec);
			break;

		case SPK_HP:
			WM1811_Set_Playback_SPK_Off(codec);
			WM1811_Set_Playback_EAR_Off(codec);
			break;

		default:
			DEBUG_LOG_ERR("!!!!playback path setting failed!!!\n");
			break;
	}
}

bool WM1811_Set_Playback_SPK(struct snd_soc_codec *codec)
{
	u16 val = 0;

	// Power management (1)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_1);

	//for aparent codec setting, sejong
#if 0
	if ((WM1811_SPKOUTL_ENA_MASK & val)==WM1811_SPKOUTL_ENA)	{
		DEBUG_LOG("Speaker already turn on");
		return 0;
	}
	else		{
		val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_1);
		val &= ~(WM1811_SPKOUTL_ENA_MASK);
		val |= (WM1811_SPKOUTL_ENA);
		wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
		DEBUG_LOG("Speaker turn on");
	}
#else
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_1);
	val &= ~(WM1811_SPKOUTL_ENA_MASK);
	val |= (WM1811_SPKOUTL_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
	DEBUG_LOG("Speaker turn on");
#endif

	// Temporary code for estimated codec bug that speaker output level decreased by AIF1ADCL_ENA
	if ((STATUS_OFF==wm1811_recording_device) && (wm1811_vr_mode == STATUS_OFF))
	{
		DEBUG_LOG("WM1811_AIF1ADCL_ENA Disalbed");
		// Power Management (4)
		val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
		val &= ~(WM1811_AIF1ADCL_ENA_MASK | WM1811_AIF1ADCR_ENA_MASK);
		wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);
	}
	// Speaker Mixer Left Attenuation
	val = wm1811_read_cache(WM1811_SPKMIXL_ATTENUATION);
	val &= ~(WM1811_DAC1L_SPKMIXL_VOL_MASK | WM1811_SPKMIXL_VOL_MASK);
	val |= WM1811_SPKMIXL_VOL_0dB;
	wm1811_write(codec, WM1811_SPKMIXL_ATTENUATION, val);

	// Speaker Mixer Right Attenuation
	val = wm1811_read_cache(WM1811_SPKMIXR_ATTENUATION);
	val &= ~(WM1811_DAC1R_SPKMIXR_VOL_MASK | WM1811_SPKMIXR_VOL_MASK);
	val |= WM1811_SPKMIXR_VOL_0dB;
	wm1811_write(codec, WM1811_SPKMIXR_ATTENUATION, val);

	// Speaker Output Mixer
	val = wm1811_read_cache(WM1811_SPKOUT_MIXERS);
	val &= ~(WM1811_SPKMIXL_TO_SPKOUTL_MASK | WM1811_SPKMIXR_TO_SPKOUTL_MASK);
	val |= (WM1811_SPKMIXL_TO_SPKOUTL | WM1811_SPKMIXR_TO_SPKOUTL);
	wm1811_write(codec, WM1811_SPKOUT_MIXERS, val);

	// Class D
	val = wm1811_read_cache(WM1811_CLASSD);
	val &= ~(WM1811_SPKOUTL_BOOST_MASK);
	//SPK output level fix, sejong
	//val |= 0x0168;
	val |= 0x0160;
	wm1811_write(codec, WM1811_CLASSD, val);

	// Speaker Mixer
	val = wm1811_read_cache( WM1811_SPEAKER_MIXER);
	val &= ~(WM1811_DAC1L_TO_SPKMIXL_MASK | WM1811_DAC1R_TO_SPKMIXR_MASK);
	val |= WM1811_DAC1L_TO_SPKMIXL | WM1811_DAC1R_TO_SPKMIXR;
	wm1811_write(codec, WM1811_SPEAKER_MIXER, val);


	// Enable Dynamic range control
	WM1811_Set_Playback_SPK_DRC(codec, STATUS_ON);

	//msleep(50);
#ifdef WM1811_SPK_EQ_EN
	wm1811_write(codec, 0x0480, 0x0318);
	wm1811_write(codec, 0x0480, 0x0298);
	wm1811_write(codec, 0x0482, 0x0F8D);
	wm1811_write(codec, 0x0483, 0x0401);
	wm1811_write(codec, 0x0494, 0x0000);
	wm1811_write(codec, 0x0484, 0x01CC);
	wm1811_write(codec, 0x0485, 0x1EB5);
	wm1811_write(codec, 0x0486, 0xF145);
	wm1811_write(codec, 0x0487, 0x0B75);
	wm1811_write(codec, 0x0488, 0x01C5);
	wm1811_write(codec, 0x0489, 0x1C58);
	wm1811_write(codec, 0x048A, 0xF373);
	wm1811_write(codec, 0x048B, 0x0A54);
	wm1811_write(codec, 0x048C, 0x0558);
	wm1811_write(codec, 0x048D, 0x168E);
	wm1811_write(codec, 0x048E, 0xF829);
	wm1811_write(codec, 0x048F, 0x07AD);
	wm1811_write(codec, 0x0490, 0x1103);
	wm1811_write(codec, 0x0491, 0x0564);	
	wm1811_write(codec, 0x0492, 0x0559);
	wm1811_write(codec, 0x0493, 0x4000);
	wm1811_write(codec, 0x0481, 0x6280);	
	wm1811_write(codec, 0x0481, 0x62C0);	
	wm1811_write(codec, 0x0480, 0x0299);
#endif
	// Speaker volume unmute
	WM1811_Set_Playback_SPK_Volume(codec, STATUS_ON);
}


void WM1811_Set_Playback_SPK_Off(struct snd_soc_codec *codec)
{
#if 0
	#define CLASS_D_DELAY 	30
	#define SPKVOL_DELAY	5
	#define SPKMIX_DELAY	30
	#define SPKOUT_DELAY	30
#endif
	#define TABLE_NUM		30

	u16 speaker_volume_table[TABLE_NUM] = {0x3a, 0x39, 0x38, 0x37, 0x36, 0x34, 0x32, 0x30, 0x2e, 0x2c,
										      0x2a, 0x28, 0x26, 0x24, 0x22, 0x20, 0x1e, 0x1c, 0x1a, 0x19,
										      0x17, 0x15, 0x13, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x00};
	
	u16 val = 0;
	u16 table_cnt;

	// Power management (1)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_1);
	if ( !(WM1811_SPKOUTL_ENA_MASK & val) )	{
		DEBUG_LOG("Speaker already turn off");

		return;

	}


	// 1. SPKMIX : 0dB -> -6dB
	// Speaker Mixer Right Attenuation
	val = wm1811_read_cache(WM1811_SPKMIXR_ATTENUATION);
	val &= ~(WM1811_DAC1R_SPKMIXR_VOL_MASK);
	val |= (WM1811_DAC1R_SPKMIXR_VOL);
	wm1811_write(codec, WM1811_SPKMIXR_ATTENUATION, val);
	//msleep(SPKMIX_DELAY);
	
	// Speaker Mixer Left Attenuation
	val = wm1811_read_cache(WM1811_SPKMIXL_ATTENUATION);
	val &= ~(WM1811_DAC1L_SPKMIXL_VOL_MASK);
	val |= (WM1811_DAC1L_SPKMIXL_VOL);
	wm1811_write(codec, WM1811_SPKMIXL_ATTENUATION, val);


	// 2. SPKVOL : current -> -57dB
	for (table_cnt=0; table_cnt<TABLE_NUM; table_cnt++)	{
		// SPK Volume left
		val = wm1811_read_cache( WM1811_SPEAKER_VOLUME_LEFT);
		val &= ~(WM1811_SPKOUTL_VU_MASK | WM1811_SPKOUTL_VOL_MASK);
		val |= (WM1811_SPKOUTL_VU | speaker_volume_table[table_cnt]);
		wm1811_write(codec, WM1811_SPEAKER_VOLUME_LEFT, val);

		// SPK Volume Right
		val = wm1811_read_cache( WM1811_SPEAKER_VOLUME_RIGHT);
		val &= ~(WM1811_SPKOUTR_VU_MASK | WM1811_SPKOUTR_VOL_MASK);
		val |= (WM1811_SPKOUTR_VU | speaker_volume_table[table_cnt]);
		wm1811_write(codec, WM1811_SPEAKER_VOLUME_RIGHT, val);

		//msleep(SPKVOL_DELAY);

//		DEBUG_LOG("VOL=%x, %x, %d", wm1811_read_cache( WM1811_SPEAKER_VOLUME_LEFT), speaker_volume_table[table_cnt], table_cnt);
	}

	// 3. Disable SPKMIXR to SPKOUTR
	// Speaker Output Mixer
	val = wm1811_read_cache(WM1811_SPKOUT_MIXERS);
	val &= ~(WM1811_SPKMIXR_TO_SPKOUTR_MASK);
	wm1811_write(codec, WM1811_SPKOUT_MIXERS, val);
	//msleep(SPKOUT_DELAY);

#if 0	// not used in the disable sequence
	// Speaker Mixer
	val = wm1811_read_cache( WM1811_SPEAKER_MIXER);
	val &= ~(WM1811_DAC1L_TO_SPKMIXL_MASK | WM1811_DAC1R_TO_SPKMIXR_MASK);
	wm1811_write(codec, WM1811_SPEAKER_MIXER, val);
#endif

	// 4. Speaker boost : 6dB -> 4.5dB
	// Class D
	val = wm1811_read_cache(WM1811_CLASSD);
	val &= ~(WM1811_SPKOUTL_BOOST_MASK | WM1811_SPKOUTR_BOOST_MASK);
	val |= 0x158;
	wm1811_write(codec, WM1811_CLASSD, val);
	//msleep(CLASS_D_DELAY);

	// 5. Off the left speaker
	// Power management (1)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_1);
	val &= ~(WM1811_SPKOUTL_ENA_MASK);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);

#ifdef WM1811_SPK_EQ_EN
	DEBUG_LOG("Speaker turn off");
	wm1811_write(codec, 0x0480, 0x0298);
#endif
	// Disabled Dynamic range control
	WM1811_Set_Playback_SPK_DRC(codec, STATUS_OFF);
}


void WM1811_Set_Playback_SPK_DRC(struct snd_soc_codec *codec, WM1811_STATUS status)
{
	DEBUG_LOG("DRC=%d", status);
#ifndef WM1811_USE_SPK_DRC
	return;
#endif
	
	if (STATUS_ON == status)		{
		wm1811_write(codec, WM1811_AIF1_DRC1_2, 0x0253);
		wm1811_write(codec, WM1811_AIF1_DRC1_3, 0x0010);
		wm1811_write(codec, WM1811_AIF1_DRC1_4, 0x0187);
		wm1811_write(codec, WM1811_AIF1_DRC1_5, 0x0000);
		wm1811_write(codec, WM1811_AIF1_DRC1_1, 0x001E);
	}
	else		{
		wm1811_write(codec, WM1811_AIF1_DRC1_2, 0x0845);
		wm1811_write(codec, WM1811_AIF1_DRC1_3, 0x0000);
		wm1811_write(codec, WM1811_AIF1_DRC1_4, 0x0000);
		wm1811_write(codec, WM1811_AIF1_DRC1_5, 0x0000);
		wm1811_write(codec, WM1811_AIF1_DRC1_1, 0x0098);
	}
}


void WM1811_Set_Playback_EAR(struct snd_soc_codec *codec)
{

	int val1,val2;
#if 0
	u16 val = 0;
	
	// Output mixer (1)
	val = wm1811_read_cache( WM1811_OUTPUT_MIXER_1);
	val &= ~(WM1811_DAC1L_TO_MIXOUTL_MASK);
	val |= WM1811_DAC1L_TO_MIXOUTL;
	wm1811_write(codec, WM1811_OUTPUT_MIXER_1, val);

	// Output mixer (2)
	val = wm1811_read_cache( WM1811_OUTPUT_MIXER_2);
	val &= ~(WM1811_DAC1R_TO_MIXOUTR_MASK);
	val |= WM1811_DAC1R_TO_MIXOUTR;
	wm1811_write(codec, WM1811_OUTPUT_MIXER_2, val);

	// Power management (3)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_3);
	val &= ~(WM1811_MIXOUTL_ENA_MASK | WM1811_MIXOUTR_ENA_MASK);
	val |= (WM1811_MIXOUTL_ENA | WM1811_MIXOUTR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_3, val);

	// Power management (5)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_5);
	val &= ~(WM1811_AIF1DACL_ENA_MASK | WM1811_AIF1DACR_ENA_MASK |
			WM1811_DAC1L_ENA_MASK | WM1811_DAC1R_ENA_MASK);
	val |= (WM1811_AIF1DACL_ENA | WM1811_AIF1DACR_ENA |
			WM1811_DAC1L_ENA | WM1811_DAC1R_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_5, val);
#endif

	DEBUG_LOG("");

	//for the case of music volume 0 noise, sejong.
	//does not unmute at the music volume 0.
	if((uMusicStreamVol != 0) || (uIsMusicStreamOn == STATUS_OFF))		
	{
		/* add mute code after fade out volume */
		DEBUG_LOG( "UN-mute after ear Path-----------------------------------------------");
		val1 = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
		val1 &= ~(WM1811_HPOUT1L_MUTE_N_MASK);
		val1 |= (WM1811_HPOUT1L_MUTE_N_MASK);
		wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val1);

		// Right output volume
		val2 = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
		val2&= ~(WM1811_HPOUT1R_MUTE_N_MASK);
		val2 |= (WM1811_HPOUT1R_MUTE_N_MASK);
		wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val2);	
	}

	WM1811_Set_Playback_EAR_Volume(codec, STATUS_ON);
		
}


void WM1811_Set_Playback_EAR_Off(struct snd_soc_codec *codec)
{
	DEBUG_LOG("");
	int val1,val2;

	if(wm1811_fm_device != FM_OFF)
		gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_ON);

	// Earphone volume mute
	WM1811_Set_Playback_EAR_Volume(codec, STATUS_OFF);
		/* add mute code after fade out volume */
/*
		DEBUG_LOG( "mute after ear off-----------------------------------------------");
		val1 = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
		val1 &= ~(WM1811_HPOUT1L_MUTE_N_MASK);
		wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val1);

		// Right output volume
		val2 = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
		val2&= ~(WM1811_HPOUT1R_MUTE_N_MASK);
		wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val2);	
*/

#if 0
	u16 val = 0;

	// Output mixer (1)
	val = wm1811_read_cache( WM1811_OUTPUT_MIXER_1);
	val &= ~(WM1811_DAC1L_TO_MIXOUTL_MASK);
	wm1811_write(codec, WM1811_OUTPUT_MIXER_1, val);

	// Output mixer (2)
	val = wm1811_read_cache( WM1811_OUTPUT_MIXER_2);
	val &= ~(WM1811_DAC1R_TO_MIXOUTR_MASK);
	wm1811_write(codec, WM1811_OUTPUT_MIXER_2, val);

	// Power management (3)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_3);
	val &= ~(WM1811_MIXOUTL_ENA_MASK | WM1811_MIXOUTR_ENA_MASK);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_3, val);

	// Power management (5)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_5);
	val &= ~(WM1811_AIF1DACL_ENA_MASK | WM1811_AIF1DACR_ENA_MASK |
			WM1811_DAC1L_ENA_MASK | WM1811_DAC1R_ENA_MASK);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_5, val);
#endif
}

int receverStatus=STATUS_OFF;
void WM1811_Set_Playback_RCV(struct snd_soc_codec *codec)
{
	u16 val = 0;

	if(STATUS_ON==receverStatus) return;
		
	// Power management (1)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_1);

	//Ensured setting, sejong.
#if 0
	if ((WM1811_HPOUT2_ENA_MASK & val)==WM1811_HPOUT2_ENA)	{
		DEBUG_LOG("Receiver already turn on");
		return;
	}
	else		{
		DEBUG_LOG("Receiver turn on");
		val &= ~WM1811_HPOUT2_ENA_MASK;
		val |= WM1811_HPOUT2_ENA;
		wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
	}
#else	
	DEBUG_LOG("Receiver turn on");
	val &= ~WM1811_HPOUT2_ENA_MASK;
	val |= WM1811_HPOUT2_ENA;
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
#endif

	WM1811_Set_Playback_RCV_Volume(codec, STATUS_ON);

	// Antipop (1)
	val = wm1811_read_cache(WM1811_ANTIPOP_1);
	val &= ~(WM1811_HPOUT2_IN_ENA_MASK);
	val |= (WM1811_HPOUT2_IN_ENA);
	wm1811_write(codec, WM1811_ANTIPOP_1, val);

	// HPOUT2 Mixer
	val = wm1811_read_cache(WM1811_HPOUT2_MIXER);
	val &= ~(WM1811_MIXOUTLVOL_TO_HPOUT2_MASK | WM1811_MIXOUTRVOL_TO_HPOUT2_MASK);
	val |= WM1811_MIXOUTLVOL_TO_HPOUT2 | WM1811_MIXOUTRVOL_TO_HPOUT2;
	wm1811_write(codec, WM1811_HPOUT2_MIXER, val);

	//Mix Out Volume
	val = wm1811_read_cache(WM1811_OUTPUT_MIXER_5);
	val &= ~(WM1811_DACL_MIXOUTL_VOL_MASK);
	val |= (0x01 << WM1811_DACL_MIXOUTL_VOL_SHIFT);
	wm1811_write(codec, WM1811_OUTPUT_MIXER_5, val);

	//Mix Out Volume
	val = wm1811_read_cache(WM1811_OUTPUT_MIXER_6);
	val &= ~(WM1811_DACR_MIXOUTR_VOL_MASK);
	val |= (0x01 << WM1811_DACR_MIXOUTR_VOL_SHIFT);
	wm1811_write(codec, WM1811_OUTPUT_MIXER_6, val);

	// Power Management (3)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_3);
	val &= ~(WM1811_MIXOUTLVOL_ENA_MASK | WM1811_MIXOUTRVOL_ENA_MASK);
	val |= (WM1811_MIXOUTLVOL_ENA | WM1811_MIXOUTRVOL_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_3, val);

	//msleep(50);
	receverStatus=STATUS_ON;

	DEBUG_LOG("");
}


void WM1811_Set_Playback_RCV_Off(struct snd_soc_codec *codec)
{
	u16 val = 0;
	if(STATUS_OFF==receverStatus) return;
	
	// Power management (1)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_1);
#if 0	
	if (!(WM1811_HPOUT2_ENA_MASK & val))	{
		DEBUG_LOG("Receiver already turn off");
		return;
	}
	else		{
		DEBUG_LOG("Receiver turn off");

		// Receiver volume mute
		WM1811_Set_Playback_RCV_Volume(codec, STATUS_OFF);
		
		val &= ~WM1811_HPOUT2_ENA_MASK;
		wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
		msleep(50);
	}
#else
	DEBUG_LOG("Receiver turn off");

	// Receiver volume mute
	WM1811_Set_Playback_RCV_Volume(codec, STATUS_OFF);
	
	val &= ~WM1811_HPOUT2_ENA_MASK;
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
	//msleep(50);
#endif

	// Antipop (1)
	val = wm1811_read_cache(WM1811_ANTIPOP_1);
	val &= ~(WM1811_HPOUT2_IN_ENA_MASK);
	wm1811_write(codec, WM1811_ANTIPOP_1, val);

	// HPOUT2 Mixer
	val = wm1811_read_cache(WM1811_HPOUT2_MIXER);
	val &= ~(WM1811_MIXOUTLVOL_TO_HPOUT2_MASK | WM1811_MIXOUTRVOL_TO_HPOUT2_MASK);
	wm1811_write(codec, WM1811_HPOUT2_MIXER, val);

	//Mix Out Volume
	val = wm1811_read_cache(WM1811_OUTPUT_MIXER_5);
	val &= ~(WM1811_DACL_MIXOUTL_VOL_MASK);
	wm1811_write(codec, WM1811_OUTPUT_MIXER_5, val);

	//Mix Out Volume
	val = wm1811_read_cache(WM1811_OUTPUT_MIXER_6);
	val &= ~(WM1811_DACR_MIXOUTR_VOL_MASK);
	wm1811_write(codec, WM1811_OUTPUT_MIXER_6, val);

	// Power Management (3)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_3);
	val &= ~(WM1811_MIXOUTLVOL_ENA_MASK | WM1811_MIXOUTRVOL_ENA_MASK);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_3, val);

	receverStatus=STATUS_OFF;
	DEBUG_LOG("");
}


void WM1811_Set_Playback_Dock(struct snd_soc_codec *codec)
{
	u16 val = 0;
	
	DEBUG_LOG("");

	// Antipop (1)
	val = wm1811_read_cache(WM1811_ANTIPOP_1);
	val &= ~(WM1811_LINEOUT_VMID_BUF_ENA_MASK);
	val |= (WM1811_LINEOUT_VMID_BUF_ENA);
	wm1811_write(codec, WM1811_ANTIPOP_1, val);


	// Power Management (3)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_3);
	val &= ~(WM1811_LINEOUT1N_ENA_MASK | WM1811_LINEOUT1P_ENA_MASK
		|WM1811_MIXOUTLVOL_ENA_MASK|WM1811_MIXOUTRVOL_ENA_MASK
		|WM1811_MIXOUTL_ENA_MASK|WM1811_MIXOUTR_ENA_MASK);
	val |= (WM1811_LINEOUT1N_ENA | WM1811_LINEOUT1P_ENA
		|WM1811_MIXOUTLVOL_ENA|WM1811_MIXOUTRVOL_ENA
		|WM1811_MIXOUTL_ENA|WM1811_MIXOUTR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_3, val);

	// Line mixer (1)
	val = wm1811_read_cache(WM1811_LINE_MIXER_1);
	val &= ~(WM1811_MIXOUTL_TO_LINEOUT1P_MASK | WM1811_MIXOUTR_TO_LINEOUT1N_MASK |
			WM1811_LINEOUT1_MODE_MASK);
	val |= (WM1811_MIXOUTL_TO_LINEOUT1P | WM1811_MIXOUTR_TO_LINEOUT1N | WM1811_LINEOUT1_MODE);
	wm1811_write(codec, WM1811_LINE_MIXER_1, val);


	// MIX OUTPUT LEFT
	val = wm1811_read_cache(WM1811_LEFT_OPGA_VOLUME);
	val &= ~(WM1811_MIXOUT_VU_MASK | WM1811_MIXOUTL_ZC_MASK 
		|WM1811_MIXOUTL_MUTE_N_MASK|WM1811_MIXOUTL_VOL_MASK);
#ifdef WM1811_USE_LINEOUT
	val |= (WM1811_MIXOUT_VU | WM1811_MIXOUTL_ZC |WM1811_MIXOUTL_MUTE_N|LINEOUT_VOL[uMusicStreamVol]);
#else
	val |= (WM1811_MIXOUT_VU | WM1811_MIXOUTL_ZC |WM1811_MIXOUTL_MUTE_N|gLineOutVolume);
#endif	
	wm1811_write(codec, WM1811_LEFT_OPGA_VOLUME, val);

	// MIX OUTPUT RIGHT
	val = wm1811_read_cache(WM1811_RIGHT_OPGA_VOLUME);
	val &= ~(WM1811_MIXOUT_VU_MASK | WM1811_MIXOUTR_ZC_MASK 
		|WM1811_MIXOUTR_MUTE_N_MASK|WM1811_MIXOUTR_VOL_MASK);
#ifdef WM1811_USE_LINEOUT
	val |= (WM1811_MIXOUT_VU | WM1811_MIXOUTR_ZC |WM1811_MIXOUTR_MUTE_N|LINEOUT_VOL[uMusicStreamVol]);
#else
	val |= (WM1811_MIXOUT_VU | WM1811_MIXOUTR_ZC |WM1811_MIXOUTR_MUTE_N|gLineOutVolume);
#endif
	wm1811_write(codec, WM1811_RIGHT_OPGA_VOLUME, val);

/*
	// LINE_OUTPUTS_VOLUME
	val = wm1811_read_cache(WM1811_LINE_OUTPUTS_VOLUME);
	val &= ~(WM1811_LINEOUT1N_MUTE_MASK | WM1811_LINEOUT1P_MUTE_MASK|WM1811_LINEOUT1_VOL_MASK);
	wm1811_write(codec, WM1811_LINE_OUTPUTS_VOLUME, val);
*/

	WM1811_Set_Playback_Dock_Volume(codec, STATUS_ON);
}


void WM1811_Set_Playback_Dock_Off(struct snd_soc_codec *codec)
{
	u16 val = 0;
	
	DEBUG_LOG("");

	WM1811_Set_Playback_Dock_Volume(codec, STATUS_OFF);

	// Antipop (1)
	val = wm1811_read_cache(WM1811_ANTIPOP_1);
	val &= ~(WM1811_LINEOUT_VMID_BUF_ENA_MASK);
	wm1811_write(codec, WM1811_ANTIPOP_1, val);

//////////////////////////////////////////////////

	// Power Management (3)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_3);
	val &= ~(WM1811_LINEOUT1N_ENA_MASK | WM1811_LINEOUT1P_ENA_MASK
		|WM1811_MIXOUTLVOL_ENA_MASK|WM1811_MIXOUTRVOL_ENA_MASK
		|WM1811_MIXOUTL_ENA_MASK|WM1811_MIXOUTR_ENA_MASK);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_3, val);

	// Line mixer (1)
	val = wm1811_read_cache(WM1811_LINE_MIXER_1);
	val &= ~(WM1811_MIXOUTL_TO_LINEOUT1P_MASK | WM1811_MIXOUTR_TO_LINEOUT1N_MASK |
			WM1811_LINEOUT1_MODE_MASK);
	wm1811_write(codec, WM1811_LINE_MIXER_1, val);

	// MIX OUTPUT LEFT
	val = wm1811_read_cache(WM1811_LEFT_OPGA_VOLUME);
	val &= ~(WM1811_MIXOUT_VU_MASK | WM1811_MIXOUTL_ZC_MASK 
		|WM1811_MIXOUTL_MUTE_N_MASK|WM1811_MIXOUTL_VOL_MASK);
	val |= (WM1811_MIXOUT_VU | WM1811_MIXOUTL_ZC |WM1811_MIXOUTL_MUTE_N|0x39);
	wm1811_write(codec, WM1811_LEFT_OPGA_VOLUME, val);

	// MIX OUTPUT RIGHT
	val = wm1811_read_cache(WM1811_RIGHT_OPGA_VOLUME);
	val &= ~(WM1811_MIXOUT_VU_MASK | WM1811_MIXOUTR_ZC_MASK 
		|WM1811_MIXOUTR_MUTE_N_MASK|WM1811_MIXOUTR_VOL_MASK);
	val |= (WM1811_MIXOUT_VU | WM1811_MIXOUTR_ZC |WM1811_MIXOUTR_MUTE_N|0x39);
	wm1811_write(codec, WM1811_RIGHT_OPGA_VOLUME, val);

//////////////////////////////////////////////////
/*
	// Power Management (3)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_3);
	val &= ~(WM1811_LINEOUT1N_ENA_MASK | WM1811_LINEOUT1P_ENA_MASK);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_3, val);

	// Line mixer (1)
	val = wm1811_read_cache(WM1811_LINE_MIXER_1);
	val &= ~(WM1811_MIXOUTR_TO_LINEOUT1N_MASK | WM1811_LINEOUT1_MODE_MASK |
			WM1811_IN1R_TO_LINEOUT1P_MASK);
	wm1811_write(codec, WM1811_LINE_MIXER_1, val);
*/
}


void WM1811_Set_SPK_Mixer(struct snd_soc_codec *codec, WM1811_STATUS status)
{
	u16 val = 0;

	DEBUG_LOG("");

	if (STATUS_ON == status)
	{
		// Speaker Mixer
		val = wm1811_read_cache( WM1811_SPEAKER_MIXER);
		val &= ~(WM1811_DAC1L_TO_SPKMIXL_MASK | WM1811_DAC1R_TO_SPKMIXR_MASK);
		val |= WM1811_DAC1L_TO_SPKMIXL | WM1811_DAC1R_TO_SPKMIXR;
		wm1811_write(codec, WM1811_SPEAKER_MIXER, val);
	}
	else
	{
		// Speaker Mixer
		val = wm1811_read_cache( WM1811_SPEAKER_MIXER);
		val &= ~(WM1811_DAC1L_TO_SPKMIXL_MASK | WM1811_DAC1R_TO_SPKMIXR_MASK);
		wm1811_write(codec, WM1811_SPEAKER_MIXER, val);
	}
}


void WM1811_Set_Output_Mixer(struct snd_soc_codec *codec, WM1811_STATUS status)
{
	u16 val = 0;

	DEBUG_LOG("Status=%d", status);

	if (STATUS_ON == status)
	{
		DEBUG_LOG("");
		
		// Output mixer (1)
		val = wm1811_read_cache( WM1811_OUTPUT_MIXER_1);
		val &= ~(WM1811_DAC1L_TO_MIXOUTL_MASK);
		val |= WM1811_DAC1L_TO_MIXOUTL;
		wm1811_write(codec, WM1811_OUTPUT_MIXER_1, val);

		// Output mixer (2)
		val = wm1811_read_cache( WM1811_OUTPUT_MIXER_2);
		val &= ~(WM1811_DAC1R_TO_MIXOUTR_MASK);
		val |= WM1811_DAC1R_TO_MIXOUTR;
		wm1811_write(codec, WM1811_OUTPUT_MIXER_2, val);
	}
	else
	{
		// Output mixer (1)
		val = wm1811_read_cache( WM1811_OUTPUT_MIXER_1);
		val &= ~(WM1811_DAC1L_TO_MIXOUTL_MASK);
		wm1811_write(codec, WM1811_OUTPUT_MIXER_1, val);

		// Output mixer (2)
		val = wm1811_read_cache( WM1811_OUTPUT_MIXER_2);
		val &= ~(WM1811_DAC1R_TO_MIXOUTR_MASK);
		wm1811_write(codec, WM1811_OUTPUT_MIXER_2, val);
	}
}


void WM1811_Set_ADC_To_DAC_Sidetone(struct snd_soc_codec *codec, WM1811_STATUS status)
{
	u16 val_l=0;
	u16 val_r=0;

	DEBUG_LOG("Statue=%d", status);

	val_l = wm1811_read_cache(WM1811_DAC1_LEFT_MIXER_ROUTING);
	val_l &= ~(WM1811_ADCL_TO_DACL_MASK);

	val_r = wm1811_read_cache(WM1811_DAC1_RIGHT_MIXER_ROUTING);
	val_r &= ~(WM1811_ADCR_TO_DACR_MASK);

	if (STATUS_ON == status)	{
		val_l |= WM1811_ADCL_TO_DACL;
		val_r |= WM1811_ADCR_TO_DACR;
	}
	wm1811_write(codec, WM1811_DAC1_LEFT_MIXER_ROUTING, val_l);
	wm1811_write(codec, WM1811_DAC1_RIGHT_MIXER_ROUTING, val_r);
}

void WM1811_Set_Playback_SPK_Volume(struct snd_soc_codec *codec, WM1811_STATUS status)
{
	u16 val=0;

	DEBUG_LOG("Status=%d", status);

	if (STATUS_ON == status)
	{
		// SPK Volume left
		val = wm1811_read_cache( WM1811_SPEAKER_VOLUME_LEFT);
		val &= ~(WM1811_SPKOUTL_VU_MASK | WM1811_SPKOUTL_MUTE_N_MASK |
				WM1811_SPKOUTL_ZC_MASK | WM1811_SPKOUTL_VOL_MASK);
		val |= (WM1811_SPKOUTL_VU | WM1811_SPKOUTL_MUTE_N | WM1811_SPKOUTL_ZC);
		
		if (FM_RADIO == wm1811_mode)
			val |= FM_SPK_VOL[uMusicStreamVol];
		else
			val |= gSpkVolume;
		
		wm1811_write(codec, WM1811_SPEAKER_VOLUME_LEFT, val);


		// SPK Volume Right
		val = wm1811_read_cache( WM1811_SPEAKER_VOLUME_RIGHT);
		val &= ~(WM1811_SPKOUTR_VU_MASK | WM1811_SPKOUTR_MUTE_N_MASK |
				WM1811_SPKOUTR_ZC_MASK | WM1811_SPKOUTR_VOL_MASK);

		val |= (WM1811_SPKOUTR_VU | WM1811_SPKOUTR_MUTE_N | WM1811_SPKOUTR_ZC);
		
		if (FM_RADIO == wm1811_mode)
			val |= FM_SPK_VOL[uMusicStreamVol];
		else
			val |= gSpkVolume;
		
		wm1811_write(codec, WM1811_SPEAKER_VOLUME_RIGHT, val);

	}
	else	
	{
		// SPK Volume left
		val = wm1811_read_cache( WM1811_SPEAKER_VOLUME_LEFT);
		val &= ~(WM1811_SPKOUTL_MUTE_N_MASK | WM1811_SPKOUTL_VU_MASK
				| WM1811_SPKOUTL_ZC_MASK | WM1811_SPKOUTL_VOL_MASK);
		val |= (WM1811_SPKOUTL_ZC | WM1811_SPKOUTL_VU);
		wm1811_write(codec, WM1811_SPEAKER_VOLUME_LEFT, val);

		// SPK Volume Right
		val = wm1811_read_cache( WM1811_SPEAKER_VOLUME_RIGHT);
		val &= ~(WM1811_SPKOUTR_MUTE_N_MASK | WM1811_SPKOUTR_VU_MASK	
				| WM1811_SPKOUTR_ZC_MASK | WM1811_SPKOUTR_VOL_MASK);
		val |= (WM1811_SPKOUTR_ZC | WM1811_SPKOUTR_VU);
		wm1811_write(codec, WM1811_SPEAKER_VOLUME_RIGHT, val);
	}
	
}


void WM1811_Set_Playback_EAR_Volume_Only(struct snd_soc_codec *codec, WM1811_STATUS status)
{
	u16 val=0;
	u16 readVal=0;	
	u16 val1, val2 = 0;
	u16 table_cnt=0;
	u16 target = 0;
	short step = 0;
	short i = 0;
	short gap = 0;

#ifdef MID_LOCALE_VOLUME
	/* added by jinho.lim to make different volume as locale */
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);
#endif

	if (STATUS_ON == status)
	{		
		if ( (STATUS_OFF==uIsMusicStreamOn) && (FM_RADIO != wm1811_mode) )	
		{
			/* added by jinho.lim to make different volume as locale */
			#ifdef MID_LOCALE_VOLUME
			if(wm1811->locale == LOCALE_EUR)
			{
				target |= gHpVolumeEUR;
				DEBUG_LOG("[EUR]Headset Volume(not Music)=%x", gHpVolumeEUR);
			}
			else
			{
				target |= gHpVolumeWW;
				DEBUG_LOG("[WW]Headset Volume(not Music)=%x", gHpVolumeWW);
			}
			#endif
		}
		else
		{
			/* added by jinho.lim to make different volume as locale */
			#ifdef MID_LOCALE_VOLUME		
			if(wm1811->locale == LOCALE_EUR)
			{
				if (FM_RADIO == wm1811_mode)
					target |= FM_HP_Vol_EUR[uMusicStreamVol];
				else
					target |= HP_Vol_EUR[uMusicStreamVol];
			}
			else
			{
				if (FM_RADIO == wm1811_mode)
					target |= FM_HP_Vol_WW[uMusicStreamVol];
				else
					target |= HP_Vol_WW[uMusicStreamVol];
			}
			#endif

			//wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val);				
		}

		readVal = wm1811_read(codec, WM1811_LEFT_OUTPUT_VOLUME);
		readVal &= WM1811_HPOUT1L_VOL_MASK;

		//gap = (short)(abs(target - readVal)/2);
		gap = (short)(abs(target - readVal));
		
		DEBUG_LOG("POPUP before var = 0x%x)", readVal);
		DEBUG_LOG("POPUP target = 0x%x)", target);		
		DEBUG_LOG("POPUP Volume gap = 0x%x)", gap);		

		if(target > readVal)
			step = 1;
		else
			step = -1;
		
		for(i=0; i<gap; i++)
		{
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", (readVal+(step*(i+1))));

			// Left output volume
			val1 = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
			val1 &= ~(WM1811_HPOUT1L_VOL_MASK | WM1811_HPOUT1L_VU_MASK | WM1811_HPOUT1L_ZC_MASK);

			// Right output volume
			val2 = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
			val2&= ~(WM1811_HPOUT1R_VOL_MASK | WM1811_HPOUT1R_VU_MASK | WM1811_HPOUT1R_ZC_MASK);

			val1 |= (WM1811_HPOUT1L_VU | WM1811_HPOUT1L_ZC|(u16)(readVal+(step*(i+1))));
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", val1);
			wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val1);

			val2 |= (WM1811_HPOUT1R_VU | WM1811_HPOUT1R_ZC|(u16)(readVal+(step*(i+1))));
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", val2);
			wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val2);	

			msleep(0);
		}			
		
		//DEBUG_LOG("!!!!!!!!!!!!!!!MUTE OFF!!!!!!!!!!!!!!!!!");
		//gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_OFF);
	}
#if 0
	else	
	{
		val = wm1811_read(codec, WM1811_LEFT_OUTPUT_VOLUME);
		val &= WM1811_HPOUT1L_VOL_MASK;

		//gap = (short)(abs(gHpTempVolumeEUR - val)/2);
		gap = (short)abs(gHpTempVolumeEUR - val);
		
		DEBUG_LOG("POPUP before var = 0x%x)", val);
		DEBUG_LOG("POPUP gHpTempVolumeEUR = 0x%x)", gHpTempVolumeEUR);		
		DEBUG_LOG("POPUP Volume gap = 0x%x)", gap);		

		if(gHpTempVolumeEUR > val)
			step = 1;
		else
			step = -1;
		
		for(i=0; i<gap; i++)
		{
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", (val+(step*(i+1))));

			// Left output volume
			val1 = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
			val1 &= ~(WM1811_HPOUT1L_VOL_MASK | WM1811_HPOUT1L_VU_MASK | WM1811_HPOUT1L_ZC_MASK);

			// Right output volume
			val2 = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
			val2&= ~(WM1811_HPOUT1R_VOL_MASK | WM1811_HPOUT1R_VU_MASK | WM1811_HPOUT1R_ZC_MASK);

			val1 |= (WM1811_HPOUT1L_VU | WM1811_HPOUT1L_ZC|(u16)(val+(step*(i+1))));
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", val1);
			wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val1);

			val2 |= (WM1811_HPOUT1R_VU | WM1811_HPOUT1R_ZC|(u16)(val+(step*(i+1))));
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", val2);
			wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val2);	

			//step = (u16)((step + gHpTempVolumeEUR)/2);			

			msleep(0);
		}
	}
#endif
}


void WM1811_Set_Playback_EAR_Volume(struct snd_soc_codec *codec, WM1811_STATUS status)
{
	u16 val=0;
	u16 readVal=0;	
	u16 val1, val2 = 0;
	u16 table_cnt=0;
	u16 target = 0;
	short step = 0;
	short i = 0;
	short gap = 0;

#ifdef MID_LOCALE_VOLUME
	/* added by jinho.lim to make different volume as locale */
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);
#endif

	if (STATUS_ON == status)
	{		
		if ( (STATUS_OFF==uIsMusicStreamOn) && (FM_RADIO != wm1811_mode) )	
		{
			/* added by jinho.lim to make different volume as locale */
			#ifdef MID_LOCALE_VOLUME
			if(wm1811->locale == LOCALE_EUR)
			{
				target |= gHpVolumeEUR;
				DEBUG_LOG("[EUR]Headset Volume(not Music)=%x", gHpVolumeEUR);
			}
			else
			{
				target |= gHpVolumeWW;
				DEBUG_LOG("[WW]Headset Volume(not Music)=%x", gHpVolumeWW);
			}
			#endif
		}
		else
		{
			/* added by jinho.lim to make different volume as locale */
			#ifdef MID_LOCALE_VOLUME		
			if(wm1811->locale == LOCALE_EUR)
			{
				if (FM_RADIO == wm1811_mode)
					target |= FM_HP_Vol_EUR[uMusicStreamVol];
				else
					target |= HP_Vol_EUR[uMusicStreamVol];
			}
			else
			{
				if (FM_RADIO == wm1811_mode)
					target |= FM_HP_Vol_WW[uMusicStreamVol];
				else
					target |= HP_Vol_WW[uMusicStreamVol];
			}
			#endif

			//wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val);				
		}

		readVal = wm1811_read(codec, WM1811_LEFT_OUTPUT_VOLUME);
		readVal &= WM1811_HPOUT1L_VOL_MASK;

		//gap = (short)(abs(target - readVal)/2);
		gap = (short)(abs(target - readVal));
		
		DEBUG_LOG("POPUP before var = 0x%x)", readVal);
		DEBUG_LOG("POPUP target = 0x%x)", target);		
		DEBUG_LOG("POPUP Volume gap = 0x%x)", gap);		

		if(target > readVal)
			step = 1;
		else
			step = -1;
		
		for(i=0; i<gap; i++)
		{
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", (readVal+(step*(i+1))));

			// Left output volume
			val1 = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
			val1 &= ~(WM1811_HPOUT1L_VOL_MASK | WM1811_HPOUT1L_VU_MASK | WM1811_HPOUT1L_ZC_MASK);

			// Right output volume
			val2 = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
			val2&= ~(WM1811_HPOUT1R_VOL_MASK | WM1811_HPOUT1R_VU_MASK | WM1811_HPOUT1R_ZC_MASK);

			val1 |= (WM1811_HPOUT1L_VU | WM1811_HPOUT1L_ZC|(u16)(readVal+(step*(i+1))));
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", val1);
			wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val1);

			val2 |= (WM1811_HPOUT1R_VU | WM1811_HPOUT1R_ZC|(u16)(readVal+(step*(i+1))));
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", val2);
			wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val2);	

			msleep(0);
		}			
		
		DEBUG_LOG("!!!!!!!!!!!!!!!MUTE OFF!!!!!!!!!!!!!!!!!");
		gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_OFF);
	}
	else	
	{
		val = wm1811_read(codec, WM1811_LEFT_OUTPUT_VOLUME);
		val &= WM1811_HPOUT1L_VOL_MASK;

		//gap = (short)(abs(gHpTempVolumeEUR - val)/2);
		gap = (short)abs(gHpTempVolumeEUR - val);
		
		DEBUG_LOG("POPUP before var = 0x%x)", val);
		DEBUG_LOG("POPUP gHpTempVolumeEUR = 0x%x)", gHpTempVolumeEUR);		
		DEBUG_LOG("POPUP Volume gap = 0x%x)", gap);		

		if(gHpTempVolumeEUR > val)
			step = 1;
		else
			step = -1;
		
		for(i=0; i<gap; i++)
		{
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", (val+(step*(i+1))));

			// Left output volume
			val1 = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
			val1 &= ~(WM1811_HPOUT1L_VOL_MASK | WM1811_HPOUT1L_VU_MASK | WM1811_HPOUT1L_ZC_MASK);

			// Right output volume
			val2 = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
			val2&= ~(WM1811_HPOUT1R_VOL_MASK | WM1811_HPOUT1R_VU_MASK | WM1811_HPOUT1R_ZC_MASK);

			val1 |= (WM1811_HPOUT1L_VU | WM1811_HPOUT1L_ZC|(u16)(val+(step*(i+1))));
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", val1);
			wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val1);

			val2 |= (WM1811_HPOUT1R_VU | WM1811_HPOUT1R_ZC|(u16)(val+(step*(i+1))));
			//DEBUG_LOG("POPUP Volume Level = 0x%x)", val2);
			wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val2);	

			//step = (u16)((step + gHpTempVolumeEUR)/2);			

			msleep(0);
		}
	}
}


void WM1811_Set_Playback_RCV_Volume(struct snd_soc_codec *codec, WM1811_STATUS status)
{
	u16 val=0;

	DEBUG_LOG("Status=%d", status);

	if (STATUS_ON == status)
	{
		// RCV Volume
		val = wm1811_read_cache( WM1811_HPOUT2_VOLUME);
		val &= ~(WM1811_HPOUT2_VOL_MASK | WM1811_HPOUT2_MUTE_MASK);
		val |= gRcvVolume;
		wm1811_write(codec, WM1811_HPOUT2_VOLUME, val);
	}
	else	
	{
		// RCV Volume
		val = wm1811_read_cache( WM1811_HPOUT2_VOLUME);
		val &= ~(WM1811_HPOUT2_MUTE_MASK);
		val |= WM1811_HPOUT2_MUTE;
		wm1811_write(codec, WM1811_HPOUT2_VOLUME, val);
	}
}


void WM1811_Set_Handsfee_Digital_Volume(struct snd_soc_codec *codec,int handsfreeVolume)
{
	u16 val=0;

	DEBUG_LOG("set handsfree volume=%d\n",handsfreeVolume);

	if (0==handsfreeVolume) handsfreeVolume =1;
		
	if (-1==handsfreeVolume)   // restore digital volume 
	{
		// DAC Left Volume 
		val = wm1811_read_cache(WM1811_DAC1_LEFT_VOLUME);
		val &= ~(WM1811_DAC1L_MUTE_MASK | WM1811_DAC1_VU_MASK | WM1811_DAC1L_VOL_MASK);
		val |=  WM1811_DAC1_VU | WM1811_DAC1L_VOL_0dB;
		wm1811_write(codec, WM1811_DAC1_LEFT_VOLUME, val);

		// DAC Right Volume
		val = wm1811_read_cache(WM1811_DAC1_RIGHT_VOLUME);
		val &= ~(WM1811_DAC1R_MUTE_MASK | WM1811_DAC1_VU_MASK | WM1811_DAC1R_VOL_MASK);
		val |= WM1811_DAC1_VU | WM1811_DAC1R_VOL_0dB;
		wm1811_write(codec, WM1811_DAC1_RIGHT_VOLUME, val);	
	}else
	{

		/* use digital voluem for handsfree volume control */	
		val = wm1811_read_cache(WM1811_DAC1_LEFT_VOLUME);
		val &= ~(WM1811_DAC1L_MUTE_MASK | WM1811_DAC1_VU_MASK|WM1811_DAC1L_VOL_MASK);
		val |= WM1811_DAC1_VU|(0xE0-(15-handsfreeVolume)*6);  
		wm1811_write(codec, WM1811_DAC1_LEFT_VOLUME, val);

		/* use digital voluem for handsfree volume control */	
		val = wm1811_read_cache(WM1811_DAC1_RIGHT_VOLUME);
		val &= ~(WM1811_DAC1R_MUTE_MASK | WM1811_DAC1_VU_MASK|WM1811_DAC1R_VOL_MASK);
		val |= WM1811_DAC1_VU|(0xE0-(15-handsfreeVolume)*6);  
		wm1811_write(codec, WM1811_DAC1_RIGHT_VOLUME, val);
	}

	
}


void WM1811_Set_Playback_Dock_Volume(struct snd_soc_codec *codec, WM1811_STATUS status)
{
	u16 val=0;

	DEBUG_LOG("Status=%d", status);

	if (STATUS_ON == status)
	{
		// LINE_OUTPUTS_VOLUME
		val = wm1811_read_cache(WM1811_LINE_OUTPUTS_VOLUME);
		val &= ~(WM1811_LINEOUT1N_MUTE_MASK | WM1811_LINEOUT1P_MUTE_MASK|WM1811_LINEOUT1_VOL_MASK);
		wm1811_write(codec, WM1811_LINE_OUTPUTS_VOLUME, val);

		/*
		val = wm1811_read_cache( WM1811_LINE_OUTPUTS_VOLUME);
		val &= ~(WM1811_LINEOUT2N_MUTE_MASK | WM1811_LINEOUT2P_MUTE_MASK |
				WM1811_LINEOUT1P_MUTE_MASK | WM1811_LINEOUT1N_MUTE_MASK);
		val |= WM1811_LINEOUT2N_MUTE | WM1811_LINEOUT2P_MUTE;
		wm1811_write(codec, WM1811_LINE_OUTPUTS_VOLUME, val);
		*/
	}
	else		
	{
		// LINE_OUTPUTS_VOLUME
		val = wm1811_read_cache(WM1811_LINE_OUTPUTS_VOLUME);
		val &= ~(WM1811_LINEOUT1N_MUTE_MASK | WM1811_LINEOUT1P_MUTE_MASK|WM1811_LINEOUT1_VOL_MASK);
		val |= (WM1811_LINEOUT1N_MUTE | WM1811_LINEOUT1P_MUTE);
		wm1811_write(codec, WM1811_LINE_OUTPUTS_VOLUME, val);

		/*
		val = wm1811_read_cache( WM1811_LINE_OUTPUTS_VOLUME);
		val &= ~(WM1811_LINEOUT2N_MUTE_MASK | WM1811_LINEOUT2P_MUTE_MASK |
				WM1811_LINEOUT1P_MUTE_MASK | WM1811_LINEOUT1N_MUTE_MASK);
		val |= WM1811_LINEOUT2N_MUTE | WM1811_LINEOUT2P_MUTE |
				WM1811_LINEOUT1P_MUTE | WM1811_LINEOUT1N_MUTE;
		wm1811_write(codec, WM1811_LINE_OUTPUTS_VOLUME, val);
		*/
	}
}
#endif		// WM1811_PLAYBACK_ENABLE


#ifdef WM1811_FMRADIO_ENABLE
bool wm1811_Set_FM_Radio_Off(struct snd_soc_codec *codec)
{
	u16 val=0;

	DEBUG_LOG("wm1811_fm_device=%d", wm1811_fm_device);

	if (STATUS_OFF==wm1811_fm_device)	{
		DEBUG_LOG("return!!!");
		return 0;
	}

	wm1811_fm_device = STATUS_OFF;

	// Power management (2)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, WM1811_PM2_RESET);

	// Power management (4)
	val = wm1811_read_cache(WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);

	// Antipop (1)
	wm1811_write(codec, WM1811_ANTIPOP_1, WM1811_ANTIPOP_1_DEFAULT);

	// Lineout Volume
	wm1811_write(codec, WM1811_LINE_OUTPUTS_VOLUME, WM1811_LINEOUT_VOL_RESET);

	// Left line input 1&2 volume
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_1_2_VOLUME, WM1811_IN1L_RESET);

	// Left line input 3&4 volume
	wm1811_write(codec, WM1811_LEFT_LINE_INPUT_3_4_VOLUME, WM1811_IN2L_RESET);

	// Right line input 1&2 volume
	wm1811_write(codec, WM1811_RIGHT_LINE_INPUT_1_2_VOLUME, WM1811_INR1_RESET);

	// Input mixer (1)
	wm1811_write(codec, WM1811_INPUT_MIXER_1, WM1811_INMIX_RESET);

	// Input mixer (2)
	wm1811_write(codec, WM1811_INPUT_MIXER_2, WM1811_INMIX2_RESET);

	// Input mixer (3)
	wm1811_write(codec, WM1811_INPUT_MIXER_3, WM1811_IN2LMIX_RESET);

	// Input mixer (4)
	wm1811_write(codec, WM1811_INPUT_MIXER_4, WM1811_INMIX4_RESET);

	// Input mixer (5)
	wm1811_write(codec, WM1811_INPUT_MIXER_5, WM1811_INMIX5_RESET);

	// Input mixer (6)
	wm1811_write(codec, WM1811_INPUT_MIXER_6, WM1811_INMIX6_RESET);

	// DAC1 Mixer Volumes
	wm1811_write(codec, WM1811_DAC1_MIXER_VOLUMES, WM1811_DAC1_MIXER_VOL_DEFAULT);

	// Class W (1)
	val = wm1811_read_cache(WM1811_CLASS_W_1);
	val &= ~(WM1811_CLASS_W_RESERVED_MASK | WM1811_CP_DYN_PWR_MASK);
	val |= WM1811_CLASS_W_RESERVED | WM1811_CP_DYN_PWR;
	wm1811_write(codec, WM1811_CLASS_W_1, val);
}
EXPORT_SYMBOL(wm1811_Set_FM_Radio_Off);

void wm1811_Set_FM_Radio(struct snd_soc_codec *codec)
{
	u16 val = 0;
	
	DEBUG_LOG("");

	// Antipop (2)
	val = wm1811_read_cache( WM1811_ANTIPOP_2);
	val &= ~(WM1811_VMID_BUF_ENA_MASK);
	val |= (WM1811_VMID_BUF_ENA);
	wm1811_write(codec, WM1811_ANTIPOP_2, val);

	/* ---------------------IN1LP & IN1RP to ADC---------------------*/
	// Input mixer (1)
	val = wm1811_read_cache( WM1811_INPUT_MIXER_1);
	val &= ~(WM1811_IN1RP_MIXINL_BOOST_MASK | WM1811_IN1LP_MIXINL_BOOST_MASK);
	val |= 0x0180;
	wm1811_write(codec, WM1811_INPUT_MIXER_1, val);

	// Input mixer (5)
	val = wm1811_read_cache( WM1811_INPUT_MIXER_5);
	val &= ~(WM1811_IN1LP_MIXINL_VOL_MASK);
	val |= 0x0180;
	wm1811_write(codec, WM1811_INPUT_MIXER_5, val);

	// Input mixer (6)
	val = wm1811_read_cache( WM1811_INPUT_MIXER_6);
	val &= ~(WM1811_IN1RP_MIXINR_VOL_MASK);
	val |= 0x0180;
	wm1811_write(codec, WM1811_INPUT_MIXER_6, val);


	/* ---------------------ADC to DAC---------------------*/
	// DAC1 Mixer Volumes
	val = wm1811_read_cache(WM1811_DAC1_MIXER_VOLUMES);
	val &= ~(WM1811_ADCR_DAC1_VOL_MASK | WM1811_ADCL_DAC1_VOL_MASK);
	val |= 0xC << WM1811_ADCR_DAC1_VOL_SHIFT;
	val |= 0xC;
	wm1811_write(codec, WM1811_DAC1_MIXER_VOLUMES, val);

	// DAC1 Left Mixer Routing
	val = wm1811_read_cache( WM1811_DAC1_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_ADCL_TO_DACL_MASK | WM1811_AIF1DACL_TO_DACL_MASK);
	val |= WM1811_ADCL_TO_DACL | WM1811_AIF1DACL_TO_DACL;
	wm1811_write(codec, WM1811_DAC1_LEFT_MIXER_ROUTING, val);

	// DAC1 Right Mixer Routing
	val = wm1811_read_cache( WM1811_DAC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_ADCR_TO_DACR_MASK | WM1811_AIF1DACR_TO_DACR_MASK);
	val |= WM1811_ADCR_TO_DACR | WM1811_AIF1DACR_TO_DACR;
	wm1811_write(codec, WM1811_DAC1_RIGHT_MIXER_ROUTING, val);

	// AIF1 ADC1 Left Mixer Routing
	val = wm1811_read_cache( WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING);
	val &= ~(WM1811_ADC1L_TO_AIF1ADC1L_MASK);
	val |= WM1811_ADC1L_TO_AIF1ADC1L;
	wm1811_write(codec, WM1811_AIF1_ADC1_LEFT_MIXER_ROUTING, val);

	// AIF1 ADC1 Right Mixer Routing
	val = wm1811_read_cache( WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING);
	val &= ~(WM1811_ADC1R_TO_AIF1ADC1R_MASK);
	val |= WM1811_ADC1R_TO_AIF1ADC1R;
	wm1811_write(codec, WM1811_AIF1_ADC1_RIGHT_MIXER_ROUTING, val);

	// Class W (1)
	val = wm1811_read_cache(WM1811_CLASS_W_1);
	val &= ~(WM1811_CLASS_W_RESERVED_MASK | WM1811_CP_DYN_PWR_MASK);
	val |= WM1811_CLASS_W_RESERVED;
	wm1811_write(codec, WM1811_CLASS_W_1, val);

	// Power Management (2)
	val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_2);
	val &= ~(WM1811_PM2_ALL_MASK);
	val |= (WM1811_TSHUT_ENA | WM1811_TSHUT_OPDIS |
			WM1811_MIXINL_ENA | WM1811_MIXINR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, val);

	// Power Management (4)
	val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_4);
	val &= ~(WM1811_ADCL_ENA_MASK | WM1811_ADCR_ENA_MASK);
	val |= (WM1811_ADCL_ENA | WM1811_ADCR_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, val);

	WM1811_Set_ADC_To_DAC_Sidetone(codec, STATUS_ON);

	McDrv_Ctrl_fm(codec, uMusicStreamVol);

	//msleep(50);
}
EXPORT_SYMBOL(wm1811_Set_FM_Radio);

bool McDrv_Ctrl_fm(struct snd_soc_codec *codec, unsigned int volume)
{
#ifdef MID_LOCALE_VOLUME
	/* added by jinho.lim to make different volume as locale */
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);
#endif

	u16 val_l = 0;
	u16 val_r = 0;
	u16 val = 0;
	
	DEBUG_LOG("FM Radio, wm1811_radio_path : %d", wm1811_radio_path);
	DEBUG_LOG("1: speaker, 2: earphone");
	DEBUG_LOG("volume : %d", volume);

	if ( (uMusicStreamVol==volume) && (FM_RADIO==wm1811_mode) )
	{
		DEBUG_LOG("FM Volume returned");
		//return 0;
	}

	uMusicStreamVol = volume;

	if(volume == 0)
	{
		//FM Mute CTRL, sejong
		wm1811_Set_FM_Mute_Switch(codec, FM_MUTE_ON);
	}
	else if(volume == FM_SWITCH_MUTE)
	{
		//FM Mute CTRL, sejong
		wm1811_Set_FM_Mute_Switch(codec, FM_MUTE_ON);
		//reset to state when set FM playback.
		wm1811_fm_device = STATUS_OFF;
		//should not set the 0xFF to the register.
		return 0;
	}

	if (wm1811_radio_path == FM_SPK)
	{
		DEBUG_LOG("Speaker Path Volume");

		// prevent another path, sejong
		WM1811_Set_Playback_EAR_Off(codec);
		
		// Speaker Left Output Volume
		val_l = wm1811_read_cache( WM1811_SPEAKER_VOLUME_LEFT);
		val_l &= ~(WM1811_SPKOUTL_VU_MASK | WM1811_SPKOUTL_VOL_MASK);

		// Speaker Right Output Volume
		val_r = wm1811_read_cache( WM1811_SPEAKER_VOLUME_RIGHT);
		val_r &= ~(WM1811_SPKOUTR_VU_MASK | WM1811_SPKOUTR_VOL_MASK);

		val_l |= WM1811_SPKOUTL_VU | FM_SPK_VOL[volume];
		val_r |= WM1811_SPKOUTR_VU | FM_SPK_VOL[volume];

		wm1811_write(codec, WM1811_SPEAKER_VOLUME_LEFT, val_l);
		wm1811_write(codec, WM1811_SPEAKER_VOLUME_RIGHT, val_r);

		DEBUG_LOG("FM Radio Speaker Volume=(%d, %x)", volume, FM_SPK_VOL[volume]);
	}
	else if(wm1811_radio_path == FM_HP)
	{
		DEBUG_LOG("Earphone Path Volume");	

		// prevent another path, sejong
		//WM1811_Set_Playback_SPK_Off(codec);
		
		// Headset Left Output Volume
		val_l = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
		val_l &= ~(WM1811_HPOUT1L_VOL_MASK | WM1811_HPOUT1L_VU_MASK);

		// Headset Right Output Volume
		val_r = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
		val_r &= ~(WM1811_HPOUT1R_VOL_MASK | WM1811_HPOUT1R_VU_MASK);
		
		/* added by jinho.lim to make different volume as locale */
		#ifdef MID_LOCALE_VOLUME
		if(wm1811->locale == LOCALE_EUR)
		{
			val_l |= WM1811_HPOUT1L_VU | FM_HP_Vol_EUR[volume];
			val_r |= WM1811_HPOUT1R_VU | FM_HP_Vol_EUR[volume];
			DEBUG_LOG("[EUR]FM Radio Headset Volume=(%d, %x)", volume, FM_HP_Vol_EUR[volume]);
		}
		else
		{
			val_l |= WM1811_HPOUT1L_VU | FM_HP_Vol_WW[volume];
			val_r |= WM1811_HPOUT1R_VU | FM_HP_Vol_WW[volume];
			DEBUG_LOG("[WW]FM Radio Headset Volume=(%d, %x)", volume, FM_HP_Vol_WW[volume]);
		}		
		#endif

		wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val_l);
		wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val_r);
	}

	if(volume > 0)
	{
		DEBUG_LOG("volume >0, wm1811_fm_device=(%d)", wm1811_fm_device);
		//FM Mute CTRL, sejong
		if((wm1811_fm_device != STATUS_OFF))
			wm1811_Set_FM_Mute_Switch(codec, FM_MUTE_OFF);

		wm1811_radio_mute = FM_MUTE_OFF;
	}

	return 0;
}
EXPORT_SYMBOL(McDrv_Ctrl_fm);

void wm1811_Set_FM_Mute_Switch(struct snd_soc_codec *codec, FMRADIO_MUTE_STATUS isMute)
{
	unsigned int val = 0;

	DEBUG_LOG("wm1811_Set_FM_Mute_Switch, Mute : %d, wm1811_radio_path : %d",(int)isMute, wm1811_radio_path);

	if(isMute == FM_MUTE_ON)
	{
		if(wm1811_radio_path == FM_SPK)
		{
			DEBUG_LOG("!!!set FM SPK mute");
			val = wm1811_read_cache(WM1811_SPKOUT_MIXERS);
			val &= ~(WM1811_SPKMIXL_TO_SPKOUTL_MASK | WM1811_SPKMIXR_TO_SPKOUTL_MASK);
			wm1811_write(codec, WM1811_SPKOUT_MIXERS, val);
		}
		else//		if((wm1811_radio_path == FM_HP)||)
		{
			DEBUG_LOG("!!!set FM ear mute");
			//WM1811_Set_Path_EAR_Mute(codec, STATUS_ON);
			gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_ON);			
		}

	}
	else
	{
		if(wm1811_radio_path == FM_SPK)
		{
			val = wm1811_read_cache(WM1811_SPKOUT_MIXERS);
			if ((WM1811_SPKMIXL_TO_SPKOUTL_MASK & val)==WM1811_SPKMIXL_TO_SPKOUTL)	
			{
				DEBUG_LOG("Speaker already turn on");
			}
			else
			{
				//wm1811_Set_FM_Radio(codec);
				// Speaker Output Mixer
				DEBUG_LOG("!!!set FM SPK UNmute");
				val = wm1811_read_cache(WM1811_SPKOUT_MIXERS);
				val &= ~(WM1811_SPKMIXL_TO_SPKOUTL_MASK | WM1811_SPKMIXR_TO_SPKOUTL_MASK);
				val |= (WM1811_SPKMIXL_TO_SPKOUTL | WM1811_SPKMIXR_TO_SPKOUTL);
				wm1811_write(codec, WM1811_SPKOUT_MIXERS, val);			
			}
		}
		else //if(wm1811_radio_path == FM_HP)
		{
			if(gpio_get_value(MUTE_SWITCH) == MUTE_SWITCH_ON)
			{
				DEBUG_LOG("!!!set FM ear UNmute");
				//WM1811_Set_Path_EAR_Mute(codec, STATUS_OFF);
				gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_OFF);
			}
			else
			{
				DEBUG_LOG("HP already turn on");
			}
		}
		
	}
	return;
}

void wm1811_Set_FM_Mute_Switch_Flag(int isMute)
{
	DEBUG_LOG("wm1811_Set_FM_Mute_Switch_Flag : %d", isMute);
	wm1811_radio_mute = isMute;

	return;				
}
EXPORT_SYMBOL(wm1811_Set_FM_Mute_Switch_Flag);

#endif		// WM1811_FMRADIO_ENABLE


static int wm1811_get_mic_mute(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol,unsigned int codec_mode)
{
	//DEBUG_LOG_ERR("wm1811_get_mic_mute return is %d\n", wm1811_mic_mute_enable);
	return wm1811_mic_mute_enable;
}

static int wm1811_get_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int i =0;

	while(audio_path[i] !=NULL)
	{
		if(!strncmp(audio_path[i], kcontrol->id.name,sizeof(audio_path[i])))
		{
			switch(i)
			{
				case PLAY_BACK:
				case VOICE_CALL:
				case VOICE_MEMO:
				case VOIP_CALL:
				case FM_RADIO:
				case IDLE_MODE:
				case LOOP_BACK:
				case VT_CALL:
					return 0;
				case MIC_MUTE:
	                return wm1811_get_mic_mute(kcontrol, ucontrol, i);
				default:
					return 0;
			}
		}
		i++;
	}
	return 0;
}


static int wm1811_get_status(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    //P("wm1811_get_status codec_mode = %d !!!\n", wm1811_get_codec_mode());

	//DEBUG_LOG("ucontrol->value.integer.value[0]=%x", ucontrol->value.integer.value[0]);

	ucontrol->value.integer.value[0] = wm1811_get_codec_mode()+1;

	return 0;
}

static int wm1811_set_path(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	int i =0;
	int state = 0;
	
	DEBUG_LOG_ERR("\nwm1811_set_path");
	DEBUG_LOG_ERR("wm1811_set name=%s", kcontrol->id.name);

#ifdef TUNING_ANALOG
	if (STATUS_ON == isLoadSoundConfig)	{
	      	if (STATUS_OFF == ReadSoundConfigFile("/sdcard/soundcfg/analog.txt", 0))
           		isLoadSoundConfig = STATUS_ON;
	}
#endif

	while (audio_path[i] !=NULL)
	{
		if (!strncmp(audio_path[i], kcontrol->id.name, sizeof(audio_path[i])) )
		{	
			wm1811_old_mode = wm1811_mode;
			wm1811_mode = i;

			DEBUG_LOG_ERR("wm1811_set old_mode=%d cur_mode=%d", wm1811_old_mode, wm1811_mode);
			
			if (wm1811_old_mode != wm1811_mode)
			{
				DEBUG_LOG("wm1811_old_mode=%d", wm1811_old_mode);

				switch (wm1811_old_mode)
				{
					case PLAY_BACK:
						break;
						
					case VOICE_CALL:
						wm1811_call_device = 0;
						break;

					case VOICE_MEMO:
						if (wm1811_mode != PLAY_BACK)
							wm1811_record_off(codec);

						//in case of wrong VOIP setting, sejong.
						if(wm1811_mode  == VOIP_CALL)
							wm1811_voip_device = 0;
						
						break;

					case VOIP_CALL:
						wm1811_voip_device = 0;
						break;

					case FM_RADIO:
						wm1811_Set_FM_Radio_Off(codec);
						wm1811_fm_device = 0;
						break;

					case HF_MODE:
						/* restore digital volume for handsfree */
						if (wm1811_hf_device!=0)
						{
							WM1811_Set_Handsfee_Digital_Volume (codec,-1);
						}
						wm1811_hf_device = 0;
						wm1811_record_off(codec);
						break;
					default:
						break;
				}
			}

			switch(i)
			{				
				case PLAY_BACK:
					state =wm1811_set_playback_path(kcontrol, ucontrol,i);
					return 0;

				case VOICE_CALL:
					state = wm1811_set_voicecall_path(kcontrol, ucontrol,i);			
					return 0;

				case VOICE_MEMO:
					wm1811_set_voicememo_path(kcontrol, ucontrol, i);
					return 0;

				case VT_CALL:
					state = wm1811_set_voicecall_path(kcontrol, ucontrol,i);					
					return 0;

				case VOIP_CALL:
					wm1811_set_voipcall_path(kcontrol, ucontrol, i ,0 );
					return 0;

				case FM_RADIO:
					state = wm1811_set_fmradio_path(kcontrol, ucontrol, i);
					return 0;

				case IDLE_MODE:
					wm1811_set_idle_mode(kcontrol, ucontrol,i);
					return 0;

				case MIC_MUTE:
               			wm1811_set_mic_mute(kcontrol, ucontrol, i);
                			return 0;

				case HF_MODE:
	               		wm1811_set_handsfree_path(kcontrol, ucontrol, i);
                  			return 0;

				default:
					return 0;
			}
		}
		i++;
	}

	DEBUG_LOG("[LSW]wm1811_old_mode=%d, wm1811_old_mode=%d", wm1811_mode, wm1811_old_mode);
	
	return 0;
}

static int wm1811_get_dtmf_volume(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return 0;
}



static int wm1811_set_dtmf_volume(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
    //struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
    DEBUG_LOG_ERR("wm1811_set_dtmf_volume %ld ", ucontrol->value.integer.value[0]);

    return 0;
}

static int wm1811_get_dtmf_generator(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return 0;
}

static int wm1811_set_dtmf_generator(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	//struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	DEBUG_LOG_ERR("wm1811_set_dtmf_generator %ld ", ucontrol->value.integer.value[0]);

    
	return 0;
}

 static int wm1811_get_rec_8k_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return wm1811_rec_8k_enable;
}
static int wm1811_set_rec_8k_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	wm1811_rec_8k_enable = ucontrol->value.integer.value[0];
	return 0;
}

static int wm1811_get_fm_radio_mute_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return wm1811_fm_radio_mute_enable;
}
static int wm1811_set_fm_radio_mute_enable(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
//return 0;
//	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	DEBUG_LOG_ERR("wm1811_set_fm_radio_mute_enable %ld\n", ucontrol->value.integer.value[0]);

	wm1811_fm_radio_mute_enable = ucontrol->value.integer.value[0];

	if (wm1811_fm_radio_mute_enable)
		wm1811_fm_radio_mute_switch(STATUS_ON);
	else
		wm1811_fm_radio_mute_switch(STATUS_OFF);

	return 0;
}


int gHandsfreeVolume=0;
/* add by park dong yun for BT Handsfree Volume Control */
static int wm1811_get_handsfree_volumeIndex(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
      /* add handsfree volume control code here */
	return gHandsfreeVolume;
}
static int wm1811_set_handsfree_volumeIndex(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	
	DEBUG_LOG_ERR("wm1811_set_handsfree_volumeIndex %ld\n", ucontrol->value.integer.value[0]);
	gHandsfreeVolume= ucontrol->value.integer.value[0];
	WM1811_Set_Handsfee_Digital_Volume(codec,gHandsfreeVolume);
    
	return 0;
}
/* end of update by park dong yun */


/* add by park dong yun for Digital/Analog Mixing volume table  */
static int wm1811_get_music_volumeIndex(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
      /* add handsfree volume control code here */
	return 0;
}
static int wm1811_set_music_volumeIndex(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	int val1,val2 = 0;
//	DEBUG_LOG_ERR("wm1811_set_music_volumeIndex 1 %ld\n", ucontrol->value.integer.value[0]);
	uMusicStreamVol = ucontrol->value.integer.value[0]; 
		
	if ( (uMusicStreamVol==ucontrol->value.integer.value[0]) ||
		(STATUS_OFF==curMusicStreamOn) ||
		(FM_RADIO == wm1811_mode) )
		return 0;

	DEBUG_LOG_ERR("wm1811_set_music_volumeIndex %ld\n", ucontrol->value.integer.value[0]);

	//un-mute if re-control the volume from 0, sejong.
	if((uMusicStreamVol == 0) && (ucontrol->value.integer.value[0] != 0) && (wm1811_headset_status != HEADSET_DISCONNECT))
	{
		/* add mute code after fade out volume */
		DEBUG_LOG( "UN-mute after ear Volume Up------------------------");
		val1 = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
		val1 &= ~(WM1811_HPOUT1L_MUTE_N_MASK);
		val1 |= (WM1811_HPOUT1L_MUTE_N_MASK);
		wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val1);

		// Right output volume
		val2 = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
		val2&= ~(WM1811_HPOUT1R_MUTE_N_MASK);
		val2 |= (WM1811_HPOUT1R_MUTE_N_MASK);
		wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val2);	
	
	}

	uMusicStreamVol = ucontrol->value.integer.value[0];

	wm1811_set_mixing_volume_table(codec);

	return 0;
}
static int wm1811_get_music_stream_on(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
      /* add handsfree volume control code here */
	return (int)uIsMusicStreamOn;
}
static int wm1811_set_music_stream_on(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	
	DEBUG_LOG_ERR("wm1811_set_music_stream_on %ld\n", ucontrol->value.integer.value[0]);

	if (STATUS_ON == ucontrol->value.integer.value[0])	{
		uIsMusicStreamOn = STATUS_ON;
		curMusicStreamOn = STATUS_ON;

		//WM1811_Set_Playback_EAR_Volume(codec, STATUS_ON);
		WM1811_Set_Playback_EAR_Volume_Only(codec, STATUS_ON);
	}
	else		{
		uIsMusicStreamOn = STATUS_OFF;
		curMusicStreamOn = STATUS_OFF;
	}
    
	return 0;
}




static int wm1811_get_codec_status(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return 0;
}
static int wm1811_set_codec_status(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
#ifdef MID_LOCALE_VOLUME
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);
#endif	

      int cmd=ucontrol->value.integer.value[0];
	DEBUG_LOG_ERR("!!!!!!!!!!!!! wm1811_set_codec_status %d\n", cmd);

      switch(cmd)
      {
#ifdef MID_LOCALE_VOLUME			
	/* added by jinho.lim */
	case CMD_LOCALE_EUR:
			wm1811->locale = LOCALE_EUR;
			DEBUG_LOG("=======>    locale is @@@ EUR EUR EUR @@@\n");
			break;
#endif			
	default:
		break;
     	}
    
	return 0;
}

/* add by park dong yun for codec status udpate by park dong yun*/

#define VOIP_OFF 0 
#define VOIP_ON 1
 
static int wm1811_get_voip_status(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
      /* add handsfree volume control code here */
	return (int)voipStatus;
}


static int wm1811_set_voip_status(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	int cmd=ucontrol->value.integer.value[0];
	DEBUG_LOG("%d\n", cmd);
      switch(cmd)
      {
  	case RCV:
	case SPK:
	case HP3P:
	case HP4P:
			wm1811_set_voipcall_path(kcontrol,ucontrol,0,cmd);
			voipStatus=true;
 			break;
	case 0:
			// shut up all device 
			WM1811_Set_Playback_SPK_DRC(codec, STATUS_OFF);
			wm1811_mic_enable(codec, MIC_OFF, STATUS_OFF);
			WM1811_Set_Playback_SPK_Off(codec);
			WM1811_Set_Playback_RCV_Off(codec);

			wm1811_voip_earkey_check=STATUS_ON;
			voipStatus=false;
			break;
	default:
		break;
     	}
	voipStatus=cmd;
    
	return 0;
}

static int wm1811_get_mixing_volume_table()
{
	DEBUG_LOG ("Music stream HP, Vol=%x\n", HP_Vol_WW[uMusicStreamVol]);
	
	return HP_Vol_WW[uMusicStreamVol];
}
	
void wm1811_set_mixing_volume_table(struct snd_soc_codec *codec)
{
#ifdef MID_LOCALE_VOLUME
	/* added by jinho.lim to make different volume as locale */
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);
#endif
	u16 val_l = 0;
	u16 val_r = 0;
	
	if (STATUS_ON == uIsMusicStreamOn)	{
		// Headset Left Output Volume
		val_l = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
/*
		val_l &= ~(WM1811_HPOUT1L_VOL_MASK | WM1811_HPOUT1L_VU_MASK |
				WM1811_HPOUT1L_MUTE_N_MASK | WM1811_HPOUT1L_ZC_MASK);
*/
		val_l &= ~(WM1811_HPOUT1L_VOL_MASK | WM1811_HPOUT1L_VU_MASK |
				 WM1811_HPOUT1L_ZC_MASK);

		// Headset Right Output Volume
		val_r = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
/*
		val_r &= ~(WM1811_HPOUT1R_VOL_MASK | WM1811_HPOUT1R_VU_MASK |
				WM1811_HPOUT1R_MUTE_N_MASK | WM1811_HPOUT1R_ZC_MASK);
*/	
		val_r &= ~(WM1811_HPOUT1R_VOL_MASK | WM1811_HPOUT1R_VU_MASK |
				 WM1811_HPOUT1R_ZC_MASK);
		
		/* added by jinho.lim to make different volume as locale */
		#ifdef MID_LOCALE_VOLUME		
		if(wm1811->locale == LOCALE_EUR)
		{
		/*
			val_l |= WM1811_HPOUT1L_VU | WM1811_HPOUT1L_MUTE_N |
					WM1811_HPOUT1L_ZC | HP_Vol_EUR[uMusicStreamVol];
			val_r |= WM1811_HPOUT1R_VU | WM1811_HPOUT1R_MUTE_N | 
					WM1811_HPOUT1R_ZC | HP_Vol_EUR[uMusicStreamVol];
		*/
			val_l |= WM1811_HPOUT1L_VU | 
					WM1811_HPOUT1L_ZC | HP_Vol_EUR[uMusicStreamVol];
			val_r |= WM1811_HPOUT1R_VU |
					WM1811_HPOUT1R_ZC | HP_Vol_EUR[uMusicStreamVol];
		
			DEBUG_LOG("[EUR] Headset Volume=(%d, %x)", uMusicStreamVol, HP_Vol_EUR[uMusicStreamVol]);
		}
		else
		{
		/*
			val_l |= WM1811_HPOUT1L_VU | WM1811_HPOUT1L_MUTE_N | 
					WM1811_HPOUT1L_ZC | HP_Vol_WW[uMusicStreamVol];
			val_r |= WM1811_HPOUT1R_VU | WM1811_HPOUT1R_MUTE_N | 
					WM1811_HPOUT1R_ZC | HP_Vol_WW[uMusicStreamVol];
		*/					
			val_l |= WM1811_HPOUT1L_VU | 
					WM1811_HPOUT1L_ZC | HP_Vol_WW[uMusicStreamVol];
			val_r |= WM1811_HPOUT1R_VU |
					WM1811_HPOUT1R_ZC | HP_Vol_WW[uMusicStreamVol];

			DEBUG_LOG("[WW] Headset Volume=(%d, %x)", uMusicStreamVol, HP_Vol_WW[uMusicStreamVol]);
		}		
		#endif

		wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val_l);
		wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val_r);
	}
	else		{
		// Left Output Volume : Volue Control
		val_l = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
		val_l &= ~(WM1811_HPOUT1L_VOL_MASK | WM1811_HPOUT1L_VU_MASK);
		val_l |= WM1811_HPOUT1L_VU;

		/* added by jinho.lim to make different volume as locale */
		#ifdef MID_LOCALE_VOLUME
		if(wm1811->locale == LOCALE_EUR)
		{
			val_l |= gHpVolumeEUR;
			DEBUG_LOG("[EUR]Headset Volume(not Music)=%x", gHpVolumeEUR);
		}
		else
		{
			val_l |= gHpVolumeWW;
			DEBUG_LOG("[WW]Headset Volume(not Music)=%x", gHpVolumeWW);
		}		
		#endif
		wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val_l);

		// Right Output Volume : Volue Control
		val_r = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
		val_r &= ~(WM1811_HPOUT1R_VOL_MASK | WM1811_HPOUT1R_VU_MASK);
		val_r |= WM1811_HPOUT1R_VU;

		/* added by jinho.lim to make different volume as locale */
		#ifdef MID_LOCALE_VOLUME
		if(wm1811->locale == LOCALE_EUR)
		{
			val_r |= gHpVolumeEUR;
			DEBUG_LOG("[EUR]Headset Volume(not Music)=%x", gHpVolumeEUR);
		}
		else
		{
			val_r |= gHpVolumeWW;
			DEBUG_LOG("[WW]Headset Volume(not Music)=%x", gHpVolumeWW);
		}		
		#endif
		wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val_r);
	}

	#ifdef WM1811_USE_LINEOUT
		// MIX OUTPUT LEFT
		val_l = wm1811_read_cache(WM1811_LEFT_OPGA_VOLUME);
		val_l &= ~(WM1811_MIXOUT_VU_MASK | WM1811_MIXOUTL_ZC_MASK 
			|WM1811_MIXOUTL_MUTE_N_MASK|WM1811_MIXOUTL_VOL_MASK);
		val_l |= (WM1811_MIXOUT_VU | WM1811_MIXOUTL_ZC |WM1811_MIXOUTL_MUTE_N|LINEOUT_VOL[uMusicStreamVol]);
		wm1811_write(codec, WM1811_LEFT_OPGA_VOLUME, val_l);

		// MIX OUTPUT RIGHT
		val_r = wm1811_read_cache(WM1811_RIGHT_OPGA_VOLUME);
		val_r &= ~(WM1811_MIXOUT_VU_MASK | WM1811_MIXOUTR_ZC_MASK 
			|WM1811_MIXOUTR_MUTE_N_MASK|WM1811_MIXOUTR_VOL_MASK);
		val_r |= (WM1811_MIXOUT_VU | WM1811_MIXOUTR_ZC |WM1811_MIXOUTR_MUTE_N|LINEOUT_VOL[uMusicStreamVol]);
		wm1811_write(codec, WM1811_RIGHT_OPGA_VOLUME, val_r);
	#endif	
}
EXPORT_SYMBOL(wm1811_set_mixing_volume_table);
/* end of update by park dong yun */


static const struct soc_enum path_control_enum[]=
{
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(playback_path),playback_path),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(voicecall_path),voicecall_path),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(voicememo_path),voicememo_path),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(voip_path),voip_path),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(fmradio_path),fmradio_path),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(idle_mode),idle_mode),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(mic_mute),mic_mute), // hskwon-ss-db05, to support mic mute/unmute for CTS test
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(loopback_path),loopback_path),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(bt_dtmf_enum),bt_dtmf_enum),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(vr_mode),vr_mode),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(handsfree_path),handsfree_path),
	/* add by park dong yun for hansfree */
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(hands_volume_control),hands_volume_control),  

	/* Add by park dong yun for Digital Analog Mixing Volume table  */
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(music_index_volume),music_index_volume),  
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(music_stream_on_off),music_stream_on_off) ,
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(codec_status),codec_status),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(voip_status),voip_status)
};


static const struct snd_kcontrol_new wm1811_snd_controls[] = {

	/* normal play setting */
	SOC_ENUM_EXT("Playback Path", path_control_enum[0],
                wm1811_get_path,wm1811_set_path),
	/* voice call setting */
	SOC_ENUM_EXT("Voice Call Path", path_control_enum[1],
			wm1811_get_path,wm1811_set_path),

	/* voice memol setting */
	SOC_ENUM_EXT("Memo Path", path_control_enum[2],
			wm1811_get_path,wm1811_set_path),

	/* VOIP call setting */
	SOC_ENUM_EXT("VT Call Path", path_control_enum[1],
					wm1811_get_path, wm1811_set_path),
	SOC_ENUM_EXT("VOIP Call Path", path_control_enum[3],
			wm1811_get_path,wm1811_set_path),

	/* FM Radio Path setting */
	SOC_ENUM_EXT("FM Radio Path", path_control_enum[4],
					wm1811_get_path,wm1811_set_path),
	/* Idle Mode setting */
	SOC_ENUM_EXT("Idle Mode", path_control_enum[5],
			wm1811_get_status,wm1811_set_path),

	/* Mic Mute setting, hskwon-ss-db05, to support mic mute/unmute for CTS test */
	SOC_ENUM_EXT("Mic Mute", path_control_enum[6],
			wm1811_get_path,wm1811_set_path),

	SOC_ENUM_EXT("VR Mode", path_control_enum[9],
					wm1811_get_vr_mode,wm1811_set_vr_mode),

	SOC_ENUM_EXT("BT DTMF Volume", path_control_enum[8],
					wm1811_get_dtmf_volume, wm1811_set_dtmf_volume),

	SOC_ENUM_EXT("BT DTMF Generator", path_control_enum[8],
					wm1811_get_dtmf_generator, wm1811_set_dtmf_generator),

#if 0
	SOC_ENUM_EXT("Loopback Path", path_control_enum[7],
					wm1811_get_path, wm1811_set_loopback_path),
#endif

	SOC_ENUM_EXT("Rec 8K Enable", path_control_enum[6],
					wm1811_get_rec_8k_enable, wm1811_set_rec_8k_enable),

	SOC_ENUM_EXT("FM Radio Mute Enable", path_control_enum[6],
					wm1811_get_fm_radio_mute_enable, wm1811_set_fm_radio_mute_enable),
					
	/* Add by park dong yun for Handsfree path control */
	SOC_ENUM_EXT("Handsfree Path", path_control_enum[10],
					wm1811_get_path, wm1811_set_path),
	/* Add by park dong yun for Handsfree volume control */
      SOC_ENUM_EXT("BT HF Volume", path_control_enum[11],
					wm1811_get_handsfree_volumeIndex, wm1811_set_handsfree_volumeIndex),							

	/*Add by park dong yun for Digital Analog volume Mixing */
      SOC_ENUM_EXT("music_index_volume", path_control_enum[12],
					wm1811_get_music_volumeIndex, wm1811_set_music_volumeIndex),							
      SOC_ENUM_EXT("music_steam_on", path_control_enum[13],
					wm1811_get_music_stream_on, wm1811_set_music_stream_on),
	SOC_ENUM_EXT("Codec Status", path_control_enum[14],
                        wm1811_get_codec_status, wm1811_set_codec_status),
     	SOC_ENUM_EXT("Voip Status", path_control_enum[15],
                        wm1811_get_voip_status, wm1811_set_voip_status),

};

/* add non dapm controls */
static int wm1811_add_controls(struct snd_soc_codec *codec)
{
	int err, i;

	for (i = 0; i < ARRAY_SIZE(wm1811_snd_controls); i++) {
		DEBUG_LOG_ERR("add control");

		err = snd_ctl_add(codec->card,
			snd_soc_cnew(&wm1811_snd_controls[i], codec, NULL));

		if (err < 0)

		return err;
	}

	return 0;
}

static const struct snd_soc_dapm_widget wm1811_dapm_widgets[] = {
};

static const struct snd_soc_dapm_route intercon[] = {
};

static int wm1811_add_widgets(struct snd_soc_codec *codec)
{
	DEBUG_LOG("");
	snd_soc_dapm_new_controls(codec->dapm, wm1811_dapm_widgets,
				 ARRAY_SIZE(wm1811_dapm_widgets));

	snd_soc_dapm_add_routes(codec->dapm, intercon, ARRAY_SIZE(intercon));

	return 0;
}


void wm1811_set_ear_state(WM1811_HEADSET_STATUS headset_status)
{
	wm1811_headset_status = headset_status;
}
EXPORT_SYMBOL_GPL(wm1811_set_ear_state);


u16 wm1811_get_ear_state(void)
{
	return wm1811_headset_status;
}


static int wm1811_set_bias_level(struct snd_soc_codec *codec,
				  enum snd_soc_bias_level level)
{
	switch (level) {
		
		case SND_SOC_BIAS_ON:
			break;
			
		case SND_SOC_BIAS_PREPARE:
			break;
			
		case SND_SOC_BIAS_STANDBY:
			break;
		case SND_SOC_BIAS_OFF:
			WM1811_Set_Playback_EAR_Volume(codec, STATUS_ON);

			wm1811_main_mic_bias_regulator_mode_on_off(codec, STATUS_OFF);
			wm1811_ear_mic_bias_regulator_mode_on_off(codec, STATUS_OFF);
			wm1811_regulator_mode_hidden_register_on_off(codec, STATUS_OFF);
			wm1811_subMIC_on_off(STATUS_OFF);

			//WM1811_Power_Off(codec);

			wm1811_reset_registers(codec);
			wm1811_codec_enable(STATUS_OFF);
			break;
	}
	codec->dapm->bias_level = level;

	return 0;
}

static void wm1811_constraints(struct wm1811_priv *wm1811,
				struct snd_pcm_substream *mst_substream)
{
	struct snd_pcm_substream *slv_substream;
	
	DEBUG_LOG("");

	/* Pick the stream, which need to be constrained */
	if (mst_substream == wm1811->master_substream)
		slv_substream = wm1811->slave_substream;
	else if (mst_substream == wm1811->slave_substream)
		slv_substream = wm1811->master_substream;
	else /* This should not happen.. */
		return;

	/* Set the constraints according to the already configured stream */
	snd_pcm_hw_constraint_minmax(slv_substream->runtime,
				SNDRV_PCM_HW_PARAM_RATE,
				wm1811->rate,
				wm1811->rate);

	snd_pcm_hw_constraint_minmax(slv_substream->runtime,
				SNDRV_PCM_HW_PARAM_SAMPLE_BITS,
				wm1811->sample_bits,
				wm1811->sample_bits);

	snd_pcm_hw_constraint_minmax(slv_substream->runtime,
				SNDRV_PCM_HW_PARAM_CHANNELS,
				wm1811->channels,
				wm1811->channels);
}

extern void omap_dpll3_errat_wa(int disable);

static int wm1811_startup(struct snd_pcm_substream *substream,
			   struct snd_soc_dai *codec_dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);
	
//	u16 gVal=0;
	
	DEBUG_LOG("");

	if (wm1811->master_substream) {
		wm1811->slave_substream = substream;
		/* The DAI has one configuration for playback and capture, so
		 * if the DAI has been already configured then constrain this
		 * substream to match it. */
		if (wm1811->configured)
			wm1811_constraints(wm1811, wm1811->master_substream);
	} else {
//		if (!(wm1811_read_cache( WM1811_REG_CODEC_MODE) &
	//		WM1811_OPTION_1)) {
			/* In option2 4 channel is not supported, set the
			 * constraint for the first stream for channels, the
			 * second stream will 'inherit' this cosntraint */
		//	snd_pcm_hw_constraint_minmax(substream->runtime,
			//			SNDRV_PCM_HW_PARAM_CHANNELS,
				//		2, 2);
	//	}
		wm1811->master_substream = substream;
	}

	omap_pm_set_max_mpu_wakeup_lat(&substream->latency_pm_qos_req, 18);     
	omap_dpll3_errat_wa(0); 

	DEBUG_LOG("");
	
	return 0;
}

static void wm1811_shutdown(struct snd_pcm_substream *substream,
			     struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);

	DEBUG_LOG("");

	if (wm1811->master_substream == substream)
		wm1811->master_substream = wm1811->slave_substream;

	wm1811->slave_substream = NULL;

#if 0
	/* If all streams are closed, or the remaining stream has not yet
	 * been configured than set the DAI as not configured. */
	if (!wm1811->master_substream)
		wm1811->configured = 0;
	 else if (!wm1811->master_substream->runtime->channels)
		wm1811->configured = 0;

	 /* If the closing substream had 4 channel, do the necessary cleanup */
	if (substream->runtime->channels == 4)
	{
        // do nothing for tdm
        }
#endif

	omap_pm_set_max_mpu_wakeup_lat(&substream->latency_pm_qos_req, -1);     
	omap_dpll3_errat_wa(1);   
}

static int wm1811_hw_params(struct snd_pcm_substream *substream,
			   struct snd_pcm_hw_params *params,
			   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);

	DEBUG_LOG("");

	if (wm1811->configured)	/* Ignoring hw_params for already configured DAI */
		return 0;

	/* Store the important parameters for the DAI configuration and set
	 * the DAI as configured */
//	wm1811->configured = 1;
	wm1811->rate = params_rate(params);
	wm1811->sample_bits = hw_param_interval(params,
					SNDRV_PCM_HW_PARAM_SAMPLE_BITS)->min;
	wm1811->channels = params_channels(params);

	/* If both playback and capture streams are open, and one of them
	 * is setting the hw parameters right now (since we are here), set
	 * constraints to the other stream to match the current one. */
	if (wm1811->slave_substream)
		wm1811_constraints(wm1811, substream);

#ifdef GPS_TEST_FW
	wm1811_codec_enable(STATUS_OFF);
#else

	gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_ON);

	wm1811_codec_init(codec);
	wm1811_codec_clocking(codec);
	WM1811_Set_Playback_Init(codec);

	WM1811_Set_Playback_From_Headset_Status(codec, STATUS_ON);

	gpio_set_value(MUTE_SWITCH, MUTE_SWITCH_ON);


	DEBUG_LOG("\n");
#endif
	wm1811->configured = 1;
	return 0;
}

static int wm1811_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{	
	struct snd_soc_codec *codec = codec_dai->codec;
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);

	DEBUG_LOG("clk_id=%d, freq=%d, dir=%d\n", clk_id, freq, dir);

	return 0;
}

static int wm1811_set_dai_fmt(struct snd_soc_dai *codec_dai,
			     unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct wm1811_priv *wm1811 = snd_soc_codec_get_drvdata(codec);

	DEBUG_LOG("fmt=%d", fmt);

	return 0;
}

static int wm1811_set_tristate(struct snd_soc_dai *dai, int tristate)
{
	struct snd_soc_codec *codec = dai->codec;

	DEBUG_LOG("");

	return 0;
}

#ifdef MID_CUSTOM
void WM1811_Power_Off(struct snd_soc_codec *codec)
{
	u16 val=0;

	DEBUG_LOG("");

	// Power management (1)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, 0x0000);

	// Power management (2)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_2, 0x0000);

	// Power management (3)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_3, 0x0000);

	// Power management (4)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_4, 0x0000);

	// Power management (5)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_5, 0x0000);

	// Power management (6)
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_6, 0x0000);
}


void wm1811_codec_init(struct snd_soc_codec *codec)
{
	u16 val=0;

	DEBUG_LOG("");

	wm1811_codec_probe_enable = STATUS_ON;
	
	// Antipop (2)
	val = wm1811_read_cache( WM1811_ANTIPOP_2);
	val &= ~(WM1811_RESERVED_1_MASK | WM1811_RESERVED_2_MASK |
			WM1811_VMID_RAMP_MASK | WM1811_STARTUP_BIAS_ENA_MASK |
			WM1811_VMID_BUF_ENA_MASK);
	val |= (WM1811_RESERVED_1 | WM1811_RESERVED_2 |
			WM1811_VMID_RAMP_SOFT_FAST | WM1811_STARTUP_BIAS_ENA |
			WM1811_VMID_BUF_ENA);
	wm1811_write(codec, WM1811_ANTIPOP_2, val);

	// Power management (1)
	val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
	val &= ~(WM1811_VMID_SEL_MASK | WM1811_BIAS_ENA_MASK);
	val |= (WM1811_VMID_SEL_2X40K | WM1811_BIAS_ENA);
	wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
	msleep( 50 );
}
EXPORT_SYMBOL(wm1811_codec_init);

struct snd_soc_codec *wm1811_get_snd_soc_codec()
{
	return gCodec;
}
EXPORT_SYMBOL(wm1811_get_snd_soc_codec);
	
void wm1811_fm_radio_mute_switch(u16 mute_enable)
{
	//void (*fm_radio_switch)(void);
	unsigned int val = 0;

	struct snd_soc_codec *codec = wm1811_get_snd_soc_codec();
	
	DEBUG_LOG("");
	
	if (mute_enable)	
	{
		// Input mixer (1)
		wm1811_write(codec, WM1811_INPUT_MIXER_1, WM1811_INMIX_RESET);

		// Input mixer (2)
		wm1811_write(codec, WM1811_INPUT_MIXER_2, WM1811_INMIX2_RESET);

		// Input mixer (3)
		wm1811_write(codec, WM1811_INPUT_MIXER_3, WM1811_IN2LMIX_RESET);

		// Input mixer (4)
		wm1811_write(codec, WM1811_INPUT_MIXER_4, WM1811_INMIX4_RESET);

		// Input mixer (5)
		wm1811_write(codec, WM1811_INPUT_MIXER_5, WM1811_INMIX5_RESET);

		// Input mixer (6)
		wm1811_write(codec, WM1811_INPUT_MIXER_6, WM1811_INMIX6_RESET);		/*
		fm_radio_switch = symbol_get(Si4709_dev_MUTE_ON);
		if (fm_radio_switch) {
			fm_radio_switch();
			symbol_put(Si4709_dev_MUTE_ON);
		}
		*/
	}
	else		
	{
		/* ---------------------IN1LP & IN1RP to ADC---------------------*/
		// Input mixer (1)
		val = wm1811_read_cache( WM1811_INPUT_MIXER_1);
		val &= ~(WM1811_IN1RP_MIXINL_BOOST_MASK | WM1811_IN1LP_MIXINL_BOOST_MASK);
		val |= 0x0180;
		wm1811_write(codec, WM1811_INPUT_MIXER_1, val);

		// Input mixer (5)
		val = wm1811_read_cache( WM1811_INPUT_MIXER_5);
		val &= ~(WM1811_IN1LP_MIXINL_VOL_MASK);
		val |= 0x0180;
		wm1811_write(codec, WM1811_INPUT_MIXER_5, val);

		// Input mixer (6)
		val = wm1811_read_cache( WM1811_INPUT_MIXER_6);
		val &= ~(WM1811_IN1RP_MIXINR_VOL_MASK);
		val |= 0x0180;
		wm1811_write(codec, WM1811_INPUT_MIXER_6, val);
		/*
		fm_radio_switch = symbol_get(Si4709_dev_MUTE_OFF);
		if (fm_radio_switch) {
			fm_radio_switch();
			symbol_put(Si4709_dev_MUTE_OFF);
		}
		*/
	}
}


void wm1811_main_mic_bias_regulator_mode_on_off(struct snd_soc_codec *codec, u16 on_off)
{
	if (STATUS_ON == on_off)		{
		wm1811_write(codec, 0x03E, 0x002F);
		wm1811_write(codec, 0x03D, 0x0039);
	}
	else	
		wm1811_write(codec, 0x03E, 0x0039);
}

void wm1811_ear_mic_bias_regulator_mode_on_off(struct snd_soc_codec *codec, u16 on_off)
{
	if (STATUS_ON == on_off)		{
		wm1811_write(codec, 0x03D, 0x002F);
		wm1811_write(codec, 0x03E, 0x0039);
	}
	else	
		wm1811_write(codec, 0x03D, 0x0039);
}

void wm1811_regulator_mode_hidden_register_on_off(struct snd_soc_codec *codec, u16 on_off)
{
	if (STATUS_ON == on_off)		{
		wm1811_write(codec, 0x102, 0x0003);
		wm1811_write(codec, 0x0CB, 0x5121);
		wm1811_write(codec, 0x0D3, 0x3F3F);
		wm1811_write(codec, 0x0D4, 0x3F3F);
		wm1811_write(codec, 0x0D5, 0x3F3F);
		wm1811_write(codec, 0x0D6, 0x3226);
		wm1811_write(codec, 0x102, 0x0000);
	}
	else		{
		wm1811_write(codec, 0x102, 0x0003);
		wm1811_write(codec, 0x0CB, 0x3920);
		wm1811_write(codec, 0x0D3, 0x3636);
		wm1811_write(codec, 0x0D4, 0x2B22);
		wm1811_write(codec, 0x0D5, 0x150D);
		wm1811_write(codec, 0x0D6, 0x0702);
		wm1811_write(codec, 0x102, 0x0000);
	}
}
//defence for ear key, sejong
extern void ear_key_disable_irq(void);
extern void ear_key_enable_irq(void);
void wm1811_mic_bias_on_off(struct snd_soc_codec *codec, WM1811_MIC_BIAS select)
{
	u16 val=0;

	DEBUG_LOG("MIC Bias=%d", select);
	
	switch (select)
	{
		case ALL_MIC_OFF:
			// Regulator mode : Main(OFF), Ear(OFF)
			wm1811_main_mic_bias_regulator_mode_on_off(codec, STATUS_OFF);
			wm1811_ear_mic_bias_regulator_mode_on_off(codec, STATUS_OFF);
			wm1811_regulator_mode_hidden_register_on_off(codec, STATUS_OFF);

			// Sub MIC Bias : OFF
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x33);

			//ear_key_disable_irq();
			// MIC Bias : Main(OFF), Ear(OFF)
			val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
			//val &= ~(WM1811_MICB2_ENA_MASK | WM1811_MICB1_ENA_MASK);
			val &= ~(WM1811_MICB2_ENA_MASK);
			wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
			break;

		case MAIN_MIC_ON:
			// Regulator mode : Main(ON)
			wm1811_main_mic_bias_regulator_mode_on_off(codec, STATUS_ON);
			wm1811_regulator_mode_hidden_register_on_off(codec, STATUS_ON);

			// Sub MIC Bias : OFF
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x33);

			//ear_key_disable_irq();
			// MIC Bias : Main(ON), Ear(OFF)
			val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
			//val &= ~(WM1811_MICB2_ENA_MASK | WM1811_MICB1_ENA_MASK);
			val &= ~(WM1811_MICB2_ENA_MASK);
			val |= WM1811_MICB2_ENA;
			wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
			break;

		case MAIN_MIC_OFF:
			// Regulator mode : Main(OFF)
			wm1811_main_mic_bias_regulator_mode_on_off(codec, STATUS_OFF);
			wm1811_regulator_mode_hidden_register_on_off(codec, STATUS_OFF);

			// MIC Bias : Main(OFF)
			val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
			val &= ~(WM1811_MICB2_ENA_MASK);
			wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
			break;

		case HP_MIC_ON:
			// Regulator mode : Ear(ON)
			wm1811_ear_mic_bias_regulator_mode_on_off(codec, STATUS_ON);
			wm1811_regulator_mode_hidden_register_on_off(codec, STATUS_ON);

			// Sub MIC Bias : OFF
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x33);

			// MIC Bias : Main(OFF), Ear(ON)
			val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
			val &= ~(WM1811_MICB2_ENA_MASK | WM1811_MICB1_ENA_MASK);
			val |= WM1811_MICB1_ENA;
			wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
			ear_key_enable_irq();
			break;

		case HP_MIC_OFF:
			// Regulator mode : Ear(OFF)
			wm1811_ear_mic_bias_regulator_mode_on_off(codec, STATUS_OFF);
			wm1811_regulator_mode_hidden_register_on_off(codec, STATUS_OFF);

			//ear_key_disable_irq();
			// MIC Bias : Ear(OFF)
			/*
			val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
			val &= ~(WM1811_MICB1_ENA_MASK);
			wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
			*/
			break;

		case SUB_MIC_ON:
			// Regulator mode : Main(OFF), Ear(OFF)
			wm1811_main_mic_bias_regulator_mode_on_off(codec, STATUS_OFF);
			wm1811_ear_mic_bias_regulator_mode_on_off(codec, STATUS_OFF);
			wm1811_regulator_mode_hidden_register_on_off(codec, STATUS_OFF);

			// Sub MIC Bias : ON
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x35);
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x9, 0x36);
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x20, 0x33);

			//ear_key_disable_irq();			
			// MIC Bias : Main(OFF), Ear(OFF)
			val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
			//val &= ~(WM1811_MICB2_ENA_MASK | WM1811_MICB1_ENA_MASK);
			val &= ~(WM1811_MICB2_ENA_MASK);
			wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
			break;

		case SUB_MIC_OFF:
			twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x33);
			break;
	}
}


void wm1811_subMIC_on_off(u16 on)
{
	if (STATUS_ON == on)		{	// ON
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x35);
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x9, 0x36);
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x20, 0x33);
	}
	else					// OFF
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x00, 0x33);
}


void wm1811_ear_micbias_on_off(struct snd_soc_codec *codec, u16 on)
{
	u16 val=0;

	DEBUG_LOG("ear_micbias=%d\n", on);

	if (STATUS_ON == on)	{
		// Regulator mode : Ear(ON)
		wm1811_ear_mic_bias_regulator_mode_on_off(codec, STATUS_ON);
		wm1811_regulator_mode_hidden_register_on_off(codec, STATUS_ON);

		// MIC Bias : Main(OFF), Ear(ON)
		val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
		val &= ~(WM1811_MICB1_ENA_MASK | WM1811_MICB2_ENA_MASK);
		val |= WM1811_MICB1_ENA;
		wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
	}
	else		{
		ear_key_disable_irq();
		// Regulator mode : Ear(OFF)
		wm1811_ear_mic_bias_regulator_mode_on_off(codec, STATUS_OFF);
		wm1811_regulator_mode_hidden_register_on_off(codec, STATUS_OFF);

		// MIC Bias : Main(OFF), Ear(OFF)
		val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
		val &= ~(WM1811_MICB1_ENA_MASK);
		wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
	}
}
EXPORT_SYMBOL(wm1811_ear_micbias_on_off);


void wm1811_ear_micbias_set(struct snd_soc_codec *codec, u16 on)
{
	u16 val=0;

	DEBUG_LOG("ear_micbias=%d\n", on);

	if (STATUS_ON == on)	{
		val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
		val &= ~(WM1811_MICB1_ENA_MASK);
		val |= WM1811_MICB1_ENA;
		wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
	}
	else		{
		ear_key_disable_irq();

		val = wm1811_read_cache( WM1811_POWER_MANAGEMENT_1);
		val &= ~(WM1811_MICB1_ENA_MASK);
		wm1811_write(codec, WM1811_POWER_MANAGEMENT_1, val);
	}
}
EXPORT_SYMBOL(wm1811_ear_micbias_set);


void WM1811_Set_Playback_From_Headset_Status(struct snd_soc_codec *codec, WM1811_STATUS status)
{
	DEBUG_LOG("wm1811_headset_status=%d", wm1811_headset_status);
	//it does wrong setting for FM, sejong
	if(wm1811_radio_path != FM_OFF)
	{
		DEBUG_LOG("WM1811_Set_Playback_From_Headset_Status RETURNED!!");
		return 0;
	}
	
	switch (wm1811_headset_status)	{
		case HEADSET_3POLE:
			if (STATUS_ON==status)	{
				//WM1811_Set_Playback_EAR(codec);		// it doesn't need on VR mode
				wm1811_playback_device = HP3P;
			}
			else		{
				//WM1811_Set_Playback_EAR_Off(codec);		// it doesn't need on VR mode
				wm1811_playback_device = SPK;
			}
			break;

		case HEADSET_4POLE_WITH_MIC:
			if (STATUS_ON==status)	{
				wm1811_ear_micbias_on_off(codec, STATUS_ON);
				//WM1811_Set_Playback_EAR(codec);		// it doesn't need on VR mode
				wm1811_playback_device = HP4P;
			}
			else		{
				wm1811_ear_micbias_on_off(codec, STATUS_OFF);
				//WM1811_Set_Playback_EAR_Off(codec);		// it doesn't need on VR mode
				wm1811_playback_device = SPK;
			}
			break;

		case HEADSET_DISCONNECT:
			if (STATUS_ON==status)	{
				//WM1811_Set_Playback_SPK(codec);		 // it doesn't need on VR mode
				wm1811_playback_device = SPK;
			}
			else		{
				//WM1811_Set_Playback_SPK_Off(codec);		 // it doesn't need on VR mode
				wm1811_playback_device = SPK;
			}		
			break;

		default:
			break;
	}
}


u16 wm1811_is_headset()
{
	if (HEADSET_DETECTED == gpio_get_value(EAR_DET_GPIO))
		return HEADSET_CONNECT;
	else	
		return HEADSET_DISCONNECT;
}

void WM1811_Switch_To_Earphone(struct snd_soc_codec *codec, u16  status)
{
	u16 val1, val2 = 0;

#if 0
	if((wm1811_radio_path == FM_OFF)&&(wm1811_vr_mode == STATUS_OFF))
	{
		if (STATUS_ON == status)	
		{
			DEBUG_LOG("Switch : Ear");
			//WM1811_Set_Playback_SPK_Off(codec);
			//WM1811_Set_Playback_SPK_Volume(codec, STATUS_OFF);
			if (MAIN_MIC == wm1811_recording_device)	
			{
				DEBUG_LOG("Switch : Main MIC ==> Ear MIC");
				wm1811_mic_enable(codec, MAIN_MIC, STATUS_OFF);
				wm1811_record_main_mic(codec, STATUS_OFF);
				
				wm1811_mic_enable(codec, HP_MIC, STATUS_ON);
				wm1811_record_ear_mic(codec, STATUS_ON);
			}
		}
		else		
		{	
			DEBUG_LOG("Switch : Speaker");
			WM1811_Set_Playback_EAR_Off(codec);
			//WM1811_Set_Playback_EAR_Volume(codec, STATUS_OFF);
			if (HP_MIC == wm1811_recording_device)	
			{
				DEBUG_LOG("Switch : Ear MIC ==> Main MIC");
				
				wm1811_mic_enable(codec, HP_MIC, STATUS_OFF);
				wm1811_record_ear_mic(codec, STATUS_OFF);

				wm1811_mic_enable(codec, MAIN_MIC, STATUS_ON);
				wm1811_record_main_mic(codec, STATUS_ON);
			}
		}
	}
#endif

	if (STATUS_OFF == status)	
	{
		DEBUG_LOG( "mute after ear off-----------------------------------------------");
		val1 = wm1811_read_cache( WM1811_LEFT_OUTPUT_VOLUME);
		val1 &= ~(WM1811_HPOUT1L_MUTE_N_MASK);
		wm1811_write(codec, WM1811_LEFT_OUTPUT_VOLUME, val1);

		// Right output volume
		val2 = wm1811_read_cache( WM1811_RIGHT_OUTPUT_VOLUME);
		val2&= ~(WM1811_HPOUT1R_MUTE_N_MASK);
		wm1811_write(codec, WM1811_RIGHT_OUTPUT_VOLUME, val2);	
	}
	return;
}
EXPORT_SYMBOL(WM1811_Switch_To_Earphone);


void wm1811_codec_clocking(struct snd_soc_codec *codec)
{
	DEBUG_LOG("");

	/*-------------- FLL & Clocking --------------*/
	wm1811_write(codec, WM1811_AIF1_CLOCKING_1, 0x0010 );
	wm1811_write(codec, WM1811_FLL1_CONTROL_2, 0x0700 );
	wm1811_write(codec, WM1811_FLL1_CONTROL_3, 0x1E12 );
	wm1811_write(codec, WM1811_FLL1_CONTROL_4, 0x00C0 );
	wm1811_write(codec, WM1811_FLL1_CONTROL_5, 0x0C88 );
	wm1811_write(codec, WM1811_FLL1_EFS_1, 0x1FBD );
	wm1811_write(codec, WM1811_FLL1_EFS_2, 0x0007 );
	wm1811_write(codec, WM1811_FLL1_CONTROL_1, 0x0001 );
	wm1811_write(codec, WM1811_AIF1_RATE, 0x0073);
	wm1811_write(codec, WM1811_AIF1_CONTROL_1, 0x4010);
	wm1811_write(codec, WM1811_AIF1_MASTER_SLAVE, 0x0070);
	wm1811_write(codec, WM1811_AIF1_BCLK, 0x0070);
	wm1811_write(codec, WM1811_CLOCKING_1, 0x000A );
	wm1811_write(codec, WM1811_AIF1_CLOCKING_1, 0x0011 );
	wm1811_write(codec, WM1811_AIF1_MASTER_SLAVE, 0x4000);
}
#endif

#define WM1811_RATES	 (SNDRV_PCM_RATE_8000_96000)
#define WM1811_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
			SNDRV_PCM_FMTBIT_S24_LE)
			
static struct snd_soc_dai_ops wm1811_dai_hifi_ops = {
	.startup	= wm1811_startup,
	.shutdown	= wm1811_shutdown,
	.hw_params	= wm1811_hw_params,
	.set_sysclk	= wm1811_set_dai_sysclk,
	.set_fmt	= wm1811_set_dai_fmt,
	.set_tristate	= wm1811_set_tristate,
};


static struct snd_soc_dai_driver wm1811_dai[] = {
	{
		.name = "wm1811-aif1",
		.playback = {
			.stream_name = "HiFi Playback",
			.channels_min = 2,
			.channels_max = 4,
			.rates = SNDRV_PCM_RATE_44100,
			.formats = WM1811_FORMATS,},
		.capture = {
			.stream_name = "Capture",
			.channels_min = 2,
			.channels_max = 4,
			.rates = SNDRV_PCM_RATE_44100,
			.formats = WM1811_FORMATS,},
		.ops = &wm1811_dai_hifi_ops,
	}
};

static int wm1811_soc_suspend(struct snd_soc_codec *codec, pm_message_t state)
{	
	if((wm1811_mode !=VOICE_CALL)&&(wm1811_mode !=VOICE_MEMO)&&(wm1811_mode!=HF_MODE)
		&&(wm1811_mode !=VOIP_CALL)&&(wm1811_mode !=FM_RADIO)&&(wm1811_mode !=VT_CALL))
	{
		DEBUG_LOG_ERR("wm1811_suspend testmode is %d\n", wm1811_mode);
		 wm1811_mode = STATUS_OFF;
		 wm1811_old_mode = STATUS_OFF;
		 wm1811_mic_mute_enable = STATUS_OFF;
		 wm1811_rec_8k_enable = STATUS_OFF;
		 wm1811_fm_radio_mute_enable = STATUS_OFF;
		 //wm1811_vr_mode = STATUS_OFF;
		 //considering after the resume, sejong
		//wm1811_radio_path = STATUS_OFF;

		 wm1811_playback_device = STATUS_OFF;
		 wm1811_recording_device = STATUS_OFF;
		 wm1811_voip_device = STATUS_OFF;
		 wm1811_fm_device = STATUS_OFF;
		 wm1811_hf_device = STATUS_OFF;

		 wm1811_set_bias_level(codec, SND_SOC_BIAS_OFF);

#ifdef REG_DEBUG_SUSPEND
		int i;
		for (i=0; i<WM1811_CACHEREGNUM; i++)	{
			if (access_masks[i].readable != 0)
				printk ("wm1811_reg[%x]=%x\n", i, wm1811_reg[i]);
		}
#endif
	}

	//for sleep / wake dc offset correction, sejong.
	 wm1811_codec_hp_optimized_check = STATUS_OFF;
		
	return 0;
}

static int wm1811_soc_resume(struct snd_soc_codec *codec)
{
	if ((wm1811_mode !=VOICE_CALL)&&(wm1811_mode !=VOICE_MEMO)&&(wm1811_mode!=HF_MODE)
  		&&(wm1811_mode !=VOIP_CALL)&&(wm1811_mode !=FM_RADIO)&&(wm1811_mode !=VT_CALL))
	{		

#ifndef GPS_TEST_FW
		wm1811_codec_enable(STATUS_ON);
#endif
		msleep( 20 );
		wm1811_codec_init(codec);
		wm1811_codec_clocking(codec);
		WM1811_Set_Playback_Init(codec);

		WM1811_Set_Playback_From_Headset_Status(codec, STATUS_ON);
	}
	
#ifdef REG_DEBUG_RESUME
	int i;
	for (i=0; i<WM1811_CACHEREGNUM; i++)	{
			if (access_masks[i].readable != 0)
				printk ("wm1811_reg[%x]=%x\n", i, wm1811_reg[i]);
	}
#endif

	return 0;
}

static int wm1811_soc_probe(struct snd_soc_codec *codec)
{
	struct wm1811_priv *wm1811;

	DEBUG_LOG_ERR("wm1811_soc_probe \n");

	codec->control_data = dev_get_drvdata(codec->dev->parent);

	gCodec = codec;

	wm1811 = kzalloc(sizeof(struct wm1811_priv), GFP_KERNEL);
	if (wm1811 == NULL) {
		DEBUG_LOG_ERR("Can not allocate memroy\n");
		return -ENOMEM;
	}
	snd_soc_codec_set_drvdata(codec, wm1811);
	/* Set the defaults, and power up the codec */

	codec->dapm->bias_level = SND_SOC_BIAS_OFF;
	codec->idle_bias_off = 1;

#ifdef MID_LOCALE_VOLUME
	/* default : by jinho.lim */
	wm1811->locale = LOCALE_WW; 
#endif

	wm1811_init_chip(codec);

	snd_soc_add_controls(codec, wm1811_snd_controls,
				ARRAY_SIZE(wm1811_snd_controls));

	DEBUG_LOG_ERR("add control done !!\n");
    
	wm1811_add_widgets(codec);
	
	return 0;
}

static int wm1811_soc_remove(struct snd_soc_codec *codec)
{
	DEBUG_LOG("wm1811_soc_remove");
	
	wm1811_set_bias_level(codec, SND_SOC_BIAS_OFF);
	
	return 0;
}

struct snd_soc_codec_driver soc_codec_dev_wm1811 = {
	.probe = wm1811_soc_probe,
	.remove = wm1811_soc_remove,
	.suspend = wm1811_soc_suspend,
	.resume = wm1811_soc_resume,
	.read = wm1811_read,
	.write = wm1811_write,
	.set_bias_level = wm1811_set_bias_level,
	.reg_cache_size = WM1811_REGISTER_COUNT,
	.reg_cache_default = wm1811_reg,
	.readable_register = wm1811_read_cacheable,
	.reg_word_size = 2,
};

static int __devinit wm1811_codec_probe(struct platform_device *pdev)
{
	DEBUG_LOG_ERR("");

	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_wm1811,
			wm1811_dai, ARRAY_SIZE(wm1811_dai));
}

static int __devexit wm1811_codec_remove(struct platform_device *pdev)
{
	struct wm1811_priv *wm1811 = dev_get_drvdata(&pdev->dev);

	DEBUG_LOG_ERR("wm1811_codec_remove \n");

	snd_soc_unregister_codec(&pdev->dev);
	kfree(wm1811);
	return 0;
}

static struct platform_driver wm1811_codec_driver = {
	.probe		= wm1811_codec_probe,
	.remove		= __devexit_p(wm1811_codec_remove),
	.driver		= {
		.name	= "wm1811-codec",
		.owner	= THIS_MODULE,
	},
};

static int __init wm1811_modinit(void)
{
	DEBUG_LOG_ERR("wm1811_modinit \n");

	return platform_driver_register(&wm1811_codec_driver);
}
module_init(wm1811_modinit);

static void __exit wm1811_exit(void)
{
       DEBUG_LOG_ERR("wm1811_exit \n");
	platform_driver_unregister(&wm1811_codec_driver);
}
module_exit(wm1811_exit);

MODULE_DESCRIPTION("ASoC WM1811 driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:wm1811-codec");
