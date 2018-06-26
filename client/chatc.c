#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "chat.h"
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

//void Listener();
//void* thread_listen(void * sock);
//void* thread_chat(void * sock);
void ConnectToPeer(int peerport);
//void* thread_recive_box(void * sock);
int recipientlisten(int clientSocket);
void* thread_send(void * sock);
void* thread_recv(void * sock);
void* thread_resipient(void * clientSocket);
int new_fd;






in_port_t myPort = 0;
pthread_t sendthread,recvthread,recvboxthreadid,recipientthread;
int pingAlive=0, sendAlive=0, rcvAlive=0; //to check running status of the 3 threads.
int connflag=0;
int main(int argc, char** argv) {
	int clientSocket, new_fd;
    int sockfd,upflag=0,choose=0;

    msg_up_t upMsg;
    msg_ack_t ack;
    msg_down_t dwnMsg;
    msg_who_t whoMsg;
    msg_hdr_t hdrMsg;
    msg_peer_t peerMsg;



printf("	*~*~  	My chat 	~*~*\n\n");


//creating sets of file descriptors to be used for select command to poll them for activity
fd_set readfds;
FD_ZERO(&readfds); //clears the new set
printf("%d\n",sockfd);

while(1){
	//select what to do up down or who or exit



	printf("Choose between the following commands: (1-4)\n");
	printf("1. Send UP message to the server\n");
	printf("2. Send DOWN message to the server\n");
	printf("3. Send WHO message to the server\n");
	printf("4. Exit\n");


			//If peer gets first
	if(upflag == 1 && connflag == 0)
	{
		//clientSocket =recipientlisten(clientSocket);
		if(pthread_create(&recipientthread, NULL ,thread_resipient, (void*)&clientSocket)!=0) //for send
			    							{
			    								//cout<<"Failed to create new thread for chat. Connection to peer will be closed ";
			    								close(clientSocket);
			    								rcvAlive=0, sendAlive=0;

			    							}
	}
	 if(connflag==0)
	{
		scanf("%d",&choose);
		getchar();

		printf("the choose number is %d\n",choose);
		switch(choose){
		    	case 1:
		    	{
		    		if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
		    		    		perror("client to server socket");
		    		    	}
		    		    struct sockaddr_in servaddr;
		    		    servaddr.sin_family = AF_INET;
		    		    servaddr.sin_port = htons(C_SRV_PORT);
		    		    inet_aton("127.0.0.1", &servaddr.sin_addr);
		    		    	if ((connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
		    		    		perror("connect");
		    		    	}
		    		        printf("%d\n",sockfd);




		    	    	upMsg.m_addr = servaddr.sin_addr.s_addr; // change to client address struct(same address)
		    	       	upMsg.m_type = MSG_UP;
		    	       	char* name = "Mat";
		    	       	strcpy(upMsg.m_name ,name);
		    	       	if (send(sockfd,&upMsg,sizeof(msg_up_t),0)<0){
		    	        	perror("send");
		    	    	}

		    	    	if (recv(sockfd,(void *) &ack,sizeof(msg_ack_t),0)<0){
		    	    		perror("recv ack failed");
		    	    	}
		    	    	myPort = htons(ack.m_port);
		    	    	printf("%d\n",myPort);
		    	    	upflag=1;

		    	    	close(sockfd);
		    	    break;
		    	}

//		    	case 2:
//		    	 {
//		    	    	dwnMsg.m_addr = servaddr.sin_addr.s_addr; // change to client address struct(same address)
//		    	    	dwnMsg.m_type = MSG_DOWN;
//		    	    	dwnMsg.m_port = myPort;
//		    	    	       	if (send(sockfd,&dwnMsg,sizeof(msg_down_t),0)<0){
//		    	    	        	perror("send");
//		    	    	    	}
//		    	    	       		upflag=0;
//		    	    	       	//CLOSE THE LISTENER THREAD
//		    	    	       	break;
//		    	 }
		    	case 3:
		    	 {
		    		 if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
		    		     		perror("client to server socket");
		    		     	}
		    		     struct sockaddr_in servaddr;
		    		     servaddr.sin_family = AF_INET;
		    		     servaddr.sin_port = htons(C_SRV_PORT);
		    		     inet_aton("127.0.0.1", &servaddr.sin_addr);
		    		     	if ((connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
		    		     		perror("connect");
		    		     	}
		    		         printf("%d\n",sockfd);






		    	    	whoMsg.m_type = MSG_WHO;
		    	    	       	if (send(sockfd, &whoMsg,sizeof(msg_who_t),0)<0){
		    	    	         	perror("send");
		    	    	    	}
		    	     	if (recv(sockfd,(void *) &hdrMsg,sizeof(msg_hdr_t),0)<0){
		    	      		perror("recv ack failed");
		    	    	  	}
		    	     	for(int i=0; i< hdrMsg.m_count; i++){
		    	     		if (recv(sockfd,(void *) &peerMsg,sizeof(msg_peer_t),0)<0){
		    	     	      		perror("recv ack failed");
		    	     	   	}

		    	     		printf("Name: %s  Port Number: %d\n",peerMsg.m_name,htons(peerMsg.m_port));
		    	     	}

		    	     	close(sockfd);
		    	     	 int chosenport=0;
		    	     			printf("Please choose a port number from the list:\n");
		    	     			scanf("%d",&chosenport);
		    	     			//connect
		    	     			connflag=1;
		    	     			ConnectToPeer(chosenport);


		    	     	break;
		    	 }
		    	case 4:
		    	{
		    		//close my threads
		    		//close the sockets
		    		return 0;
		    	}
		    	 default:
		    		break;
		    }
		}
}

	return 0;
}



void ConnectToPeer(int peerport){

	//connect to chosen peer

	int p2psockfd;

	//connect socket ->port
	if((p2psockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
	    		perror("client to server socket");
	    	}
	    struct sockaddr_in servaddr;
	    servaddr.sin_family = AF_INET;
	    servaddr.sin_port = htons(peerport);
	    inet_aton("127.0.0.1", &servaddr.sin_addr);
	    	if ((connect(p2psockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
	    		perror("connect");
	    	}
	    	msg_conn_t msgconn;
	    	msgconn.m_port=myPort+5;
	    	char* name = "Matan";
	    	strcpy(msgconn.m_name ,name);
	    	msgconn.m_type=MSG_CONN;
	    	msgconn.m_addr=servaddr.sin_addr.s_addr;
	    		printf("sending msgconn with #: %d\n",myPort+5);
	    	send(p2psockfd, (void*) &msgconn, sizeof(msg_conn_t), 0);
	    	//create threads for chat send and chat rcv.

	    							int listen_resipient;
	    								 struct sockaddr_in clientServer;

	    								if((listen_resipient = socket(AF_INET,SOCK_STREAM,0)) < 0){
	    									        perror("server socket");
	    									    }
	    									    clientServer.sin_family = AF_INET;
	    									    clientServer.sin_port = htons(myPort+5);
	    									    inet_aton("127.0.0.1", &clientServer.sin_addr);

	    									    if (bind(listen_resipient, (struct sockaddr *) &clientServer, sizeof(clientServer)) < 0) {
	    									        perror("binding server failed1");
	    									    }
	    									    listen(listen_resipient, 3);


	    									    listen_resipient = accept(listen_resipient, (struct sockaddr *)NULL, NULL);



	    									    rcvAlive=1, sendAlive=1;

	    							if(pthread_create(&sendthread, NULL , thread_send, (void*)&p2psockfd)!=0) //for send
	    							{
	    							close(p2psockfd);
	    							rcvAlive=0, sendAlive=0;
	    							return;
	    									    	    							}
	    							if( pthread_create(&recvthread, NULL , thread_recv, (void*)&listen_resipient)!=0)
	    							{
	    							//	cout<<"Failed to create new thread for chat. Connection to peer will be closed ";
	    								close(p2psockfd);
	    								rcvAlive=0, sendAlive=0;
	    								return;
	    							}

	    							//wait for both threads to complete
	    	while(sendAlive && rcvAlive);
}




void* thread_send(void * sock){
    int *sock_chat = (int * )sock;
    msg_text_t msg;
    msg.m_type = MSG_TEXT;

    printf("start chat\n");
    while(1){
        gets(msg.m_text);
        printf("got msg to m.text\n");
        send(*sock_chat, (void*) &msg, sizeof(msg_text_t), 0);
        printf("msg sent\n");
    }
}

void* thread_recv(void * sock){
    int *sock_recv = (int * )sock;
    msg_text_t msg;

    while(1){
    	 printf("the recv socket is: %d\n",*sock_recv);
        recv(*sock_recv, (void*) &msg, sizeof(msg_text_t), 0);
        printf("%s\n", msg.m_text);
    }
}


void* thread_resipient(void * clientSocket){
	int sock_client;
	 struct sockaddr_in clientServer;

	if((sock_client = socket(AF_INET,SOCK_STREAM,0)) < 0){
		        perror("server socket");
		    }
		    clientServer.sin_family = AF_INET;
		    clientServer.sin_port = htons(myPort);
		    inet_aton("127.0.0.1", &clientServer.sin_addr);

		    if (bind(sock_client, (struct sockaddr *) &clientServer, sizeof(clientServer)) < 0) {
		        perror("binding server failed");
		    }
		    listen(sock_client, 3);


new_fd = accept(sock_client, (struct sockaddr *)NULL, NULL);
		connflag=1;
		printf("after accept %d\n",new_fd);

		if (new_fd == -1)
		{
						perror("accept");
						return 0;
		}
		msg_conn_t msgconn;
			 	recv(new_fd, (void*) &msgconn, sizeof(msg_conn_t), 0);

			 	printf("got the msg conn #: %d\n",msgconn.m_port);
			 	int p2psockfd;

			 		//connect socket ->port
			 		if((p2psockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){
			 		    		perror("client to server socket");
			 		    	}
			 		    struct sockaddr_in servaddr;
			 		    servaddr.sin_family = AF_INET;
			 		    servaddr.sin_port = htons(msgconn.m_port);
			 		    inet_aton("127.0.0.1", &servaddr.sin_addr);
			 		    	if ((connect(p2psockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
			 		    		perror("connect");
			 		    	}



		rcvAlive=1, sendAlive=1;
					if(pthread_create(&sendthread, NULL , thread_send, (void*)&p2psockfd)!=0) //for send
					{
						//cout<<"Failed to create new thread for chat. Connection to peer will be closed ";
						close(new_fd);
						rcvAlive=0, sendAlive=0;
						return 0;
					}
					if( pthread_create(&recvthread, NULL , thread_recv, (void*)&new_fd)!=0)
					{
						//cout<<"Failed to create new thread for chat. Connection to peer will be closed ";
						rcvAlive=0, sendAlive=0;
						close(new_fd);
						return 0;
					}

					//wait till the above threads die.
	//	while(sendAlive && rcvAlive);

					return 0;
}
