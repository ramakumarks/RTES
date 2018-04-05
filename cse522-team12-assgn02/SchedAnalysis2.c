
#include "dataset.h"
/* function to generate tasksets based on number of tasks,utilization and deadline distribution*/
struct task_set generate_tasks(int n_tasks,float utilisation,int dd){
double t, c, d,a[n_tasks];
int i=0;
		
		UUnifast(n_tasks, utilisation, a);
		for(i = 0; i < n_tasks; i++) {
			t = cal_period(i, n_tasks);
			c = a[i] * t;//calculate wcet using utilization and period
			if(dd == 1) {
				d = rand_range(c, t);
			} else {
				d = rand_range(c + (t-c)/2.0, t);
			}
		st.t_d[i].WCET=c;
		st.t_d[i].deadline=d;
		st.t_d[i].period=t;
		}
		
		
return st;
}

void UUnifast(int n, double U, double *a) {
	double sumU = U, nextsumU = 0.0;
	int i = 0;
	
	for(i = 1; i < n; i++) {
		nextsumU = sumU * pow(drand48(), (double)1.0/((double)n - (double)i));
		a[i-1] = sumU - nextsumU;
		sumU = nextsumU;
		//printf("%lf\t", a[i-1]);
	}
	a[i-1] = sumU;
	//printf("%lf\n", a[i-1]);
}
double rand_range(double min, double max)
{
	return (min + rand() * (max-min)/ (RAND_MAX));
}
/*Calculate period taking random values*/ 
double cal_period(int i, int n_tasks) {
	
	if(i < n_tasks/2) {
		return (1000 + rand() / (RAND_MAX/(10000 - 1000 + 1) + 1));
	} else {
		return (10000 + rand() / (RAND_MAX/(100000 - 10000 + 1) + 1));
	}
}
/*function to write schedulability analysis log to output file*/
void output_to_file(int count[],char *algo, int c, int d,int n) {
	int i = 0;
	float u = 0.05;
	freopen(OUTPUT_FILE, "a", stdout);
	if(d == 1)
		printf("\n\n ALGORITHM %s D[C, T] with %d tasks\n", algo,n);
	else
		printf("\n\n ALGORITHM %s D[C+(T-C)/2, T] with %d tasks\n", algo,n);
	printf("U   \t\t S   \t Percentage \n");
	for(i = 0; i < 10; i++) {
		printf("%f,\t%d\t,%f\n", u, count[i]*100, (count[i]/(c * 50.0))*100.0);
		u += 0.1;
	}
	freopen("/dev/tty", "w", stdout);
}


