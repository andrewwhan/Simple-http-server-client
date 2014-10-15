all: client server

client: http_client.c
	gcc http_client.c -Wall -o http_client
	
server: http_server.c
	gcc http_server.c -Wall -o http_server
	
clean:
	rm -f http_client http_server *.exe