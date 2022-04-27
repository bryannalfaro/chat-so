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

static _Atomic unsigned int clients_count = 0; //permite evitar race conditions ya que es variable para todos los hilos
//Estructura para el cliente
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	char name[40];
	char ip_user[20];
	char status[10];
	time_t last_interaction;
} client_t;

typedef struct{
	char message[200];
	char from[200];
	char deliver_at[200];
	char to[200];
} message;

client_t *clients[40]; //Lista de clientes
message *messages[200]; //Lista de mensajes
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

void remove_client(char *id_cliente){
	pthread_mutex_lock(&clients_mutex);
	int j =0;
	for(j ; j<40; j++){
		if(clients[j]!=NULL){
				if(clients[j]->name == id_cliente){ //verificar que sea el id del cliente a  eliminar
				
			clients[j] = NULL; // se elimina
			break;
		}
			}
		
	}
	
	pthread_mutex_unlock(&clients_mutex);
}

void broadcast_message(struct json_object *body, char *name){
		pthread_mutex_lock(&clients_mutex);
		// int l=0;
		//time_t actualTime;
  		//struct tm * timeinfo;

  		//time ( &actualTime );
  		//timeinfo = localtime ( &actualTime );

		message *msg = (message*)malloc(sizeof(message));	
		
		sprintf(msg->message, "%s", json_object_get_string(json_object_array_get_idx(body, 0)));
		sprintf(msg->from, "%s", json_object_get_string(json_object_array_get_idx(body, 1)));
		sprintf(msg->deliver_at, "%s",json_object_get_string(json_object_array_get_idx(body, 2)));
		sprintf(msg->to, "%s",json_object_get_string(json_object_array_get_idx(body, 3)));		
		int j =0;
		for(j ; j<200; j++){
			if(messages[j]==NULL){				
				messages[j] = msg;
				break;
			}
		}
		
		j =0;
		for(j ; j<40; j++){
			if(clients[j]!=NULL){
				if(clients[j]->name == name){ //verificar que sea el id del cliente a  eliminar
				
					char description[200];
					sprintf(description, "{'response': 'POST_CHAT','code':200}");
					write(clients[j]->sockfd, description,strlen(description));
				}else{
					char description[200];
					sprintf(description, "{'response': 'NEW_MESSAGE','body': %s }",json_object_get_string(body));
					write(clients[j]->sockfd, description,strlen(description));
					}
			}
		
		}
		
		pthread_mutex_unlock(&clients_mutex);
	}

void message_user(struct json_object *body, char *name){
		pthread_mutex_lock(&clients_mutex);
		 int l=0;
		// int i = 0;
		message *msg = (message*)malloc(sizeof(message));
		sprintf(msg->message, "%s", json_object_get_string(json_object_array_get_idx(body, 0)));
		sprintf(msg->from, "%s", json_object_get_string(json_object_array_get_idx(body, 1)));		
		sprintf(msg->deliver_at, json_object_get_string(json_object_array_get_idx(body, 2))); //HARDCODED AL IGUAL QUE EN BROADCASTS		
		sprintf(msg->to, json_object_get_string(json_object_array_get_idx(body, 3)));
		
		int j =0;
		for(j ; j<200; j++){
			if(messages[j]==NULL){				
				messages[j] = msg;
				break;
			}
		}
		 for(l; l<40; l++){
			//printf("%d",l);
		    if(clients[l]!=NULL){
			if(clients[l]->name == name){
				char description[200];
				 sprintf(description, "{'response': 'POST_CHAT','code':200}");
				write(clients[l]->sockfd, description,strlen(description));
}
			if(strcmp(clients[l]->name,msg->to)==0){
					
					char description[200];
					sprintf(description, "{'response': 'NEW_MESSAGE','body': %s }",json_object_get_string(body));
					write(clients[l]->sockfd, description,strlen(description));
				}
		 	}
		    }
			
		 
		pthread_mutex_unlock(&clients_mutex);
}

void get_broadcast(char *message, char *client_name){
	pthread_mutex_lock(&clients_mutex);
		int j =0;
		for(j ; j<200; j++){
			
		}
	pthread_mutex_unlock(&clients_mutex);


}

void get_message_user(char *message, char *client_name){
	pthread_mutex_lock(&clients_mutex);
		int j =0;
		for(j ; j<200; j++){

		}
	pthread_mutex_unlock(&clients_mutex);
}


void change_status(char *message, char *client_name){
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		int i = 0;
		for(l; l<40; l++){
			//printf("%d",l);
		   if(clients[l]!=NULL){
			
			if(strcmp(clients[l]->name,client_name)==0){
				bzero(clients[l]->status, strlen(clients[l]->status));
				sprintf(clients[l]->status, "%s", message);
				char description[200];
				sprintf(description, "{'response': 'PUT_STATUS','code':200}");
				write(clients[l]->sockfd, description,strlen(description));
				printf("Changed status to: %s\n", clients[l]->status);
				
				
			}
		   }
			
		}
		pthread_mutex_unlock(&clients_mutex);
	}



void show_connected(char *client_name){
		char users[40][200];
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		int i = 0;
		char description[200];
		char arrayf[1000]="";

		for(l; l<40; l++){
			if(clients[l]!=NULL){
			if(clients[l]->name == client_name){
				for(i; i<40; i++){
					if(clients[i]!=NULL){
						if(clients[i]->name != client_name){	
							//printf("im here");
							
                        				    sprintf(description, " ['%s','%s'], ",clients[i]->status,clients[i]->name);
							    strcat(arrayf, description);
							    bzero(description, 200);
						}  
						
					}
                    
                    		}
				sprintf(description, "{'response': 'GET_USER','code':200,'body':[%s]}",arrayf);
				write(clients[l]->sockfd, description,strlen(description));
				break;
				
				
			}
		}
			
		}
		pthread_mutex_unlock(&clients_mutex);
	}

void send_res(char * name){
		char users[40][200];
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		int i = 0;
        char description[200];
        char arrayf[1000]="";
		for(l; l<40; l++){
			if(clients[l]!=NULL){
			if(clients[l]->name == name){
				
				//char stkr[100] = "'paco','perez'";
				sprintf(description, "{'response': 'END_CONEX','code':200}");
				write(clients[l]->sockfd, description,strlen(description));
				break;
				
				
			}
	}
			
	}
		pthread_mutex_unlock(&clients_mutex);
	}

void info_user(char *name, char *client_name){
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		int k = 0;
		char description[200];
 		char arrayf[1000]="";
		//printf("%s",name);

		for(l; l<40; l++){
			//printf("%d",l);
		   if(clients[l]!=NULL){
			if(clients[l]->name == client_name){
				for(k; k<40; k++){
						if(clients[k]!=NULL){
							if(strcmp(clients[k]->name,name)==0){
								//printf("hereeee");
								bzero(description,200);
								 sprintf(description, "%s", clients[k]->ip_user,clients[k]->status);
								 strcat(arrayf, description);
								bzero(description, 200);
								 sprintf(description, "{'response': 'GET_USER','code':200,'body': ['%s','%s']}",clients[k]->ip_user,clients[k]->status);
								 write(clients[l]->sockfd, description,strlen(description));
								
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
	recv(client->sockfd, buffer, sizeof(buffer),0);
	//Establecer nombre de usuario
	struct json_object *parsed_json;
        struct json_object *request;
        struct json_object *data_body;
        parsed_json= json_tokener_parse(buffer);
        json_object_object_get_ex(parsed_json,"request",&request);
        //printf("%s vino", json_object_get_string(request));
        if(strcmp(json_object_get_string(request),"INIT_CONEX")==0){
        		//printf("si %d",client->uid);
        		struct json_object *body;
        		int i;
        		json_object_object_get_ex(parsed_json,"body",&body);
        		size_t num = json_object_array_length(body);
        		//printf(" este string%s",json_object_get_string(body));
        		bzero(buffer, 2000);
        		for(i=0; i<num; i++){
        			data_body = json_object_array_get_idx(body, i);
        			if(i==0){
        				client->last_interaction = (int)json_object_get_string(data_body);
        			}else{
        				if( strlen(json_object_get_string(data_body))<2 || strlen(json_object_get_string(data_body)) > 39){
			
					printf("NO name");
					active_user = 1 ;
					}
					else{
						strcpy(client->name, json_object_get_string(data_body));
						sprintf(buffer, "(%s) %s se ha unido\n",client->status, client->name);
						printf("%s", buffer);
						
						//broadcast_message(buffer, client->uid); DESCOMENTAR CUANDO SE ARREGLE
					}
        			}
  
        		}
        	}
        write(client->sockfd,"{'response': 'INIT_CONEX','code':200}",44);
        
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
                            show_connected(client->name);
                    }else {
                        sprintf(debug, json_object_get_string(body),strlen(json_object_get_string(body)));
                      printf("%s",debug);
                        info_user((char *)json_object_get_string(body),client->name);
                    }
					bzero(buffer, 2000);

		}
		if(strcmp(json_object_get_string(request), "POST_CHAT")==0){
			//int receive2 = recv(client->sockfd, msg_client, 2000,0);
			bzero(buffer, 2000);
			//printf("im here broadcast");
			// sprintf(buffer, "(%s) %s:%s\n", client->status,client->name,msg_client);
			//str_trim_lf(buffer, strlen(buffer));
			// printf("%s\n", buffer);

			    struct json_object *body;
			    json_object_object_get_ex(parsed_json,"body",&body);
			    
			    if(strcmp(json_object_get_string(json_object_array_get_idx(body, 3)),"all")==0){
					//printf("im here to all broadcast");
							//bzero(buffer, 2000);
				broadcast_message(body, client->name);
			    }else {
				//USER_MSG
				message_user(body, client->name);
			      }

		}/*if(strcmp(buffer, "info_user")==0){
					//int receive2 = recv(client->sockfd, msg_client, 2000,0);
					//bzero(buffer, 2000);
					//printf("im here compare");
					//sprintf(buffer, "%s:%s\n", client->name,msg_client);
					//info_user(msg_client,client->uid);
					//str_trim_lf(buffer, strlen(buffer));
					//printf("%s\n", buffer);

				}*/
		if(strcmp(json_object_get_string(request), "PUT_STATUS")==0){
				bzero(msg_client,2000);

					//printf("im here compare");
					//int receive2 = recv(client->sockfd, msg_client, 2000,0); //mensaje
				    struct json_object *body;
				    json_object_object_get_ex(parsed_json,"body",&body);
				    if(strcmp(json_object_get_string(body),"0")==0){
					    change_status("0", client->name);
				    }else if(strcmp(json_object_get_string(body),"1")==0){
					change_status("1", client->name);
				    }else if(strcmp(json_object_get_string(body),"2")==0){
					change_status("2", client->name);
				    }
				    sprintf(buffer, "changing status %s:%s\n", client->name,json_object_get_string(body));
							printf("%s",buffer);
									

				}

		if(strcmp(json_object_get_string(request), "END_CONEX")==0){
					sprintf(buffer, "%s ha salido\n", client->name);
					printf("%s",buffer);
					send_res(client->name);
					
					// broadcast_message(buffer, client->uid); QUITAR CUANDO SE ARREGLE BROADCAST
					
					active_user = 1;

			}
				
		} 
		}else if(receive ==0){
			sprintf(buffer, "%s ha salido\n", client->name);
			printf("%s",buffer);
			send_res(client->name);
			
			//broadcast_message(buffer, client->uid); QUITAR CUANDO SE ARREGLE BROADCAST 
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
	remove_client(client->name);
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
    serv_addr.sin_addr.s_addr = inet_addr("192.168.1.11"); //change according to machine
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
	
	client_t *client = (client_t*)malloc(sizeof(client_t));
	client->address = cli_addr;
	client->sockfd = newsockfd;
	sprintf(client->ip_user, "%s",inet_ntoa(cli_addr.sin_addr));
	sprintf(status, "0");
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
