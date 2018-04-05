#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<time.h>
#define UNUSED(expr) do { (void)(expr); } while (0)
#define OUTPUT_FILE "output.txt"
#define GENERATE_ST 'y'
struct task_data{
	double WCET,deadline,period;
	double priority;
};
struct task_set{
int task_count;
struct task_data t_d[25];
};
struct task{
int count;
struct task_set t_s[50];
};
struct task_set generate_tasks(int n_tasks,float utilisation,int dd);
struct task_set st;
struct task gt;
static const struct task_set EmptySet;
static const struct task Emptytask;
int count_edf[25];
int t_count_edf;

int count_rm[25];
int t_count_rm;
int count_dm[25];
int t_count_dm;
void UUnifast(int n, double U, double *a);
double cal_period(int i, int n_tasks);
void output_to_file(int count[], char *algo, int c, int d,int n);
double rand_range(double min, double max);
