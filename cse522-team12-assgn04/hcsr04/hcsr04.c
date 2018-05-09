#include <kernel.h>
#include <gpio.h>
#include <board.h>
#include <sys_io.h>
#include <init.h>
#include <misc/util.h>
#include <misc/__assert.h>
#include <clock_control.h>
#include <misc/util.h>
#include <device.h>
#include <sys_clock.h>
#include <limits.h>
#include <pinmux.h>
#include<sensor.h>
#include<timestamp.h>
#ifdef CONFIG_SHARED_IRQ
#include <shared_irq.h>
#endif

#ifdef CONFIG_IOAPIC
#include <drivers/ioapic.h>
#endif

#if defined(CONFIG_STDOUT_CONSOLE)
  #include <stdio.h>
  #define PRINT           printf
  #else
  #include <misc/printk.h>
  #define PRINT           printk
#endif

#include <hcsr04.h>

#define SLEEPTIME  500
#define SLEEPTICKS (SLEEPTIME * sys_clock_ticks_per_sec / 1000000)

struct hcsr04_driver_api* api0;
struct hcsr04_driver_api* api1;
uint32_t  tsc1;
uint32_t  tsc2;

#define HW_CYCLES_TO_USEC(__hw_cycle__) \
        ( \
                ((uint64_t)(__hw_cycle__) * (uint64_t)sys_clock_us_per_tick) / \
                ((uint64_t)sys_clock_hw_cycles_per_tick) \
        )
int hcsr_dw_initialize(struct device *port);


void gpio_callback(struct device *port,struct gpio_callback *cb, uint32_t pins)
{
	struct device *hcsr04_local = NULL;
	struct hcsr_dw_config *config;
	struct hcsr_dw_runtime *dev ;
	
	if(cb->pin_mask == BIT(CONFIG_GPIO_INT_PIN_2))
		hcsr04_local = device_get_binding(CONFIG_HCSR04_DW_1_NAME);

	if(cb->pin_mask == BIT(CONFIG_GPIO_INT_PIN_1))
		hcsr04_local = device_get_binding(CONFIG_HCSR04_DW_0_NAME);

	
	 config = hcsr04_local->config->config_info;
		dev= hcsr04_local->driver_data;
	int int_value;
	int rc1=0;
	gpio_pin_read(port, config->irq_pin, &int_value);
	//PRINT("gpio_callback value = %d............", int_value);
	tsc2=_tsc_read();
	if(int_value == 1)
	{
			
		
	rc1 = gpio_pin_configure(port,config->irq_pin , (GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE
			 | GPIO_INT_ACTIVE_LOW| GPIO_INT_DEBOUNCE|GPIO_PUD_PULL_DOWN));
		//printk("its in 1");
		if (rc1 != 0) {
			PRINT("Low GPIO config error %d!!\n", rc1);
		}
	}

	else
	{
	rc1 = gpio_pin_configure(port,config->irq_pin , (GPIO_DIR_IN | GPIO_INT | GPIO_INT_EDGE
	 | GPIO_INT_ACTIVE_HIGH| GPIO_INT_DEBOUNCE | GPIO_PUD_PULL_DOWN));

	if (rc1 != 0) {
	PRINT("High GPIO config error %d!!\n", rc1);
	}
	  	
	//printk("its in 2");
	  //tsc2=_tsc_read();
		dev->per_dev_buffer = (HW_CYCLES_TO_USEC((tsc2)-(tsc1))*17)/100000 ;//(340/2)/1000000                     
		
	dev->ongoing =0;
	}
	
	
	
	k_sem_give(&dev->dev_sem);
	

}


int sensor_samples_fetch(struct device *port, uint32_t pin)
{
	int rc1;
	

	struct hcsr_dw_config *config = port->config->config_info;
	struct hcsr_dw_runtime * dev = port->driver_data;
	dev->per_dev_buffer =0;
	
	
	{
	dev->ongoing =1;
	rc1=gpio_pin_write(config->exp1,dev->trigger_pin, 1);
			if (rc1 != 0) {
			PRINT("gpio_pin_write error %d!!\n", rc1);
				}
				k_sleep(10);
				
			rc1=gpio_pin_write(config->exp1,dev->trigger_pin, 0);
			if (rc1 != 0) {
			PRINT("gpio_pin_write error %d!!\n", rc1);
			}
			tsc1=_tsc_read();
				k_sleep(10);
	}
	return 0;

}



int sensor_chanel_get(struct device *port, uint32_t ticks, uint32_t *value)
{
	struct hcsr_dw_config *config = port->config->config_info;
	struct hcsr_dw_runtime * dev = port->driver_data;
	int rc1;

 
	
	if(dev->per_dev_buffer == 0)
	{	
		if(dev->ongoing !=1)
		{
			PRINT(" meassurement is in progress\n");
						
			
			
		
		dev->ongoing =1;
		rc1=gpio_pin_write(config->exp1,dev->trigger_pin, 1);
			if (rc1 != 0) {
			PRINT("gpio_pin_write error %d!!\n", rc1);
				}
				k_sleep(10);
			
			rc1=gpio_pin_write(config->exp1,dev->trigger_pin, 0);
			if (rc1 != 0) {
			PRINT("gpio_pin_write error %d!!\n", rc1);
			}
				k_sleep(10);
				
				if (k_sem_take(&dev->dev_sem, ticks)!= 1) //user
					{PRINT("not available");
					return -1;}
		*value = dev->per_dev_buffer;			
		}
	
		
	}

	else
	*value = dev->per_dev_buffer;
	
	k_sem_init(&dev->dev_sem,0,1);
	dev->per_dev_buffer=0;
	
	return 0;

}



int hcsr_dw_reset(struct device *port)
{
	struct hcsr_dw_config *config = port->config->config_info;
	struct hcsr_dw_runtime * const dev = port->driver_data;
	
	if (config->trigger_pin_config == CONFIG_GPIO_TRIGGER_PIN_1)
	{config->irq_pin = CONFIG_GPIO_INT_PIN_1;
	config->trigger_pin_config = CONFIG_GPIO_TRIGGER_PIN_1;
	config->echo_pin_config = CONFIG_GPIO_INT_PIN_1;
	}
	else if (config->trigger_pin_config == CONFIG_GPIO_TRIGGER_PIN_2)
	{
	config->irq_pin = CONFIG_GPIO_INT_PIN_2;
	config->trigger_pin_config = CONFIG_GPIO_TRIGGER_PIN_2;
	config->echo_pin_config = CONFIG_GPIO_INT_PIN_2;
	
	}
	
	k_sem_init(&dev->dev_sem,0,1);
	dev->trigger_pin = config->trigger_pin_config ;
	dev->echo_pin = config->echo_pin_config ;
	dev->ongoing = 0;
	dev->per_dev_buffer =0;
	
	
	
	return 0;
}


 struct hcsr04_driver_api api_funcs = {
	
	
	.reset = hcsr_dw_reset,
	.sample_fetch =  sensor_samples_fetch,
	.channel_get =  sensor_chanel_get,
 };



static struct device *int_pin_configure(struct device *port)
{
	
	struct device *int_pin_1;
	int rc1, ret;
	
	struct hcsr_dw_config *config = port->config->config_info;
	struct hcsr_dw_runtime * const dev = port->driver_data;
//PRINT("int_pin_configure  %d\n", config->irq_pin);
	int_pin_1 = device_get_binding(CONFIG_GPIO_DW_0_NAME);
	
	if (!int_pin_1) 
		{
		PRINT("DW GPIO not found!!\n");
		} 
		else {
		rc1 = gpio_pin_configure(int_pin_1,config->irq_pin , (GPIO_DIR_IN | GPIO_INT |  GPIO_INT_EDGE
	 | GPIO_INT_ACTIVE_HIGH| GPIO_INT_DEBOUNCE | GPIO_PUD_PULL_DOWN));


	if (rc1 != 0) {
			PRINT("Legacy GPIO config error %d!!\n", rc1);
		}
		}

	if(int_pin_1)
	{				
	gpio_init_callback(&dev->gpio_cb, gpio_callback, BIT(config->irq_pin));
	rc1 = gpio_add_callback(int_pin_1, &dev->gpio_cb);
	if (rc1) {
		PRINT("Cannot setup callback!\n");
	}



	rc1 = gpio_pin_enable_callback(int_pin_1, config->irq_pin);
	if (rc1) {
	PRINT("Error enabling callback!\n");
			}
	}
	return int_pin_1;
}


int hcsr_dw_initialize(struct device *port)
{
	int ret;
	

	struct hcsr_dw_config *config = port->config->config_info;
	struct hcsr_dw_runtime * const dev = port->driver_data;
	struct device *gpio;
	int rc1;
	config->int_pin_1 = int_pin_configure(port);
	dev->trigger_pin = config->trigger_pin_config ;
	dev->echo_pin = config->echo_pin_config ;
	dev->ongoing = 0;
	
	dev->per_dev_buffer =0;
	k_sem_init(&dev->dev_sem,0,1);
	if(dev->echo_pin == CONFIG_GPIO_INT_PIN_2)
	{	
		gpio = device_get_binding(CONFIG_PWM_PCA9685_0_DEV_NAME);
		if (!gpio) {
		PRINT("GPIO not found!!\n");
		} 
		else {
	
		 pinmux_pin_set(gpio, CONFIG_GPIO_MUX_PIN_2, 0);  	
		if (rc1 != 0){ 
			PRINT("GPIO config error %d!!\n", rc1);
		}
	
		}
	}
	
	config->exp1 = device_get_binding(CONFIG_GPIO_PCAL9535A_1_DEV_NAME);

	if(!config->exp1) 
	{
	PRINT("GPIO not found!!\n");
	} 

	ret = gpio_pin_configure(config->exp1,dev->trigger_pin, GPIO_DIR_OUT);
	if (ret != 0) {
	PRINT("GPIO config error %d!!\n", ret);
	}

	ret = gpio_pin_write(config->exp1,dev->trigger_pin, 0);
	if (ret != 0) {
			PRINT("GPIO write1 error %d!!\n", ret);
		}

	
	port->driver_api=&api_funcs;	
return 0;
}

struct hcsr_dw_config hcsr_config_0 = {
	.irq_pin = CONFIG_GPIO_INT_PIN_1,
	.trigger_pin_config = CONFIG_GPIO_TRIGGER_PIN_1,
	.echo_pin_config = CONFIG_GPIO_INT_PIN_1
};

struct hcsr_dw_config hcsr_config_1 = {
	.irq_pin = CONFIG_GPIO_INT_PIN_2,
	.trigger_pin_config = CONFIG_GPIO_TRIGGER_PIN_2,
	.echo_pin_config = CONFIG_GPIO_INT_PIN_2
};

struct hcsr_dw_runtime hcsr_0_runtime;
struct hcsr_dw_runtime hcsr_1_runtime;

DEVICE_INIT(HCSR0, CONFIG_HCSR04_DW_0_NAME, hcsr_dw_initialize,
				&hcsr_0_runtime, &hcsr_config_0,
				APPLICATION, CONFIG_HCSR04_DW_INIT_PRIORITY);   

DEVICE_INIT(HCSR1, CONFIG_HCSR04_DW_1_NAME, hcsr_dw_initialize,
				&hcsr_1_runtime, &hcsr_config_1,
				APPLICATION, CONFIG_HCSR04_DW_INIT_PRIORITY);  
