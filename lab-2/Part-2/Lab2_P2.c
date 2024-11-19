#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#define MY_PRIORITY 51  // kernel is priority 50

pthread_t thread_reader1, thread_reader2, thread_writer;
char buf[100];		// buffer
char c[20][100];	// store the reading
int line = -3; 		// total lines from reading
			// set line = -3, because the when just load thread1 and thread2, the order has some problem
FILE *fptr1, *fptr2;	// file reading pointers
struct timeval start, end; // timer

// thread function for reading 1
void* reader1() {
	char fn1[15] = "first.txt";
	fptr1 = fopen(fn1, "r");	
	int timer_fd1;
	timer_fd1 = timerfd_create(CLOCK_MONOTONIC, 0);
	if(timer_fd1 == -1) printf("ERROR: timerfd_create1\n");
	struct itimerspec itval;
	// set period time
	itval.it_interval.tv_sec = 1;
	itval.it_interval.tv_nsec = 10;
	// set start time
	itval.it_value.tv_sec = 0;
	itval.it_value.tv_nsec = 250000000;
	// start the timer
	if(timerfd_settime(timer_fd1, 0, &itval, NULL) == -1) printf("ERROR: timeset1\n");

	// initialize current thread as Real Time
	struct sched_param param;
	// set priority
	param.sched_priority = MY_PRIORITY;

	// wait to timer expire
	uint64_t num_periods = 0;
	ssize_t rr;
	int i;
	// scheduling
	sched_setscheduler(0, SCHED_FIFO, &param);
	for (i =0; i < 10; i++){
		line += 1;
		if (line >=0 ){
		printf("thread1 read line %d\n", line);
		fgets(buf, 100, fptr1);
		}
		rr = read(timer_fd1, &num_periods, sizeof(num_periods));
		//if (rr == -1) printf("ERROR: read1\n");
		if(num_periods > 1) {
	  	  	puts("MISSED WINDOW");
	       		exit(1);
	  	}
	}
	fclose(fptr1);
	pthread_exit(NULL);
}

// thread function for reading 2
void* reader2() {
	char fn2[15] = "second.txt";
	fptr2 = fopen(fn2, "r");
	int timer_fd2;
	timer_fd2 = timerfd_create(CLOCK_MONOTONIC, 0);
	if(timer_fd2 == -1) printf("ERROR: timerfd_create2\n");
	struct itimerspec itval;
	// set period time
	itval.it_interval.tv_sec = 1;
	itval.it_interval.tv_nsec = 10;
	// set start time
	itval.it_value.tv_sec = 0;
	itval.it_value.tv_nsec = 750000000;
	// start the timer
	if(timerfd_settime(timer_fd2, 0, &itval, NULL) == -1) printf("ERROR: timeset2\n");

	// initialize current thread as Real Time
	struct sched_param param;
	// set priority
	param.sched_priority = MY_PRIORITY;
	// wait to timer expire
	uint64_t num_periods = 0;
	ssize_t rr;
	int i;
	// scheduling
	sched_setscheduler(0, SCHED_FIFO, &param);
	for (i =0; i < 10; i++){
		
		line += 1;
		if (line > 0){
			printf("thread2 read line %d\n", line);
			fgets(buf, 100, fptr2);
		}
		rr = read(timer_fd2, &num_periods, sizeof(num_periods));
		//if (rr == -1) printf("ERROR: read2\n");
		if(num_periods > 1) {
	  	    	puts("MISSED WINDOW");
	       		exit(1);
	  	}
	}
	
	fclose(fptr2);
	pthread_exit(NULL);
}

// thread function for writing to array
void* writer() {
	int timer_fd3;
	timer_fd3 = timerfd_create(CLOCK_MONOTONIC, 0);
	if(timer_fd3 == -1) printf("ERROR: timerfd_create3\n");
	struct itimerspec itval;
	// set period time
	itval.it_interval.tv_sec = 0;
	itval.it_interval.tv_nsec = 500000000; // 0.5 second
	// set start time
	itval.it_value.tv_sec = 0;
	itval.it_value.tv_nsec = 500000000;  // 0.5 second
	// start the timer
	if(timerfd_settime(timer_fd3, 0, &itval, NULL) == -1) printf("ERROR: timeset3\n");

	// initialize current thread as Real Time
	struct sched_param param;
	// set priority
	param.sched_priority = MY_PRIORITY;
	
	// wait to timer expire
	uint64_t num_periods = 0;
	ssize_t rr;
	int i;
	// scheduling
	if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) printf("schedule w");
	for (i =0; i < 18; i++){
		if (line >= 0) {
			printf("thread3 write line %d\n", line);
			strcpy(c[line],buf);
		}	
	
		rr = read(timer_fd3, &num_periods, sizeof(num_periods));
		//if (rr == -1) printf("ERROR: read3\n");
		if(num_periods > 1) {
  		   	puts("MISSED WINDOW");
    	   		exit(1);
  		}
	}
	pthread_exit(NULL);
}

int main(){
	// set start time
	gettimeofday(&start, NULL);
	int rc1;
	int rc2;
	int rc3;
	if (rc1 = pthread_create(&thread_reader1, NULL, reader1, NULL)) printf("ERROR: thread_1\n");
	if (rc2 = pthread_create(&thread_reader2, NULL, reader2, NULL)) printf("ERROR: thread_2\n");
	if (rc3 = pthread_create(&thread_writer, NULL, writer, NULL)) printf("ERROR: thread_3\n");
	
	
	// check threads termiated
	if(pthread_join(thread_reader1, NULL)) printf("ERROR: joint1\n");
	if(pthread_join(thread_reader2, NULL)) printf("ERROR: joint2\n");
	if(pthread_join(thread_writer, NULL)) printf("ERROR: joint3\n");
	
	printf("reading...\n\n");
	// printing
	int i;
	for (i=0; i <= line; i++) {
		printf("%s", c[i]);
	}
	// set end time
	gettimeofday(&end, NULL);
	float time = (end.tv_sec - start.tv_sec) + ((float)end.tv_usec - (float)start.tv_usec)/1000000;
	// print time
	printf("\ntime used: %lfs\n", time);
	return 0;
}


