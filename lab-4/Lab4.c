#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>

unsigned int   pre_gps = 0;
unsigned int pre_time = 0;
unsigned int   now_gps = 0;
unsigned int now_time = 0;

sem_t mutex;
// pthread lock
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int simple_pipe[2];

void* print_info(){
	unsigned char info[200];
	while(1) {
		read(simple_pipe[0], info, sizeof(info));
		printf("%s\n", info);
	}
	pthread_exit(0);
}

void* child_thread(void*timestamp) {	
	unsigned int btn_time;
	btn_time = (unsigned int)timestamp;
	//printf("ts: %u\n", btn_time);
	unsigned int pre = now_time;
	unsigned int pgps = pre_gps;
	if (pre_time != now_time) {
		pre = pre_time;
	}
	// lock the pthread
	pthread_mutex_lock(&mut);
	while (btn_time > now_time) { }
	// unlock the pthread
    	pthread_mutex_unlock(&mut);

	unsigned int now = now_time;
	unsigned int ngps = now_gps;
	if (pre == now) printf("!!!!\n");

	float btn_gps;
	btn_gps = (float)(btn_time-pre)/(float)(now-pre) + (float)pre_gps;
	// store the information into a string
	unsigned char info[200];
	sprintf(info, 
		"Pre GPS: %u     Pre Time: %u \nBtn GPS: %.3f Btn Time: %u \nNow GPS: %u     Now Time: %u \n ", 
		pgps, pre, btn_gps, btn_time, ngps, now);
	// write the information into a simple pipe
	sem_wait(&mutex);
	write(simple_pipe[1], info, sizeof(info));
	sem_post(&mutex);
	pthread_exit(0);
}

void* check_button(){
	int fd_cb;
	char* pipeBP = "/tmp/N_pipeBP";
	size_t* timestamp;
	timestamp = (size_t*)malloc(sizeof(size_t));
	if (timestamp == NULL){
		printf("Memory allocation failed\n");
		exit(1);
	} else {
		printf("successful\n");
	}
	int i = 0;
	while(1){
		if (fd_cb = open(pipeBP, O_RDONLY)){
			read(fd_cb, timestamp+i, sizeof(timestamp));
			// create child thread
			pthread_t thread_child;
			pthread_create(&thread_child, NULL, child_thread,(void*)*(timestamp+i));
			i++;
		}
	}
	free(timestamp);
}

int main(){
	// set time
	struct timeval spec;

	// initialize sem
	sem_init(&mutex, 0, 1);	

	// create simple pipe
	pipe(simple_pipe);
	
	// create thread to print
	pthread_t thread_print;
	pthread_create(&thread_print, NULL, print_info, NULL);
	
	// create thread to check button 
	pthread_t thread_check_btn;
	pthread_create(&thread_check_btn, NULL, check_button, NULL);
	

	// reading the information from N_pipe1
	int fd;
	char* pipe1 = "/tmp/N_pipe1";
	int gps=0;
	while(1){
		if (fd=open(pipe1, O_RDONLY)) {
			pre_gps = now_gps;
			pre_time = now_time;
			// Read the GPS
			read(fd, &gps, 1);
			// store GPS 
			now_gps = gps;	
			// get the time and store
			gettimeofday(&spec, NULL);
			now_time = spec.tv_sec*1000 + spec.tv_usec/1000;
			sleep(0.01);
		}
		
	}
	close(fd);
	sem_destroy(&mutex);

	return 0;
}
