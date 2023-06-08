#include "raylib.h"

#ifndef ROOMS_H
#define ROOMS_H

#define ROOM_COUNT 255

typedef struct {
	bool IsActive;
	Rectangle Rec;
	char *Name;
	int GroupIndex;
} Room;

extern Room rooms[ROOM_COUNT];

#endif