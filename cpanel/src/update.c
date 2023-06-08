#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include "update.h"
#include "pthread.h"

#define MAX_SIZE 1024
#define MSG_STOP "exit"
#define QUEUE_NAME "/raysidence"

void *MessageQueueThread(void *arg);

void StartMQThread()
{
	pthread_t pid = -1;

	pthread_create(&pid, NULL, MessageQueueThread, NULL);

	TraceLog(LOG_INFO, TextFormat("Update thread started: %d", pid));
}

void *MessageQueueThread(void *arg)
{
	mqd_t mq;
	char buffer[MAX_SIZE + 1];
	bool keep_going = true;

	mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY);
	if (-1 == mq)
	{
		TraceLog(LOG_ERROR, "Failed to open message queue.");
		return;
	}

	do
	{
		ssize_t c = mq_receive(mq, buffer, MAX_SIZE, NULL);
		if (c == 0)
			continue;

		buffer[c] = '\0';

		printf("\n\nReceived message: %s\n\n", buffer);
	} while (keep_going);
}