// Referencia> basado en tutorial de : https://www.youtube.com/watch?v=fNerEo6Lstw
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <time.h>
#include <json-c/json.h>

static _Atomic unsigned int clients_count = 0; // permite evitar race conditions ya que es variable para todos los hilos
// Estructura para el cliente
typedef struct
{
	struct sockaddr_in address;
	int sockfd;
	char name[40];
	char ip_user[20];
	char status[10];
	char last_interaction[50];
} client_t;

// Estructura para mensaje
typedef struct
{
	char message[200];
	char from[200];
	char deliver_at[200];
	char to[200];
} message;

client_t *clients[40];	// Lista de clientes
message *messages[200]; // Lista de mensajes
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void str_trim_lf(char *arr, int length)
{
	int i = 0;
	for (i; i < length; i++)
	{
		if (arr[i] == '\n')
		{
			arr[i] == '\0';
			break;
		}
	}
}

void add_client(client_t *cliente)
{
	pthread_mutex_lock(&clients_mutex);
	int j = 0;
	int flag = 0;
	for (j; j < 40; ++j)
	{
		if (clients[j] == NULL)
		{ // verificar que ese espacio este vacio
			flag = 1;
			clients[j] = cliente; // agregarlo al queue
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void remove_client(char *id_cliente)
{
	pthread_mutex_lock(&clients_mutex);
	int j = 0;
	for (j; j < 40; j++)
	{
		if (clients[j] != NULL)
		{
			if (clients[j]->name == id_cliente)
			{ // verificar que sea el id (nombre) del cliente a  eliminar

				clients[j] = NULL; // se elimina
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void broadcast_message(struct json_object *body, char *name)
{
	pthread_mutex_lock(&clients_mutex);

	message *msg = (message *)malloc(sizeof(message));

	sprintf(msg->message, "%s", json_object_get_string(json_object_array_get_idx(body, 0)));
	sprintf(msg->from, "%s", json_object_get_string(json_object_array_get_idx(body, 1)));
	sprintf(msg->deliver_at, "%s", json_object_get_string(json_object_array_get_idx(body, 2)));
	sprintf(msg->to, "%s", json_object_get_string(json_object_array_get_idx(body, 3)));
	int j = 0;
	for (j; j < 200; j++)
	{
		if (messages[j] == NULL)
		{
			messages[j] = msg;
			break;
		}
	}

	j = 0;
	for (j; j < 40; j++)
	{
		if (clients[j] != NULL)
		{
			if (clients[j]->name == name)
			{ // verificar que sea el id del cliente que mando el broadcast

				char description[200];
				sprintf(description, "{\"response\": \"POST_CHAT\",\"code\":200}");
				write(clients[j]->sockfd, description, strlen(description));
			}
			else
			{ // A los demas usuarios
				char description[200];
				sprintf(description, "{\"response\": \"NEW_MESSAGE\",\"body\": %s }", json_object_get_string(body));
				write(clients[j]->sockfd, description, strlen(description));
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void message_user(struct json_object *body, char *name)
{
	pthread_mutex_lock(&clients_mutex);
	int l = 0;
	int flag = 0;
	message *msg = (message *)malloc(sizeof(message));
	sprintf(msg->message, "%s", json_object_get_string(json_object_array_get_idx(body, 0)));
	sprintf(msg->from, "%s", json_object_get_string(json_object_array_get_idx(body, 1)));
	sprintf(msg->deliver_at, json_object_get_string(json_object_array_get_idx(body, 2)));
	sprintf(msg->to, json_object_get_string(json_object_array_get_idx(body, 3)));

	int j = 0;
	for (j; j < 200; j++)
	{
		if (messages[j] == NULL)
		{
			messages[j] = msg;
			break;
		}
	}
	for (l; l < 40; l++)
	{
		if (clients[l] != NULL)
		{

			if (strcmp(clients[l]->name, msg->to) == 0)
			{
				flag = 1;
				char description[200];
				sprintf(description, "{\"response\": \"NEW_MESSAGE\",\"body\": %s }", json_object_get_string(body));
				if (flag)
				{
					write(clients[l]->sockfd, description, strlen(description));
					break;
				}
			}
		}
	}
	if (flag == 0 || flag == 1)
	{
		l = 0;
		for (l; l < 40; l++)
		{
			if (clients[l]->name == name)
			{
				if (flag == 0)
				{
					char description[200];
					sprintf(description, "{\"response\": \"POST_CHAT\",\"code\":102}"); // Enviar a usuario que mando el mensaje si falla
					write(clients[l]->sockfd, description, strlen(description));
				}
				else
				{
					char description[200];
					sprintf(description, "{\"response\": \"POST_CHAT\",\"code\":200}"); // Mandar si no falla
					write(clients[l]->sockfd, description, strlen(description));
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void get_broadcast_message(char *client_name)
{
	pthread_mutex_lock(&clients_mutex);
	char description[200];
	char arrayf[1000] = "";
	int l = 0;
	int j = 0;
	for (l; l < 40; l++)
	{
		if (clients[l] != NULL)
		{
			if (clients[l]->name == client_name)
			{
				for (j; j < 200; j++)
				{
					if (messages[j] != NULL)
					{ // SE COMPRUEBA QUE NO SEA NULO
						if (strcmp(messages[j]->to, "all") == 0)
						{
							sprintf(description, " [\"%s\",\"%s\", \"%s\"], ", messages[j]->message, messages[j]->from, messages[j]->deliver_at);
							strcat(arrayf, description);
							bzero(description, 200);
						}
					}
				}
				sprintf(description, "{\"response\": \"GET_CHAT\",\"code\":200,\"body\":[%s]}", arrayf);
				write(clients[l]->sockfd, description, strlen(description));
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void get_message_user(char *client_name)
{
	pthread_mutex_lock(&clients_mutex);
	char description[200];
	char arrayf[1000] = "";
	int l = 0;
	int j = 0;
	for (l; l < 40; l++)
	{
		if (clients[l] != NULL)
		{
			if (strcmp(clients[l]->name, client_name) == 0)
			{
				for (j; j < 200; j++)
				{
					if (messages[j] != NULL)
					{
						if (strcmp(messages[j]->to, client_name) == 0)
						{
							//printf("si %s, %s\n", messages[j]->to, client_name);
							sprintf(description, " [\"%s\",\"%s\", \"%s\"], ", messages[j]->message, messages[j]->from, messages[j]->deliver_at);
							strcat(arrayf, description);
							bzero(description, 200);
						}
					}
				}
				sprintf(description, "{\"response\": \"GET_CHAT\",\"code\":200,\"body\":[%s]}", arrayf);
				write(clients[l]->sockfd, description, strlen(description));
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void change_status(char *message, char *client_name)
{
	pthread_mutex_lock(&clients_mutex);
	int l = 0;
	int i = 0;
	for (l; l < 40; l++)
	{
		// printf("%d",l);
		if (clients[l] != NULL)
		{

			if (strcmp(clients[l]->name, client_name) == 0)
			{
				bzero(clients[l]->status, strlen(clients[l]->status));
				sprintf(clients[l]->status, "%s", message);
				char description[200];
				sprintf(description, "{\"response\": \"PUT_STATUS\",\"code\":200}");
				write(clients[l]->sockfd, description, strlen(description));
				printf("Changed status to: %s\n", clients[l]->status);
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

void show_connected(char *client_name)
{
	char users[40][200];
	pthread_mutex_lock(&clients_mutex);
	int l = 0;
	int i = 0;
	int flag = 0;
	char description[200];
	char arrayf[1000] = "";

	for (l; l < 40; l++)
	{
		if (clients[l] != NULL)
		{
			if (clients[l]->name == client_name)
			{
				for (i; i < 40; i++)
				{
					if (clients[i] != NULL)
					{
						if (clients[i]->name != client_name)
						{
							flag = 1;
							sprintf(description, " [\"%s\",\"%s\"], ", clients[i]->status, clients[i]->name);
							strcat(arrayf, description);
							bzero(description, 200);
						}
					}
				}
				if (flag)
				{
					sprintf(description, "{\"response\": \"GET_USER\",\"code\":200,\"body\":[%s]}", arrayf);
					write(clients[l]->sockfd, description, strlen(description));
					break;
				}
				if (flag == 0)
				{
					char description[200];
					sprintf(description, "{\"response\": \"GET_USER\",\"code\":103}");
					write(clients[l]->sockfd, description, strlen(description));
				}
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

void send_res(char *name)
{
	char users[40][200];
	pthread_mutex_lock(&clients_mutex);
	int l = 0;
	int i = 0;
	char description[200];
	char arrayf[1000] = "";
	for (l; l < 40; l++)
	{
		if (clients[l] != NULL)
		{
			if (clients[l]->name == name)
			{
				sprintf(description, "{\"response\": \"END_CONEX\",\"code\":200}");
				write(clients[l]->sockfd, description, strlen(description));
				break;
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

void info_user(char *name, char *client_name)
{
	pthread_mutex_lock(&clients_mutex);
	int l = 0;
	int k = 0;
	int flag = 0;
	char description[200];
	char arrayf[1000] = "";

	for (l; l < 40; l++)
	{
		if (clients[l] != NULL)
		{
			if (clients[l]->name == client_name)
			{
				for (k; k < 40; k++)
				{
					if (clients[k] != NULL)
					{
						if (strcmp(clients[k]->name, name) == 0)
						{
							flag = 1;
							bzero(description, 200);
							strcat(arrayf, description);
							bzero(description, 200);
							sprintf(description, "{\"response\": \"GET_USER\",\"code\":200,\"body\": [\"%s\",\"%s\"]}", clients[k]->ip_user, clients[k]->status);
							if (flag)
							{
								write(clients[l]->sockfd, description, strlen(description));
								break;
							}
						}
					}
				}
				if (flag == 0)
				{
					char description[200];
					sprintf(description, "{\"response\": \"GET_USER\",\"code\":102}");
					write(clients[l]->sockfd, description, strlen(description));
					break;
				}
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

int verify_name(char *name, char *ip)
{
	pthread_mutex_lock(&clients_mutex);
	int valid = 1;
	int j = 0;
	for (j; j < 40; j++)
	{
		if (clients[j] != NULL)
		{
			if ((strcmp(clients[j]->name, name) == 0) && (strcmp(clients[j]->ip_user, ip) == 0))
			{ // TODO YA VALIDA IP Y NOMBRE ,CAMBIAR POR OR SI ES ALGUNA SOLO VER SI VAN JUNTOS O SOLO UNO DE LOS DOS
				valid = 0;
				break;
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
	return valid;
}

void *handle_client(void *arg)
{
	char buffer[2000];
	char option[2000];
	char msg_client[15];
	char user_msg[2000];
	char name[40];
	char debug[20];
	int active_user = 0;
	int validuser2 = 0;
	clients_count++;

	client_t *client = (client_t *)arg;
	recv(client->sockfd, buffer, sizeof(buffer), 0);
	// Establecer nombre de usuario
	struct json_object *parsed_json;
	struct json_object *request;
	struct json_object *data_body;
	parsed_json = json_tokener_parse(buffer);
	json_object_object_get_ex(parsed_json, "request", &request);
	if (strcmp(json_object_get_string(request), "INIT_CONEX") == 0)
	{
		struct json_object *body;
		int i;
		json_object_object_get_ex(parsed_json, "body", &body);
		size_t num = json_object_array_length(body);
		bzero(buffer, 2000);
		for (i = 0; i < num; i++)
		{
			data_body = json_object_array_get_idx(body, i);
			if (i == 0)
			{
				strcpy(client->last_interaction, json_object_get_string(data_body));
			}
			else
			{
				if (strlen(json_object_get_string(data_body)) < 2 || strlen(json_object_get_string(data_body)) > 39)
				{
					printf("NO valid name\n");
					write(client->sockfd, "{\"response\": \"INIT_CONEX\",\"code\":105}", 44);
					active_user = 1;
					break;
				}
				else
				{
					strcpy(client->name, json_object_get_string(data_body));
					int valid1 = verify_name(client->name, client->ip_user);
					printf("%d", valid1);
					if (valid1)
					{
						add_client(client);
						sprintf(buffer, "(%s) %s se ha unido\n", client->status, client->name);
						printf("%s", buffer);
						write(client->sockfd, "{\"response\": \"INIT_CONEX\",\"code\":200}", 44);
						validuser2 = 1;
					}
					else
					{
						write(client->sockfd, "{\"response\": \"INIT_CONEX\",\"code\":101}", 44);
						validuser2 = 0;
					}
				}
			}
		}
	}
	bzero(buffer, 2000);
	while (1)
	{

		int j = 0;
		for (j; j < 40; j++)
		{
			if(clients[j] != NULL){
				if (clients[j] != client)
				{
					time_t actualTime;
					actualTime = time(0);

					if (((float)actualTime - atof(clients[j]->last_interaction)) > (float)10.00)
					{
						char status[10];
						sprintf(status, "1");
						sprintf(clients[j]->status, "%s", status);
					}
				}
			}
		}
		
		

		if (active_user)
		{
			break;
		}
		if (validuser2 == 0)
		{
			break;
		}
		int receive = recv(client->sockfd, buffer, 2000, 0);

		if (receive > 0)
		{
			if (strlen(buffer) > 0)
			{
				// Aqui parsear
				struct json_object *parsed_json;
				struct json_object *request;
				parsed_json = json_tokener_parse(buffer);
				json_object_object_get_ex(parsed_json, "request", &request);

				if (strcmp(json_object_get_string(request), "GET_USER") == 0)
				{

					char status[10];
					sprintf(status, "0");
					sprintf(client->status, "%s", status);
;
					time_t actualTime;
					actualTime = time(0);
					char userTime[50];
					sprintf(userTime, "%f", (float)actualTime);
					strcpy(client->last_interaction, userTime);

			

					struct json_object *body;
					json_object_object_get_ex(parsed_json, "body", &body);

					if (strcmp(json_object_get_string(body), "all") == 0)
					{
						show_connected(client->name);
					}
					else
					{
						sprintf(debug, json_object_get_string(body), strlen(json_object_get_string(body)));
						printf("%s", debug);
						info_user((char *)json_object_get_string(body), client->name);
					}
					bzero(buffer, 2000);
				}
				if (strcmp(json_object_get_string(request), "GET_CHAT") == 0)
				{ // OBTENER LISTA DE MENSAJES GLOBALES Y DE USUARIO

					char status[10];
					sprintf(status, "0");
					sprintf(client->status, "%s", status);

					time_t actualTime;
					actualTime = time(0);
					char userTime[50];
					sprintf(userTime, "%f", (float)actualTime);
					strcpy(client->last_interaction, userTime);

					struct json_object *body;
					char private_name[40];
					json_object_object_get_ex(parsed_json, "body", &body);

					if (strcmp(json_object_get_string(body), "all") == 0)
					{
						get_broadcast_message(client->name);
					}
					else
					{
						sprintf(private_name, "%s", json_object_get_string(body));
						get_message_user(private_name);
					}
					bzero(buffer, 2000);
				}
				if (strcmp(json_object_get_string(request), "POST_CHAT") == 0)
				{
					char status[10];
					sprintf(status, "0");
					sprintf(client->status, "%s", status);

					time_t actualTime;
					actualTime = time(0);
					char userTime[50];
					sprintf(userTime, "%f", (float)actualTime);
					strcpy(client->last_interaction, userTime);

					bzero(buffer, 2000);
					struct json_object *body;
					json_object_object_get_ex(parsed_json, "body", &body);
					// time_t actualTime = time (0);

					if (strcmp(json_object_get_string(json_object_array_get_idx(body, 3)), "all") == 0)
					{
						broadcast_message(body, client->name);
					}
					else
					{
						// USER_MSG
						message_user(body, client->name);
					}
				}

				if (strcmp(json_object_get_string(request), "PUT_STATUS") == 0)
				{

					char status[10];
					sprintf(status, "0");
					sprintf(client->status, "%s", status);

					time_t actualTime;
					actualTime = time(0);
					char userTime[50];
					sprintf(userTime, "%f", (float)actualTime);
					strcpy(client->last_interaction, userTime);
					
					bzero(msg_client, 2000);

					struct json_object *body;
					json_object_object_get_ex(parsed_json, "body", &body);
					if (strcmp(json_object_get_string(body), "0") == 0)
					{
						change_status("0", client->name);
					}
					else if (strcmp(json_object_get_string(body), "1") == 0)
					{
						change_status("1", client->name);
					}
					else if (strcmp(json_object_get_string(body), "2") == 0)
					{
						change_status("2", client->name);
					}
					sprintf(buffer, "changing status %s:%s\n", client->name, json_object_get_string(body));
					printf("%s", buffer);
				}

				if (strcmp(json_object_get_string(request), "END_CONEX") == 0)
				{
					sprintf(buffer, "%s ha salido\n", client->name);
					printf("%s", buffer);
					send_res(client->name);

					active_user = 1;
				}
			}
		}
		else if (receive == 0)
		{
			sprintf(buffer, "%s ha salido\n", client->name);
			printf("%s", buffer);
			send_res(client->name);

			active_user = 1;
		}
		else
		{
			printf("ERROR");
			active_user = 1;
		}
		bzero(buffer, 2000);
		bzero(msg_client, 2000);
		bzero(option, 2000);
		bzero(user_msg, 2000);
	}
	if (active_user)
	{
		close(client->sockfd);
		remove_client(client->name);
		free(client);
		clients_count--;
		pthread_detach(pthread_self());
		return NULL;
	}
	if (validuser2 == 0)
	{
		close(client->sockfd);
		free(client);
		clients_count--;
		pthread_detach(pthread_self());
		return NULL;
	}
}

int main(int argc, char **argv)
{
	int sockfd, newsockfd, portno, clilen;
	pthread_t tid;
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2)
	{
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("ERROR opening socket");
		exit(1);
	}

	portno = atoi(argv[1]);
	int option = 1;
	char status[10];
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("192.168.1.11"); // change according to machine
	serv_addr.sin_port = htons(portno);

	signal(SIGPIPE, SIG_IGN);

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR on binding");
		exit(1);
	}

	listen(sockfd, 10);
	// printf("WELCOME");
	while (1)
	{
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0)
		{
			perror("ERROR on accept");
			exit(1);
		}

		client_t *client = (client_t *)malloc(sizeof(client_t));
		client->address = cli_addr;
		client->sockfd = newsockfd;
		sprintf(client->ip_user, "%s", inet_ntoa(cli_addr.sin_addr));
		sprintf(status, "0");
		sprintf(client->status, "%s", status);


		pthread_create(&tid, NULL, &handle_client, (void *)client); // Hilo del cliente
	}
	signal(SIGPIPE, SIG_IGN);
	close(newsockfd);
	close(sockfd);

	return 0;
}
