#include "unp.h"

int main(int argc, char **argv)
{
	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t n;
	fd_set rset, allset;
	char buf[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
	printf("bind successful..\n");
	
	Listen(listenfd, LISTENQ);
	printf("Listening .....\n");
	
	maxfd = listenfd;
	maxi = -1;
	
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1; 						/* initialize */
										/* index into client[] array */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for ( ; ; ) 
	{
		rset = allset;							/* structure assignment */
		nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);
		
		if (FD_ISSET(listenfd, &rset)) 
		{								/* new client connection*/
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
			printf("Client %d is Joined\n",connfd);
			
			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0) 
				{
					client[i] = connfd; 			/* save descriptor */
					break;
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
				{
										/* connection closed by client */
					Close(sockfd);
					printf("Client %d left the chat\n",sockfd);
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
							//printf("sent to %d\n",send_fd);
							Writen(send_fd, buf, n);
						}
					}
					for(int k=0; k<MAXLINE; k++)
					{
						buf[k] = '\0';
					}
				}
				if (--nready <= 0)
					break;					/* no more readable descriptors */
			}
		}
			
	}
}
