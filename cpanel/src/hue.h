#ifndef HUE_H
#define HUE_H

#include "raylib.h"

typedef struct HueGroup
{
	bool IsActive;
	char Name[255];
	int LightCount;
	char Lights[32 * 32];
	char Type[255];
	bool AnyOn;
	bool AllOn;
	bool JustUpdated;
} HueGroup;

typedef struct HueLight
{
	bool IsActive;
	char Key[255];
	char Name[255];
	char Type[255];
	char UniqueId[255];
	bool On;
	int Brightness;
	bool JustUpdated;
} HueLight;

#define HUE_GROUP_COUNT 255
extern HueGroup groups[HUE_GROUP_COUNT];

#define HUE_LIGHT_COUNT 255
extern HueLight lights[HUE_LIGHT_COUNT];

void InitHue();
void SetLight(HueLight light);
HueLight *GetLight(const char *key);
HueLight *NextFreeLight();
HueGroup *GetGroup(const char *name);
HueGroup *NextFreeGroup();
void LogHueStuff(bool activeOnly);

void set_light(char *light_name, int state);
char *hue_query(char *cmd, char *method, char *body);

#endif