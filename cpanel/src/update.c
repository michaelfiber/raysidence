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


#define MAX_SIZE 32
#define QUEUE_NAME "/raysidence-info"

void *UpdateThread(void *arg);

void StartUpdateThread()
{
	pthread_t pid = -1;

	pthread_create(&pid, NULL, UpdateThread, NULL);

	TraceLog(LOG_INFO, TextFormat("Update thread started: %d", pid));
}

void *UpdateThread(void *arg)
{
	int sfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sfd == -1) {
		printf("could not open socket");
		return;
	}

	while (true)
	{
		printf("Hello\n");
		sleep(5);	
	}
}