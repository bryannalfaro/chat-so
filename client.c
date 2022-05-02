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
	printf("get-global\n");
	printf("get-private\n");
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
		int flag_code=0;

		if(receive > 0){
			struct json_object *code = 200;
			struct json_object *parsed_json;
			parsed_json = json_tokener_parse(msg);
			char search[]="code";
			char *ptr = strstr(msg, search); //TODO Verificar si asi o pasarle code a response de NEW MESSAGE
			if(ptr !=NULL){
				json_object_object_get_ex(parsed_json,"code",&code);
				if(json_object_get_int(code)==200){flag_code=1;}
			}
			struct json_object *response;
			json_object_object_get_ex(parsed_json,"response",&response);
			if(strcmp(json_object_get_string(response),"NEW_MESSAGE")==0){flag_code=1;}
			
			if(flag_code){
				
			if(strcmp(json_object_get_string(response),"PUT_STATUS")==0){
				struct json_object *code;
				json_object_object_get_ex(parsed_json, "code", &code);
				if(json_object_get_int(code) == 200){
						printf("%s\n","Cambio de estado exitoso");
					} 
			}if(strcmp(json_object_get_string(response),"POST_CHAT")==0){
				struct json_object *code;
				json_object_object_get_ex(parsed_json, "code", &code);
				if(json_object_get_int(code) == 200){
						printf("%s\n","Mensaje enviado");
						
					} 
			}if(strcmp(json_object_get_string(response),"END_CONEX")==0){
				struct json_object *code;
				json_object_object_get_ex(parsed_json, "code", &code);
				if(json_object_get_int(code) == 200){
						printf("%s\n","FIN");
						flag=1;
					} 
			}if(strcmp(json_object_get_string(response),"GET_CHAT")==0){ //GET CHAT LISTA DE MENSAJES
				struct json_object *body;
				json_object_object_get_ex(parsed_json, "body", &body);
				size_t n_users = json_object_array_length(body);
				size_t i;
				char state[30];
				struct json_object *user_info;
				if(n_users==0){
					printf("SIN MENSAJES\n");
				}
				for(i=0; i< n_users; i++){
					user_info = json_object_array_get_idx(body, i);
					
					printf("(%s): %s - %s\n", json_object_get_string(json_object_array_get_idx(user_info, 1)), json_object_get_string(json_object_array_get_idx(user_info, 0)), json_object_get_string(json_object_array_get_idx(user_info, 2)));
				}
				
			}if(strcmp(json_object_get_string(response),"NEW_MESSAGE")==0){
				struct json_object *body;
				json_object_object_get_ex(parsed_json, "body", &body);
				
				if(strcmp(json_object_get_string(json_object_array_get_idx(body, 3)),"all")==0){
					printf("Tienes un nuevo mensaje global de %s\n", json_object_get_string(json_object_array_get_idx(body, 1)));
				}else{
					printf("Tienes un nuevo mensaje privado de %s\n", json_object_get_string(json_object_array_get_idx(body, 1)));
				}
				
			}if(strcmp(json_object_get_string(response),"GET_USER")==0){
				struct json_object *code;
				json_object_object_get_ex(parsed_json, "code", &code);

				if(json_object_get_int(code) == 200){
						struct json_object *body;
						json_object_object_get_ex(parsed_json, "body", &body);
						
						size_t num = json_object_array_length(body);
			
						struct json_object *user = json_object_array_get_idx(body,0);
						if(json_object_array_length(user)==2){
							printf("ACTIVE USERS \n");
							size_t n_users = json_object_array_length(body);
							size_t i;
							char state[30];
							struct json_object *user_info;
							for(i=0; i< n_users; i++){
								user_info = json_object_array_get_idx(body, i);
								if(strcmp(json_object_get_string(json_object_array_get_idx(user_info, 1)),"0")==0){
										sprintf(state, "%s", "ACTIVO");
									}
								if(strcmp(json_object_get_string(json_object_array_get_idx(user_info, 1)),"1")==0){
										sprintf(state, "%s", "INACTIVO");
									}
								if(strcmp(json_object_get_string(json_object_array_get_idx(user_info, 1)),"2")==0){
										sprintf(state, "%s", "OCUPADO");
									}
								printf("Estado: %s, Nombre: %s\n", state,json_object_get_string(json_object_array_get_idx(user_info, 0)));
							}
							//printf("%s\n",json_object_get_string(body));
						}else{
								printf("INFO ABOUT USER\n");	
								char state[30];
								if(strcmp(json_object_get_string(json_object_array_get_idx(body, 1)),"0")==0){
										sprintf(state, "%s", "ACTIVO");
									}
								if(strcmp(json_object_get_string(json_object_array_get_idx(body, 1)),"1")==0){
										sprintf(state, "%s", "INACTIVO");
									}
								if(strcmp(json_object_get_string(json_object_array_get_idx(body, 1)),"2")==0){
										sprintf(state, "%s", "OCUPADO");
									}
								printf("ip: %s, estado: %s\n",json_object_get_string(json_object_array_get_idx(body, 0)),state);
						}	
						
					} 
			}

			str_overwrite();
			}
			else if(json_object_get_int(code)==101){
				printf("Usuario ya registrado\n");
				str_overwrite();
			}else if(json_object_get_int(code)==102){
				printf("Usuario no conectado\n");
				str_overwrite();
			}else if(json_object_get_int(code)==103){
				printf("No hay usuarios conectados\n");
				str_overwrite();
			}
			
			
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
		sprintf(description, "Para utilizar el chat debes escribir la opcion que deseas (ejemplo: broadcast)\n Luego de esto debes presionar enter y escribir de acuerdo a tu opcion \n Si eliges show solamente lo debes colocar. \n Si eliges broadcast debes luego escribir tu mensaje. \n Si eliges info_user debes luego escribir el nombre del usuario que deseas. \n Si eliges user_msg debes enviar el mensaje privado de la siguiente manera <usuario> <mensaje>\n Si eliges get-global se obtienen los chats globales. \n Si eliges get-private se obtienen los chats privados\n");
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
				sprintf(buffer, "{\"request\": \"%s\"}",req);//opcion
				send(sockfd, buffer, strlen(buffer),0);
				
			}else if(strcmp(msg, "show")==0){
				
				bzero(buffer, 2040);
				char req[12] = "GET_USER";
				sprintf(buffer, "{\"request\": \"%s\",\"body\": \"all\"}",req);//opcion
			
				send(sockfd, buffer, strlen(buffer),0);
				
			}else if(strcmp(msg, "get-global")==0){
				
				bzero(buffer, 2040);
				char req[12] = "GET_CHAT";
				sprintf(buffer, "{\"request\": \"%s\",\"body\": \"all\"}",req);//opcion
			
				send(sockfd, buffer, strlen(buffer),0);
				
			}else if(strcmp(msg, "get-private")==0){
				
				bzero(buffer, 2040);
				char req[12] = "GET_CHAT";
				sprintf(buffer, "{\"request\": \"%s\",\"body\": \"%s\"}",req,name);//opcion
			
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
				char req[12] = "POST_CHAT";

				char delivered_at[8];
				time_t deliver;
				struct tm *timestruct;
				deliver = time(NULL);
				timestruct = localtime(&deliver);
				sprintf(delivered_at, "%d:%d", timestruct->tm_hour,timestruct->tm_min);
				
				sprintf(buffer, "{\"request\": \"%s\",\"body\": [\"%s\",\"%s\",\"%s\",\"all\"]}",req,option,name,delivered_at);//opcion
				
				send(sockfd, buffer, strlen(buffer),0);
				bzero(buffer, 2040);
				
			}else if(strcmp(msg, "info_user")==0){
				
				bzero(buffer, 2040);
				
				str_overwrite();
				fgets(option, 2000, stdin);
				
				char req[12] = "GET_USER";
				str_trim_lf(option, 2000);
				sprintf(buffer, "{\"request\": \"%s\",\"body\": \"%s\"}",req,option);//opcion
				send(sockfd, buffer, strlen(buffer),0);
				bzero(buffer, 2040);

			}else if(strcmp(msg, "user_msg")==0){
				
				bzero(buffer, 2040);
				
				
				str_overwrite();
				
				fgets(option, 2000, stdin);
				
				//printf("here 3");
				str_trim_lf(msg, 2000);
				str_trim_lf(option, 2000); //a quien mensaje
				sprintf(buffer, "%s",msg);//opcion

				char req[12] = "POST_CHAT";
				str_trim_lf(option, 2000);
				char *s1;
				char *s2;
				char *sp;

				sp = strchr(option, ' ');
				s1 = strndup(option, sp-option);
				s2 =  sp+1;
				char delivered_at[8];
				time_t deliver;
				struct tm *timestruct;
				deliver = time(NULL);
				timestruct = localtime(&deliver);
				sprintf(delivered_at, "%d:%d", timestruct->tm_hour, timestruct->tm_min);
				sprintf(buffer, "{\"request\": \"%s\",\"body\": [\"%s\",\"%s\",\"%s\",\"%s\"]}",req,s2, name, delivered_at, s1);//opcion
				send(sockfd, buffer, strlen(buffer),0);
				bzero(buffer, 2040);
				
				
				
				
			}else if(strcmp(msg, "help")==0){
				
				get_help();
				
				
				
			}else if(strcmp(msg, "change-status")==0){
				printf("Escoge entre > ACTIVO , INACTIVO, OCUPADO \n");
				//printf("broadcast here\n");
				bzero(buffer, 2040);
				//bzero(msg, 2000);
				
				str_overwrite();
				//printf("Here 2");
				fgets(option, 2000, stdin);
				
				//printf("here 3");
				str_trim_lf(msg, 2000); // opcion 
				str_trim_lf(option, 2000);//que cambio hacer
				char choice[1];
				if(strcmp(option, "ACTIVO")==0){
					sprintf(choice, "0");//opcion
					//choice = "0";
				}else if (strcmp(option, "INACTIVO")==0){
					sprintf(choice, "1");//opcion
					//choice = "1";
				}else if (strcmp(option, "OCUPADO")==0){
					sprintf(choice, "2");//opcion
					//choice ="2";
				}
				char req[12] = "PUT_STATUS";
				sprintf(buffer, "{\"request\": \"%s\",\"body\": \"%s\"}",req,choice);//opcion
				
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
		flag=1;
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
	/*if(strlen(name)<2 || strlen(name) > 39){
		printf("name not correct");
		exit(1);
	}*/ //TODO SE IGNORO PARA VALIDAR DESDE EL SERVIDOR
    

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
sprintf(description, " \"%s\", ",timear);
strcat(data,description);
bzero(description, 200);
sprintf(description, " \"%s\" ", name);
strcat(data, description);
//printf("%s", data);

sprintf(buffer, "{\"request\": \"INIT_CONEX\",\"body\": [%s]}",data);//INIT_CONEX
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
			}else if (json_object_get_int(code)==101){
				printf("Usuario ya registrado\n");
				exit(1);
			}else if (json_object_get_int(code)==105){
				printf("Error al ingresar dato\n");
				exit(1);
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

    close(sockfd);
    return 0;

}

