#include "hue.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "json/json.h"
#include "json/cmonson.h"

char response[32768] = {0};
char *hue_server_ip;
char *hue_key;
char *sendline_pattern = "%s /api/%s/%s HTTP/1.0\r\nHost: %s\r\nContent-Type: application/json\r\nContent-length: %d\r\n\r\n%s\r\n";

void InitHue()
{
	hue_key = getenv("HUE_KEY");
	hue_server_ip = getenv("HUE_SERVER_IP");
}

HueGroup groups[HUE_GROUP_COUNT] = {0};
HueLight lights[HUE_LIGHT_COUNT] = {0};

void SetLight(HueLight light)
{
	int i = -1;

	for (int j = 0; j < HUE_LIGHT_COUNT; j++)
	{
		if (!lights[j].IsActive)
		{
			if (i == -1)
			{
				i = j;
			}
			continue;
		}

		if (strncmp(lights[j].Key, light.Key, 32) == 0)
		{
			i = j;
			break;
		}
	}

	lights[i].IsActive = true;
	snprintf(lights[i].Key, 255, "%s", light.Key);
	snprintf(lights[i].Name, 255, "%s", light.Name);
	snprintf(lights[i].Type, 255, "%s", light.Type);
	snprintf(lights[i].UniqueId, 255, "%s", light.UniqueId);
	lights[i].On = light.On;
	lights[i].Brightness = light.Brightness;
}

HueLight *GetLight(const char *key)
{
	for (int i = 0; i < HUE_LIGHT_COUNT; i++)
	{
		if (strncmp(lights[i].Key, key, 32) == 0)
		{
			return &lights[i];
		}
	}

	return NULL;
}

HueLight *NextFreeLight()
{
	for (int i = 0; i < HUE_LIGHT_COUNT; i++)
	{
		if (!lights[i].IsActive)
		{
			lights[i].IsActive = true;
			return &lights[i];
		}
	}
	return NULL;
}

HueGroup *GetGroup(const char *name)
{
	for (int i = 0; i < HUE_GROUP_COUNT; i++)
	{
		if (strncmp(groups[i].Name, name, 255) == 0)
		{
			return &groups[i];
		}
	}

	return NULL;
}

HueGroup *NextFreeGroup()
{
	for (int i = 0; i < HUE_GROUP_COUNT; i++)
	{
		if (!groups[i].IsActive)
		{
			groups[i].IsActive = true;
			return &groups[i];
		}
	}
	return NULL;
}

void LogHueStuff(bool activeOnly)
{
	for (int i = 0; i < HUE_GROUP_COUNT; i++)
	{
		if (activeOnly && !groups[i].IsActive)
			continue;
		printf("Group details: %d\n\t%s\n\t%d\n\t%s\n\t%d\n\t%d\n\t%d\n", groups[i].IsActive, groups[i].Name, groups[i].LightCount, groups[i].Type, groups[i].AnyOn, groups[i].AllOn, groups[i].JustUpdated);
	}
}

// "1" 1 = turn light "1" on. "7" 0 = turn light "7" off.
void set_light(char *light_name, int state)
{
	HueLight *light = GetLight(light_name);
	if (light == NULL)
		return;
	char cmd[256] = {0};
	sprintf(cmd, "lights/%s/state", light_name);
	if (state == 1)
	{
		hue_query(cmd, "PUT", "{\"on\": true}");
	}
	else
	{
		hue_query(cmd, "PUT", "{\"on\": false}");
	}
}

char *hue_query(char *cmd, char *method, char *body)
{
	memset(&response, 0, 32768);

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

	int content_length = 0;

	if (body != NULL)
	{
		content_length = strlen(body);
	}
	else
	{
		body = "";
	}

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
	int body_length = snprintf(&sendline[0], 1024, sendline_pattern, method, hue_key, cmd, hue_server_ip, content_length, body);
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
	for (int i = 0; i < total_size - 10; i++)
	{
		if (strncmp("\r\n\r\n", &response[i], 4) == 0)
		{
			response_body_index = i + 4;
		}
	}

	if (response_body_index > -1)
	{
		return &response[response_body_index];
	}

	return NULL;
}
