#ifndef HUE_H
#define HUE_H

#include "raylib.h"

typedef struct
{
	bool IsActive;
	char Name[255];
	int LightCount;
	char Lights[255][255];
	char Type[255];
	bool AnyOn;
	bool AllOn;
} HueGroup;

typedef struct
{
	bool IsActive;
	char Key[255];
	char Name[255];
	char Type[255];
	char UniqueId[255];
	bool On;
	int Brightness;
} HueLight;

#define HUE_GROUP_COUNT 255
extern HueGroup groups[HUE_GROUP_COUNT];

#define HUE_LIGHT_COUNT 255
extern HueLight lights[HUE_LIGHT_COUNT];

void SetLight(HueLight light);
HueLight *GetLight(const char *key);

#endif