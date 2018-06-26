#include "chat.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_CLIENTS 10

//Global variables//
in_port_t give_port = 12361;
msg_peer_t *clients[MAX_CLIENTS];
int count_clients = 0;
//functions
/*implementation of the servers thread*/
void* thread_implem(void * socket);
/* Add client to list */
void list_add(msg_peer_t *cl);
void list_delete(in_port_t uport);
int peer_alive(in_port_t uport);
pthread_t t; //thread for the accepted request

int main(int argc, char** argv) {

    struct sockaddr_in server, client;
    int serv_socket, client_sock, sizeof_addrin;

    //Create socket with given port num
    serv_socket = socket(AF_INET, SOCK_STREAM, 0); //create a server socket
    if (serv_socket < 0) {
        perror("Failed to create socket");
    }
    //Create sockaddr_in struct
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(C_SRV_PORT);

    if (bind(serv_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("binding server failed");
        return 1;
    }
    listen(serv_socket, 3); //start receiving upcoming connections from sockets
    //Accept and incoming connection
    puts("IM ALIVE");
    sizeof_addrin = sizeof(struct sockaddr_in);

    //connections get accepted here untill shutdown is initiated
    while ((client_sock = accept(serv_socket, (struct sockaddr *) &client,
            (socklen_t*) &sizeof_addrin))) {
        puts("Connection accepted");
        printf("%d\n",client_sock);


        int sock;
        sock = client_sock;

        //create thread and run the thread implementation function
        if (pthread_create(&t, NULL, thread_implem, (void*) &sock) != 0) {

            perror("could not create thread");
            return 1;
        }
    }

    return 0;
}

/*thread_implementation handles the messages by type*/
void* thread_implem(void * socket) {
    int *sock = (int * )socket;
    int  typ;
    msg_ack_t ak;
    while(1){
    recv(*sock, (void*) &typ, sizeof(int), MSG_PEEK);

    switch (typ) {

	case MSG_UP: {
            msg_up_t msgup;
            recv(*sock, (void*) &msgup, sizeof(msg_up_t), MSG_WAITALL);
            /*add new client */
            msg_peer_t *new_peer = (msg_peer_t *)malloc(sizeof(msg_peer_t));
            new_peer->m_addr = msgup.m_addr;
            strcpy(new_peer->m_name, msgup.m_name);
            new_peer->m_type = msgup.m_type;
            new_peer->m_port = htons(give_port);

            list_add(new_peer);
            /*server answer to client*/
            ak.m_port = htons(give_port);
            ak.m_type = MSG_ACK;
            send(*sock, (void*) &ak, sizeof(msg_ack_t), 0); //after appending new msg sent ok (ACK)
            give_port++;


            pthread_cancel(t);
            break;
        }
        case MSG_DOWN: {
            msg_down_t msgdown;
            recv(*sock, (void*) &msgdown, sizeof(msg_down_t), 0);
            list_delete(msgdown.m_port);
            break;
        }
        case MSG_WHO: {
            int i=0;
            msg_who_t who;
            msg_hdr_t hdr;
            recv(*sock, (void*) &who, sizeof(msg_who_t), 0);
            puts("got who request");
            hdr.m_count = count_clients;
            hdr.m_type = MSG_HDR;
            send(*sock, (void*) &hdr, sizeof(msg_hdr_t), 0);
            puts("hdr msg sent");
            for(i=0;i < MAX_CLIENTS; i++){
		if(clients[i]){
                    send(*sock, (void*) clients[i], sizeof(msg_peer_t), 0);
		}
            }


            pthread_cancel(t);
            break;
        }
        case MSG_ALIVE: {
            msg_alive_t peerAlive;
            recv(*sock, (void*) &peerAlive, sizeof(msg_alive_t), 0);
            if(peer_alive(peerAlive.m_port)){
                ak.m_port = peerAlive.m_port;
                ak.m_type = MSG_ACK;
                send(*sock, (void*) &ak, sizeof(msg_ack_t), 0);
            }else{
                msg_nack_t nk;
                nk.m_type = MSG_NACK;
                send(*sock, (void*) &nk, sizeof(msg_nack_t), 0);
            }
            break;
        }
        default:
            break;
    }
    }
    return 0;
}


void list_add(msg_peer_t *cl){
    int i;
    puts("add new peer");
    for(i=0; i < MAX_CLIENTS; i++){
        if(!clients[i]){
            clients[i] = cl;
            count_clients++;
            return;
        }
    }//end for
}

void list_delete(in_port_t uport){
    int i;
    for(i=0;i < MAX_CLIENTS; i++){
        if(clients[i]){
            if(clients[i]->m_port == uport){
                clients[i] = NULL;
                count_clients--;
                return;
            }
        }
    }
}

int peer_alive(in_port_t uport){
    int i;
    for(i=0;i < MAX_CLIENTS; i++){
        if(clients[i]){
            if(clients[i]->m_port == uport){
                return 1;
            }
        }
    }
    return 0;
}
