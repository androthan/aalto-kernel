/*
 * sdp3430.c  --  SoC audio for TI OMAP3430 SDP
 *
 * Author: Misael Lopez Cruz <x0052729@ti.com>
 *
 * Based on:
 * Author: Steve Sakoman <steve@sakoman.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/jack.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/mcbsp.h>
#include <plat/mux.h>

#include "../codecs/wm1811.h"

/* C2 State Patch */
#include <plat/omap-pm.h>

#include "omap-mcbsp.h"
#include "omap-pcm.h"

static int sdp3430_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret;

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Configure CODEC clocking, these are only used with Wolfson driver */
	ret = snd_soc_dai_set_pll(codec_dai, WM1811_FLL1, WM1811_FLL_SRC_MCLK1,
				  26000000, 256 * params_rate(params));

	ret = snd_soc_dai_set_sysclk(codec_dai, WM1811_SYSCLK_MCLK1, 256 * params_rate(params),
					    SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	return 0;
}


int sdp3430_hw_free(struct snd_pcm_substream *substream) 
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;     
	int ret;        

	/* Use function clock for mcBSP2 */     

	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_SYSCLK_CLKS_FCLK, 
					0, SND_SOC_CLOCK_OUT);  

	return 0; 
}

static int snd_hw_latency;
extern void omap_dpll3_errat_wa(int disable);

int sdp3430_i2s_startup(struct snd_pcm_substream *substream)
{      
	struct snd_soc_pcm_runtime *rtd = substream->private_data;

	/*        
	  * Hold C2 as min latency constraint. Deeper states   
	  * MPU RET/OFF is overhead and consume more power than    
	  * savings.    
	  * snd_hw_latency check takes care of playback and capture      
	  * usecase.      
	  */     

	  if (!snd_hw_latency++) { 
	  	omap_pm_set_max_mpu_wakeup_lat(&substream->latency_pm_qos_req, 18);        

	  	/*           
	  	  * As of now for MP3 playback case need to enable dpll3 
	  	  * autoidle part of dpll3 lock errata.          
	  	  * REVISIT: Remove this, Once the dpll3 lock errata is     
	  	  * updated with with a new workaround without impacting mp3 usecase.          
	  	  */              

//              printk("sdp3430_i2s_startup  \n");
	  	
	  	omap_dpll3_errat_wa(0);  
	  }      

	  return 0;
}

int sdp3430_i2s_shutdown(struct snd_pcm_substream *substream)
{    
	struct snd_soc_pcm_runtime *rtd = substream->private_data;	

	/* remove latency constraint */       
	snd_hw_latency--;     

	if (!snd_hw_latency) {   
		omap_pm_set_max_mpu_wakeup_lat(&substream->latency_pm_qos_req, -1);     
//		printk("sdp3430_i2s_shutdown \n");
		omap_dpll3_errat_wa(1);    
	}   

	return 0;
}	
static struct snd_soc_ops sdp3430_ops = {
	.startup = sdp3430_i2s_startup,	
	.hw_params = sdp3430_hw_params,	
	.hw_free = sdp3430_hw_free,	
	.shutdown = sdp3430_i2s_shutdown,
};


/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link sdp3430_dai[] = {
	{
		.name = "TWL4030 I2S",
		.stream_name = "TWL4030 Audio",
		.cpu_dai_name = "omap-mcbsp-dai.1",
		.codec_dai_name = "wm1811-aif1",
		.platform_name = "omap-pcm-audio",
		.codec_name = "wm1811-codec",
		.ops = &sdp3430_ops,
	},
};

/* Audio machine driver */
static struct snd_soc_card snd_soc_sdp3430 = {
	.name = "SDP3430",
	.long_name = "sdp3430 (wm1811)",
	.dai_link = sdp3430_dai,
	.num_links = ARRAY_SIZE(sdp3430_dai),
};

static struct platform_device *sdp3430_snd_device;

static int __init sdp3430_soc_init(void)
{
	int ret;
	u8 pin_mux;

	printk(KERN_EMERG " %s : %s : %i \n", __FILE__, __FUNCTION__, __LINE__);

	#if 0
	if (!machine_is_omap_3430sdp()) {
		pr_debug("Not SDP3430!\n");
		return -ENODEV;
	}
	#endif
	printk(KERN_INFO "SDP3430 SoC init\n");

	sdp3430_snd_device = platform_device_alloc("soc-audio", -1);
	if (!sdp3430_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(sdp3430_snd_device, &snd_soc_sdp3430);


	ret = platform_device_add(sdp3430_snd_device);
	if (ret)
		goto err1;
	else
		printk(KERN_EMERG "platform_device_add success\n");

	return 0;

err1:

	printk(KERN_EMERG " %s : %s : %i UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\n", __FILE__, __FUNCTION__, __LINE__);
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(sdp3430_snd_device);

	return ret;
}
module_init(sdp3430_soc_init);

static void __exit sdp3430_soc_exit(void)
{
	platform_device_unregister(sdp3430_snd_device);
}
module_exit(sdp3430_soc_exit);

MODULE_AUTHOR("Misael Lopez Cruz <x0052729@ti.com>");
MODULE_DESCRIPTION("ALSA SoC SDP3430");
MODULE_LICENSE("GPL");
