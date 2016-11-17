/* Name: Tushev, Miroslav
 * Project: PA-3a
 * Instructor: Feng Chen
 * Class: cs7103-au16
 * LogonID: cs710305
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* size of buffer used for message exchange */
#define BUFSIZE 1000

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFSIZE];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    printf("Connection is established\n");
    printf("*********************************\n");
    printf("Welcome to Public Bulletin Board!\n");
    printf("Please, enter your username\n");
    while(1) {
        printf("> ");
        memset(buffer,'\0',BUFSIZE);
        fgets(buffer,BUFSIZE,stdin);
        n = write(sockfd,buffer,BUFSIZE);
          if (n < 0)
               error("ERROR writing to socket");
          memset(buffer,'\0',BUFSIZE);

        n = read(sockfd,buffer,BUFSIZE);
          if (n < 0)
               error("ERROR reading from socket");
          printf("%s\n",buffer);
          memset(buffer,'\0',BUFSIZE);

    }
    close(sockfd);
    return 0;
}
