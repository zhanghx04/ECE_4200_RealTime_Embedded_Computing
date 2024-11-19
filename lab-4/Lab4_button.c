#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <stdint.h>
#include <unistd.h> 
#include <pthread.h>
#include <wiringPi.h>
#include <semaphore.h>
#include <sched.h>
#include "ece4220lab3.h"
#include <time.h>
#include <math.h>
#include <string.h>


int main() {
	// set for wiringPi
	wiringPiSetup();
	pinMode(27,INPUT);	// btn1
	// set timer
	int timer_fd;
	timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
	if(timer_fd == -1) printf("ERROR: timerfd_create1\n");
	struct itimerspec itval;
	itval.it_interval.tv_sec = 0;
	itval.it_interval.tv_nsec = 6e7;
	itval.it_value.tv_sec = 0;
	itval.it_value.tv_nsec = 6e7;
	// start the timer
	if(timerfd_settime(timer_fd, 0, &itval, NULL) == -1) printf("ERROR: timeset1\n");
	// set pipe
	int fd;
	// FIFO file path
	char* pipe2 = "/tmp/N_pipeBP";
	mkfifo(pipe2, 0777);
	unsigned int bp;
	
	
	// initialize current thread as Real Time
	struct sched_param param;
	param.sched_priority = 55;

	// wait to timer expire
	uint64_t num_periods = 0;
	ssize_t rr;
	
	struct timeval spec;
	clear_button();
	sched_setscheduler(0, SCHED_FIFO, &param);
	printf("start\n");
	while (1) {
		if (!check_button()) {
			// set time
			gettimeofday(&spec, NULL);
			bp = spec.tv_sec*1000 + spec.tv_usec/1000;
			//printf("bp: %u\n", bp);
			fd = open(pipe2, O_WRONLY);
			write(fd, &bp, sizeof(bp));
			
			clear_button();
			
		}
		rr = read(timer_fd, &num_periods, sizeof(num_periods));
		if (rr == -1) printf("ERROR: read1\n");
		if(num_periods > 1) {
	  	  	puts("MISSED WINDOW");
	       		exit(1);
	  	}
	  	
	}
	close(fd);
	pthread_exit(NULL);
	return 0;
}
