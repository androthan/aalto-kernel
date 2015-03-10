/*
 * wm1811-irq.c  --  Interrupt controller support for Wolfson WM1811
 *
 * Copyright 2010 Wolfson Microelectronics PLC.
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/mfd/core.h>
#include <linux/interrupt.h>

#include <linux/mfd/wm1811/core.h>
#include <linux/mfd/wm1811/registers.h>

#include <linux/delay.h>

struct wm1811_irq_data {
	int reg;
	int mask;
};

static struct wm1811_irq_data wm1811_irqs[] = {
	[WM1811_IRQ_TEMP_SHUT] = {
		.reg = 2,
		.mask = WM1811_TEMP_SHUT_EINT,
	},
	[WM1811_IRQ_MIC1_DET] = {
		.reg = 2,
		.mask = WM1811_MIC1_DET_EINT,
	},
	[WM1811_IRQ_MIC1_SHRT] = {
		.reg = 2,
		.mask = WM1811_MIC1_SHRT_EINT,
	},
	[WM1811_IRQ_MIC2_DET] = {
		.reg = 2,
		.mask = WM1811_MIC2_DET_EINT,
	},
	[WM1811_IRQ_MIC2_SHRT] = {
		.reg = 2,
		.mask = WM1811_MIC2_SHRT_EINT,
	},
	[WM1811_IRQ_FLL1_LOCK] = {
		.reg = 2,
		.mask = WM1811_FLL1_LOCK_EINT,
	},
	[WM1811_IRQ_FLL2_LOCK] = {
		.reg = 2,
		.mask = WM1811_FLL2_LOCK_EINT,
	},
	[WM1811_IRQ_SRC1_LOCK] = {
		.reg = 2,
		.mask = WM1811_SRC1_LOCK_EINT,
	},
	[WM1811_IRQ_SRC2_LOCK] = {
		.reg = 2,
		.mask = WM1811_SRC2_LOCK_EINT,
	},
	[WM1811_IRQ_AIF1DRC1_SIG_DET] = {
		.reg = 2,
		.mask = WM1811_AIF1DRC1_SIG_DET,
	},
#if 0
	[WM1811_IRQ_AIF1DRC2_SIG_DET] = {
		.reg = 2,
		.mask = WM1811_AIF1DRC2_SIG_DET_EINT,
	},
	[WM1811_IRQ_AIF2DRC_SIG_DET] = {
		.reg = 2,
		.mask = WM1811_AIF2DRC_SIG_DET_EINT,
	},
#endif
	[WM1811_IRQ_FIFOS_ERR] = {
		.reg = 2,
		.mask = WM1811_FIFOS_ERR_EINT,
	},
	[WM1811_IRQ_WSEQ_DONE] = {
		.reg = 2,
		.mask = WM1811_WSEQ_DONE_EINT,
	},
#if 0
	[WM1811_IRQ_DCS_DONE] = {
		.reg = 2,
		.mask = WM1811_DCS_DONE_EINT,
	},
	[WM1811_IRQ_TEMP_WARN] = {
		.reg = 2,
		.mask = WM1811_TEMP_WARN_EINT,
	},
#endif
	[WM1811_IRQ_GPIO(1)] = {
		.reg = 1,
		.mask = WM1811_GP1_EINT,
	},
	[WM1811_IRQ_GPIO(2)] = {
		.reg = 1,
		.mask = WM1811_GP2_EINT,
	},
	[WM1811_IRQ_GPIO(3)] = {
		.reg = 1,
		.mask = WM1811_GP3_EINT,
	},
	[WM1811_IRQ_GPIO(4)] = {
		.reg = 1,
		.mask = WM1811_GP4_EINT,
	},
	[WM1811_IRQ_GPIO(5)] = {
		.reg = 1,
		.mask = WM1811_GP5_EINT,
	},
	[WM1811_IRQ_GPIO(6)] = {
		.reg = 1,
		.mask = WM1811_GP6_EINT,
	},
	[WM1811_IRQ_GPIO(7)] = {
		.reg = 1,
		.mask = WM1811_GP7_EINT,
	},
	[WM1811_IRQ_GPIO(8)] = {
		.reg = 1,
		.mask = WM1811_GP8_EINT,
	},
	[WM1811_IRQ_GPIO(9)] = {
		.reg = 1,
		.mask = WM1811_GP8_EINT,
	},
	[WM1811_IRQ_GPIO(10)] = {
		.reg = 1,
		.mask = WM1811_GP10_EINT,
	},
	[WM1811_IRQ_GPIO(11)] = {
		.reg = 1,
		.mask = WM1811_GP11_EINT,
	},
};

static inline int irq_data_to_status_reg(struct wm1811_irq_data *irq_data)
{
	return WM1811_INTERRUPT_STATUS_1 - 1 + irq_data->reg;
}

static inline int irq_data_to_mask_reg(struct wm1811_irq_data *irq_data)
{
	return WM1811_INTERRUPT_STATUS_1_MASK - 1 + irq_data->reg;
}

static inline struct wm1811_irq_data *irq_to_wm1811_irq(struct wm1811 *wm1811,
							int irq)
{
	return &wm1811_irqs[irq - wm1811->irq_base];
}

static void wm1811_irq_lock(unsigned int irq)
{
	struct wm1811 *wm1811 = get_irq_chip_data(irq);

	mutex_lock(&wm1811->irq_lock);
}

static void wm1811_irq_sync_unlock(unsigned int irq)
{
	struct wm1811 *wm1811 = get_irq_chip_data(irq);
	int i;

	for (i = 0; i < ARRAY_SIZE(wm1811->irq_masks_cur); i++) {
		/* If there's been a change in the mask write it back
		 * to the hardware. */
		if (wm1811->irq_masks_cur[i] != wm1811->irq_masks_cache[i]) {
			wm1811->irq_masks_cache[i] = wm1811->irq_masks_cur[i];
			wm1811_reg_write(wm1811,
					 WM1811_INTERRUPT_STATUS_1_MASK + i,
					 wm1811->irq_masks_cur[i]);
		}
	}

	mutex_unlock(&wm1811->irq_lock);
}

static void wm1811_irq_unmask(unsigned int irq)
{
	struct wm1811 *wm1811 = get_irq_chip_data(irq);
	struct wm1811_irq_data *irq_data = irq_to_wm1811_irq(wm1811, irq);

	wm1811->irq_masks_cur[irq_data->reg - 1] &= ~irq_data->mask;
}

static void wm1811_irq_mask(unsigned int irq)
{
	struct wm1811 *wm1811 = get_irq_chip_data(irq);
	struct wm1811_irq_data *irq_data = irq_to_wm1811_irq(wm1811, irq);

	wm1811->irq_masks_cur[irq_data->reg - 1] |= irq_data->mask;
}

static struct irq_chip wm1811_irq_chip = {
	.name = "wm8994",
	.bus_lock = wm1811_irq_lock,
	.bus_sync_unlock = wm1811_irq_sync_unlock,
	.mask = wm1811_irq_mask,
	.unmask = wm1811_irq_unmask,
};

/* The processing of the primary interrupt occurs in a thread so that
 * we can interact with the device over I2C or SPI. */
static irqreturn_t wm1811_irq_thread(int irq, void *data)
{
	struct wm1811 *wm1811 = data;
	unsigned int i;
	u16 status[WM1811_NUM_IRQ_REGS];
	int ret;

	ret = wm1811_bulk_read(wm1811, WM1811_INTERRUPT_STATUS_1,
			       WM1811_NUM_IRQ_REGS, status);
	if (ret < 0) {
		dev_err(wm1811->dev, "Failed to read interrupt status: %d\n",
			ret);
		return IRQ_NONE;
	}

	/* Apply masking */
	for (i = 0; i < WM1811_NUM_IRQ_REGS; i++)
		status[i] &= ~wm1811->irq_masks_cur[i];

	/* Report */
	for (i = 0; i < ARRAY_SIZE(wm1811_irqs); i++) {
		if (status[wm1811_irqs[i].reg - 1] & wm1811_irqs[i].mask)
			handle_nested_irq(wm1811->irq_base + i);
	}

	/* Ack any unmasked IRQs */
	for (i = 0; i < ARRAY_SIZE(status); i++) {
		if (status[i])
			wm1811_reg_write(wm1811, WM1811_INTERRUPT_STATUS_1 + i,
					 status[i]);
	}

	return IRQ_HANDLED;
}

int wm1811_irq_init(struct wm1811 *wm1811)
{
	int i, cur_irq, ret;

	mutex_init(&wm1811->irq_lock);

	/* Mask the individual interrupt sources */
	for (i = 0; i < ARRAY_SIZE(wm1811->irq_masks_cur); i++) {
		wm1811->irq_masks_cur[i] = 0xffff;
		wm1811->irq_masks_cache[i] = 0xffff;
		wm1811_reg_write(wm1811, WM1811_INTERRUPT_STATUS_1_MASK + i,
				 0xffff);
	}

	if (!wm1811->irq) {
		dev_warn(wm1811->dev,
			 "No interrupt specified, no interrupts\n");
		wm1811->irq_base = 0;
		return 0;
	}

	if (!wm1811->irq_base) {
		dev_err(wm1811->dev,
			"No interrupt base specified, no interrupts\n");
		return 0;
	}

	/* Register them with genirq */
	for (cur_irq = wm1811->irq_base;
	     cur_irq < ARRAY_SIZE(wm1811_irqs) + wm1811->irq_base;
	     cur_irq++) {
		set_irq_chip_data(cur_irq, wm1811);
		set_irq_chip_and_handler(cur_irq, &wm1811_irq_chip,
					 handle_edge_irq);
		set_irq_nested_thread(cur_irq, 1);

		/* ARM needs us to explicitly flag the IRQ as valid
		 * and will set them noprobe when we do so. */
#ifdef CONFIG_ARM
		set_irq_flags(cur_irq, IRQF_VALID);
#else
		set_irq_noprobe(cur_irq);
#endif
	}

	ret = request_threaded_irq(wm1811->irq, NULL, wm1811_irq_thread,
				   IRQF_TRIGGER_HIGH | IRQF_ONESHOT,
				   "wm8994", wm1811);
	if (ret != 0) {
		dev_err(wm1811->dev, "Failed to request IRQ %d: %d\n",
			wm1811->irq, ret);
		return ret;
	}

	/* Enable top level interrupt if it was masked */
	wm1811_reg_write(wm1811, WM1811_INTERRUPT_CONTROL, 0);

	return 0;
}

void wm1811_irq_exit(struct wm1811 *wm1811)
{
	if (wm1811->irq)
		free_irq(wm1811->irq, wm1811);
}
