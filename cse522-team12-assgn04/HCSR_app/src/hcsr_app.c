#include <kernel.h>
#include <board.h>
#include <device.h>
#include <init.h>
#include <shell/shell.h>
#include <zephyr.h>
#include <i2c.h>
#include <gpio.h>
#include <pinmux.h>
#include <hcsr04.h>
#include <sensor.h>
#include <string.h>
#include <misc/printk.h>
#include <eeprom_24fc256.h>
#define APP_SHELL_MODULE "hcsr"


#define DEV_OK  0
#define I2C_SLV_ADDR	0x54
#define STACKSIZE 2000

struct device *gpio;
struct device *gpio_sus;
struct device *i2c_dev, *i2c;
struct device *pinmux;
struct device *exp0;
struct device *hcsr04;



int p;
int p1;
int p2;

int str_to_int(char *str)
{	
	int  i, len;
	int res=0;
	len = strlen(str);
	for(i=0; i<len; i++){
		res = res * 10 + ( str[i] - '0' );
	}
	return res;	
}

static int shell_cmd_enable(int argc, char *argv[])
{
	int enable = str_to_int(argv[1]);
	printk("enable = %d\n", enable);
	if(enable==0)
	{
		printk("None of the devices are enabled\n");
	}
	else if(enable==1)
	{	
		printk("HCSR0 is enabled\n");
		enable=1;
		hcsr04 = device_get_binding(CONFIG_HCSR04_DW_0_NAME);
		if(!hcsr04)
		printk("device is not binding\n");
		
		
			}
	else if(enable==2)
	{	
		printk("HCSR1 is enabled\n");
		enable=2;
		hcsr04 = device_get_binding(CONFIG_HCSR04_DW_1_NAME);
		if(!hcsr04)
		printk("device is not binding\n");
		
	}
	else
		printk("Enter either 0,1 or 2\n");
return 0;
}


static int shell_cmd_start(int argc, char *argv[])
{	int rc1;
	uint32_t val;
	int i;
	i=0;
	 p= str_to_int(argv[1]);
	 while(i<p){
		
		rc1 =sensor_samples_fetch(hcsr04, CONFIG_GPIO_INT_PIN_1);
		i+=1;
		k_sleep(20);
	
		rc1=sensor_chanel_get(hcsr04, 7, &val);
		printk("distance  = %d cm\n", val);
	}

return 0;
	
}

static int shell_cmd_dump(int argc, char *argv[])
{
	printk("dump\n");
	p1=str_to_int(argv[1]);
	p2=str_to_int(argv[2]);
	
	
return 0;
}



static struct shell_cmd commands[] = {
	{"enable",  shell_cmd_enable, "enable n (n=0, 1, or 2, to enable none, HCSR0, or HCSR1)"},
	{"start", shell_cmd_start , "start p where p<=512 "},
	{"dump", shell_cmd_dump, "dump p1 p2, where p1<p2.It will print out the distance measures recorded in pages p1 to p2 on console"},
	{NULL,NULL}
};
void main(void) {
 
	SHELL_REGISTER(APP_SHELL_MODULE, commands);

	
}
