//Referencia> basado en tutorial de : https://www.youtube.com/watch?v=fNerEo6Lstw
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<signal.h>
#include<arpa/inet.h>
#include<time.h>
#include <json-c/json.h>

static int uid = 1;
static _Atomic unsigned int clients_count = 0; //permite evitar race conditions ya que es variable para todos los hilos
//Estructura para el cliente
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[40];
	char ip_user[20];
	char status[10];
	time_t last_interaction;
} client_t;
client_t *clients[40]; //Lista de clientes
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;


void str_trim_lf(char* arr, int length){
	int i=0;
	for( i; i<length;i++){
			if(arr[i] == '\n'){
			arr[i] == '\0';
			break;
		}
		}
}

void add_client(client_t *cliente){
	pthread_mutex_lock(&clients_mutex);
	int j =0;
	for(j; j<40; ++j){
		if(clients[j] == NULL || clients[j]->name == cliente->name){ //verificar que ese espacio este vacio
			clients[j] = cliente; //agregarlo al queue
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int id_cliente){
	pthread_mutex_lock(&clients_mutex);
	int j =0;
	for(j ; j<40; j++){
		if(clients[j]!=NULL){
				if(clients[j]->uid == id_cliente){ //verificar que sea el id del cliente a  eliminar
			clients[j] = NULL; // se elimina
			break;
		}
			}
		
	}
	pthread_mutex_unlock(&clients_mutex);
}

void broadcast_message(char *message, int uid){
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		//time_t actualTime;
  		//struct tm * timeinfo;

  		//time ( &actualTime );
  		//timeinfo = localtime ( &actualTime );
		for(l; l<40; l++){
			//printf("%d",l);
			if(clients[l]!=NULL){
				//printf("si entre");
			if(clients[l]->uid != uid){
				//printf("here broadcast\n");
				//printf("nombre broadcast %s\n",clients[l]->name);
				char description[200];
				sprintf(description, "(public) %s\n",message);
				write(clients[l]->sockfd,description, strlen(description));
				//sprintf("");
				//break;
			}
	}
			
	}
		pthread_mutex_unlock(&clients_mutex);
	}

void message_user(char *message,char *receiver_user, int uid_sender){
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		int i = 0;
		for(l; l<40; l++){
			//printf("%d",l);
		   if(clients[l]!=NULL){
			if(clients[l]->uid == uid_sender){
				for(i;i<40;i++){
					if(strcmp(clients[i]->name,receiver_user)==0){
						char description[200];
						sprintf(description, "(private) (%s) %s: %s\n",clients[l]->status,clients[l]->name, message);
						write(clients[i]->sockfd, description, strlen(description));
						break;
					}
				}
				
			}
		   }
			
		}
		pthread_mutex_unlock(&clients_mutex);
	}

void change_status(char *message,int uid_client){
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		int i = 0;
		for(l; l<40; l++){
			//printf("%d",l);
		   if(clients[l]!=NULL){
			if(clients[l]->uid == uid_client){
				bzero(clients[l]->status, strlen(clients[l]->status));
				sprintf(clients[l]->status, "%s", message);
                char description[200];
                sprintf(description, "{'response': 'PUT_STATUS','code':200}");
                write(clients[l]->sockfd, description,strlen(description));
				printf("yey: %s", clients[l]->status);
				
				
			}
		   }
			
		}
		pthread_mutex_unlock(&clients_mutex);
	}



void show_connected(int uid){
		char users[40][200];
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		int i = 0;
        char description[200];
        char arrayf[1000]="";
		for(l; l<40; l++){
			if(clients[l]!=NULL){
			if(clients[l]->uid == uid){
				for(i; i<40; i++){
					if(clients[i]!=NULL){
						if(clients[i]->uid != uid){	
							//printf("im here");
							
                            sprintf(description, " '(%s) %s', ",clients[i]->status,clients[i]->name);
							strcat(arrayf, description);
							bzero(description, 200);
						}
						
					}
                    
                    
                    //send(clients[l]->sockfd, description, strlen(description),0);	
				}
                //char stkr[100] = "'paco','perez'";
                sprintf(description, "{'response': 'GET_USER','code':200,'body':[%s]}",arrayf);
                write(clients[l]->sockfd, description,strlen(description));
                break;
				
				
			}
	}
			
	}
		pthread_mutex_unlock(&clients_mutex);
	}

void info_user(char *name, int uid){
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		int k = 0;
		char description[200];
		//printf("%s",name);

		for(l; l<40; l++){
			//printf("%d",l);
		   if(clients[l]!=NULL){
			if(clients[l]->uid == uid){
				for(k; k<40; k++){
						if(clients[k]!=NULL){
							if(strcmp(clients[k]->name,name)==0){
								//printf("hereeee");
								bzero(description,200);
                
                                 sprintf(description, "{'response': 'GET_USER','code':200,'body':'%s'}",clients[k]->ip_user);
                                 write(clients[l]->sockfd, description,strlen(description));
								//sprintf(description, "Nombre: %s, Ip: %s, Estado: %s", clients[k]->name, clients[k]->ip_user, clients[k]->status);
								//send(clients[l]->sockfd, description, strlen(description),0);
                                break;
								}  
								
						}
				}
				if(strlen(description)==0){
						write(clients[l]->sockfd, "No existe", 9);
				}
				
			}
			
		   }
			
		}
		
		pthread_mutex_unlock(&clients_mutex);
	}

void *handle_client(void *arg){
	char buffer[2000];
	char option[2000];
	char msg_client[15];
	char user_msg[2000];
	char name[40];
    char debug[20];
	int active_user = 0;
	clients_count++;

	client_t *client = (client_t*)arg;
	printf("Bienvenido %d con ip: %s ", client->uid, client->ip_user);
	//Establecer nombre de usuario
	if(recv(client->sockfd, name, 40,0)<=0 || strlen(name)<2 || strlen(name) > 39){
			
		printf("NO name");
		active_user = 1 ;
		}
	else{
		strcpy(client->name, name);
		sprintf(buffer, "(%s) %s se ha unido\n",client->status, client->name);
		printf("%s", buffer);
		//broadcast_message(buffer, client->uid); DESCOMENTAR CUANDO SE ARREGLE
	}
	bzero(buffer, 2000);
	while(1){
		//bzero(buffer, 2000);
		
		if(active_user){
			break;
		}
		int receive = recv(client->sockfd, buffer, 2000, 0);
		//int receive2 = recv(client->sockfd, msg_client, 2000,0);
		//printf("buffer %s",buffer);
		if(receive>0){
			//printf("im here re");
			if(strlen(buffer)>0){
				//Aqui parsear
                struct json_object *parsed_json;
                struct json_object *request;
                parsed_json= json_tokener_parse(buffer);
                json_object_object_get_ex(parsed_json,"request",&request);

				if(strcmp(json_object_get_string(request), "GET_USER")==0){
                    struct json_object *body;
                    json_object_object_get_ex(parsed_json,"body",&body);
                    
                    if(strcmp(json_object_get_string(body), "all")==0){
                            show_connected(client->uid);
                    }else {
                        sprintf(debug, json_object_get_string(body),strlen(json_object_get_string(body)));
                      printf("%s",debug);
                        info_user(json_object_get_string(body),client->uid);
                    }
					bzero(buffer, 2000);

				}
				if(strcmp(buffer, "broadcast")==0){
					int receive2 = recv(client->sockfd, msg_client, 2000,0);
					bzero(buffer, 2000);
					//printf("im here compare");
					sprintf(buffer, "(%s) %s:%s\n", client->status,client->name,msg_client);
					broadcast_message(buffer, client->uid);
					//str_trim_lf(buffer, strlen(buffer));
					printf("%s\n", buffer);

				}/*if(strcmp(buffer, "info_user")==0){
					//int receive2 = recv(client->sockfd, msg_client, 2000,0);
					//bzero(buffer, 2000);
					//printf("im here compare");
					//sprintf(buffer, "%s:%s\n", client->name,msg_client);
					//info_user(msg_client,client->uid);
					//str_trim_lf(buffer, strlen(buffer));
					//printf("%s\n", buffer);

				}*/if(strcmp(buffer, "user_msg")==0){
					//printf("im here compare");
					int receive2 = recv(client->sockfd, msg_client, 2000,0); //user mensaje
					sprintf(buffer, "initial %s:%s\n", client->name,msg_client);
					printf("%s",buffer);
					// se separa por primer espacio
					char *s1;
					char *s2;
					char *sp;

					sp = strchr(msg_client, ' ');
					s1 = strndup(msg_client, sp-msg_client);
					s2 =  sp+1;

					//const char delimitier[] = "-";
					//char *mess_2;
					//strtok(msg_client, delimitier);
					//mess_2 = strtok(NULL, delimitier);
					
					bzero(buffer, 2000);
					//printf("im here compare");
					//sprintf(buffer, "hehe%s:%s\n", client->name,msg_client);
					//printf("%s\n", buffer);
					
					message_user(s2, s1, client->uid);
					
					//show_connected(client->uid);
					//str_trim_lf(buffer, strlen(buffer));
					//printf("hoho%s\n", buffer);

				}if(strcmp(json_object_get_string(request), "PUT_STATUS")==0){
					bzero(msg_client,2000);

					//printf("im here compare");
					//int receive2 = recv(client->sockfd, msg_client, 2000,0); //mensaje
					struct json_object *body;
                    json_object_object_get_ex(parsed_json,"body",&body);
                    if(json_object_get_int(body)==0){
                            change_status("ACTIVO", client->uid);
                    }else if(json_object_get_int(body)==1){
                        change_status("OCUPADO", client->uid);
                    }else if(json_object_get_int(body)==2){
                        change_status("INACTIVO", client->uid);
                    }
                    sprintf(buffer, "changing status %s:%d\n", client->name,json_object_get_int(body));
					printf("%s",buffer);
									

				}

				if(strcmp(buffer, "exit")==0){
					sprintf(buffer, "%s ha salido\n", client->name);
					printf("%s",buffer);
					broadcast_message(buffer, client->uid);
					active_user = 1;

				}
				
			} 
		}else if(receive ==0 || strcmp(buffer, "exit")==0){
			sprintf(buffer, "%s ha salido\n", client->name);
			printf("%s",buffer);
			broadcast_message(buffer, client->uid);
			active_user = 1;
		}
		
		else{
			printf("ERROR");
			active_user = 1;
		}
		bzero(buffer, 2000);
		bzero(msg_client,2000);
		bzero(option, 2000);
		bzero(user_msg,2000);
	}
	close(client->sockfd);
	remove_client(client->uid);
	free(client);
	clients_count--;
	pthread_detach(pthread_self());
	return NULL;
}

int main(int argc, char **argv){
    int sockfd, newsockfd, portno, clilen;
    pthread_t tid;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if(argc < 2){
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("ERROR opening socket");
        exit(1);
    }
   
    portno = atoi(argv[1]);
    int option = 1;
    char status[10];
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("192.168.1.13"); //change according to machine
    serv_addr.sin_port = htons(portno);
	
	signal(SIGPIPE, SIG_IGN);
   
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("ERROR on binding");
        exit(1);
    }

    listen(sockfd, 10);
	//printf("WELCOME");
    while(1){
	
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0){
            perror("ERROR on accept");
            exit(1);
            }
	//char d[300];
	//sprintf(d, "Your ip: %s", inet_ntoa(cli_addr.sin_addr)); 
        //send(newsockfd, "Hello, world from server!\n", 26, 0);
	//send(newsockfd,(const void *)d,25,0);
	client_t *client = (client_t*)malloc(sizeof(client_t));
	client->address = cli_addr;
	client->sockfd = newsockfd;
	client->uid = uid++;
	sprintf(client->ip_user, "%s",inet_ntoa(cli_addr.sin_addr));
	sprintf(status, "ACTIVO");
	sprintf(client->status, "%s", status); 
	add_client(client);
	pthread_create(&tid, NULL, &handle_client, (void*)client); //Hilo del cliente
	//sleep(1);        

    }
	signal(SIGPIPE, SIG_IGN);
    close(newsockfd);
    close(sockfd);

    return 0;
}
