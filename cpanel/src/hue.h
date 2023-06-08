#ifndef HUE_H
#define HUE_H

#include "raylib.h"

typedef struct {
	bool IsActive;
	char *Name;
	int LightCount;
	char *Lights;
	char *Type;
	bool AnyOn;
	bool AllOn;
} HueGroup;

#define HUE_GROUP_COUNT 255

extern HueGroup groups[HUE_GROUP_COUNT]; 

#endif