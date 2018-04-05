#include "dataset.h"
#define MN 50

/*test for condition if deadline < period*/
int preutilisation_test(int i,struct task *t) {
	int j;
	for(j=0;j<t->t_s[i].task_count;j++){
		if(t->t_s[i].t_d[j].deadline<t->t_s[i].t_d[j].period)
		//printf("%0.2lf\t%d\n",t->t_s[i].t_d[j].deadline,t->t_s[i].t_d[j].period);
		return -1;
	}

	return 1;
}
/*function to Test utilization*/
double utilisation_test(int i,struct task *t, int dens_check) {
	int j;
	double U = 0.0;
	//struct task_data temp;
	for(j=0;j<t->t_s[i].task_count;j++)
	{
		if(dens_check == 0)
			U += t->t_s[i].t_d[j].WCET/t->t_s[i].t_d[j].period;
		else
			U += t->t_s[i].t_d[j].WCET/(t->t_s[i].t_d[j].period> t->t_s[i].t_d[j].deadline ? t->t_s[i].t_d[j].deadline:t->t_s[i].t_d[j].period);
		//printf("%lf\n",U);	
	}
	
	//printf("Utilization : %lf\n",U);
	//UNUSED(U);
	return U;	
}
/*Function to get Urm value*/
double utilisation_value(int n) {
	return (n * (pow(2.0, 1.0/n) - 1));
}
/*Sort task based on priority*/
struct task sort_task(int i,struct task *t,int sp){
int k,j,min;
struct task_data temp;

	for(j=0;j<t->t_s[i].task_count;j++)
	{
		switch(sp)
		{
		case 0:
			t->t_s[i].t_d[j].priority = t->t_s[i].t_d[j].period;
			break;
		case 1:
			t->t_s[i].t_d[j].priority = t->t_s[i].t_d[j].deadline;
			break;
		}
	}

	for(k=0;k<t->t_s[i].task_count;k++)
	{
		min = k;
		for(j=k+1; j<t->t_s[i].task_count; j++)
		{
			if(t->t_s[i].t_d[min].priority > t->t_s[i].t_d[j].priority)
			{
				min = j;
			}
		}
		temp = t->t_s[i].t_d[min];
		t->t_s[i].t_d[min] = t->t_s[i].t_d[k];
		t->t_s[i].t_d[k] = temp;
	}


return *t;
}
long calculate_busyperiod(int i,struct task *t) {
	long prevsum = 0, cursum = 0;
	int k;
		
	for(k=0;k<t->t_s[i].task_count;k++){
			prevsum += t->t_s[i].t_d[k].WCET;
			}

	while(1) {
		cursum = 0;		
		for(k=0;k<t->t_s[i].task_count;k++){
			cursum += ceil(prevsum/t->t_s[i].t_d[k].period) * t->t_s[i].t_d[k].WCET;
			//printf("Cur: %ld \t", cursum);
			}
		//printf("\nP: %ld  C: %ld\n", prevsum, cursum);
		//printf("ok");
		if(prevsum == cursum || cursum < 0) 
			{
			
			return cursum;
		}
		else
			prevsum = cursum;	
	}
}
double h_cal(int i,struct task *t, int tm) {
	int k = 0,j=0, l;
	
	double h = 0.0;
	for(k=0;k<t->t_s[i].task_count;k++) {
		while(1) {
			l =  (t->t_s[i].t_d[k].period * j) + t->t_s[i].t_d[k].deadline;
			if(l > tm) break;
			h += t->t_s[i].t_d[k].WCET;
			j++;	
		}
		j = 0;
		
	}
	
	return h;
}

int test_load_factor(int i,struct task *t, long l_factor) {
	long j = 0, tm;
	double h, u;
	int k;
	for(k=0;k<t->t_s[i].task_count;k++){ 
		while(1) {
			tm = (t->t_s[i].t_d[k].period * j) + t->t_s[i].t_d[k].deadline;
			h = h_cal(i,t, tm);
			u = h/tm;
			//printf("t: %ld  h:%lf  u: %lf\n", t, h, u);
			if(tm > l_factor) break;
			if(u > 1.0) return -1;
			j++;
		}
		
		j = 0;
	}
	return 1;
}


int rt_analysis(int i,struct task *t, int check) {
	int j,k, flag = 1;
	double U;
	int prevsum = 0, cursum = 0;
	for(j=1; j<t->t_s[i].task_count; j++){
		U=0.0;
		for(k=0;k<j;k++){
			if(check == 0)
				U += t->t_s[i].t_d[k].WCET/t->t_s[i].t_d[k].period;
			else
				U += t->t_s[i].t_d[k].WCET/(t->t_s[i].t_d[k].period> t->t_s[i].t_d[k].deadline ? t->t_s[i].t_d[k].deadline:t->t_s[i].t_d[k].period);
			//printf("%lf\n",U);	
		}
	
		if(U <= utilisation_value(j)) {
			//count_rm[t_count_rm]++;

			printf("RT analysis for task set %d: Schedulable U for RT with %d jobs\n",i, j);
		} else {
			
			for(k=0;k<j;k++){
			prevsum += t->t_s[i].t_d[k].WCET;
			}
			while(1) {
	 		
			cursum = 0;

			for(k=0;k<j;k++){
			cursum += ceil(prevsum/t->t_s[i].t_d[k].period) * t->t_s[i].t_d[k].WCET;
			}
			cursum +=t->t_s[i].t_d[j].WCET;	

			//printf("\nP: %d  C: %d\n", prevsum, cursum);
			if(prevsum == cursum) { 
			if(cursum < (t->t_s[i].t_d[j].period>t->t_s[i].t_d[j].deadline?t->t_s[i].t_d[j].deadline:t->t_s[i].t_d[j].period) && (cursum > 0)) {
				printf("RT analysis for task set %d:\nRT Schedulable for job %d is: %d\n",i, j, cursum);
			} else {
				flag = 0;
				if (cursum > 0)
					printf("RT analysis for task set %d:\n Not RT Schedulable for job %d\n",i, j);
				else 
					printf("RT analysis for task set %d:\n Not RT Schedulable for job doesn't exists\n",i);
			}
			break;
			} else {
			prevsum = cursum;
			}
		}
		}
	}
	
	return flag;
}
/*function to test EDF Scheduling Algorithm*/
void EDF_test(struct task *t){
int i;
double U = 0.0;
long l_factor;
UNUSED(l_factor);
//printf("%d\n",t->count);
for(i=0;i<t->count;i++){
	if(preutilisation_test(i,t)==1){
		U=utilisation_test(i,t,0);
		if(U <= 1.0) {
		count_edf[t_count_edf]++;

		printf("Utilization Based test for EDF:\n Task set %d is EDF schedulable\n",i); 

		}
		else
		printf("Utilisation Based test for EDF:\n Task set %d is not EDF Scheduable\n", i);
	}
	else{
		U=utilisation_test(i,t,1);
		printf("Density Test\n");
		if(U <= 1.0) {
		count_edf[t_count_edf]++;

		printf("Utilization Basedtest for EDF:\n Task set %d is EDF schedulable\n",i); 

		}
		else{
		printf("Testing Loading factor for taskset %d\n",i);
		l_factor=calculate_busyperiod(i,t);
		if((test_load_factor(i,t, l_factor) == 1) && (l_factor > 0)) {
										
					count_edf[t_count_edf]++;
					
					printf("Task set %d is EDF Schedulable with load factor: %ld\n", i, l_factor);
				} else {
					if(l_factor > 0)
						printf("Task set %d is EDF not Schedulable with load factor: %ld\n", i, l_factor);
					else
						printf("Task set %d is EDF not Schedulable load factor doesn't exists\n", i);	
				}
		
		}
	}
}

}
/*function to test RM Scheduling Algorithm*/
void RM_test(struct task *t){
int i=0;
double U = 0.0,N=0.0;
int flag;
UNUSED(flag);

for(i=0;i<t->count;i++){
	if(preutilisation_test(i,t)==1){
		U=utilisation_test(i,t,0);
		N=utilisation_value(t->t_s[i].task_count);
		if(U <= N) {
		count_rm[t_count_rm]++;
		printf("Utilization test for RM:\n Task set %d is RM schedulable\n",i); 
		}
		else if(U > 1.0)
		printf("Utilisation test for RM:\n Task set %d is not RM Scheduable\n", i);
		else{
		*t=sort_task(i,t,0);
		flag=rt_analysis(i,t,0);
		if(flag==1)count_rm[t_count_rm]++;
		}
	}
	else{
		U=utilisation_test(i,t,1);
		N=utilisation_value(t->t_s[i].task_count);
		if(U <= N) {
		count_rm[t_count_rm]++;
		printf("Utilization test for RM:\n Task set %d is RM schedulable\n",i); 
		}
		else{
		*t=sort_task(i,t,0);
		flag=rt_analysis(i,t,1);
		if(flag==1)count_rm[t_count_rm]++;
		}
	}
}
}
/*function to test DM Scheduling Algorithm*/
void DM_test(struct task *t){
int i=0;
double U = 0.0,N=0.0;
int flag;
UNUSED(flag);

for(i=0;i<t->count;i++){
	if(preutilisation_test(i,t)==1){
		U=utilisation_test(i,t,0);
		N=utilisation_value(t->t_s[i].task_count);
		if(U <= N) {
		printf("Utilization test for DM:\n Task set %d is DM schedulable\n",i); 
		count_dm[t_count_dm]++;
		}
		else if(U > 1.0)
		printf("Utilisation test for DM:\n Task set %d is not DM Scheduable\n", i);
		else{
		*t=sort_task(i,t,1);
		flag=rt_analysis(i,t,0);
		if(flag==1)count_dm[t_count_dm]++;
		}
	}
	else{
		U=utilisation_test(i,t,1);
		N=utilisation_value(t->t_s[i].task_count);
		if(U <= N) {
		count_dm[t_count_dm]++;
		printf("Utilization test for DM:\n Task set %d is DM schedulable\n",i); 
		}
		else{
		*t=sort_task(i,t,1);
		flag=rt_analysis(i,t,1);
		if(flag==1)count_dm[t_count_dm]++;
		}
	}
}
}
/*Function to reset count values*/
void reset_counts() {
	memset(count_edf, 0, sizeof(int) * 25);
	memset(count_rm, 0, sizeof(int) * 25);
	memset(count_dm, 0, sizeof(int) * 25);
	
}
/*Function to reset the values in task structure to empty structure*/
void reset_taskvals(){
gt=Emptytask;
st=EmptySet;
}

int main()
{ 
	FILE *fp;
	char *line = NULL;
	size_t n = 0;
	char ch;
	char* buf1;
	int i,j;
	int n_tasks,dl_dist;
	float u;
	size_t len;
	fp = stdin;
	char* b_char;
	char* c_char;
	int count=0;
	struct task t;
	if (fp == NULL){
		printf("null");
    		return 1;}
	/*start reading data from input file*/
	getline(&line, &n, fp);
	fseek(fp,0,SEEK_SET);
	fscanf(fp,"%d\n",&t.count);
	printf("total number of task sets :%d\n",t.count);
	for(i=0;i<t.count;i++){
		len = getline(&line, &n, fp);
		if(len){
		buf1= (char *)calloc(100,sizeof(char));
		strcpy(buf1,line);
		ch=buf1[0];
		t.t_s[i].task_count=ch-'0';
		for(j=0;j<t.t_s[i].task_count;j++){
		len = getline(&line, &n, fp);
		
		c_char=strtok(line, " ");
		b_char=strtok(NULL, " ");
		t.t_s[i].t_d[j].WCET=atof(c_char);
		t.t_s[i].t_d[j].deadline=atof(b_char);
		t.t_s[i].t_d[j].period=atof(strtok(NULL, " "));
		}
		}
	}
	for(i=0;i<t.count;i++){
	printf("%d\n",t.t_s[i].task_count);
	for(j=0;j<t.t_s[i].task_count;j++){
	printf("%0.2lf\t",t.t_s[i].t_d[j].WCET);
		printf("%0.2lf\t",t.t_s[i].t_d[j].deadline);
		printf("%0.2lf\n",t.t_s[i].t_d[j].period);}

}	
	/*The tasks and task sets data is stored in structure t ,pass this structure to each testing algorithms*/
	printf("=========schedulability test for Earliest Deadline First scheduling algorithm========\n");
	EDF_test(&t);
	printf("===========schedulability test for Rate Monotonic scheduling algorithm===============\n");
	RM_test(&t);
	printf("=========schedulability test for Deadline Monotonic scheduling algorithm=============\n");
	DM_test(&t);
	reset_counts();
	/*Generate synthetic taskset*/
	if(GENERATE_ST == 'y' || GENERATE_ST == 'Y') {
		gt.count=1;
	/* Loop for the deadline distributions of [Ci+(Ti-Ci)/2,,Ti] and [Ci,,Ti]*/
	for(dl_dist=0;dl_dist<2;dl_dist++){

	/*Loop for tasks 10 and 25*/
	for(n_tasks=10;n_tasks<26;n_tasks+=15){
	
	t_count_edf = 0;
	t_count_rm = 0;
	t_count_dm = 0;
	/*utilization ranges from 0.05 to 0.95 with 0.1 step*/
	for(u=0.05; u<1; u+=.1){
	/*Loop for each number of tassets*/
			for(i=0; i<MN; i++){
				/*get tasksets from generate_tasks function based on number of tasks,utilization and deadline distribution*/
				 gt.t_s[0]=generate_tasks(n_tasks, u, dl_dist);
				 gt.count=1;
				 gt.t_s[0].task_count=n_tasks;
				
				 EDF_test(&gt);
				 
				 RM_test(&gt);
				 
				 DM_test(&gt);
				 
				 reset_taskvals();
				 
			}
		t_count_edf++;
		t_count_rm++;
		t_count_dm++;	
	}
	
	count++;
	output_to_file(count_edf, "EDF", count, dl_dist,n_tasks);
	output_to_file(count_rm, "RM", count, dl_dist,n_tasks);
	output_to_file(count_dm, "DM", count, dl_dist,n_tasks);
	reset_counts();
	count=0;
	}
	}
	} else {
		printf("End Program\n");
	}
	/*printf("%d\t",gt.count);
	printf("%0.2lf\t",gt.t_s[0].t_d[0].WCET);
	printf("%0.2lf\t",gt.t_s[0].t_d[0].deadline);
	printf("%0.2lf\n",gt.t_s[0].t_d[0].period);*/
	free(line);
    return 0;
}


