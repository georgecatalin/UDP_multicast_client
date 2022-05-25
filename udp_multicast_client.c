/*
 ============================================================================
 Name: udp_multicast_client.c
 Author: George Calin
 Email: george.calin@gmail.com
 Created on: May 25, 2022
 Description : 
 ============================================================================
 */

#include <sys/types.h>// sys/types.h - data types
#include <sys/socket.h> // sys/socket.h - Internet Protocol family
#include <netinet/in.h> // netinet/in.h - internet address family
#include <arpa/inet.h> // arpa/inet.h - definitions for internet operations
#include <netdb.h> //  netdb.h - definitions for network database operations
#include <stdio.h>
#include <string.h>
#include <unistd.h> // unistd.h - standard symbolic constants and types

#define APPLICATION_VERSION 0.1
#define MAX_MESSAGE_LENGTH 7000
#define EXIT_FAILURE  -1
#define DEBUG_LEVEL 2

int create_socket()
{
	int file_descriptor=0;

	file_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if(file_descriptor < 0)
	{
		perror("Can't open socket ");
		return EXIT_FAILURE;
	}

	return file_descriptor;
}

int bind_server_port(struct sockaddr_in server_address, int file_descriptor, int server_port)
{
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(server_port);

	if(bind(file_descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) <0)
	{
		perror("Can not bind the port");
		return EXIT_FAILURE;
	}

	return 0;
}

int set_socket_options(int socket_file_descriptor, struct ip_mreq message_request)
{
	int return_from_setsockopt=0;

	return_from_setsockopt=setsockopt(socket_file_descriptor, IPPROTO_IP,IP_ADD_MEMBERSHIP,&message_request,sizeof(message_request));

	if(return_from_setsockopt<0)
	{
		perror("Error when setsockopt() ");
		return EXIT_FAILURE;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int socket_file_descriptor;
	int port_number= 0;

	char *group_multicast_address= NULL;
    char message[MAX_MESSAGE_LENGTH];

    struct sockaddr_in server_address;
    struct ip_mreq multicast_request;
    int address_length=0;

    memset(&server_address,0, sizeof(server_address));

    if(argc!=3)
    	{
    		printf("The usage of this client is : \t >> %s  <group multicast address>  <port> \n", argv[0]);
    		puts("Please run the application correctly.");
    		exit(EXIT_FAILURE);
    	}

    group_multicast_address=argv[1];
    port_number = atoi(argv[2]);

    socket_file_descriptor=create_socket();

    /* ========= Fill in the server details ========= */
    server_address.sin_family= AF_INET;
    server_address.sin_addr.s_addr=htonl(INADDR_ANY);
    server_address.sin_port=htons(port_number);
    address_length=sizeof(server_address);

    bind_server_port(server_address, socket_file_descriptor, port_number);

    multicast_request.imr_multiaddr.s_addr=inet_addr(group_multicast_address);
    multicast_request.imr_interface.s_addr=htonl(INADDR_ANY);

    set_socket_options(socket_file_descriptor, multicast_request);

    while(1)
    {
    	int number_of_characters=recvfrom(socket_file_descriptor, (char *) message, sizeof(message), MSG_WAITALL, (struct sockaddr *) &server_address, &address_length);

    	if(number_of_characters<0)
    	{
    		perror("Error when recvfrom() ");
    		return EXIT_FAILURE;
    	}
    	else if(number_of_characters==0)
    	{
    		break;
    	}

#if DEBUG_LEVEL>=2
    	printf("%s: message = \" %s \" \n\n" , inet_ntoa(server_address.sin_addr), message);
#endif
    }
}
