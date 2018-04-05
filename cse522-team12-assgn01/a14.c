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

typedef struct timespec tspec;

pthread_barrier_t barrier;
tspec at;
pthread_mutex_t mutex[10];
sem_t t_sem[10];
pthread_mutexattr_t mattr;
static int task_count = 0;
//static __thread int t_id;
sem_t start_at_same_time;
void *mouse_event(void *ptr);
int computation_function(int data);
void changemode(int);
int  kbhit(void);
sem_t r_click;
sem_t l_click;


void *aperiodic_task(void *arg){
	strtok(arg, " ");
	int event,iterations;
	struct sched_param param;
	param.sched_priority=atoi(strtok(NULL, " "));
	event=atoi(strtok(NULL, " "));
	iterations=atoi(strtok(NULL, " "));
	
	pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);	
	sem_wait(&start_at_same_time);		// to make sure that threads are triggered simultaneously 
	//pthread_barrier_wait(&barrier);

		while(1){
			if(event==1){//right click
				
				sem_wait(&l_click);
				computation_function(iterations);
				
			}
			else if(event == 0){//left click
				sem_wait(&r_click);
				computation_function(iterations);
				
			}
		}
	return NULL;
}
void *periodic_task(void *arg) {
	strtok(arg, " ");
	char *check,c;
	int period,iterations,lock_id;
	struct sched_param param;
	struct timeval tp;
	param.sched_priority=atoi(strtok(NULL, " "));
	period=atoi(strtok(NULL, " "));
	iterations=atoi(strtok(NULL, " "));
	pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);	
	
	suseconds_t rdtsc_t2,wait_time;
	float remtime;
	gettimeofday(&tp,NULL);
	wait_time=((tp.tv_sec%100)*1000000)+tp.tv_usec+(period*1000);
	sem_wait(&start_at_same_time);		// to make sure that threads are triggered simultaneously 
	//pthread_barrier_wait(&barrier);
	while(1){
	computation_function(iterations);
	gettimeofday(&tp,NULL);
	rdtsc_t2=((tp.tv_sec%100)*1000000)+tp.tv_usec;		// in micro secs
	if(rdtsc_t2>wait_time){
		double temp= ceil((rdtsc_t2-wait_time)/(period*1000.0));
		wait_time=wait_time + (temp)*(period*1000);
		continue;}
		remtime=(wait_time-rdtsc_t2);  //remtime in milisecond
		usleep((remtime));
		wait_time=wait_time+(period*1000);
		
		if ((check = strtok(NULL, " ")) != NULL) {
		c = check[0];
		if (c == 'L') {
			//printf("Lock variable: %s\n", check);
			lock_id=atoi(&check[1]);								
			pthread_mutex_lock(&mutex[lock_id]);
			
		} else if (c == 'U') {
			//printf("Unlock variable: %s\n", check);								
			lock_id=atoi(&check[1]);
			pthread_mutex_unlock(&mutex[lock_id]);
			
		} else {
			//printf("Iteration variable: %s\n", check);								
			iterations=atoi(check);

		} }
		else{
		break;}
	
		}
	
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
	int i=0;
	size_t len;
	cpu_set_t cpu_mask;
	CPU_ZERO(&cpu_mask);       // Initialize it all to 0,so that no multiple CPUs selected
	CPU_SET(1, &cpu_mask);  
	sched_setaffinity(0,sizeof(cpu_set_t),&cpu_mask);
	
	fp = stdin;

	if (fp == NULL){
		printf("null");
    		return 1;}
	sem_init(&l_click,0,0);
	sem_init(&r_click,0,0);
	sem_init(&start_at_same_time,0,0);
	pthread_mutexattr_init(&mattr);
	//pthread_mutexattr_setprotocol(&mattr,PTHREAD_PRIO_INHERIT);
	for(i=0;i<10;i++){	
	pthread_mutex_init(&mutex[i],&mattr);}
	
	for (i = 0; i <10; i++) {
		sem_init(&t_sem[i], 0, 0);}
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
						
			pthread_create(&pthrd[i],NULL,periodic_task,(void *)buf);	// for periodic task
		}
		else if(ch=='A'){

			
			pthread_create(&pthrd[i],NULL,aperiodic_task,(void *)buf);	//for aperodic task
		}
		
	}
	pthread_barrier_init(&barrier, NULL, (task_count + 1));
	
	for(i=0;i<total_count;i++){
	sem_post(&start_at_same_time);		// semaphore post
	}
	//pthread_barrier_wait(&barrier);
	usleep((execution_term)*1000);
	printf("Execution term of %d ms has ended",execution_term);
	for(i = 0; i < task_count; i++) {
    	pthread_cancel(pthrd[i]);
	}
    for(i = 0; i < task_count; i++) {
    	pthread_join(pthrd[i], NULL);
	}
	pthread_join(thread_mouse, NULL);
	free(line);
    return 0;
}


int computation_function(int data)
{	int x=data;
	int i = 0, j = 0;	
	for (i = 0; i < x; i++) {
		j += i;
	}
	//printf("j:%d\n",j);
	return 0;
}


void *mouse_event(void *ptr)
{
	int MouseEventFd;
	int policy;
	struct input_event MouseEvent;
	struct sched_param param;
	param.sched_priority=sched_get_priority_max(SCHED_FIFO);
	pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
	pthread_getschedparam(pthread_self(),&policy,&param);
	if((MouseEventFd = open(PATH_TO_MICE_FILE, O_RDONLY)) < 0)
	{
		perror("opening device");
		return NULL;
	}
	while(read(MouseEventFd, &MouseEvent, sizeof(struct input_event)))
	{
		if((MouseEvent.type == EV_KEY) && (MouseEvent.value == 0) && (MouseEvent.code == BTN_RIGHT))
		{
			
					//printf("\nright click detected\n");
					sem_post(&l_click);
		}
		if((MouseEvent.type == EV_KEY) && (MouseEvent.value == 0) && (MouseEvent.code == BTN_LEFT))
		{
			
					//printf("\nleft click detected\n");
					sem_post(&r_click);
					
		}
	}
	printf("\n Exiting the MouseHandlerThread");
return 0;
}
