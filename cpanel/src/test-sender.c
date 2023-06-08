#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <errno.h>

#define MAX_SIZE 32
#define QUEUE_NAME "/raysidence-info"

int main()
{
	mqd_t mq;
	char buffer[MAX_SIZE];
	unsigned int priority = 0;

	printf("Opening queue...");
	mq = mq_open(QUEUE_NAME, O_WRONLY);

	printf("Got errno: %d\n", errno);

	if (-1 == mq)
	{
		printf("Failed to open message queue\n");
		return -1;
	}

	int i = (int)mq;
	printf("Connected to mq: %d\n", (int)mq);

	memset(buffer, 0, MAX_SIZE);
	printf("Buffer cleared...\n");

	buffer[0] = 'H';
	buffer[1] = 'i';

	printf("Sending message %s...\n", buffer);

	int result = mq_send(mq, buffer, MAX_SIZE, 1);
	int e = errno;

	printf("Got send result %d and errno %d\n", result, e);

	switch (e)
	{
	case 22:
		printf("invalid argument\n");
		break;
	}
}