/*
 * wm8994-stub.c  --  Simple register write WM8994 driver
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

#include <linux/mfd/wm8994/core.h>
#include <linux/mfd/wm8994/registers.h>
#include <linux/mfd/wm8994/pdata.h>
#include <linux/mfd/wm8994/gpio.h>

static int wm8994_write(struct snd_soc_codec *codec, unsigned int reg,
	unsigned int value)
{
	dev_dbg(codec->dev, "0x%x = 0x%x\n", reg, value);
	dev_err(codec->dev, "0x%x = 0x%x\n", reg, value);

	return wm8994_reg_write(codec->control_data, reg, value);
}

static unsigned int wm8994_read(struct snd_soc_codec *codec,
				unsigned int reg)
{
	return wm8994_reg_read(codec->control_data, reg);
}

static int wm8994_set_fll(struct snd_soc_dai *dai, int id, int src,
			  unsigned int freq_in, unsigned int freq_out)
{
	return 0;
}

static int wm8994_set_dai_sysclk(struct snd_soc_dai *dai,
		int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static int wm8994_set_dai_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	return 0;
}

static int wm8994_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;

	snd_soc_write(codec, 0x00, 0x0000 );
	msleep( 500 );
	snd_soc_write(codec, 0x200, 0x0010 );
	snd_soc_write(codec, 0x221, 0x0700 );
	snd_soc_write(codec, 0x222, 0x1E12 );
	snd_soc_write(codec, 0x223, 0x00C0 );
	snd_soc_write(codec, 0x224, 0x0C88 );
	snd_soc_write(codec, 0x226, 0x1FBD );
	snd_soc_write(codec, 0x227, 0x0007 );
	snd_soc_write(codec, 0x220, 0x0001 );
	snd_soc_write(codec, 0x210, 0x0073 );
	snd_soc_write(codec, 0x300, 0x4050 );
	snd_soc_write(codec, 0x300, 0x4010 );
	snd_soc_write(codec, 0x303, 0x0070 );
	snd_soc_write(codec, 0x302, 0x4000 );
	snd_soc_write(codec, 0x208, 0x000A );
	snd_soc_write(codec, 0x200, 0x0011 );
	snd_soc_write(codec, 0x39, 0x01E0 );
	snd_soc_write(codec, 0x01, 0x3003 );
	msleep( 50 );
	snd_soc_write(codec, 0x03, 0x0300 );
	snd_soc_write(codec, 0x22, 0x0000 );
	snd_soc_write(codec, 0x23, 0x0000 );
	snd_soc_write(codec, 0x25, 0x0176 );
	snd_soc_write(codec, 0x26, 0x017B );
	snd_soc_write(codec, 0x27, 0x017B );
	snd_soc_write(codec, 0x36, 0x0003 );
	snd_soc_write(codec, 0x05, 0x0303 );
	snd_soc_write(codec, 0x601, 0x0001 );
	snd_soc_write(codec, 0x602, 0x0001 ); 	
	snd_soc_write(codec, 0x430, 0x0069 );
	snd_soc_write(codec, 0x102, 0x0003 );
	snd_soc_write(codec, 0xC1, 0x0001 );
	snd_soc_write(codec, 0x102, 0x0000 );
	snd_soc_write(codec, 0x610, 0x01CA );
	snd_soc_write(codec, 0x611, 0x01CA );
	snd_soc_write(codec, 0x420, 0x0000 );

	return 0;
}

static void wm8994_shutdown(struct snd_pcm_substream *substream,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;

	snd_soc_write(codec, WM8994_SOFTWARE_RESET, 0x1811);
}

#define WM8994_RATES SNDRV_PCM_RATE_8000_96000

#define WM8994_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
			SNDRV_PCM_FMTBIT_S24_LE)

static struct snd_soc_dai_ops wm8994_aif1_dai_ops = {
	.set_sysclk	= wm8994_set_dai_sysclk,
	.set_fmt	= wm8994_set_dai_fmt,
	.hw_params	= wm8994_hw_params,
	.set_pll	= wm8994_set_fll,
	.shutdown	= wm8994_shutdown,
};

static struct snd_soc_dai_ops wm8994_aif2_dai_ops = {
	.set_sysclk	= wm8994_set_dai_sysclk,
	.set_fmt	= wm8994_set_dai_fmt,
	.hw_params	= wm8994_hw_params,
	.set_pll	= wm8994_set_fll,
	.shutdown	= wm8994_shutdown,
};

static struct snd_soc_dai_ops wm8994_aif3_dai_ops = {
};

static struct snd_soc_dai_driver wm8994_dai[] = {
	{
		.name = "wm8994-aif1",
		.playback = {
			.stream_name = "AIF1 Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = WM8994_RATES,
			.formats = WM8994_FORMATS,
		},
		.capture = {
			.stream_name = "AIF1 Capture",
			.channels_min = 2,
			.channels_max = 2,
			.rates = WM8994_RATES,
			.formats = WM8994_FORMATS,
		 },
		.ops = &wm8994_aif1_dai_ops,
	},
	{
		.name = "wm8994-aif2",
		.playback = {
			.stream_name = "AIF2 Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = WM8994_RATES,
			.formats = WM8994_FORMATS,
		},
		.capture = {
			.stream_name = "AIF2 Capture",
			.channels_min = 2,
			.channels_max = 2,
			.rates = WM8994_RATES,
			.formats = WM8994_FORMATS,
		},
		.ops = &wm8994_aif2_dai_ops,
	},
	{
		.name = "wm8994-aif3",
		.playback = {
			.stream_name = "AIF3 Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = WM8994_RATES,
			.formats = WM8994_FORMATS,
		},
		.capture = {
			.stream_name = "AIF3 Capture",
			.channels_min = 2,
			.channels_max = 2,
			.rates = WM8994_RATES,
			.formats = WM8994_FORMATS,
		},
		.ops = &wm8994_aif3_dai_ops,
	}
};

static int wm8994_codec_probe(struct snd_soc_codec *codec)
{
	codec->control_data = dev_get_drvdata(codec->dev->parent);

	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_wm8994 = {
	.probe =	wm8994_codec_probe,
	.read =		wm8994_read,
	.write =	wm8994_write,
};

static int __devinit wm8994_probe(struct platform_device *pdev)
{
	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_wm8994,
			wm8994_dai, ARRAY_SIZE(wm8994_dai));
}

static int __devexit wm8994_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static struct platform_driver wm8994_codec_driver = {
	.driver = {
		   .name = "wm8994-codec",
		   .owner = THIS_MODULE,
	},
	.probe = wm8994_probe,
	.remove = __devexit_p(wm8994_remove),
};

static __init int wm8994_init(void)
{
	return platform_driver_register(&wm8994_codec_driver);
}
module_init(wm8994_init);

static __exit void wm8994_exit(void)
{
	platform_driver_unregister(&wm8994_codec_driver);
}
module_exit(wm8994_exit);

MODULE_DESCRIPTION("ASoC WM8994 driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:wm8994-codec");
