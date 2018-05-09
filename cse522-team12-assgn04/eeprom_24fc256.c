

#include <logging/sys_log.h>
#include <kernel.h>
#include <device.h>
#include <string.h>
#include <flash.h>
#include <errno.h>
#include <init.h>
#include <zephyr.h>
#include <i2c.h>
#include <gpio.h>
#include <pinmux.h>
#include <misc/printk.h>
#include <soc.h>

#ifndef __FLASH_H__
#define __FLASH_H__


#include <zephyr/types.h>
#include <stddef.h>
#include <sys/types.h>

#include<eeprom_24fc2556.h>
#define DEV_OK  0

struct device *gpio;
struct device *gpio_sus;
struct device *i2c_dev, *i2c;
struct device *pinmux;
struct device *exp0;
struct flash_pages_info {
	off_t start_offset; /* offset from the base of flash address */
	size_t size;
	u32_t index;
};


typedef bool (*flash_page_cb)(const struct flash_pages_info *info, void *data);


void flash_page_foreach(struct device *dev, flash_page_cb cb, void *data);
#endif /* CONFIG_FLASH_PAGE_LAYOUT */

#include <syscalls/flash.h>


struct flash_priv {
	flash_config_t config;
	struct k_sem write_lock;
};


static int flash_24fc256_erase(struct device *dev, off_t offset, size_t len)
{
	const struct flash_driver_api *api = dev->driver_api;




	return api->erase(dev, offset, size);

	
}

static int flash_24fc256_read(struct device *dev, off_t offset,
				void *data, size_t len)
{
	const struct flash_driver_api *api = dev->driver_api;



	return api->read(dev, offset, data, len);
	
}

static int flash_24fc256_write(struct device *dev, off_t offset,
				const void *data, size_t len)
{

	const struct flash_driver_api *api = dev->driver_api;





	return api->write(dev, offset, data, len);
}



#if defined(CONFIG_FLASH_PAGE_LAYOUT)
static const struct flash_pages_layout dev_layout = {
	.pages_count = KB(CONFIG_FLASH_SIZE) / FLASH_ERASE_BLOCK_SIZE,
	.pages_size = FLASH_ERASE_BLOCK_SIZE,
};

static void flash_24fc256_pages_layout(struct device *dev,
									const struct flash_pages_layout **layout,
									size_t *layout_size)
{
	*layout = &dev_layout;
	*layout_size = 1;
}
#endif /* CONFIG_FLASH_PAGE_LAYOUT */

static struct flash_priv flash_data;

static const struct flash_driver_api flash_24fc256_api = {
	.write_protection = flash_24fc256_write_protection,
	.erase = flash_24fc256_erase,
	.write = flash_24fc256_write,
	.read = flash_24fc256_read,
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
	.page_layout = flash_24fc256_pages_layout,
#endif
	.write_block_size = FSL_FEATURE_FLASH_PFLASH_BLOCK_WRITE_UNIT_SIZE,
};

static int flash_24fc256_init(struct device *dev)
{

	int rc1;
	gpio = device_get_binding(CONFIG_GPIO_PCAL9535A_2_DEV_NAME);
	i2c = device_get_binding("I2C_0");
	if (!i2c) {
		printk("I2C not found!!\n");
	}
	i2c_dev = device_get_binding(CONFIG_GPIO_PCAL9535A_1_I2C_MASTER_DEV_NAME);
		
	if (!gpio) {
		printk("GPIO not found!!\n");
	} else {
		rc1 = gpio_pin_write(gpio, 12, 0);  	
		if (rc1 != DEV_OK){ 
			printk("GPIO config error %d!!\n", rc1);
		}
		rc1 = gpio_pin_configure(gpio, 12, GPIO_DIR_OUT);  	
		if (rc1 != DEV_OK){ 
			printk("GPIO config error %d!!\n", rc1);
		}
	}
  	
	if (!i2c_dev) {
		printk("I2C not found!!\n");
	} else {
		rc1 = i2c_configure(i2c_dev, (I2C_SPEED_FAST << 1) | I2C_MODE_MASTER);
		if (rc1 != DEV_OK) {
			printk("I2C configuration error: %d\n", rc1);
		}
		else {
			//printk("I2C configuration : %d\n", rc1);
		}
	}
}

DEVICE_AND_API_INIT(24FC256EEPROM, FLASH_DEV_NAME,
			flash_24fc256_init, &flash_data, NULL, POST_KERNEL,
			CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &flash_24fc256_api);

