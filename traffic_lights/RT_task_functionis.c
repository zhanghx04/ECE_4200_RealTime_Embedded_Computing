/*
 * RT_tasks_functions.c
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <time.h>

#define MY_PRIORITY 51  /* Kernel's priority is 50 */

/* --- Configure Timer --- */
int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
/* Always good to check for errors. Look for timerfd_create() */

/* structure to hold period and starting time  */
struct itimerspec itval;
/* timer fires every ## sec, ## nsec */
itval.it_interval.tv_sec = Period_seconds;		/*check the data type*/ 
itval.it_interval.tv_nsec = Period_nseconds;	/*check the data type*/

/* Timer will start in ## sec, ## nsec from the moment the timer is started */
itval.it_value.tv_sec = InitTime_seconds;		/*check the data type*/
itval.it_value.tv_nsec = InitTime_nseconds;		/*check the data type*/

/* start the timer (you need the file descriptor and itval structure above) */
timerfd_settime(timer_fd, 0, &itval, NULL);	
/* This function has a return value, alwasy good to check errors. Look for timerfd_settime() */


/* --- Initialize current thread as RT task --- */
struct sched_param param;
/* Declare ourself as a real time task by elevating our priority and setting an 
 * appropriate scheduling policy.
 */
param.sched_priority = MY_PRIORITY;
sched_setscheduler(0, SCHED_FIFO, &param);	
/* This function has a return value, alwasy good to check errors. Look for sched_setscheduler() */

/* --- Wait for the timer to expire --- */
uint64_t num_periods = 0;
/* The following is typically inside a loop. It will wait whatever is left of the
 * current period.
 * read tells you how many periods it's been since you last checked in.
 */
read(timer_fd, &num_periods, sizeof(num_periods));
/* This function has a return value, always good to check errors. Look for read() */

if(num_periods > 1) {
        puts("MISSED WINDOW");
        exit(1);
}
