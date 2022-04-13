//CLIENT
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

int main(int argc, char *argv[]){
    char *ip = NULL;
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    if(argc < 3){
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    ip = argv[1];
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("ERROR opening socket");
        exit(1);
    }

    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("ERROR connecting");
        exit(1);
    }
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if(n < 0){
        perror("ERROR reading from socket");
        exit(1);
    }
    printf("%s\n", buffer);
    close(sockfd);
    return 0;

}

