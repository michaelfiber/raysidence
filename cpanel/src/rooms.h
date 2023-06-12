#include "raylib.h"

#ifndef ROOMS_H
#define ROOMS_H

#define ROOM_COUNT 255

typedef struct {
	bool IsActive;
	Rectangle Rec;
	char Name[255];
} Room;

extern Room rooms[ROOM_COUNT];

void LoadRoomsJson();

#endif