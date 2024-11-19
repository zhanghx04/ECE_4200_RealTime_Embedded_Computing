#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#include <semaphore.h>
#include "ece4220lab3.h"
#define RED 8
#define YELLOW 9
#define GREEN 7
#define BLUE 21
#define BTN 27
// Priority of lights
#define PRIORITY_GREEN  53
#define PRIORITY_YELLOW 52
#define PRIORITY_RED	52


sem_t mutex;	// declaration of mutex

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
void* greeLight(){
	struct sched_param param;
	param.sched_priority = PRIORITY_GREEN;
	// set scheduler
	sched_setscheduler(0, SCHED_FIFO, &param);
	// runing
	
	while(1){
		sem_wait(&mutex);
		printf("		green light\n");
		digitalWrite(GREEN,1);
		sleep(1);
		digitalWrite(GREEN,0);
		sem_post(&mutex);
		sleep(1);
	}
	
	pthread_exit(0);
}
void* yellowLight(){
	struct sched_param param;
	param.sched_priority = PRIORITY_YELLOW;
	// set scheduler
	sched_setscheduler(0, SCHED_FIFO, &param);
	// runing
	while(1){
		sem_wait(&mutex);
		printf("yellow light\n");
		digitalWrite(YELLOW,1);
		sleep(1);
		digitalWrite(YELLOW,0);
		sem_post(&mutex);		
		sleep(1);
	}
	
	pthread_exit(0);
}
void* pedestrianLight(){
	struct sched_param param;
	param.sched_priority = PRIORITY_RED;
	// set scheduler
	sched_setscheduler(0, SCHED_FIFO, &param);
	// run
	while(1){
		sem_wait(&mutex);
		if (check_button()) {
			printf("				Red light\n");
			digitalWrite(RED,1);
			sleep(1);
			digitalWrite(RED,0);
			clear_button();
		}
		sem_post(&mutex);
	}

	pthread_exit(0);
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
	
	// create thread
	pthread_t thread_green, thread_yellow, thread_pedestrian;
	// initialize semaphore
	sem_init(&mutex, 0, 1);	
	// threads create
	pthread_create(&thread_green, NULL, greeLight, NULL);
	pthread_create(&thread_yellow, NULL, yellowLight, NULL);
	pthread_create(&thread_pedestrian, NULL, pedestrianLight, NULL);
	// threads join
	pthread_join(thread_green, NULL);
	pthread_join(thread_yellow, NULL);
	pthread_join(thread_pedestrian, NULL);

	// destroy semaphore
	sem_destroy(&mutex);	


	return 0;
}
