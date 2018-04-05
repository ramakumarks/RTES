
#include <zephyr.h>
#include <misc/printk.h>
#include <shell/shell.h>
#include <timestamp.h>
#include <tc_util.h>
#include <kernel.h>
#include <tc_util.h>
#include <ksched.h>
#include <device.h>
#include <pwm.h>
#include <board.h>
#include <gpio.h>
#include <kernel.h>
#include <misc/util.h>
#define STACK_SIZE 1024
#define SLEEP_TIME 	50
/* in clock cycles */
#define PERIOD	4096//(USEC_PER_SEC / 25)
#define PULL_UP 0
#define BUF_SIZE 500
/* in clock cycles */
#define EDGE GPIO_INT_ACTIVE_HIGH
u32_t start_time;
u32_t stop_time;
u32_t cycles_spent;
u32_t nanoseconds_spent;
u32_t tm_off;        /* time necessary to read the time */
int error_count;        /* track number of errors */
K_THREAD_STACK_DEFINE(test_thread_stack_area, STACK_SIZE);
static struct k_thread thread_data;
extern int coop_ctx_switch(void);
static K_THREAD_STACK_DEFINE(my_stack_area, STACK_SIZE);
static K_THREAD_STACK_DEFINE(my_stack_area_0, STACK_SIZE);
static struct k_thread thread_s;
static struct k_thread thread_r;
k_tid_t p_t;
k_tid_t c_t;
static struct gpio_callback gpio_cb;
K_MSGQ_DEFINE(my_msgq, sizeof(int), 10, 4);
uint32_t buffer[BUF_SIZE];
uint32_t buffer2[BUF_SIZE];


/**
 *
 *producer_thread
 *
 *this thread implements the prducer part for background computation,where it sends the message to consumer thread
 *
 * @return N/A
 */
void producer_thread(void)
{
   int data;
   int status=1;
    while (1) {
        /* create data item to send (e.g. measurement, timestamp, ...) */
        data = 100;
	
        /* send data to consumers */
        if(status == 0) {
            /* message queue is full: purge old data & try again */
            k_msgq_purge(&my_msgq);
        }
	status=k_msgq_put(&my_msgq, &data, 500);
	//k_sleep(500);
        /* data item was successfully added to message queue */
    }
}
/**
 *
 *consumer_thread
 *
 *this thread implements the consumer part for background computation,where it receives the message sent by producer thread
 *
 * @return N/A
 */
void consumer_thread(void)
{
    int data;

    while (1) {
        /* get a data item */
        k_msgq_get(&my_msgq, &data, K_FOREVER);
	//printk("received data: %d\n", data);
        /* process data item */
        
    }
}
/**
 *
 *test_thread
 *
 *test thread is used to measure average time taken to compute context switch 
 *
 * @return N/A
 */
void test_thread(void *arg1, void *arg2, void *arg3)
{
	

	bench_test_init();

	coop_ctx_switch();
	printk("Press Enter to give next shell command"); 

}
void isr(struct device *gpiob, struct gpio_callback *cb,u32_t pins)
{	stop_time = k_cycle_get_32();

	//printk("Interrupt detected at %d\n", k_cycle_get_32());
}
/**
 *
 * shell_cmd_m2
 *
 * shell module to implement the interrupt latency without background computation
 * signals are generated in pin 7 and read back from pin 2.The difference in time is measured with high precision
 * @return 0
 */

static int shell_cmd_m2(int argc, char *argv[])
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	struct device *pwm_dev;
	u32_t sum=0;
	u32_t i;
	//struct device *EXP0;
	//u32_t period = PERIOD;
	//int flag1;
	struct device *gpiob;
	struct device *dev2;
	int flag1,flag2;
	int cnt = 1;
	struct device *dev;
	printk("Checking interrupt latency without background computation \n");
	gpiob=device_get_binding(CONFIG_GPIO_SCH_1_DEV_NAME);
	if (!gpiob) {
		printk("Cannot find device!\n");
		return 0;
	}
	
	pwm_dev = device_get_binding(CONFIG_PINMUX_NAME);
	if (!pwm_dev) {
		printk("Cannot find device!\n");
		return 0;
	}
	dev = device_get_binding(CONFIG_GPIO_PCAL9535A_1_DEV_NAME);
	if (!dev) {
		printk("Cannot find device!\n");
		return 0;
	}
	dev2 = device_get_binding(CONFIG_GPIO_PCAL9535A_2_DEV_NAME);
	if (!dev2) {
		printk("Cannot find device!\n");
		return 0;
	}
	/* Set pin 7 as output */
	flag1 = gpio_pin_configure(dev, 6, GPIO_DIR_OUT);
	gpio_pin_write(dev, 6, 0);
	flag2 = gpio_pin_configure(dev, 7, GPIO_DIR_OUT);
	gpio_pin_write(dev, 7, 0);
	/* Set pin 2 as input */
	flag1 = gpio_pin_configure(dev, 8, GPIO_DIR_OUT);
	gpio_pin_write(dev, 8, 1);
	flag2 = gpio_pin_configure(dev, 9, GPIO_DIR_OUT);
	gpio_pin_write(dev, 9, 0);
	flag1 = gpio_pin_configure(pwm_dev, 13, GPIO_DIR_OUT);
	gpio_pin_write(pwm_dev, 13, 0);
	gpio_pin_write(dev, 6, 1);
	flag2=gpio_pin_configure(dev2, 13,GPIO_DIR_IN | GPIO_INT |  PULL_UP | EDGE);
	gpio_init_callback(&gpio_cb, isr, BIT(13));
	gpio_add_callback(dev2, &gpio_cb);
	gpio_pin_enable_callback(dev2, 13);
	while (cnt<500) {
		u32_t val = 0;
		gpio_pin_write(dev, 6, cnt % 2);
		start_time = k_cycle_get_32();
		
		while(val==0)
		gpio_pin_read(gpiob, 13, &val);
		stop_time = k_cycle_get_32();
		//k_sleep(SLEEP_TIME);
		cycles_spent = stop_time - start_time;
		nanoseconds_spent = SYS_CLOCK_HW_CYCLES_TO_NS(cycles_spent);
		buffer[cnt]=nanoseconds_spent;
		//printk("nanoseconds_spent= %d%d\n",nanoseconds_spent,cnt);
		cnt++;
		sum+=nanoseconds_spent;
	}
		//k_sleep(SLEEP_TIME);
		for(i=0;i<500;i++)
		printk("%d\n",buffer[i]);
		printk("average latency without background task= %d ns\n",sum/500);
		//pwm_pin_set_cycles(pwm_dev,1,period, period/2);
	return 0;
}
/**
 *
 * shell_cmd_m3
 *
 * shell module to implement the interrupt latency with background computation
 * signals are generated in pin 7 and read back from pin 2.The difference in time is measured with high precision 
 * @return 0
 */
static int shell_cmd_m3(int argc, char *argv[])
{	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	u32_t sum=0;
	struct device *pwm_dev;
	//u32_t period = PERIOD;
	u32_t i;
	struct device *gpiob;
	struct device *dev2;
	int flag1,flag2;
	int cnt = 0;
	struct device *dev;
	printk("Checking interrupt latency with background computation\n");
	/*creating 2 threads to send and receive messages*/ 
	p_t=k_thread_create(&thread_s, my_stack_area,STACK_SIZE,(k_thread_entry_t)producer_thread, NULL,NULL, NULL, 2 /*p*/, 0, K_NO_WAIT);
	c_t=k_thread_create(&thread_r, my_stack_area_0,STACK_SIZE,(k_thread_entry_t)consumer_thread, NULL,NULL, NULL, 1 /*p*/, 0,K_NO_WAIT);
	gpiob=device_get_binding(CONFIG_GPIO_SCH_1_DEV_NAME);
	pwm_dev = device_get_binding(CONFIG_PINMUX_NAME);
	if (!pwm_dev) {
		printk("Cannot find device!\n");
		return 0;
	}
	
	

	dev = device_get_binding(CONFIG_GPIO_PCAL9535A_1_DEV_NAME);
	dev2 = device_get_binding(CONFIG_GPIO_PCAL9535A_2_DEV_NAME);
	/* Set pin 7 as output */
	flag1 = gpio_pin_configure(dev, 6, GPIO_DIR_OUT);
	gpio_pin_write(dev, 6, 0);
	flag2 = gpio_pin_configure(dev, 7, GPIO_DIR_OUT);
	gpio_pin_write(dev, 7, 0);
	/* Set pin 2 as input */
	flag1 = gpio_pin_configure(dev, 8, GPIO_DIR_OUT);
	gpio_pin_write(dev, 8, 1);
	flag2 = gpio_pin_configure(dev, 9, GPIO_DIR_OUT);
	gpio_pin_write(dev, 9, 0);
	flag1 = gpio_pin_configure(pwm_dev, 13, GPIO_DIR_OUT);
	gpio_pin_write(pwm_dev, 13, 0);
	gpio_pin_write(dev, 6, 1);
	flag2=gpio_pin_configure(dev2, 13,GPIO_DIR_IN | GPIO_INT |  PULL_UP | EDGE);
	gpio_init_callback(&gpio_cb, isr, BIT(13));
	gpio_add_callback(dev2, &gpio_cb);
	gpio_pin_enable_callback(dev2, 13);
	while (cnt<500) {
		u32_t val = 0;
		gpio_pin_write(dev, 6, cnt % 2);
		start_time = k_cycle_get_32();
		
		while(val==0)
		gpio_pin_read(gpiob, 13, &val);
		stop_time = k_cycle_get_32();
		//k_sleep(SLEEP_TIME);
		cycles_spent = stop_time - start_time;
		nanoseconds_spent = SYS_CLOCK_HW_CYCLES_TO_NS(cycles_spent);
		//printk("nanoseconds_spent= %d%d\n",nanoseconds_spent,cnt);
		buffer2[cnt]=nanoseconds_spent;
		sum+=nanoseconds_spent;
		cnt++;
		//GPIO_INT_ACTIVE_HIGH
	}	
		//k_sleep(SLEEP_TIME);
		for(i=0;i<500;i++)
		printk("%d\n",buffer2[i]);
		printk("average latency with background task= %d ns\n",sum/500);
		//pwm_pin_set_cycles(pwm_dev,1,period, period/2);
	//k_thread_abort(c_t);
	return 0;
}
/**
 *
 * shell_cmd_m1
 *
 * shell module to implement the context switch overhead.It call a thread which creates threads to implement context switch.
 *
 * @return 0
 */
static int shell_cmd_m1(int argc, char *argv[])
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	c_t=k_thread_create(&thread_data,test_thread_stack_area,STACK_SIZE,test_thread, NULL, NULL, NULL,10, 0, K_NO_WAIT);
	return 0;
}
/*define the shell command*/
#define MY_SHELL_MODULE "measure"
/*define sub commands to choose specific shell modules*/
static struct shell_cmd commands[] = {
	{ "1", shell_cmd_m1, NULL },
	{ "2", shell_cmd_m2, NULL },
	{ "3", shell_cmd_m3, NULL}
};


void main(void)
{
	SHELL_REGISTER(MY_SHELL_MODULE, commands);
}
