#include "unp.h"
#include <stdbool.h>

#define MAXNAME 100
#define MAXCLIENTS FD_SETSIZE


struct lfd_list
{
	char Group_name[10];
	int listen_fd;		
	int port;
};

typedef struct 
{
    	int sockfd;
    	char username[MAXNAME];
    	bool active;
} Client;

Client clients[MAXCLIENTS];

void startChat(int listenfd,struct lfd_list L);

void getgroupInfo(char *buffer,struct lfd_list L[10])
{
	char groupInfo[20];
	for(int i=0; i<10; i++)
	{
		sprintf(groupInfo,"%s %d\n",L[i].Group_name,L[i].port);
		strcat(buffer,groupInfo);
	}
}


int main(int argc, char **argv) 
{
	int main_listenfd;						//declarations
	socklen_t length;
	struct sockaddr_in servaddr;    
	char recvline[MAXLINE], sendline[MAXLINE];

	main_listenfd = Socket(AF_INET,SOCK_STREAM,0);			//main listening socket
		
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);			//main server address
	servaddr.sin_port = htons(SERV_PORT);
		
	if(bind(main_listenfd, (SA *)&servaddr, sizeof(servaddr))< 0)
        {
                perror("bind");
                exit(1);
        }
        
	struct lfd_list L[10];
	int lfds[10];

	for(int i=0; i<10; i++)
	{
		lfds[i] = Socket(AF_INET,SOCK_STREAM,0);		//child server listening sockets
		
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(0);				//server address

		if(bind(lfds[i],(SA *)&servaddr,sizeof(servaddr)) < 0) //binds
                {
                       perror("bind");
                       exit(1);
                }
				
		sprintf(L[i].Group_name, "Group %d",i); 
		struct sockaddr_in temp;
		int temp_len = sizeof(temp);
		getsockname(lfds[i],(SA *)&temp,&temp_len);
		L[i].port = ntohs(temp.sin_port);
		L[i].listen_fd = lfds[i];				//store port number in host byte order
	}
		
	getgroupInfo(sendline,L);
	//printf("%s\n",sendline);

        for(int i=0; i<10; i++)						//create child server processes
	{
		pid_t childpid = fork();		
		if(childpid == 0)
		{
			startChat(lfds[i],L[i]);
		}
	}

	Listen(main_listenfd,LISTENQ);
	printf("main listening socket is Listening..............\n");
	struct sockaddr_in cliaddr;
	socklen_t len ;

	for(;;)								//write the group info to requested client
	{
		int connfd = Accept(main_listenfd,(SA *)&cliaddr,&len);
		writen(connfd,sendline,strlen(sendline));
		Close(connfd);
	}
	
}



void startChat(int listenfd,struct lfd_list L)
{
	int i, maxi, maxfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t n;
	fd_set rset, allset;
	char buf[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr;

	int x = listen(listenfd,LISTENQ);
	if(x < 0)
	{
		perror("listen");
	}
	printf("server started....\n");
	
	maxfd = listenfd;
	maxi = -1;
	
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1; 						/* initialize */
										/* index into client[] array */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	int count_join = 0;
	for ( ; ; ) 
	{
		rset = allset;							/* structure assignment */
		nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);
		
		if (FD_ISSET(listenfd, &rset)) 
		{								/* new client connection*/
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
			printf("Member %d is Joined in  %s with socket descriptor %d\n",++count_join,L.Group_name,connfd);
			
			for (i = 0; i < FD_SETSIZE; i++)
			{	
				if (client[i] < 0) 
				{
					client[i] = connfd; 			/* save descriptor */
					break;
				}
			}
			if (i == FD_SETSIZE)
				err_quit("too many clients");
			FD_SET(connfd, &allset);				/* add new descriptor to set*/

			if (connfd > maxfd)
				maxfd = connfd; 				/* for select */
			if (i > maxi)
				maxi = i;					/* max index in client[] array */
			if (--nready <= 0)
				continue;					/* no more readable descriptors */
		}

		for (i = 0; i <= maxi; i++) 
		{								/* check all clients for data*/
			if ( (sockfd = client[i]) < 0)
			{
				continue;
			}
			if (FD_ISSET(sockfd, &rset)) 
			{
				if ( (n = Read(sockfd, buf, MAXLINE)) == 0) 
				{					/* connection closed by client */
					Close(sockfd);
					printf("Member with socket descriptor %d left the chat from %s\n",sockfd,L.Group_name);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} 
				else
				{
					//printf("%s\n",buf);
					int send_fd;
					for(int j=0; j<=maxi; j++)
					{	
						if((send_fd = client[j]) < 0)
							continue;
						if(send_fd != sockfd)
						{	
							Writen(send_fd, buf, n);
						}
					}
					memset(buf,'\0',MAXLINE);
				}
				if (--nready <= 0)
					break;					/* no more readable descriptors */
			}
		}
			
	}
}


