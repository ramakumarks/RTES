#include "timestamp.h"
#include <misc/printk.h>
#include <arch/cpu.h>

/* number of context switches */
#define NCTXSWITCH   500//10000
#ifndef STACKSIZE
#define STACKSIZE    512
#endif


/* stack used by the threads */
static K_THREAD_STACK_DEFINE(thread_one_stack, STACKSIZE);
static K_THREAD_STACK_DEFINE(thread_two_stack, STACKSIZE);
static struct k_thread thread_one_data;
static struct k_thread thread_two_data;

static u32_t timestamp;

/* context switches counter */
static volatile u32_t ctx_switch_counter;

/* context switch balancer. Incremented by one thread, decremented by another*/
static volatile int ctx_switch_balancer;

K_SEM_DEFINE(sync_sema, 0, 1);

/**
 *
 * thread_one
 *
 *The context switch takes place with the help of k_yield 
 *
 * @return N/A
 */
static void thread_one(void)
{
	k_sem_take(&sync_sema, K_FOREVER);
	timestamp = TIME_STAMP_DELTA_GET(0);
	while (ctx_switch_counter < NCTXSWITCH) {
		k_yield();
		ctx_switch_counter++;
		ctx_switch_balancer--;
	}
	timestamp = TIME_STAMP_DELTA_GET(timestamp);
}

/**
 *thread two
 * 
 *@return N/A
 */
static void thread_two(void)
{
	k_sem_give(&sync_sema);
	while (ctx_switch_counter < NCTXSWITCH) {
		k_yield();
		ctx_switch_counter++;
		ctx_switch_balancer++;
	}
}

/**
 *function to create threads and measure the average time taken for 500 context switches 
 *
 *
 * @return 0 on success
 */
int coop_ctx_switch(void)
{
	printk(" time for 500 context switches is\n");
	ctx_switch_counter = 0;
	ctx_switch_balancer = 0;

	bench_test_start();
	k_thread_create(&thread_one_data, thread_one_stack, STACKSIZE,
			(k_thread_entry_t) thread_one, NULL, NULL, NULL,
			6, 0, K_NO_WAIT);
	k_thread_create(&thread_two_data, thread_two_stack, STACKSIZE,
			(k_thread_entry_t) thread_two, NULL, NULL, NULL,
			6, 0, K_NO_WAIT);

	if (ctx_switch_balancer > 3 || ctx_switch_balancer < -3) {
		printk(" Balance is %d. FAILED", ctx_switch_balancer);
	}
	 else {
		printk(" context switch time is %u tcs = %u"
			     " nsec\n",
			     timestamp ,
			     SYS_CLOCK_HW_CYCLES_TO_NS(timestamp));//SYS_CLOCK_HW_CYCLES_TO_NS_AVG(timestamp,ctx_switch_counter));/// 
		printk(" Average context switch time is %u tcs = %u"
			     " nsec\n",
			     timestamp/ctx_switch_counter ,
			     SYS_CLOCK_HW_CYCLES_TO_NS_AVG(timestamp,ctx_switch_counter));
	}

	return 0;
}
