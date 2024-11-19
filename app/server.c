#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <parser.h>

#define BUFFER_SIZE 1024

void* handle_client(void* client_fd_ptr){
	int client_fd = *((int*)client_fd_ptr);
	free(client_fd_ptr);
	char buffer[BUFFER_SIZE];

	while(read(client_fd, buffer, 1024) > 0) {
		RESPObject* command = parser(buffer);
		if(!command){
			const char* error_message = "-ERR Invalid command\r\n";
			send(client_fd, error_message, 0);
			continue;
		}
		
		// Logic for handling commands
		if(command->type == RESP_ARRAY && command->array.count > 0){
			RESPObject *first_arg = command->array.elements[0];
			// Handle ping command
			if(first_arg->type == RESP_BULK_STRING && strcmp(first_arg->bulk_string.string_data, "PING") == 0){
				const char* pong_msg = "+PONG\r\n";
				send(client_fd, pong_msg, 0);
			//Handle echo command
			} else if(first_arg->type == RESP_BULK_STRING && strcmp(first_arg->bulk_string.string_data, "ECHO") == 0){
				char response[BUFFER_SIZE];
				send(client_fd, response, 0);
			}
		}
	}
	free(command);
	close(client_fd);
	return NULL;
}

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	
	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(6379),
									 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);


	pthread_t* threads = NULL;
	int* thread_args = NULL;
	int thread_count = 0;
	int max_threads = 2;
	pthread_t thread_id;

	threads = malloc(max_threads * sizeof(pthread_t));
	thread_args = malloc(max_threads * sizeof(int));
	while(1){
		if(thread_count == max_threads){
			max_threads *= 2;
			threads = realloc(threads, max_threads * sizeof(pthread_t));
			thread_args = realloc(thread_args, max_threads * sizeof(int));
			if(!threads || !thread_args){
				perror("Failed to reallocate memory");
				return 1;
			}
		}
		// Accept new client
		int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		if(client_fd == -1){
			perror("Failed to accept client");
			continue;
		}

		int* client_fd_ptr = malloc(sizeof(int));
		*client_fd_ptr = client_fd;

		// Create new thread for client
		pthread_create(&thread_id, NULL, handle_client, client_fd_ptr);
		thread_count++;
		if(thread_count >= 5) break;
	}

	for (int i = 0; i < thread_count; i++){
		pthread_join(threads[i], NULL);
	}

	free(threads);
	free(thread_args);

	printf("Client connected\n");

	close(server_fd);

	return 0;
}
