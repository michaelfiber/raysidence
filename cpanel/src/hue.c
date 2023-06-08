#include "hue.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

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