#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "update.h"
#include "pthread.h"
#include <unistd.h>
#define CISSON_IMPLEMENTATION
#include "cisson/cisson.h"
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_SIZE 32
#define QUEUE_NAME "/raysidence-info"

char *sendline_pattern = "GET /api/%s/%s HTTP/1.0\r\nHost: %s\r\nContent-Type: application/json\r\nContent-length: 0\r\n\r\n\r\n";

char *hue_server_ip;
char *hue_key;

#define MAX_LINE 16000

char response[32768] = {0};

void Fail(char *msg)
{
	printf("[update.c - ERROR] %s\n", msg);
	exit(1);
}

char *hue_query(char *cmd)
{
	int total_size = 0;
	char sendline[1024];
	char buffer[BUFSIZ];
	struct protoent *proto;
	struct hostent *host;
	int sockfd = -1;
	in_addr_t target_addr;
	struct sockaddr_in sock_addr;
	unsigned int server_port = 80u;
	ssize_t bytes_read;

	// Get a socket.
	proto = getprotobyname("tcp");
	if (proto == NULL)
		Fail("failed to get proto 'tcp'");

	sockfd = socket(AF_INET, SOCK_STREAM, proto->p_proto);
	if (sockfd == -1)
		Fail("failed to get socket");

	// Prepare target address.
	host = gethostbyname(hue_server_ip);
	if (host == NULL)
		Fail("failed to get host");

	target_addr = inet_addr(inet_ntoa(*(struct in_addr *)*(host->h_addr_list)));
	if (target_addr == (in_addr_t)-1)
		Fail("error with inet_addr");

	sock_addr.sin_addr.s_addr = target_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(server_port);

	// Connect
	if (connect(sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
		Fail("failed to connect");

	// Send
	int body_length = snprintf(&sendline[0], 1024, sendline_pattern, hue_key, cmd, hue_server_ip);
	if (write(sockfd, sendline, body_length) == -1)
		Fail("failed writing to socket");

	// Receive
	while ((bytes_read = read(sockfd, buffer, BUFSIZ)) > 0)
	{
		strncpy(response + total_size, buffer, bytes_read);
		total_size += bytes_read;
	}

	close(sockfd);

	int response_body_index = -1;
	for (int i = 0; i < total_size-10; i++)
	{
		if (strncmp("\r\n\r\n", &response[i], 4) == 0)
		{
			response_body_index = i + 4;
		}
	}

	

	if (response_body_index > -1) {
		return &response[response_body_index];
	}

	return NULL;
}

void *UpdateThread(void *arg)
{
	while (true)
	{
		printf("Update\n");

		char *response = hue_query("groups");
		
		if (response != NULL) {
			printf("Found this JSON for groups: %s\n", response);
		}

		struct json_tree json_tree = {0};
		rjson(response, &json_tree);
		printf("json parsed as:\n");
		puts(to_string_pointer(&json_tree, query(&json_tree, "/1/Name")));
		
		sleep(5);
	}
}

void StartUpdateThread()
{
	hue_key = getenv("HUE_KEY");
	hue_server_ip = getenv("HUE_SERVER");

	pthread_t pid = -1;

	pthread_create(&pid, NULL, UpdateThread, NULL);

	TraceLog(LOG_INFO, TextFormat("Update thread started: %d", pid));
}
