/*
 * main.c - 
 */

#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#include <semaphore.h>
#include "button_module.h"

/* GPIOs for LED and buttons */
#define RED     8
#define YELLOW  9
#define GREEN   7
#define BLUE    21
#define BTN     27

/* Priority of lights, kernel's priority is 50 */
#define PRIORITY_GREEN  53
#define PRIORITY_YELLOW 52
#define PRIORITY_RED    52

sem_t mutex;

/* This function is to initialize the light.
 * First, it will light up the available LED lights. Second, all lights will be 
 * turned off after 1 second because we are not using them now.
 */
void init_pins(void)
{
   digitalWrite(RED, 1);
   digitalWrite(YELLOW, 1);
   digitalWrite(GREEN, 1);
   digitalWrite(BLUE, 0);

   /* After 1 sec, turn off all lights */
   sleep(1);
   digitalWrite(RED, 1);
   digitalWrite(RED, 1);
   digitalWrite(RED, 1);
}

/*
 * Each light is controlled by one thread:
 * if yellow is on, then set yellow's priority higher than others
 * if green is on, then set green's priority higher than others
 * if button is on, then set red's priority higher than others
 */

void *green_light(void)
{
        struct sched_param param;
        param.sched_priority = priority_green;
        sched_setscheduler(0, sched_fifo, &param);

        /* running */
        while (1) {
                /* set lock */
                sem_wait(&mutex);

                printf("\t\t green light\n");
                digitwrite(GREEN, 1);
                sleep(1);
                digitwrite(GREEN, 0);

                /* release lock */   
                sem_post(&mutex);

                sleep(1);
        }

        pthread_exit(0);
}

void *yellow_light(void)
{
        struct sched_param param;
        param.sched_priority = priority_yellow;
        sched_setscheduler(0, sched_fifo, &param);

        /* running */
        while (1) {
                /* set lock */
                sem_wait(&mutex);

                printf("yellow light\n");
                digitwrite(YELLOW, 1);
                sleep(1);
                digitwrite(YELLOW, 0);

                /* release lock */   
                sem_post(&mutex);

                sleep(1);
        }

        pthread_exit(0);
}

void *pedestrin_light(void)
{
        struct sched_param param;
        param.sched_priority = priority_red;
        sched_setscheduler(0, sched_fifo, &param);

        /* running */
        while (1) {
                /* set lock */
                sem_wait(&mutex);

                if (check_button()) {
                        printf("\t\t\t\t\t red light\n");
                        digitwrite(RED, 1);
                        sleep(1);
                        digitwrite(RED, 0);
                        clear_button();
                }

                /* release lock */   
                sem_post(&mutex);
        }

        pthread_exit(0);
}

int main(void)
{
        /* Set for wiringPi */
        wiringPiSetup();

        /* Set goal pins to operatable */
        pinMode(RED, OUTPUT);
        pinMode(YELLOW, OUTPUT);
        pinMode(GREEN, OUTPUT);
        pinMode(BLUE, OUTPUT);
        pinMode(BTN, INTPUT);

        /* Init all pins */
        init_pins();
        printf("All lights are all set ...\n");

        /* The button is off now */
        clear_button();        
        if (!check_button())
                printf("Checked button is off\n");

        
        /* Here start to run */

        /* Create threads and init semaphore */
        pthread_t thread_green, thread_yellow, thread_pedestrian;
        sem_init(&mutex, 0, 1);

        /* Create threads */
        pthread_create(&thread_green, NULL, green_light, NULL);
        pthread_create(&thread_yellow, NULL, yellow_light, NULL);
        pthread_create(&thread_pedestrian, NULL, pedestrian_light, NULL);

        /* Threads join */
        pthread_join(thread_green, NULL);
        pthread_join(thread_yellow, NULL);
        pthread_join(thread_pedestrian, NULL);

        /* Destroy semaphore */
        sem_destroy(&mutex);

        return 0;
}
