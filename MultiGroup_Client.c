#include "unp.h"

void str_cli1(FILE *fp, int sockfd,char *user_name)
{
    	int maxfdp1;
    	fd_set rset;
    	char sendline[MAXLINE], recvline[MAXLINE],buf[MAXLINE-15];

    	FD_ZERO(&rset);

    	for (;;)
    	{
        	FD_SET(fileno(fp), &rset);
        	FD_SET(sockfd, &rset);
        	maxfdp1 = max(fileno(fp), sockfd)+1;
        	int n = select(maxfdp1, &rset, NULL, NULL, NULL);
        	if(n < 0)
        	{
        		perror("select");
        		continue;
        	}

        	if (FD_ISSET(sockfd, &rset))
        	{
            		if (Read(sockfd, recvline, MAXLINE) == 0)
            		{
                		fprintf(stderr, "Server terminated prematurely\n");
                		return;
            		}
            		Fputs(recvline, stdout);
            		memset(recvline,'\0',MAXLINE);
            		
            		fflush(stdout);
            		fflush(stdin);
        	}

        	if (FD_ISSET(fileno(fp), &rset))
        	{
            		if (Fgets(buf, MAXLINE, fp) == NULL)
            		{
                		return; 			// All done
            		}
           
            		sprintf(sendline,"From %s : %s",user_name,buf);
            		Writen(sockfd, sendline, strlen(sendline));
            
            		memset(buf,'\0',MAXLINE);
            		memset(sendline,'\0',MAXLINE);
            		
            		fflush(stdout);
            		fflush(stdin);
            		
        	}
    	}
}


int main(int argc, char **argv) 
{
    	int sockfd;
    	struct sockaddr_in servaddr;

    	if (argc != 3) 
    	{
        	err_quit("usage: tcpcli <IPaddress> <user name>");
    	}

    	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    	bzero(&servaddr, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
    	servaddr.sin_port = htons(SERV_PORT);
    	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
    	char buf[MAXLINE];

    	int n = read(sockfd,buf,MAXLINE);
    	fputs(buf,stdout);
    	close(sockfd);
    	
    	printf("select port number :  ");
    	int port;
    	scanf("%d",&port);

    	int sockfd2 = Socket(AF_INET, SOCK_STREAM, 0);
    	bzero(&servaddr, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
    	servaddr.sin_port = htons(port);
    	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    	Connect(sockfd2, (SA *) &servaddr, sizeof(servaddr));


    	str_cli1(stdin, sockfd2,argv[2]); //business logic

    	exit(0);
}

