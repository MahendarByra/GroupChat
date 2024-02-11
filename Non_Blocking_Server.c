#include "unp.h"

#define MAXLINE 4096
#define LISTENQ 1024
#define SERV_PORT 9877
#define USERNAME_SIZE 100
#define MESSAGE_SIZE 3996

struct Client 								//Client data structure username vs socket descriptor
{
    	int sockfd;
    	char username[USERNAME_SIZE];
};

struct Client client_list[FD_SETSIZE];					//Client array

void broadcast(int from, char *msg) 					//broadcast the message to all clients except the sender
{
    	for (int i = 0; i < FD_SETSIZE; i++) 
    	{
        	if (client_list[i].sockfd != -1 && client_list[i].sockfd != from) 
        	{
            		write(client_list[i].sockfd, msg, strlen(msg));
        	}
    	}
}

int main(int argc, char **argv) 
{
    	int listenfd, connfd, n;
    	struct sockaddr_in servaddr;
    	char buf[MESSAGE_SIZE];

    									// Initialize client list
    	for (int i = 0; i < FD_SETSIZE; i++) 
    	{
        	client_list[i].sockfd = -1;
    	}

    	listenfd = socket(AF_INET, SOCK_STREAM, 0);
    	fcntl(listenfd, F_SETFL, O_NONBLOCK);  				// Set listening socket to non-blocking mode

    	bzero(&servaddr, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
    	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    	servaddr.sin_port = htons(SERV_PORT);

    	if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)		//bind
    	{
    		perror("bind");
    		exit(1);
    	}
    	printf("Binding is completed....\n");

    	if(listen(listenfd, LISTENQ) < 0)						//listen
    	{
    		perror("listen");
    		exit(1);
    	}
	printf("Listening...\n");
	
    	while (1) 
    	{
        	connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);		
        	/* if new client arrives add it to the client list and broad cast the message to others i.e. some client joined the chat */

        	if (connfd > 0) 
        	{
            		fcntl(connfd, F_SETFL, O_NONBLOCK);  // Set the new socket to non-blocking mode

            		int i;
            		for (i = 0; i < FD_SETSIZE; i++) 
            		{
                		if (client_list[i].sockfd < 0) 
                		{
                			char msg[USERNAME_SIZE + 20];
                    			client_list[i].sockfd = connfd;
                    			read(connfd, client_list[i].username, sizeof(client_list[i].username));
                    			printf("%s joined the chat\n", client_list[i].username);
                    			sprintf(msg,"%s joined the chat\n", client_list[i].username);
		    			broadcast(client_list[i].sockfd, msg);
		    			memset(msg,0,sizeof(msg));
		    			break;
                		}
            		}

            		if (i == FD_SETSIZE) 
            		{
                		printf("Too many clients\n");
                		close(connfd);
            		}
        	}

        	// Check for messages from clients
        	for (int i = 0; i < FD_SETSIZE; i++) 
        	{
            		if (client_list[i].sockfd != -1) 
            		{
                		memset(buf, 0, sizeof(buf));
		    		n = read(client_list[i].sockfd, buf, MESSAGE_SIZE);

	    			if (n > 0) 
	    			{
		    			//broad cast the message
		    			buf[n] = '\0';
    					char message[USERNAME_SIZE + 2 + MESSAGE_SIZE];
					memset(message, 0, sizeof(message));
    					snprintf(message, sizeof(message), "%.*s: %.*s", USERNAME_SIZE, client_list[i].username, MESSAGE_SIZE, buf);
    					broadcast(client_list[i].sockfd, message);
				} 
				else if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) 
				{
    					// Client has terminated connection or a read error occurred, broadcast the left the chat message
    					char msg[USERNAME_SIZE + 20];
    					sprintf(msg,"%s left the chat\n",client_list[i].username);
    					printf("%s left the chat\n", client_list[i].username);
		    			broadcast(client_list[i].sockfd, msg);
		    			memset(msg,0,sizeof(msg));
    					close(client_list[i].sockfd);
    					client_list[i].sockfd = -1;
				}
	    		}
		}
        	usleep(1000);  // Sleep for a short while to reduce CPU usage
    	}
	return 0;
}

