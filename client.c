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
#include<arpa/inet.h>
#include<json-c/json.h>
#include<time.h>
volatile sig_atomic_t flag = 0;
int sockfd=0;
char name[40];
#define SIZE 40

void show_menu(){
	printf("\nWELCOME TO CHAT\n");
	printf("Type the option, press enter and write: \n");
	printf("show\n");
	printf("broadcast\n");
	printf("info_user\n");
	printf("user_msg\n");
	printf("exit\n");
	printf("help\n");
	printf("change-status\n");
}

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
	char msg[2000] = {};
	while(1){
		bzero(msg, 2000);
		int receive= 0;
		receive = recv(sockfd,msg,2000,0);

		if(receive > 0){
			struct json_object *parsed_json;
			struct json_object *response;
			parsed_json = json_tokener_parse(msg);
			json_object_object_get_ex(parsed_json,"response",&response);
			//printf("%s",json_object_get_string(response));
			if(strcmp(json_object_get_string(response),"PUT_STATUS")==0){
				struct json_object *code;
				json_object_object_get_ex(parsed_json, "code", &code);
				if(json_object_get_int(code) == 200){
						printf("%s\n","Cambio de estado exitoso");
					} 
			}if(strcmp(json_object_get_string(response),"END_CONEX")==0){
				struct json_object *code;
				json_object_object_get_ex(parsed_json, "code", &code);
				if(json_object_get_int(code) == 200){
						printf("%s\n","FIN");
						flag=1;
					} 
			}if(strcmp(json_object_get_string(response),"GET_USER")==0){
				struct json_object *code;
				json_object_object_get_ex(parsed_json, "code", &code);

				if(json_object_get_int(code) == 200){
						struct json_object *body;
						json_object_object_get_ex(parsed_json, "body", &body);
						size_t num = json_object_array_length(body);
						if(num>40){
								//se trata de IP de un usuario
								printf("Se obtuvo lo siguiente \n ip de usuario: %s\n",json_object_get_string(body));
						}else{		//Se trata de un array
								printf("Usuarios activos \n %s\n",json_object_get_string(body));
						}
						
					} 
			}

			str_overwrite();
			
		}else if(receive==0){
				//str_overwrite();
				
				flag=1;
				break;
			}
			
			bzero(msg, 2000);
			//str_overwrite();
	}
}

void get_help(){
		
		char description[3000];
		sprintf(description, "Para utilizar el chat debes escribir la opcion que deseas (ejemplo: broadcast)\n Luego de esto debes presionar enter y escribir de acuerdo a tu opcion \n Si eliges show solamente lo debes colocar. \n Si eliges broadcast debes luego escribir tu mensaje. \n Si eliges info_user debes luego escribir el nombre del usuario que deseas. \n Si eliges user_msg debes enviar el mensaje privado de la siguiente manera <usuario> <mensaje>\n");
		printf("%s",description);
	}

void sendv_msg(){
	char buffer[2048] = {};
	char msg[2000] = {};
	char option[2000] = {};
	while(1){
		str_overwrite();
		fgets(msg, SIZE, stdin);
		str_trim_lf(msg, 2000);
		
		if(strcmp(msg, "exit")==0){
				char req[12] = "END_CONEX";
				sprintf(buffer, "{'request': '%s'}",req);//opcion
				send(sockfd, buffer, strlen(buffer),0);
				
			}else if(strcmp(msg, "show")==0){
				
				bzero(buffer, 2040);
				char req[12] = "GET_USER";
				sprintf(buffer, "{'request': '%s','body': 'all'}",req);//opcion
			
				send(sockfd, buffer, strlen(buffer),0);
				
			}
			else if(strcmp(msg, "broadcast")==0){
				bzero(buffer, 2040);
				
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
				
				bzero(buffer, 2040);
				
				str_overwrite();
				fgets(option, 2000, stdin);
				
				char req[12] = "GET_USER";
				str_trim_lf(option, 2000);
				sprintf(buffer, "{'request': '%s','body': '%s'}",req,option);//opcion
				send(sockfd, buffer, strlen(buffer),0);
				bzero(buffer, 2040);

			}else if(strcmp(msg, "user_msg")==0){
				
				bzero(buffer, 2040);
				
				
				str_overwrite();
				
				fgets(option, 2000, stdin);
				
				//printf("here 3");
				str_trim_lf(msg, 2000);
				str_trim_lf(option, 2000);
				sprintf(buffer, "%s",msg);//opcion
				send(sockfd, buffer, strlen(buffer),0);
				
				bzero(buffer, 2040);
				
				sprintf(buffer, "%s",option); //usuario elegido
				
				send(sockfd, buffer, strlen(buffer),0);
				
				
			}else if(strcmp(msg, "help")==0){
				
				get_help();
				
				
				
			}else if(strcmp(msg, "change-status")==0){
				printf("Escoge entre > ACTIVO , OCUPADO , INACTIVO\n");
				//printf("broadcast here\n");
				bzero(buffer, 2040);
				//bzero(msg, 2000);
				
				str_overwrite();
				//printf("Here 2");
				fgets(option, 2000, stdin);
				
				//printf("here 3");
				str_trim_lf(msg, 2000); // opcion 
				str_trim_lf(option, 2000);//que cambio hacer
				int choice;
				if(strcmp(option, "ACTIVO")==0){
					choice = 0;
				}else if (strcmp(option, "OCUPADO")==0){
					choice = 1;
				}else if (strcmp(option, "INACTIVO")==0){
					choice =2;
				}
				char req[12] = "PUT_STATUS";
				sprintf(buffer, "{'request': '%s','body': '%d'}",req,choice);//opcion
				
				send(sockfd, buffer, strlen(buffer),0);
				
				bzero(buffer, 2040);
				
				
			}
			else{
				printf("error");
				flag=1;
			}
			//printf("estoy aqui");
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

char data[200]="";
char timear[50];
time_t connect_time = time(0); 
sprintf(timear, "%d", connect_time);
char description[200];
sprintf(description, " '%s', ",timear);
strcat(data,description);
bzero(description, 200);
sprintf(description, " '%s' ", name);
strcat(data, description);
//printf("%s", data);

sprintf(buffer, "{'request': 'INIT_CONEX','body': [%s]}",data);//INIT_CONEX
send(sockfd, buffer, sizeof(buffer), 0);
bzero(buffer, 256);
recv(sockfd, buffer, sizeof(buffer),0);
//printf("%s",buffer);
struct json_object *parsed_json;
struct json_object *response;
parsed_json = json_tokener_parse(buffer);
json_object_object_get_ex(parsed_json,"response",&response);

if(strcmp(json_object_get_string(response),"INIT_CONEX")==0){
			struct json_object *code;
			
			json_object_object_get_ex(parsed_json,"code",&code);
			if(json_object_get_int(code)==200){
				show_menu();
			}
	}

//if(recv(sockfd, buffer, sizeof(buffer),0);

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
				printf("ADIOS DEL CHAT \n");
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

