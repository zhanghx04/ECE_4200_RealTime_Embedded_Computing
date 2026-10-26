# Musical Keyboard

## Note
```text
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
```
