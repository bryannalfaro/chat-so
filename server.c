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

static int uid = 1;
static _Atomic unsigned int clients_count = 0;
//Estructura para el cliente
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[40];
	char ip_user[20];
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
		if(clients[j] == NULL){ //verificar que ese espacio este vacio
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
						sprintf(description, "(private) %s: %s\n",clients[l]->name, message);
						write(clients[i]->sockfd, description, strlen(description));
						break;
					}
				}
				
			}
		   }
			
		}
		pthread_mutex_unlock(&clients_mutex);
	}

void get_help(client_t *client){
		pthread_mutex_lock(&clients_mutex);
		char description[3000];
		sprintf(description, "Para utilizar el chat debes escribir la opcion que deseas (ejemplo: broadcast)\n Luego de esto debes presionar enter y escribir de acuerdo a tu opcion \n Si eliges show solamente lo debes colocar. \n Si eliges broadcast debes luego escribir tu mensaje. \n Si eliges info_user debes luego escribir el nombre del usuario que deseas. \n Si eliges user_msg debes enviar el mensaje privado de la siguiente manera <usuario>-<mensaje>");
		write(client->sockfd, description, strlen(description));

		pthread_mutex_unlock(&clients_mutex);
	}

void show_connected(int uid){
		
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		int i = 0;
		for(l; l<40; l++){
			if(clients[l]!=NULL){
			if(clients[l]->uid == uid){
				for(i; i<40; i++){
					if(clients[i]!=NULL){
						if(clients[i]->uid != uid){	
							//printf("im here");
							char description[200];
							sprintf(description, "Nombre: %s\n",clients[i]->name);
							send(clients[l]->sockfd, description, strlen(description),0);
							bzero(description, 200);
						}
						
					}	
				}
				
				
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
								sprintf(description, "Nombre: %s, Ip: %s", clients[k]->name, clients[k]->ip_user);
								send(clients[l]->sockfd, description, strlen(description),0);
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
		sprintf(buffer, "%s se ha unido\n", client->name);
		printf("%s", buffer);
		broadcast_message(buffer, client->uid);
	}
	bzero(buffer, 2000);
	while(1){
		if(active_user){
			break;
		}
		int receive = recv(client->sockfd, buffer, 2000, 0);
		//int receive2 = recv(client->sockfd, msg_client, 2000,0);
		//printf("buffer %s",buffer);
		if(receive>0){
			//printf("im here re");
			if(strlen(buffer)>0){
				
				if(strcmp(buffer, "show")==0){
					sprintf(option, buffer);
					bzero(buffer, 2000);
					//printf("im here compare");
					sprintf(buffer, "%s:%s\n", client->name,option);
					printf("%s",buffer);
					bzero(buffer, 2000);
					show_connected(client->uid);
					bzero(buffer, 2000);

				}
				if(strcmp(buffer, "broadcast")==0){
					int receive2 = recv(client->sockfd, msg_client, 2000,0);
					bzero(buffer, 2000);
					//printf("im here compare");
					sprintf(buffer, "%s:%s\n", client->name,msg_client);
					broadcast_message(buffer, client->uid);
					str_trim_lf(buffer, strlen(buffer));
					printf("%s\n", buffer);

				}if(strcmp(buffer, "info_user")==0){
					int receive2 = recv(client->sockfd, msg_client, 2000,0);
					bzero(buffer, 2000);
					//printf("im here compare");
					sprintf(buffer, "%s:%s\n", client->name,msg_client);
					info_user(msg_client,client->uid);
					str_trim_lf(buffer, strlen(buffer));
					printf("%s\n", buffer);

				}if(strcmp(buffer, "user_msg")==0){
					bzero(msg_client,2000);
					bzero(user_msg,2000);
					//printf("im here compare");
					int receive2 = recv(client->sockfd, msg_client, 2000,0); //mensaje
					sprintf(buffer, "initial %s:%s\n", client->name,msg_client);
					printf("%s",buffer);
					const char delimitier[] = "-";
					char *mess_2;
					strtok(msg_client, delimitier);
					mess_2 = strtok(NULL, delimitier);
					
					bzero(buffer, 2000);
					//printf("im here compare");
					//sprintf(buffer, "hehe%s:%s\n", client->name,msg_client);
					//printf("%s\n", buffer);
					
					message_user(mess_2, msg_client, client->uid);
					
					//show_connected(client->uid);
					str_trim_lf(buffer, strlen(buffer));
					//printf("hoho%s\n", buffer);
					bzero(buffer, 2000);
					bzero(msg_client,2000);
					bzero(option, 2000);
					bzero(user_msg,2000);

				}if(strcmp(buffer, "help")==0){
					get_help(client);

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
	
	add_client(client);
	pthread_create(&tid, NULL, &handle_client, (void*)client); //Hilo del cliente
	sleep(1);        

    }
	signal(SIGPIPE, SIG_IGN);
    close(newsockfd);
    close(sockfd);

    return 0;
}
