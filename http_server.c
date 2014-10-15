/*
 * http_server.c
 *
 *      Author: Andrew
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char* argv[]){
	if(argc < 2){
		printf("Please supply a port number.");
		return 1;
	}
	const char* port = argv[1];

	struct addrinfo hints, *res;
	int listenSocket;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int status = getaddrinfo(NULL, port, &hints, &res);
	if(status != 0){
		printf("getaddrinfo error: %s\n", gai_strerror(status));
		return 1;
	}
	listenSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(listenSocket == -1){
		printf("Socket error \n");
		return 1;
	}
	if(bind(listenSocket, res->ai_addr, res->ai_addrlen) == -1){
		printf("Bind error \n");
		close(listenSocket);
		return 1;
	}

	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	int commSocket;

	while(1){
		listen(listenSocket, 5);
		addr_size = sizeof(their_addr);
		commSocket = accept(listenSocket, (struct sockaddr *)&their_addr, &addr_size);
		char* msg = malloc(1500*sizeof(char));
		int returned = recv(commSocket, msg, 1500, 0);
		if(returned != 0){
			msg[returned] = 0;
			char method[10];
			char url[100];
			char version[20];
			sscanf(msg, "%9s%99s%19s", method, url, version);
			free(msg);
			if(strcmp(version, "HTTP/1.1")){
				char msg[128];
				snprintf(msg, sizeof(msg), "HTTP/1.1 400 Bad Request \r\n"
						"\r\n");
				send(commSocket, msg, strlen(msg), 0);
				close(commSocket);
				continue;
			}
			if(strcmp(method, "GET")){
				char msg[128];
				snprintf(msg, sizeof(msg), "HTTP/1.1 400 Bad Request \r\n"
						"\r\n");
				send(commSocket, msg, strlen(msg), 0);
				close(commSocket);
				continue;
			}
			if(strcmp(url, "/index.html")){
				char msg[128];
				snprintf(msg, sizeof(msg), "HTTP/1.1 404 File Not Found \r\n"
						"\r\n");
				send(commSocket, msg, strlen(msg), 0);
				close(commSocket);
				continue;
			}

			FILE *fp;
			fp = fopen("TMDG.html", "r");
			if(fp == NULL){
				printf("durr %s\n", strerror(errno));
			}
			struct stat st;
			stat("TMDG.html", &st);
			long lSize = st.st_size;

			char *msg = (char*) malloc(sizeof(char)*(lSize + 128));

			snprintf(msg, sizeof(char)*(lSize + 128), "HTTP/1.1 200 OK\r\n"
					"Content-Length: %ld"
					"Content-Type: text/html; charset=UTF-8\r\n"
					"\r\n", lSize);
			char *msgAfterHeaders = msg + strlen(msg);
			fread(msgAfterHeaders, 1, lSize, fp);
			send(commSocket, msg, strlen(msg), 0);
			fclose(fp);
			free(msg);

		}
		close(commSocket);
	}
	close(listenSocket);
	return 0;
}
