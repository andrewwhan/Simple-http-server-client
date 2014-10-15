/*
 * http_client.c
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
#include <sys/time.h>

int main(int argc, char* argv[]){
	int opt;
	int pflag = 0;
	const char* url;
	const char* port;

	//Parse options and other arguments
	while((opt = getopt(argc, argv, "p")) != -1){
		switch(opt){
		case 'p':
			pflag = 1;
			break;
		}
	}
	if((argc - optind) == 2){
		url = argv[optind];
		port = argv[optind+1];
	}
	else{
		printf("Please supply both a url and port. \n");
		return 1;
	}

	char host[128];
	char page[128];

	//Shift url to ignore http:// if necessary
	if(strncmp(url, "http://", 7) == 0){
		url = url+7;
	}

	//Break url into host and page
	switch(sscanf(url, "%[^/]/%s", host, page)){
	case 2:
		break;
	case 1:
		strcpy(page, "");
		break;
	case 0:
		printf("Invalid url \n");
		return 1;
	}
	int status, sockinfo;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(host, port, &hints, &res);
	if(status != 0){
		printf("getaddrinfo error: %s\n", gai_strerror(status));
		return 1;
	}

	sockinfo = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockinfo == -1){
		printf("Socket error \n");
		return 1;
	}


	struct timeval start;
	struct timeval finish;
	gettimeofday(&start, NULL);
	if(connect(sockinfo, res->ai_addr, res->ai_addrlen) == -1){
		printf("Connection error \n");
		close(sockinfo);
		return 1;
	}
	char msg[128];
	snprintf(msg, sizeof(msg), "GET /%s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"Accept: text/html\r\n"
			"Connection: close\r\n"
			"\r\n", page, host);

	if(send(sockinfo, msg, strlen(msg), 0) == -1){
		printf("Send error \n");
		close(sockinfo);
		return 1;
	}


	void* receive = malloc(1500*sizeof(char));
	int returned = recv(sockinfo, receive, 1500, 0);
	gettimeofday(&finish, NULL);
	if(pflag){
		printf("RTT: %d ms\n", (int)((finish.tv_sec - start.tv_sec)*1000 + (finish.tv_usec - start.tv_usec)/1000));
	}
	while(returned != 0){
		fwrite(receive, 1, returned, stdout);
		returned = recv(sockinfo, receive, 1500, 0);
	}
	free(receive);
	close(sockinfo);
	return 0;
}
