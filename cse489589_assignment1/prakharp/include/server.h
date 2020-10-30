
#ifndef SERVER_H_
#define SERVER_H_

typedef struct clients{
    int fd;
    char hostname[265];
    char ip_addr[256];
    int port_num;
    int num_msg_sent;
    int num_msg_rcv;
    char status[256];

    struct clients *next;
}clients;
typedef struct forClients{
    int port_num;
    char hostname[256];
    char ip_addr[256];
    char status[256];

    struct forClients *next;
}forClients;
#endif
