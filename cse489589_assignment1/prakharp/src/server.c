/**
 * @server
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
 * This file contains the server init and main while loop for tha application.
 * Uses the select() API to multiplex between network I/O and STDIN.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include "../include/global.h"
#include "../include/logger.h"
#include "../include/server.h"

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256

 /**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
clients *myClientList = NULL;

void errorRunning(char*cmd){
	cse4589_print_and_log("[%s:ERROR]\n", cmd);
	cse4589_print_and_log("[%s:END]\n", cmd);
}

void send_to_client(int fd){
	clients **head=&myClientList;
	clients *index=*head;
	char buffer[BUFFER_SIZE*5];
	char client[BUFFER_SIZE];
	memset(buffer,0,sizeof(buffer));
	memset(client,0,sizeof(client));
	while(index){
		sprintf(client,"%d_%s_%s_%s|",index->port_num, index->hostname, index->ip_addr,index->status);
		strcat(buffer,client);
		index=index->next;
	}
	send(fd, buffer, BUFFER_SIZE*5, 0);
}

void add_client(char* hostname, int fd, int port, char*ip){
	clients **head = &myClientList;
	clients *index=*head;
	while(index){
		if((index->fd==fd) && (strcmp(index->status,"logged-out")==0)){
			strcpy(index->status,"logged-in");
			return;
		}
		index=index->next;
	}
	index=*head;
	clients *add=(clients *)calloc(1,sizeof(clients));
	add->fd=fd;
	strcpy(add->hostname,hostname);
	add->port_num=port;
	strcpy(add->ip_addr,ip);
	add->num_msg_sent=0;
	add->num_msg_rcv=0;
	strcpy(add->status,"logged-in");
	add->next=NULL;
	if(*head==NULL || ((*head)->port_num)>add->port_num){
		add->next=*head;
		(*head)=add;
	}
	else{
		while((index)->next!=NULL && (index)->next->port_num < add->port_num){
			(index)=(index)->next;
		}
		add->next=(index)->next;
		(index)->next=add;
	}
}

int serverMain(int port)
{
	int server_socket, head_socket, selret, sock_index, fdaccept=0, caddr_len;
	struct sockaddr_in server_addr, client_addr;
	fd_set master_list, watch_list;
	/* Socket */
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
		perror("Cannot create socket");

	/* Fill up sockaddr_in struct */
	bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    /* Bind */
    if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
    	perror("Bind failed");

    /* Listen */
    if(listen(server_socket, BACKLOG) < 0)
    	perror("Unable to listen on port");
    /* ---------------------------------------------------------------------------- */

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the listening socket */
    FD_SET(server_socket, &master_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);

    head_socket = server_socket;

    while(TRUE){
        memcpy(&watch_list, &master_list, sizeof(master_list));

        printf("\n[PA1-Server@CSE489/589]$ ");
		fflush(stdout);

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
        if(selret > 0){
            /* Loop through socket descriptors to check which ones are ready */
            for(sock_index=0; sock_index<=head_socket; sock_index+=1){

                if(FD_ISSET(sock_index, &watch_list)){

                    /* Check if new command on STDIN */
                    if (sock_index == STDIN){
                    	char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);

                    	memset(cmd, '\0', CMD_SIZE);
						if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
							exit(-1);
            if(cmd[strlen(cmd)-1] == '\n')
						{
							cmd[strlen(cmd)-1] = '\0';
						}

						printf("\nI got: %s\n", cmd);

						//Process PA1 commands here ...

            if(strcmp(cmd,"AUTHOR")==0){
							cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
							cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n","prakharp");
							cse4589_print_and_log("[%s:END]\n", cmd);
            }
						if(strcmp(cmd,"IP")==0){
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
			        cse4589_print_and_log("[%s:SUCCESS]\n",cmd);
			        cse4589_print_and_log("IP:%s\n", ip);
			        cse4589_print_and_log("[%s:END]\n",cmd);
						}
					if(strcmp(cmd,"PORT")==0){
						cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
						cse4589_print_and_log("PORT:%d\n", port);
						cse4589_print_and_log("[%s:END]\n", cmd);
					}
					if(strcmp(cmd,"LIST")==0){
						cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
						clients *index = myClientList;
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
						cse4589_print_and_log("[%s:END]\n", cmd);
					}
					if(strcmp(cmd,"STATISTICS")==0){
						cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
						clients *index = myClientList;
						int list_id = 1;
						while(index){
							cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", list_id, index->hostname, index->num_msg_sent, index->num_msg_rcv, index->status);
							list_id++;
							index=index->next;
						}
						cse4589_print_and_log("[%s:END]\n", cmd);
					}

						free(cmd);
                    }
                    /* Check if new client is requesting connection */
                    else if(sock_index == server_socket){
                        caddr_len = sizeof(client_addr);
                        fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
                        if(fdaccept < 0)
                            perror("Accept failed.");

												printf("\nRemote Host connected!\n");


                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if(fdaccept > head_socket) head_socket = fdaccept;

												char hostname[CMD_SIZE];
												struct sockaddr_in sai = (struct sockaddr_in )client_addr;
												char clientPort[CMD_SIZE];
												recv(fdaccept,clientPort , CMD_SIZE, 0);
												char ip[CMD_SIZE];
												if(sai.sin_family==AF_INET){
													inet_ntop(AF_INET, &sai.sin_addr, ip, sizeof(ip));
												}
												if(sai.sin_family==AF_INET6){
													inet_ntop(AF_INET6, &sai.sin_addr, ip, sizeof(ip));
												}
												if(getnameinfo(&client_addr,sizeof(client_addr), hostname, sizeof(hostname),NULL, 0, NI_NAMEREQD)){
													errorRunning("FINDING HOST NAME");
												}
												add_client(hostname, fdaccept, atoi(clientPort), ip);
												send_to_client(fdaccept);

                    }
                    /* Read from existing clients */
                    else{
                        /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);

                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
                            close(sock_index);
                            printf("Remote Host terminated connection!\n");

                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else {
                        	//Process incoming data from existing clients here ...

                        	printf("\nClient sent me: %s\n", buffer);
							//printf("ECHOing it back to the remote host ... ");
							//if(send(fdaccept, buffer, strlen(buffer), 0) == strlen(buffer))
								//printf("Done!\n");
								clients **head=&myClientList;
								clients *index = *head;
								while(index){
									if((index)->fd==sock_index){
										break;
									}
									(index)=(index)->next;
								}
								if(strcmp(buffer,"LOGOUT")==0){
									strcpy((index)->status,"logged-out");
								}
								if(strcmp(buffer,"EXIT")==0){
									strcpy((index)->status,"logged-out");
								}




							fflush(stdout);
                        }

                        free(buffer);
                    }
                }
            }
        }
    }

    return 0;
}
