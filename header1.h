
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
			printf("Member %d is Joined in  %s with file descriptor %d\n",++count_join,L.Group_name,connfd);
			
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

