#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>

#define REQUEST_BUFFSIZE 256
#define BUFFSIZE 10240

/* Not proper, actually */
#define ERROR_SOCKET 1
#define ERROR_FILEOP 2

typedef struct http_packet {
	/* TODO: The request_port should be removed */
	char request_url[64];
	unsigned long request_port;
	char request_line[64];
	const char *options[64];
	unsigned int options_num;
	char payload[REQUEST_BUFFSIZE];
} HTTPPacket;

void format_header(char header_buf[], const char *request_line, const char *options[],
		size_t options_num) {
	/* TODO: Unsafe, currently. */
	strcpy(header_buf, request_line);
	for(size_t i = 0; i < options_num; i++) {
		strcat(header_buf, "\r\n");
		strcat(header_buf, options[i]);
	}
	strcat(header_buf, "\r\n\r\n");
}

int http_get(HTTPPacket *packet) {
	int sockfd;
	struct sockaddr_in server_addr;
	char payload[BUFFSIZE] = "";
	/* Initialized the target information */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		/* Shold do a proper err handling */
		return ERROR_SOCKET;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, (const char*) packet->request_url, &(server_addr.sin_addr));
	server_addr.sin_port = htons(packet->request_port);

	/* Intialized a GET message */
	char get_message[REQUEST_BUFFSIZE] = "";
	
	format_header(get_message, packet->request_line, packet->options, packet->options_num);

	/* Connect */
	if ( connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) 
		return ERROR_SOCKET;
	
	/* Request */
	if (write(sockfd, get_message, strlen(get_message)) == -1) return ERROR_FILEOP;
	
	while (read(sockfd, payload, BUFFSIZE-1) > 0)
		printf("%s\n", payload);
	close(sockfd);
	return 0;
}

int main() {
	/* TODO: Not gracefully dealing with the requests message, currently. */
	HTTPPacket request;
	memset(&request, 0, sizeof(HTTPPacket));
	strcpy(request.request_url, "140.112.42.89");
	request.request_port = 80;
	strcpy(request.request_line, "GET / HTTP/1.1");
	const char *header_options[] = {
		"Host: 140.112.42.89:80",
		"Connection: close"
	};
	request.options_num = 2;
	request.options[0] = header_options[0];
	request.options[1] = header_options[1];
	memset(request.payload, 0, REQUEST_BUFFSIZE);
	return http_get(&request);
}
