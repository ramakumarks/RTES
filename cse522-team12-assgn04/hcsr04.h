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



struct hcsr_dw_runtime {
	int time;
	
	uint32_t per_dev_buffer;
	char buffer[64];
	uint32_t trigger_pin;
	uint32_t echo_pin;
	int ongoing;
	struct gpio_callback gpio_cb;
	struct k_sem dev_sem;
};

/*config data*/

struct hcsr_dw_config {
uint32_t irq_pin;
uint32_t trigger_pin_config;
uint32_t echo_pin_config;
struct device *int_pin_1;
struct device *exp1;
};

int hcsr_dw_initialize(struct device *port);

void gpio_callback(struct device *port,struct gpio_callback *cb, uint32_t pins);

int sensor_samples_fetch(struct device *port, uint32_t pin);

int sensor_chanel_get(struct device *port,uint32_t ticks, uint32_t *value);

int hcsr_dw_reset(struct device *port);

/*
typedef int (*hcsr_dw_reset)(struct device* port);
typedef int (*hcsr_dw_read)(struct device* port,uint32_t ticks, uint32_t *value);
typedef int (*hcsr_dw_write)(struct device* port);*/

struct hcsr04_driver_api{
	int (*reset)(struct device* port);
int (*channel_get )(struct device* port,uint32_t ticks, uint32_t *value);//read
int (*sample_fetch)(struct device* port);//write
};