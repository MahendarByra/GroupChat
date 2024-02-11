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
            // Socket is readable
            if (Readline(sockfd, recvline, MAXLINE) == 0)
            {
                fprintf(stderr, "Server terminated prematurely\n");
                return;
            }
            Fputs(recvline, stdout);
        }

        if (FD_ISSET(fileno(fp), &rset))
        {
            if (Fgets(buf, MAXLINE, fp) == NULL)
            {
                return; 			// All done
            }
            sprintf(sendline,"From %s : %s",user_name,buf);
            Writen(sockfd, sendline, strlen(sendline));
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s dest_ip  USERNAME\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Socket creation
    if (sockfd == -1)
    {
        perror("socket");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr)); // Destination address structure creation
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1)
    {
        perror("inet_pton");
        exit(1);
    }

    printf("Trying to connect to the server...\n");
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) == -1) // Connection to server
    {
        perror("connect");
        exit(1);
    }

    printf("Connected to the server.\n");

    str_cli1(stdin, sockfd,argv[2]);

    // Clean up: Close the socket
    close(sockfd);

    return 0;
}

