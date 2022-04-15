//Referencia> basado en tutorial de : https://www.youtube.com/watch?v=fNerEo6Lstw
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<signal.h>

static int uid = 10;
static _Atomic unsigned int clients_count = 0;
//Estructura para el cliente
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[40];
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
		if(clients[j]->uid == id_cliente){ //verificar que sea el id del cliente a  eliminar
			clients[j] = NULL; // se elimina
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

void broadcast_message(char *message, int uid){
		pthread_mutex_lock(&clients_mutex);
		int l=0;
		for(l; l<40; l++){
			if(clients[l]!=NULL){
			if(clients[l]->uid != uid){
				write(clients[l]->sockfd, message, strlen(message));
				break;
			}
	}
			
	}
		pthread_mutex_unlock(&clients_mutex);
	}

void *handle_client(void *arg){
	char buffer[2000];
	char name[40];
	int active_user = 0;
	clients_count++;

	client_t *client = (client_t*)arg;
	printf("Bienvenido");
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
		if(receive>0){
			if(strlen(buffer)>0){
				broadcast_message(buffer, client->uid);
				str_trim_lf(buffer, strlen(buffer));
				printf("%s\n", buffer);
			}
		}else if(receive ==0 || strcmp(buffer, "exit")==0){
			sprintf(buffer, "%s ha salido\n", client->name);
			printf("%s",buffer);
			broadcast_message(buffer, client->uid);
			active_user = 1;
		}else{
			printf("ERROR");
			active_user = 1;
		}
		bzero(buffer, 2000);
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
    serv_addr.sin_addr.s_addr = inet_addr("192.168.1.11");
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
	
	add_client(client);
	pthread_create(&tid, NULL, &handle_client, (void*)client); //Hilo del cliente
	sleep(1);        
//close(newsockfd);
    }
    //close(sockfd);

    return 0;
}
