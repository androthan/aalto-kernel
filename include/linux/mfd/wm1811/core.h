/*
 * include/linux/mfd/wm1811/core.h -- Core interface for WM1811
 *
 * Copyright 2009 Wolfson Microelectronics PLC.
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#ifndef __MFD_WM1811_CORE_H__
#define __MFD_WM1811_CORE_H__

#include <linux/interrupt.h>

struct regulator_dev;
struct regulator_bulk_data;

#define WM1811_NUM_GPIO_REGS 11
#define WM1811_NUM_LDO_REGS   2
#define WM1811_NUM_IRQ_REGS   2

#define WM1811_IRQ_TEMP_SHUT		0
#define WM1811_IRQ_MIC1_DET		1
#define WM1811_IRQ_MIC1_SHRT		2
#define WM1811_IRQ_MIC2_DET		3
#define WM1811_IRQ_MIC2_SHRT		4
#define WM1811_IRQ_FLL1_LOCK		5
#define WM1811_IRQ_FLL2_LOCK		6
#define WM1811_IRQ_SRC1_LOCK		7
#define WM1811_IRQ_SRC2_LOCK		8
#define WM1811_IRQ_AIF1DRC1_SIG_DET	9
#define WM1811_IRQ_AIF1DRC2_SIG_DET	10
#define WM1811_IRQ_AIF2DRC_SIG_DET	11
#define WM1811_IRQ_FIFOS_ERR		12
#define WM1811_IRQ_WSEQ_DONE		13
#define WM1811_IRQ_DCS_DONE		14
#define WM1811_IRQ_TEMP_WARN		15

/* GPIOs in the chip are numbered from 1-11 */
#define WM1811_IRQ_GPIO(x) (x + WM1811_IRQ_TEMP_WARN)

struct wm1811 {
	struct mutex io_lock;
	struct mutex irq_lock;

	struct device *dev;
	int (*read_dev)(struct wm1811 *wm1811, unsigned short reg,
			int bytes, void *dest);
	int (*write_dev)(struct wm1811 *wm1811, unsigned short reg,
			 int bytes, void *src);

	void *control_data;

	int gpio_base;
	int irq_base;

	int irq;
	u16 irq_masks_cur[WM1811_NUM_IRQ_REGS];
	u16 irq_masks_cache[WM1811_NUM_IRQ_REGS];

	/* Used over suspend/resume */
	u16 ldo_regs[WM1811_NUM_LDO_REGS];
	u16 gpio_regs[WM1811_NUM_GPIO_REGS];

	struct regulator_dev *dbvdd;
	struct regulator_bulk_data *supplies;
};

/* Device I/O API */
int wm1811_reg_read(struct wm1811 *wm1811, unsigned short reg);
int wm1811_reg_write(struct wm1811 *wm1811, unsigned short reg,
		 unsigned short val);
int wm1811_set_bits(struct wm1811 *wm1811, unsigned short reg,
		    unsigned short mask, unsigned short val);
int wm1811_bulk_read(struct wm1811 *wm1811, unsigned short reg,
		     int count, u16 *buf);


/* Helper to save on boilerplate */
static inline int wm1811_request_irq(struct wm1811 *wm1811, int irq,
				     irq_handler_t handler, const char *name,
				     void *data)
{
	if (!wm1811->irq_base)
		return -EINVAL;
	return request_threaded_irq(wm1811->irq_base + irq, NULL, handler,
				    IRQF_TRIGGER_RISING, name,
				    data);
}
static inline void wm1811_free_irq(struct wm1811 *wm1811, int irq, void *data)
{
	if (!wm1811->irq_base)
		return;
	free_irq(wm1811->irq_base + irq, data);
}

int wm1811_irq_init(struct wm1811 *wm1811);
void wm1811_irq_exit(struct wm1811 *wm1811);

#endif
