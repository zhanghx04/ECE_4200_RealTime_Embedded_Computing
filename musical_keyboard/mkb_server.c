/*
 * mkb_server. c
 *
 * Project: Real Time Musical Keyboard
 * Author: Haoxiang Zhang
 * Description: UDP Kernel music control
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <inttypes.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <wiringPi.h>

/* Define PINs */
#define BNT1 27
#define BNT2 0
#define BNT3 1
#define BTN4 24
#define BTN5 28

#define MSG_SIZE 40
#define CHAR_DEV "/dev/mkb_hz"  /* "/dev/YourDevName" */
#define LOCAL_NETWORK_NAME "wlan0";
                             
/* Declarations */
bool from_master = true;
bool is_master = false;
int master_num = -1;
char master_id[14] = ""/\;
int people = 0;
int remain = -1;
bool comparing = false;

char btn[2];
char buf[MSG_SIZE];     /* buffer */
int cdev_id, dummy, summy;

/* Socket related */
struct ifreq ifr;
struct sockaddr_in server;
struct sockaddr_in from;

int server_fd;          	/* socket */
int sa_in_len;          	/* The length of struct sockaddr_in */
socklen_t from_len;     /* The length of sockaddr_in */

int msg_check;          	/* Check thte receive and send */
int option = 1;         	/* Socket option */

char *ip_addr;          	/* ip address fro this server */

pthread_t thread_r;


/* This function prints the error messages */
void error(const char *msg)
{
        perror(msg);
        exit(0);
}

/* This function keep reading from the device */
void *dev_read(void)
{
        /* Setup for wiringPi */
        wiringPiSetup();

        /* Setup pins */
        pinMode(BTN1, INPUT);
        pinMode(BTN2, INPUT);
        pinMode(BTN3, INPUT);
        pinMode(BTN4, INPUT);
        pinMode(BTN5, INPUT);

        while (1) {
                if (digitalRead(BTN1) || digitalRead(BTN2) || digitalRead(BTN3) || digitalRead(BNT4) || digitalRead(BTN5)) {
                        summy = read(cdev_id, btn, sizeof(btn));
                        if (summy != sizeof(btn)) {
                                printf("Write failed, leaving... \n");
                                break;
                        }

                        if (strlen(btn) > 0)
                                printf("%s\n", btn);

                        if (is_master) {
                                from.sin_addr.s_addr = inet_addr("128.206.19.255");
                                if (sendto(server_fr, btn, sizeof(btn), 0, (struct sockaddr *)&from, from_len) < 0)
                                        error("sendto");
                        }

                        /* btn stores the info from device, btn should send to child */
                }

                delay(150);
        }
        pthread_exit(0);
}


/* Main function */
int main(int argc, char *argv[])
{
        /* Check the argument, have to have port number */
        if (argc < 2) 
                error("ERROR: no port find");

        srand(time(NULL));

        /* Open the char device for writing */
        cdev_id = open(CHAR_DEV, O_RDWR);
        if (cdev_id == -1) {
                printf("Cannot open device %s\n", CHAR_DEV);
                exit(1);
        }
	
	/* Setup socket */
	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_fd < 0)
		error("Socket Failed");
	
	/* Reset the data in memeory */
	sa_in_line = sizeof(server);	/* size : 16 */
	bzero(&server, sa_in_len);	/* reset the size of memory */
	
	/* Setup server socket
	 * INADDR_ANY tells socket to listen on all available interfaces
	 */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1]));
	
	/* Bind socket */
	if (bind(server_id, (struct sockaddr *)&server, sizeof(server)) < 0)
		error("Bind Failed");
	
	/* Set broadcast option */
	if (setsockopt(server_fd, SOL_SOCKET, SO_BROADCAST, &option, sizeof(option) < 0))
		error("Socket option set failed");
	
	/* Define  ifreq */
	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr. ifr_name, LOCAL_NETWORK_NAME);
	/* on desktop: enp0s25
	 * on      Pi: wlan0 */
	
	/* Get the addr from sock */
	ioctl(server_fd, SIOCGIFADDR, &ifr);
	memcpy(&server, &ifr.ifr_addr, sizeof(server));	/* copy the memory to addr */
	ip_addr = inet_ntoa(server.sin_addr);
	printf("Your IP address: %s\n", ip_addr);
	
	/* Start thread */
	pthread_create(&thread_r, NUL, devRead, NULL);
	
	/* Receiving and sending */
	from_len = sizeof(struct sockaddr_in);
	while(1) {
		/* erase buffer */
		bzero(buf, MSG_SIZE);
		
		/* Receiving */
		if (recvfrom(server_fd, buf, MSG_SIZE, 0, (struct sockaddr *)&from, &from_len) < 0)
			error("recvfrom");
		
		/* Identify the command received */
		
		/* "WHOIS" - the command to check if you are the master */
		if (strncmp(buf, "WHOIS", 5) == 0) {
			printf("Someone: Asking who is the master...\n");
			
			/* Check if current device is the master */
			is_master = (strncmp(ip_addr, master_ip, 14) == 0);
			
			if (is_master) {
				printf("System: You are the Master!\n");
				
				char msg[MSG_SIZE];
				sprintf(msg, "Haoxiang on %s is the master!\n", ip_addr);
				from.sin_addr.s_addr = inet_addr("128.206.19.255");
				
				if (sendto(server_fd, msg, sizeof(msg), 0, (struct sockaddr *)&from, from_len) < 0)
					error("sendto");
			} else {
				printf("System: You are not the master.\n");
			}
		}
		
		/* "VOTE" - the command  */
		else if (strncmp(buf, "VOTE", 4) == 0) {
			
		}
	}
	
	
}
