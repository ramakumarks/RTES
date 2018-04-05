#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <linux/input.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>
#include <errno.h>
#include<math.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include<string.h>
#include<semaphore.h>
#include <termios.h>
#include <sys/time.h>
#include<ctype.h>
#include <sys/syscall.h>  
#define PATH_TO_MICE_FILE "/dev/input/event4"
#define PI 1

typedef struct timespec tspec;
tspec at;
pthread_mutex_t mutex[10];
sem_t t_sem[10];
pthread_mutexattr_t mattr;
static int task_count = 0;
sem_t start_at_same_time;
void *mouse_event(void *ptr);
int computation_function(int data);
void changemode(int);
int  kbhit(void);
sem_t r_click;
sem_t l_click;
/*function to convert type from integers to tspec*/
tspec tspec_conv(long tu)
{
    tspec t;
    long mm = tu % 1000;

    t.tv_sec = tu / 1000;
    t.tv_nsec = mm * 1000000;

    return t;
}
/*function to add two timespec values*/
tspec tspec_add(const tspec *a, const tspec *b) {
	tspec s;
	s.tv_nsec = a->tv_nsec + b->tv_nsec;
	s.tv_sec = a->tv_sec + b->tv_sec;
	
	while(s.tv_nsec >= 1000000000) {
		s.tv_nsec = s.tv_nsec - 1000000000;
		s.tv_sec+= 1;
	}
	
	return s;
}
/*aperiodic thread*/
void *aperiodic_task(void *arg){
	strtok(arg, " ");
	int event,iterations;
	struct sched_param param;
	param.sched_priority=atoi(strtok(NULL, " "));
	event=atoi(strtok(NULL, " "));
	iterations=atoi(strtok(NULL, " "));
	/*tasks are scheduled under real-time policy FIFO*/
	pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);	
	sem_wait(&start_at_same_time);		
	/* to make sure that threads are triggered simultaneously*/	
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		while(1){
			/*check for right click event*/
			if(event==1){
				
				sem_wait(&l_click);
				computation_function(iterations);
			}
			/*check for left click event*/
			else if(event == 0){
				sem_wait(&r_click);
				computation_function(iterations);
			}
		
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		}
	pthread_exit(0);
	return 0;
}
/*periodic thread*/
void *periodic_task(void *arg) {
	strtok(arg, " ");
	char *check,c;
	int period,iterations,lock_id;
	struct sched_param param;
	param.sched_priority=atoi(strtok(NULL, " "));
	period=atoi(strtok(NULL, " "));
	iterations=atoi(strtok(NULL, " "));
	/*tasks are scheduled under real-time policy FIFO*/
	pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);	
	
	clock_gettime(CLOCK_MONOTONIC, &at);

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	/* to make sure that threads are triggered simultaneously*/
	sem_wait(&start_at_same_time);	 
	tspec p;
	p=tspec_conv(period);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	while(1){
		computation_function(iterations);
		/*wait till the period*/
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &at, NULL);
		at = tspec_add(&at, &p);
		
		if ((check = strtok(NULL, " ")) != NULL) {
		c = check[0];
		if (c == 'L') {
			printf("Lock variable: %s\n", check);
			lock_id=atoi(&check[1]);								
			pthread_mutex_lock(&mutex[lock_id]);
			
		} else if (c == 'U') {
			printf("Unlock variable: %s\n", check);								
			lock_id=atoi(&check[1]);
			pthread_mutex_unlock(&mutex[lock_id]);
			
		} else {
			printf("Iteration variable: %s\n", check);								
			iterations=atoi(check);

		} }
		else{
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		break;
			}
	
		}
	pthread_exit(0);
	return 0;
}
int main()
{ 
	FILE *fp;
	char *line = NULL;
	size_t n = 0;
	int total_count,execution_term;
	char ch;
	char* buf;
	int s,i=0;
	size_t len;

	 /* Initialize it all to 0,so that no multiple CPUs selected*/
	cpu_set_t cpu_mask;
	CPU_ZERO(&cpu_mask);      
	CPU_SET(1, &cpu_mask);  
	sched_setaffinity(0,sizeof(cpu_set_t),&cpu_mask);
	
	fp = stdin;

	if (fp == NULL){
		printf("null");
    		return 1;}
	/*initialize semaphores for mouse clicks*/
	sem_init(&l_click,0,0);
	sem_init(&r_click,0,0);
	/*semaphore initialized to start all the tasks at same time*/
	sem_init(&start_at_same_time,0,0);
	pthread_mutexattr_init(&mattr);
	/* To check if Priority inheritance is enabled*/ 
	if(PI==1)
	pthread_mutexattr_setprotocol(&mattr,PTHREAD_PRIO_INHERIT);
	/*10 mutex are initialized*/
	for(i=0;i<10;i++){	
	pthread_mutex_init(&mutex[i],&mattr);}
	
	for (i = 0; i <10; i++) {
		sem_init(&t_sem[i], 0, 0);}
	/* A thread is used to read mouse event*/
	pthread_t thread_mouse;
	pthread_create(&thread_mouse,NULL, mouse_event, NULL);
	
	getline(&line, &n, fp);
	fseek(fp,0,SEEK_SET);
	fscanf(fp,"%d %d\n",&total_count,&execution_term);
	pthread_t pthrd[total_count];
	printf("total number of tasks:%d\ntotal execution time: %dms\n",total_count,execution_term);
	i=0;
	while ((len = getline(&line, &n, fp)) != -1){
	buf= (char *)calloc(100,sizeof(char));
	i++;
	strcpy(buf,line);
		ch=buf[0];
		task_count++;
		//printf("%c",ch);
		if(ch=='P'){
			/*threads for periodic tasks*/			
			pthread_create(&pthrd[i],NULL,periodic_task,(void *)buf);	
		}
		else if(ch=='A'){

			/*threads for aperiodic tasks*/	
			pthread_create(&pthrd[i],NULL,aperiodic_task,(void *)buf);	
		}
		
	}
	free(line);
	/*All tasks are started at sme time*/
	for(i=0;i<total_count;i++){
	sem_post(&start_at_same_time);		// semaphore post
	}
	/*the task terminates after total execution time*/
	usleep((execution_term)*1000);
	printf("Execution term of %d ms has ended\n",execution_term);
	pthread_cancel(thread_mouse);
	for(i = 0; i < task_count; i++) {
    	pthread_cancel(pthrd[i]);
	}
	
    	for(i = 0; i < task_count; i++) {
	pthread_tryjoin_np(pthrd[i], NULL);
	}
	s=pthread_tryjoin_np(thread_mouse, NULL);
	if (s != 0) {
         printf("Error while joining" );
         }
    return 0;
}

/*busy loop function*/
int computation_function(int data)
{	int x=data;
	int i = 0, j = 0;	
	for (i = 0; i < x; i++) {
		j += i;
	}
	printf("The value of j after busy loop:%d\n",j);
	return 0;
}

/*thread to read mouse event*/
void *mouse_event(void *ptr)
{
	int MouseEventFd;
	int policy;
	struct input_event MouseEvent;
	struct sched_param param;
	param.sched_priority=sched_get_priority_max(SCHED_FIFO);
	pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
	pthread_getschedparam(pthread_self(),&policy,&param);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if((MouseEventFd = open(PATH_TO_MICE_FILE, O_RDONLY)) < 0)
	{
		perror("opening device");
		return NULL;
	}
	while(read(MouseEventFd, &MouseEvent, sizeof(struct input_event)))
	{
		if((MouseEvent.type == EV_KEY) && (MouseEvent.value == 0) && (MouseEvent.code == BTN_RIGHT))
		{
			
					printf("\nright click detected\n");
					sem_post(&l_click);
		}
		if((MouseEvent.type == EV_KEY) && (MouseEvent.value == 0) && (MouseEvent.code == BTN_LEFT))
		{
			
					printf("\nleft click detected\n");
					sem_post(&r_click);
					
		}
	
	}
	printf("\n Exiting the MouseHandlerThread");
	pthread_exit(0);
return 0;
}
