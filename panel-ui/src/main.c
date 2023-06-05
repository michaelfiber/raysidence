#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include <math.h>
#include "dialog.h"
#include "coolmouse.h"

#define MODE_USE 0
#define MODE_EDIT 1

typedef struct
{
	Rectangle rec;
	char *name;
} Room;

#define MAX_ROOMS 128
Room rooms[MAX_ROOMS];
int currentRoom = 0;
int mode = MODE_EDIT;
int screenWidth = 800;
int screenHeight = 480;
bool shouldClose = false;

void UpdateUse();
void UpdateEdit();

int main(int argc, char *argv[])
{

	// Accept width and height dimensions are the first two arguments when the binary is called.
	if (argc == 3)
	{
		TraceLog(LOG_INFO, TextFormat("dim %sx%s", argv[1], argv[2]));
		screenWidth = atoi(argv[1]);
		screenHeight = atoi(argv[2]);
	}

	InitWindow(screenWidth, screenHeight, "Raysidence - Smart home tools powered by raylib");

	SetExitKey(KEY_NULL);

	while (!WindowShouldClose() && !shouldClose)
	{
		UpdateCooledMice();

		BeginDrawing();
		{
			ClearBackground(BLACK);

			switch (mode)
			{
			case MODE_EDIT:
				UpdateEdit();
				break;
			case MODE_USE:
				UpdateUse();
				break;
			}
		}

		DrawFPS(10, 10);
		EndDrawing();
	}

	CloseWindow();
}

void UpdateUse()
{
	DrawText("Use Mode", 10, 10, 24, BLUE);
}

void DeleteRoom(int index)
{
	if (index >= MAX_ROOMS)
		return;

	if (rooms[index].name != NULL)
	{
		MemFree(rooms[index].name);
		rooms[index].name = NULL;
	}
}

void UpdateEdit()
{
	static char editRoomName[32] = {0};
	static Vector2 recStart = {0};
	static bool isDrawingRec = false;
	static bool isGettingRoomName = false;
	static bool isFull = false;

	for (int i = 0; i < currentRoom; i++)
	{
		if (rooms[i].name == NULL)
		{
			continue;
		}

		Vector2 textOffset = MeasureTextEx(GetFontDefault(), rooms[i].name, GetFontDefault().baseSize * 2, 2.0f);
		textOffset.x = textOffset.x / 2.0f;
		textOffset.y = textOffset.y / 2.0f;

		DrawRectangleRec(rooms[i].rec, WHITE);
		DrawTextPro(GetFontDefault(), rooms[i].name, (Vector2){rooms[i].rec.x + rooms[i].rec.width / 2, rooms[i].rec.y + rooms[i].rec.height / 2}, textOffset, 0.0f, GetFontDefault().baseSize, 2.0f, BLACK);
	}

	if (IsKeyPressed(KEY_ESCAPE) && !isDrawingRec && !isGettingRoomName)
	{
		shouldClose = true;
	}

	if (!isGettingRoomName && GetMousePosition().x < GetScreenWidth() - 100)
	{
		if (!isDrawingRec && !isFull)
		{
			if (IsCooledMouseButtonPressed(0))
			{
				recStart = GetMousePosition();
				recStart.x = (int)(recStart.x) / 10 * 10;
				recStart.y = (int)(recStart.y) / 10 * 10;
				isDrawingRec = true;
			}
			else if (IsCooledMouseButtonPressed(1))
			{
				for (int i = 0; i < currentRoom; i++)
				{
					if (CheckCollisionPointRec(GetMousePosition(), rooms[i].rec))
					{
						DeleteRoom(i);
						break;
					}
				}
			}
		}
	}

	if (isDrawingRec)
	{
		if (IsKeyPressed(KEY_ESCAPE))
		{
			isDrawingRec = false;
		}

		Vector2 mousePos = GetMousePosition();
		mousePos.x = (int)(mousePos.x) / 10 * 10;
		mousePos.y = (int)(mousePos.y) / 10 * 10;

		Rectangle newRec = {
			fminf(mousePos.x, recStart.x),
			fminf(mousePos.y, recStart.y),
			fmaxf(mousePos.x, recStart.x),
			fmaxf(mousePos.y, recStart.y)};

		newRec.width = newRec.width - newRec.x;
		newRec.height = newRec.height - newRec.y;

		DrawRectangleRec(newRec, WHITE);

		if (IsMouseButtonReleased(0))
		{
			for (int i = 0; i < MAX_ROOMS; i++)
			{
				if (rooms[i].name == NULL)
				{
					currentRoom = i;
					break;
				}
				currentRoom = 0;
			}

			rooms[currentRoom].rec = newRec;
			isDrawingRec = false;
			isGettingRoomName = true;
		}
	}

	if (isGettingRoomName)
	{
		if (IsKeyPressed(KEY_ESCAPE))
		{
			isGettingRoomName = false;

			for (int i = 0; i < 32; i++)
			{
				editRoomName[i] = '\0';
			}
		}

		bool dialogShouldClose = false;
		if (Dialog("room name", editRoomName, 32, &dialogShouldClose))
		{
			if (rooms[currentRoom].name == NULL)
			{
				rooms[currentRoom].name = MemAlloc(sizeof(char) * 33);
			}

			TraceLog(LOG_INFO, TextFormat("Set room name to %s", editRoomName));
			strncpy(rooms[currentRoom].name, editRoomName, 32);
			currentRoom++;

			if (currentRoom >= 32)
			{
				isFull = true;
			}

			for (int i = 0; i < 32; i++)
			{
				editRoomName[i] = '\0';
			}

			isGettingRoomName = false;
		}
		
		if (dialogShouldClose)
		{
			TraceLog(LOG_INFO, "Dialog should close is true!");
			isGettingRoomName = false;
		}
	}

	DrawText("Edit Mode", 10, 10, 24, PURPLE);
}
