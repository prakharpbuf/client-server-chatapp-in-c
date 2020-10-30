/**
 * @client
 * @author  Swetank Kumar Saha <swetankk@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This file contains the client.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <strings.h>
#include<ctype.h>
#include <string.h>
#include <unistd.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include "../include/server.h"

#define CMD_SIZE 100
#define TRUE 1
#define MSG_SIZE 256
#define BUFFER_SIZE 256
#define STDIN 0

forClients *forClientList=NULL;

int connect_to_host(char *server_ip, int server_port, int port);


//IP validation citation: https://www.tutorialspoint.com/c-program-to-validate-an-ip-address (functions validate_number(char *str) and validate_ip(char *ip))

// int validate_number(char *str) {
//    while (*str) {
//       if(*str>='0' && *str<='9'){ //if the character is not a number, return false
//          str++;
//       }
// 			else return 0;
//    }
//    return 1;
// }
// int validate_ip(char *ip) { //check whether the IP is valid or not
//    int i, num, dots = 0;
//    char *ptr;
//    if (ip == NULL)
//       return 0;
//       ptr = strtok(ip, "."); //cut the string using dor delimiter
//       if (ptr == NULL)
//          return 0;
//    while (ptr) {
//       if (!validate_number(ptr)) //check whether the sub string is holding only number or not
//          return 0;
//          num = atoi(ptr); //convert substring to number
//          if (num >= 0 && num <= 255) {
//             ptr = strtok(NULL, "."); //cut the next part of the string
//             if (ptr != NULL)
//                dots++; //increase the dot count
//          } else
//             return 0;
//     }
//     if (dots != 3) //if the number of dots are not 3, return false
//        return 0;
//       return 1;
// }
// int isValidPort(char *port){
// 	for(int i=0;i<strlen(port);i++){
// 		if(isdigit(port[i])==0){
// 			return 0;
// 		}
// 	}
// 	return 1;
// }

 /**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int clientMain(int port)
{
	int listener;
	int head_socket, selret, sock_index, fdaccept=0, caddr_len;
	fd_set master_list, watch_list;
	FD_ZERO(&master_list);
  FD_ZERO(&watch_list);
  FD_SET(STDIN, &master_list);
	head_socket=STDIN;

	while(TRUE){
		memcpy(&watch_list, &master_list, sizeof(master_list));
		printf("\n[PA1-Client@CSE489/589]$ ");
		fflush(stdout);
		selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("select failed.");

        if(selret > 0)
        {
			for(sock_index=0; sock_index<=head_socket; sock_index+=1){

					if(FD_ISSET(sock_index, &watch_list))
					{
							if (sock_index == STDIN) {

		char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
    	memset(msg, '\0', MSG_SIZE);
		if(fgets(msg, MSG_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to msg
			exit(-1);
			if(msg[strlen(msg)-1] == '\n')
				{
					msg[strlen(msg)-1] = '\0';
				}
		char * token = strtok(msg, " ");
		int i=0;
		char *input[BUFFER_SIZE];
		while(token){
			input[i]=calloc(1,strlen(token)+1);
			strncpy(input[i],token,strlen(token));
			i++;
			token=strtok(NULL," ");
		}
		//printf("I got: %s(size:%d chars)", msg, strlen(msg));
		if(!strcmp(msg,"AUTHOR")){
			cse4589_print_and_log("[%s:SUCCESS]\n", msg);
			cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n","prakharp");
			cse4589_print_and_log("[%s:END]\n", msg);
		}
		else if(strcmp(msg,"IP")==0){
			char ip[BUFFER_SIZE];
			struct sockaddr_in udp;
			int fd =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			int len = sizeof(udp);
			memset((char *) &udp, 0, sizeof(udp));
			udp.sin_family = AF_INET;
			udp.sin_port = htons(53);
			inet_pton(AF_INET, "8.8.8.8", &udp.sin_addr);

			connect(fd, (struct sockaddr *)&udp, sizeof(udp));
			getsockname(fd,(struct sockaddr *)&udp,(unsigned int*) &len);

			inet_ntop(AF_INET, &(udp.sin_addr), ip, len);
			cse4589_print_and_log("[%s:SUCCESS]\n",msg);
			cse4589_print_and_log("IP:%s\n", ip);
			cse4589_print_and_log("[%s:END]\n",msg);
			}
	else if(strcmp(msg,"PORT")==0){
		cse4589_print_and_log("[%s:SUCCESS]\n", msg);
		cse4589_print_and_log("PORT:%d\n", port);
		cse4589_print_and_log("[%s:END]\n", msg);
	}
	else if(strcmp(msg,"LOGOUT")==0){
		forClients **head=&forClientList;
		forClients *index=*head;
		while(index->next!=NULL || index->port_num!=port){
			index=index->next;
		}
		strcpy(index->status,"logged-out");
		send(listener,msg,CMD_SIZE,0);
		cse4589_print_and_log("[%s:SUCCESS]\n", msg);
		cse4589_print_and_log("[%s:END]\n", msg);
	}
	else if(strcmp(msg,"EXIT")==0){
		forClients **head=&forClientList;
		forClients *index=*head;
		while(index->next!=NULL || index->port_num!=port){
			index=index->next;
		}
		strcpy(index->status,"logged-out");
		send(listener,msg,CMD_SIZE,0);
		cse4589_print_and_log("[%s:SUCCESS]\n", msg);
		cse4589_print_and_log("[%s:END]\n", msg);
	}
	else if(strcmp(input[0],"LOGIN")==0){
		// if(validate_ip(input[1])==0){
		// 	cse4589_print_and_log("[%s:ERROR]\n", msg);
		// 	cse4589_print_and_log("[%s:END]\n", msg);
		// }
		// else if(isValidPort(input[2])==0){
		// 	cse4589_print_and_log("[%s:ERROR]\n", msg);
		// 	cse4589_print_and_log("[%s:END]\n", msg);
		// }
    if(0){}
		else{
		listener=connect_to_host(input[1],atoi(input[2]), port);
		// FD_SET(listener, &master_list);
	 	// if(listener > head_socket) head_socket = listener;
		char *buffer = calloc(1,BUFFER_SIZE*5);
		forClientList=NULL;
		if(recv(listener,buffer,BUFFER_SIZE*5,0)>0){
			char * client = strtok_r(buffer,"|",&buffer);
			int j=0;
			while(client){
				j++;
				forClients **head = &forClientList;
				forClients *index = *head;
				forClients *add = (forClients*)calloc(1,sizeof(forClients));
				char*token=strtok(client,"_");
				int i=0;
				while(token){
					if(i==0){
						add->port_num=atoi(token);
					}
					if(i==1){
						strncpy(add->hostname,token,strlen(token));
					}
					if(i==2){
						strncpy(add->ip_addr,token,strlen(token));
					}
					if(i==3){
						strncpy(add->status,token,strlen(token));
					}
					i++;
					token=strtok(NULL,"_");
				}
				client=strtok_r(NULL,"|",&buffer);
				add->next=NULL;
				if(*head==NULL) *head=add;
				else{
					while(index->next!=NULL){
						index=index->next;
					}
					index->next=add;
				}
			}

		}
		cse4589_print_and_log("[%s:SUCCESS]\n", input[0]);
		cse4589_print_and_log("[%s:END]\n", input[0]);
		fflush(stdout);
	}
}
	if(strcmp(msg,"LIST")==0){
		cse4589_print_and_log("[%s:SUCCESS]\n", msg);
		forClients *index = forClientList;
		int list_id = 1;
		while(index){
			if(strcmp(index->status,"logged-out")==0){
				index=index->next;
				continue;
			}
			cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", list_id, index->hostname, index->ip_addr, index->port_num);
			list_id++;
			index=index->next;
		}
		cse4589_print_and_log("[%s:END]\n", msg);
	}
free(msg);
}
else {
// 	char *msg = (char*) malloc(sizeof(char)*MSG_SIZE);
// 		memset(msg, '\0', MSG_SIZE);
// 	if(fgets(msg, MSG_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to msg
// 		exit(-1);
// 		if(msg[strlen(msg)-1] == '\n')
// 			{
// 				msg[strlen(msg)-1] = '\0';
// 			}
//
// 	printf("I got: %s(size:%d chars)", msg, strlen(msg));
// 	printf("\nSENDing it to the remote server ... ");
// 	if(send(server, msg, strlen(msg), 0) == strlen(msg))
// 		printf("Done!\n");
// 	fflush(stdout);
//
// 	/* Initialize buffer to receieve response */
// 			char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
// 			memset(buffer, '\0', BUFFER_SIZE);
//
// 	if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
// 		printf("Server responded: %s", buffer);
// 		fflush(stdout);
// }

}
		}
	}
}
}
}
int connect_to_host(char *server_ip, int server_port, int port)
{
    int fdsocket, len;
    struct sockaddr_in remote_server_addr;

    fdsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(fdsocket < 0)
       perror("Failed to create socket");

    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);
    remote_server_addr.sin_port = htons(server_port);

    if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
        perror("Connect failed");

	char port_num[CMD_SIZE];
	memset(port_num, 0, CMD_SIZE);
	sprintf(port_num, "%d", port);
	send(fdsocket, port_num, strlen(port_num), 0);

    return fdsocket;
}
