#include "unp.h"
#define MAXLINE 4096
#define SERV_PORT 9877

int main(int argc, char **argv) 
{
    	int sockfd, n;
    	struct sockaddr_in servaddr;
    	char sendline[MAXLINE], recvline[MAXLINE];

    	if (argc != 3) 
    	{
        	fprintf(stderr, "Usage: %s <IP Address> <Username>\n", argv[0]);
        	exit(1);
    	}

    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0) 
    	{
        	perror("socket error");
        	exit(1);
    	}

    	bzero(&servaddr, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
    	servaddr.sin_port = htons(SERV_PORT);
    	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
																				
    	if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    	{
    		perror("connect");
    		exit(1);
    	}

    	// Send username immediately after connecting
    	write(sockfd, argv[2], strlen(argv[2]));
    	printf("Connected to server with username: %s\n", argv[2]);
    
    	// Set socket to non-blocking mode
    	fcntl(sockfd, F_SETFL, O_NONBLOCK);

    	// Fork the process
    	if (fork() == 0) 
    	{  // Child process to read the data from socket
        	while (1) 
        	{
            		memset(recvline, 0, sizeof(recvline));
	    		n = read(sockfd, recvline, MAXLINE);
            	
            		if (n > 0) 
            		{
                		recvline[n] = '\0';  // Null terminate
                		//printf("Received from server: %s", recvline);
				fputs(recvline, stdout);
            		} 
            		else if (n < 0) 
            		{
                		if (errno != EAGAIN && errno != EWOULDBLOCK) 
                		{
                    			perror("read error");
                    			exit(1);
                		}
            		}
            		usleep(1000);  // Sleep to reduce CPU usage
        	}
    	} 
    	else 
    	{  // Parent process to send the data to the server
        	while (1) 
        	{
			memset(sendline, 0, sizeof(sendline));
            		if (fgets(sendline, MAXLINE, stdin) != NULL) 
            		{
                		write(sockfd, sendline, strlen(sendline));
            		}
        	}
    	}

    	close(sockfd);
    	return 0;
}

