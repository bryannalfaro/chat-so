//CLIENT
//referencia : basado en https://www.youtube.com/watch?v=fNerEo6Lstw
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<signal.h>
#include<pthread.h>
volatile sig_atomic_t flag = 0;
int sockfd=0;
char name[40];
#define SIZE 40
void str_trim_lf(char* arr, int length){
	int i=0;
	for( i; i<length;i++){
			if(arr[i] == '\n'){
			arr[i] = '\0';
			break;
		}
		}
}


void str_overwrite(){
	printf("%s", ">>>> ");
	fflush(stdout);
}

void catch_exit(){
		flag = 1;
	}

void recv_msg(){
	//printf("im receiving\n");
	char msg[2000] = {};
	while(1){
		int receive = recv(sockfd,msg,2000,0);
		if(receive > 0){
			printf("%s\n",msg);
			str_overwrite();
			
		}else if(receive==0){
				break;
			}
			bzero(msg, 2000);
	}
}

void sendv_msg(){
	//printf("IM sending\n");
	char buffer[2048] = {};
	char msg[2000] = {};
	char option[2000] = {};
	while(1){
		//printf("here");
		str_overwrite();
		//printf("Here 2");
		fgets(msg, SIZE, stdin);
		//printf("here 3");
		str_trim_lf(msg, 2000);
		
		if(strcmp(msg, "exit")==0){
				break;
			}else if(strcmp(msg, "show")==0){
				//printf("f");
				bzero(buffer, 2040);
				sprintf(buffer, "%s", msg);
				send(sockfd, buffer, strlen(buffer),0);
				//send(sockfd, buffer, strlen(buffer),0);
			}
			else if(strcmp(msg, "broadcast")==0){
				//printf("broadcast here\n");
				bzero(buffer, 2040);
				//bzero(msg, 2000);
				
				str_overwrite();
				//printf("Here 2");
				fgets(option, 2000, stdin);
				//printf("here 3");
				str_trim_lf(msg, 2000);
				str_trim_lf(option, 2000);
				sprintf(buffer, "%s",msg);//opcion
				send(sockfd, buffer, strlen(buffer),0);
				bzero(buffer, 2040);
				sprintf(buffer, "%s",option); //mensaje
				//printf("buffer broadcast %s", buffer);
				send(sockfd, buffer, strlen(buffer),0);
				
			}else if(strcmp(msg, "info_user")==0){
				//printf("broadcast here\n");
				bzero(buffer, 2040);
				//bzero(msg, 2000);
				
				str_overwrite();
				//printf("Here 2");
				fgets(option, 2000, stdin);
				//printf("here 3");
				str_trim_lf(msg, 2000);
				str_trim_lf(option, 2000);
				sprintf(buffer, "%s",msg);//opcion
				send(sockfd, buffer, strlen(buffer),0);
				bzero(buffer, 2040);
				sprintf(buffer, "%s",option); //usuario elegido
				//printf("buffer broadcast %s", buffer);
				send(sockfd, buffer, strlen(buffer),0);
				
			}
			else{
				printf("error");
				flag=1;
			}
			bzero(buffer, 2040);
			bzero(msg, 2000);
			bzero(option,2000);
		}
		catch_exit();
}


int main(int argc, char *argv[]){
    char *ip = NULL;
    int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    if(argc < 4){
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    ip = argv[1];
    portno = atoi(argv[2]);
    signal(SIGINT, catch_exit);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("ERROR opening socket");
        exit(1);
    }
	strcpy(name,argv[3]);
	str_trim_lf(name, strlen(name));
	if(strlen(name)<2 || strlen(name) > 39){
		printf("name not correct");
		exit(1);
	}
    

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = inet_addr(ip);

    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("ERROR connecting");
        exit(1);
    }
send(sockfd, name, 40, 0);
printf("WELCOME\n");
printf("Type the option: \n");
printf("show\n");
printf("broadcast\n");
printf("info_user\n");
printf("exit\n");
pthread_t send_msg;
if(pthread_create(&send_msg, NULL ,(void *)sendv_msg,NULL)!=0){
		printf("ERROR");
		exit(1);
	}
pthread_t recv2_msg;
if(pthread_create(&recv2_msg, NULL ,(void *)recv_msg,NULL)!=0){
		printf("ERROR");
		exit(1);
	}

while(1){
		if(flag){
				printf("ADIOS");
				break;
			}
	}
    /*bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if(n < 0){
        perror("ERROR reading from socket");
        exit(1);
    }
    printf("%s\n", buffer);*/
    close(sockfd);
    return 0;

}

