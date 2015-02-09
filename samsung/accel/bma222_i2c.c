#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

//#include <asm/hardware.h>
//#include <asm/arch/gpio.h>
//add by inter.park
#include <mach/hardware.h>
#include <linux/gpio.h>

#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "bma222_i2c.h"

#include <plat/i2c-omap-gpio.h>

#define I2C_M_WR				0x00
#define I2C_DF_NOTIFY			0x01


//static struct i2c_client *g_client;	
static OMAP_GPIO_I2C_CLIENT * client;

char i2c_acc_bma222_read(u8 reg, u8 *val, unsigned int len )
{
	OMAP_GPIO_I2C_RD_DATA i2c_rd_param;
	//printk("%s %d gpio transfer reg : 0x%x , len :%d\n", __func__, __LINE__,reg ,len);

	i2c_rd_param.reg_addr = &reg;
	i2c_rd_param.reg_len = 1;
	i2c_rd_param.rdata_len = len;
	i2c_rd_param.rdata = val;
	return omap_gpio_i2c_read(client, &i2c_rd_param);
}

char i2c_acc_bma222_write( u8 reg, u8 *val ,unsigned int len )
{
	OMAP_GPIO_I2C_WR_DATA i2c_wr_param;

	//printk("%s %d gpio transfer \n", __func__, __LINE__);
	i2c_wr_param.reg_addr = &reg;
	i2c_wr_param.reg_len = 1;
	i2c_wr_param.wdata_len = len;
	i2c_wr_param.wdata = val;
	return omap_gpio_i2c_write(client, &i2c_wr_param);
}

#if 0
static int __devinit bma222_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct bma222_state *bma;
        printk("%s called \n",__func__);
	bma = kzalloc(sizeof(struct bma222_state), GFP_KERNEL);
	if (bma == NULL) {
		pr_err("%s: failed to allocate memory\n", __func__);
		return -ENOMEM;
	}

	bma->client = client;
	i2c_set_clientdata(client, bma);
	
	printk("[%s] slave addr = %x\n", __func__, client->addr);
		g_client = client;
        return 0;
}

static int __devexit bma222_remove(struct i2c_client *client)
{	
	g_client = NULL;
	return 0;
}

static const struct i2c_device_id bma222_ids[] = {	
	{ "bma222", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, bma222_ids);

struct i2c_driver acc_bma222_i2c_driver =
{
	.driver	= {
		.name	= "bma222",
	},
	.probe		= bma222_probe,
	.remove		= __devexit_p(bma222_remove),
	.id_table	= bma222_ids,

};
#endif

int i2c_acc_bma222_init(void)
{
        printk("%s called \n",__func__);
	client = omap_gpio_i2c_init(OMAP_GPIO_SENSOR_SDA, OMAP_GPIO_SENSOR_SCL, 0x08, 200);
	return 0;
}

void i2c_acc_bma222_exit(void)
{
	printk("[bma222] i2c_exit\n");
	kfree(client);	
}
