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
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "json/json.h"
#include "json/cmonson.h"
#include "hue.h"

#define MAX_SIZE 32
#define QUEUE_NAME "/raysidence-info"

char *sendline_pattern = "GET /api/%s/%s HTTP/1.0\r\nHost: %s\r\nContent-Type: application/json\r\nContent-length: 0\r\n\r\n\r\n";

char *hue_server_ip;
char *hue_key;

#define MAX_LINE 16000

void Fail(char *msg)
{
	printf("[update.c - ERROR] %s\n", msg);
	exit(1);
}

char response[32768] = {0};

char *hue_query(char *cmd)
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

void *UpdateThread(void *arg)
{
	while (true)
	{
		sleep(1);

		for (int i = 0; i < HUE_GROUP_COUNT; i++)
		{
			groups[i].JustUpdated = false;
		}

		for (int i = 0; i < HUE_LIGHT_COUNT; i++)
		{
			lights[i].JustUpdated = false;
		}

		char *light_response = hue_query("lights");
		struct json_value_s *light_root = json_parse(light_response, strlen(light_response));
		struct json_object_s *lights_o = (struct json_object_s *)light_root->payload;

		char *group_response = hue_query("groups");
		struct json_value_s *groups_root = json_parse(group_response, strlen(group_response));
		struct json_object_s *groups_o = (struct json_object_s *)groups_root->payload;
		struct json_object_element_s *e = groups_o->start;

		while (e != NULL)
		{
			HueGroup *target_group = NULL;

			struct json_object_s *group = (struct json_object_s *)e->value->payload;

			char group_name[255] = {0};
			if (cos_get_key_string(group, "name", group_name) != NULL)
			{
				HueGroup *existing_group = GetGroup(group_name);
				if (existing_group != NULL)
				{
					target_group = existing_group;
				}
				else
				{
					target_group = NextFreeGroup();
				}

				if (target_group == NULL)
				{
					e = e->next;
					continue;
				}

				strncpy(target_group->Name, group_name, 254);
			}

			cos_get_key_string(group, "type", target_group->Type);

			target_group->JustUpdated = true;

			struct json_object_element_s *search = cos_get_key(group, "lights");
			if (search != NULL)
			{
				target_group->LightCount = ((struct json_array_s *)search->value->payload)->length;
				cos_fill_string_array(((struct json_array_s *)search->value->payload), target_group->Lights, 32, 32);
			}

			struct json_object_s **group_state = NULL;
			if (cos_get_key_object(group, "state", &group_state) != NULL)
			{
				cos_get_key_bool(group_state, "any_on", &target_group->AnyOn);
				cos_get_key_bool(group_state, "all_on", &target_group->AllOn);
			}

			for (int i = 0; i < 32; i++)
			{
				char *str = (&target_group->Lights[0] + i * 32);
				int length = strlen(str);
				if (length > 0)
				{
					HueLight *target_light = GetLight(str);
					if (target_light == NULL)
					{
						target_light = NextFreeLight();
					}
					if (target_light == NULL)
					{
						continue;
					}

					target_light->JustUpdated = true;

					strcpy(target_light->Key, str);
					struct json_object_s **light_o = NULL;
					if (cos_get_key_object(lights_o, str, &light_o) != NULL)
					{
						struct json_object_s **state_o = NULL;
						if (cos_get_key_object(light_o, "state", &state_o) != NULL)
						{
							cos_get_key_bool(state_o, "on", &target_light->On);
							cos_get_key_int(state_o, "bri", &target_light->Brightness);
						}

						cos_get_key_string(light_o, "uniqueid", target_light->UniqueId);
						cos_get_key_string(light_o, "type", target_light->Type);
						cos_get_key_string(light_o, "name", target_light->Name);
					}
				}
			}
			e = e->next;
		}

		for (int i = 0; i < HUE_GROUP_COUNT; i++)
		{
			groups[i].IsActive = groups[i].JustUpdated;
		}

		for (int i = 0; i < HUE_LIGHT_COUNT; i++)
		{
			lights[i].IsActive = lights[i].JustUpdated;
		}
	}
}

void StartUpdateThread()
{
	printf("Getting env variables...");
	hue_key = getenv("HUE_KEY");
	hue_server_ip = getenv("HUE_SERVER_IP");
	printf("Done!\n");

	pthread_t pid = -1;

	pthread_create(&pid, NULL, UpdateThread, NULL);

	TraceLog(LOG_INFO, TextFormat("Update thread started: %d", pid));
}
