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
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "json/json.h"
#include "json/cmonson.h"
#include "hue.h"
#include "rooms.h"

#define MAX_SIZE 32
#define QUEUE_NAME "/raysidence-info"

#define MAX_LINE 16000

void Fail(char *msg)
{
	printf("[update.c - ERROR] %s\n", msg);
	exit(1);
}

bool is_first_run = true;

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

		char *light_response = hue_query("lights", "GET", NULL);
		struct json_value_s *light_root = json_parse(light_response, strlen(light_response));
		struct json_object_s *lights_o = (struct json_object_s *)light_root->payload;

		char *group_response = hue_query("groups", "GET", NULL);
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

		if (is_first_run)
		{
			LoadRoomsJson();
			is_first_run = false;
		}
	}
}

void StartUpdateThread()
{
	pthread_t pid = -1;

	pthread_create(&pid, NULL, UpdateThread, NULL);

	TraceLog(LOG_INFO, TextFormat("Update thread started: %d", pid));
}
