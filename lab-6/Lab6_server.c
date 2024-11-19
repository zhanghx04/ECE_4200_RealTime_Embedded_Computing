/*
		Project: RT_Embedded_computing Lab-6
		 Author: Haoxiang Zhang
	Description: UDP Kernel music control
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

#define MSG_SIZE 40
#define CHAR_DEV "/dev/lab6_hx" // "/dev/YourDevName"


bool fromMaster = true;
bool isMaster = false;
int master_num = -1;
char master_ip[14] = "";
int people = 0;
int remain = -1;
bool comparing = false;

char btn[2];
char buf[MSG_SIZE];	// buffer							 ===
int cdev_id, dummy, summy; // device id and dummy		 ===

	// =================   Declare Area   ======================
	//														 ===
    int server_fd;		// socket							 ===
    int sa_in_len;	 	// length of struct sockaddr_in		 ===
    socklen_t fromlen;	// length of sockaddr_in (socklen_t) ===
    int msg_check;		// check the receive and send		 ===
    int option = 1;		// socket option					 ===
    struct ifreq ifr;			//							 ===			
    struct sockaddr_in server;	//							 ===
    struct sockaddr_in from;	//							 ===
    
    char* ip_addr;		// ip address for this server		 ===
	
	pthread_t thread_r;
    //														 ===
    // =================   Declare Area   ======================
    

void error(const char *msg){
	perror(msg);
	exit(0);
}

void* devRead(){
	// setup for wiringPi
	wiringPiSetup();
	// setup pins
	pinMode(27,INPUT);	// btn1
	pinMode(0,INPUT);	// btn2
	pinMode(1,INPUT);	// btn3
	pinMode(24,INPUT);	// btn4
	pinMode(28,INPUT);	// btn5
	while(1){
			if(digitalRead(27)||digitalRead(0)||digitalRead(1)||digitalRead(24)||digitalRead(28)){
				summy = read(cdev_id, btn, sizeof(btn));
				if(summy != sizeof(btn)) {
					printf("Write failed, leaving...\n");
					break;
				}
				if (strlen(btn) > 0) printf("%s\n",btn);
				if (isMaster){
					from.sin_addr.s_addr = inet_addr("128.206.19.255");
					if (sendto(server_fd, btn, sizeof(btn), 0, (struct sockaddr*)&from, fromlen) < 0) {
						error("sendto");
					}
				}
				// btn store the info from device
				// btn should send to slave
			}
			
			delay(150);
	}
	pthread_exit(0);
}

int main(int argc, char *argv[]){
	// check the argument, have to have port number
	if (argc < 2) {
		printf("ERROR: no port find\n");
		exit(0);
	}
	
	srand(time(NULL));
	
	

    
    
	// Open the Character Device for writing
	if((cdev_id = open(CHAR_DEV, O_RDWR)) == -1) {
		printf("Cannot open device %s\n", CHAR_DEV);
		exit(1);
	}
	
    
    // setup socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    	error("Socket Failed");
    }
    
    // erase the data from memory
    sa_in_len =  sizeof(server);	// size: 16
    bzero(&server, sa_in_len);		// erase the size of memory
    
    // setup server socket
    // INADDR_ANY tells socket to listen on all available interfaces
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));
    
    // bind socket
    if (bind(server_fd, (struct sockaddr*)&server, sizeof(server)) < 0) {
    	error("Bind Failed");
    } 
    
    // set broadcast option
    if (setsockopt(server_fd, SOL_SOCKET, SO_BROADCAST, &option, sizeof(option)) < 0) {
    	error("Socket option sets Failed");
    }
    
    // define ifreq
    ifr.ifr_addr.sa_family = AF_INET;
    strcpy(ifr.ifr_name, "wlan0"); // this is the local network name
	// on desktop: enp0s25
	// on      Pi: wlan0
    
    // get the address from sock
    ioctl(server_fd, SIOCGIFADDR, &ifr);
    memcpy(&server, &ifr.ifr_addr, sizeof(server)); // copy the memory to address
    ip_addr = inet_ntoa(server.sin_addr);
    printf("Your IP address: %s\n", ip_addr);
    //strcpy(master_ip, ip_addr);
    
	// start thread
	pthread_create(&thread_r, NULL, devRead, NULL);

	
    



    // receiving and sending
    fromlen = sizeof(struct sockaddr_in);
    while(1){
    	// erase buffer
    	bzero(buf, MSG_SIZE);

		
    	
    	// receiving
    	if (recvfrom(server_fd, buf, MSG_SIZE, 0, (struct sockaddr*)&from, &fromlen) < 0){
    		error("recvfrom");
    	}

		
		
    	
    	//printf("master ip: %s, value: %d\n", master_ip, master_num);
    	
    	// print receive
    	if (strncmp(buf, "WHOIS", 5) == 0) {
    		// ======================= WHOIS =============================
	    	printf("Someone: Asking who is master.....\n");
			if (strncmp(ip_addr, master_ip, 14) == 0){
				isMaster = true;
			} else {
				isMaster = false;
			}
    		if (isMaster){
    			printf("System : You are the Master!!!!\n");
    			char msg[MSG_SIZE];
    			sprintf(msg, "Haoxiang on %s is Master!!\n", ip_addr);
    			from.sin_addr.s_addr = inet_addr("128.206.19.255");
    			if (sendto(server_fd, msg, sizeof(msg), 0, (struct sockaddr*)&from, fromlen) < 0) {
					error("sendto");
				}
    		} else {
    			printf("System : You are not master...\n");
    		}
    	} else if (strncmp(buf, "VOTE", 4) == 0){
			// ======================= VOTE =============================
			isMaster = false;
			remain = people;
			master_num = -1;
			bzero(master_ip, 14);
    		printf("Start Voting...\n");
    		int vote_num = -1;
	    	char msg[MSG_SIZE];
    		vote_num = rand() % 10;
    		sprintf(msg, "# %s %d\n", ip_addr, vote_num);
    		from.sin_addr.s_addr = inet_addr("128.206.19.255");
    		if (sendto(server_fd, msg, 40, 0, (struct sockaddr*)&from, fromlen) < 0) {
    			error("sendto");
    		}
    		
    	} else if(strncmp(buf, "# ", 2) == 0){
	    	remain = remain -1;
			comparing = true;
			char delim[] = " ";
			char* ptr = strtok(buf, delim);
			char* arr[3];
			int i = 0;
			while (ptr != NULL){
				arr[i++] = ptr;
				ptr = strtok (NULL, delim);
			}
			if(atoi(arr[2]) > master_num) {
				master_num = atoi(arr[2]);
				strcpy(master_ip, arr[1]);
			} else if (atoi(arr[2]) == master_num) {
				if (strncmp(arr[1], master_ip, 14) > 0){
					master_num = atoi(arr[2]);
					strcpy(master_ip, arr[1]);
				}
			}			
    	} else if(strncmp(buf, "@", 1) == 0){
			dummy = write(cdev_id, buf, sizeof(buf));
			if(dummy != sizeof(buf)) {
				printf("Write failed, leaving...\n");
				break;
			}
			if (isMaster && fromMaster){
				fromMaster = false;
				continue;			
			}
			if(isMaster){
				from.sin_addr.s_addr = inet_addr("128.206.19.255");
				if (sendto(server_fd, buf, sizeof(buf), 0, (struct sockaddr*)&from, fromlen) < 0) {
					error("sendto");
				}
				fromMaster = true;
			}
		}else {
	    	printf("Received a data: %s\n", buf);
		}
		
    }
	close(cdev_id);	// close the device.
    return 0;
}
// The IP address: 128.206.19.machine_num
// The destination address: 204.159.253.118







/*
==========================================================================
                          		Task note
==========================================================================
One master computer and several slaves comupter
implement server on Raspberri Pi
a client will ask all the student( include master ) "WHOIS"
if no master, the clients can ask "VOTE" to vote a new master

at the begining no one is master

"WHOIS" - ask who is master
"VOTE"  - ask everyone to vote a new master

to vote:
	each client sent a broadcast "# ip_addresss vote_number"
	highest number win
	if number are same then check highest ip win

dynamically get ip
message size: 41
vote range [1,10]
the port should be an argument of the program
	
==========================================================================
                      Understanding about the project
==========================================================================
1. Server and clients should be run under same network.
2. In order to communicate with other device, should enter in a same port.
3. To set as TCP (Connection based), shoud set socket as SOCK_STREAM type
4. To set as UDP (Connectionless), should set socket as SOCK_DGRAM tyep

Socket:
	Scoket Structures:
		sockaddr_in
			struct sockaddr_in{
				short sin_family;
				unsigned short sin_port;
				struct in_addr sin_addr;
				char sin_zero[8];
			}
		in_addr
			struct in_addr{
				unsigned long s_addr;	// address
			}
		sockaddr
			struct sockaddr{
				unsigned short sa_family;
				char sa_data[14];
			}
			
--------------------------------------------------------------------------	
	Useful Functions:
		in_addr inet_addr(char addr)	// IPV4 format
			same type  with struct in_addr, convert string to long
		char* inet_ntoa(struct in_addr in)
			convert long to string
			
--------------------------------------------------------------------------				
	Setting up a Socket
		int socket(int domain, int type, int protocol)
					AF_INET     TCP/UDP    default:0
			set up a socket's attributes
		int bind(int desc, struct sockaddr* addr, int addrlen)
		
--------------------------------------------------------------------------	
	sendto() and recvfrom() is same as write() and read()
	
	
It is better to add a error() function to print error if there is a error

void error(char* msg){
	perror(msg);
	exit(1);
}

--------------------------------------------------------------------------	
#include <strings.h>
bzero(&memory_addr, mem_size);
bzero() erase the mem_size data from memory start from memory_addr
	

==========================================================================
                          Ways to copy ip address
==========================================================================
	struct ifreq ifr;
	struct sockaddr_in sin;
1.
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	*inet_ntoa(sin.sin_addr);
2.
	struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
	*inet_ntoa(sin.sin_addr);
*/
