#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#include "ece4220lab3.h"
#define RED 8
#define YELLOW 9
#define GREEN 7
#define BLUE 21
#define BTN 27

void init_pins(){
	/*
	  to initialize the light.
	  this function will show which lights will work.
	  after 1 second, all the lights will be turned off.
	  here will set blue LED off, because we are not using it.

	  to initialize the button.
          the button is the most left button.
	*/
	digitalWrite(RED,1);
        digitalWrite(YELLOW,1);
        digitalWrite(GREEN,1);
	digitalWrite(BLUE,0);
	// sleep unit: second
	sleep(1);	
	digitalWrite(RED,0);
        digitalWrite(YELLOW,0);
        digitalWrite(GREEN,0);
}

/*
	note for P2: in each thread
	if yellow is on, then set yellow's priority higher than others
	if green is on, then set green's priority higher than others
	if button is on, the set red's priority higher than others
*/
void* traffic(){
//	int timer_fd;
//	timer_fd = timerfd_create()

	while(1){
		digitalWrite(GREEN,1);
		sleep(1);
		digitalWrite(GREEN,0);
		digitalWrite(YELLOW,1);
		sleep(1);
		digitalWrite(YELLOW,0);
		if (check_button()) {
			digitalWrite(GREEN,0);
			digitalWrite(YELLOW,0);
			digitalWrite(RED,1);
			sleep(1);
			digitalWrite(RED,0);
			clear_button();
		}
	}
}

int main(){
	// set for wiringPi
	wiringPiSetup();

	// set goal pins to operatable
	pinMode(RED,OUTPUT);
	pinMode(YELLOW,OUTPUT);
	pinMode(GREEN,OUTPUT);
	pinMode(BLUE,OUTPUT);
	pinMode(BTN,INPUT);
	
	// initialize all the pins
	init_pins();
	printf("All lights are set ...\n");
	// clear the button, now button is off
	clear_button();
	// if button is off then print notice
	if(!check_button()){
		printf("checked button is off\n");
	}
	
	/*
		start
	*/
	int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
	struct itimerspec itval;
	itval.it_interval.tv_sec = 4;
	itval.it_interval.tv_nsec = 0;
	itval.it_value.tv_sec = 0;
	itval.it_value.tv_nsec = 0;
	timerfd_settime(timer_fd, 0, &itval, NULL);
	struct sched_param param;
	param.sched_priority = 51;
	// create thread
	pthread_t thread;
	pthread_create(&thread, NULL, traffic, NULL);
	pthread_join(thread);
	// get pid
	pid_t pid = getpid();
	sched_setscheduler(pid, SCHED_FIFO, &param);
	uint64_t num_periods = 0;
	read(timer_fd, &num_periods, sizeof(num_periods));
	if(num_periods > 1) {
		puts("MISSED WINDOW");
		exit(1);	
	}
	
	pthread_exit(0);


	return 0;
}
