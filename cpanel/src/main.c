#include "raylib.h"
#include "sprites.h"
#include "rooms.h"
#include "hue.h"
#include "update.h"
#include "stdlib.h"
#include "stdio.h"

int selectedRoom = -1;
int stage = -1;
Vector2 center = {0};
bool isEditing = false;

Color darkCol = MAROON;
Color primaryCol = RED;
Color bgFadedBlack = BLACK;

void Edit();
void Use();
void DrawBackgroundGrid(Color color);

int main()
{
	InitHue();
	StartUpdateThread();

	InitWindow(800, 600, "raysidence");

	bgFadedBlack = Fade(BLACK, 0.07f);

	center.x = GetScreenWidth() / 2;
	center.y = GetScreenHeight() / 2;

	SetTargetFPS(30);

	TraceLog(LOG_INFO, "LoadRoomsJson is done.");

	while (!WindowShouldClose())
	{
		if (isEditing)
		{
			Edit();
		}
		else
		{
			Use();
		}
	}

	CloseWindow();

	return 0;
}

void Use()
{
	UpdateSprites();

	if (IsMouseButtonPressed(0))
	{
		for (int i = 0; i < ROOM_COUNT; i++)
		{
			if (!rooms[i].IsActive)
				continue;

			if (CheckCollisionPointRec(GetMousePosition(), rooms[i].Rec))
			{
				TraceLog(LOG_INFO, TextFormat("clicky click - %s", rooms[i].Name));
				HueGroup *group = GetGroup(rooms[i].Name);
				if (group != NULL)
				{
					int state = 0;
					if (!group->AnyOn)
					{
						state = 1;
					}
					for (int i = 0; i < group->LightCount; i++)
					{
						HueLight *light = GetLight(group->Lights + i * 32);
						if (light != NULL)
						{
							set_light(light->Key, state);
						}
					}
				}
			}
		}
	}

	BeginDrawing();
	{
		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), bgFadedBlack);
		DrawBackgroundGrid(Fade(darkCol, 0.1f));
		DrawSprites();

		for (int i = 0; i < ROOM_COUNT; i++)
		{
			if (!rooms[i].IsActive)
				continue;

			Room room = rooms[i];
			HueGroup *group = GetGroup(room.Name);
			if (group == NULL)
			{
				// it can take a second for the groups to load...
				return;
			}
			if (group->AnyOn)
			{
				DrawRectangleRec(room.Rec, Fade(darkCol, 0.05));
			}

			DrawRectangleLinesEx(room.Rec, 2.0, primaryCol);
			int flicker = GetRandomValue(0, 18) - 6;
			DrawRectangleLinesEx((Rectangle){
									 .x = room.Rec.x - flicker,
									 .y = room.Rec.y - flicker,
									 .width = room.Rec.width + flicker * 2,
									 .height = room.Rec.height + flicker * 2},
								 1 - (flicker + 6) / 18, Fade(darkCol, 0.1f));
			DrawText(room.Name, room.Rec.x + 5, room.Rec.y + 5, GetFontDefault().baseSize * 2, primaryCol);
			for (int j = 0; j < group->LightCount; j++)
			{
				HueLight *light = GetLight(group->Lights + j * 32);
				if (light == NULL)
				{
					DrawText("X", room.Rec.x + 5, room.Rec.y + room.Rec.height - 25 - 25 * j, GetFontDefault().baseSize, primaryCol);
				}
				else
				{
					if (light->On)
					{
						DrawRectangle(room.Rec.x + 5, room.Rec.y + room.Rec.height - 25 - 25 * j, 20, 20, primaryCol);
					}
				}
				DrawRectangleLines(room.Rec.x + 5, room.Rec.y + room.Rec.height - 25 - 25 * j, 20, 20, primaryCol);
				DrawText(light->Name, room.Rec.x + 30, room.Rec.y + room.Rec.height - 20 - 25 * j, GetFontDefault().baseSize, primaryCol);
			}
		}
	}
	EndDrawing();
}

void Edit()
{
}

void DrawBackgroundGrid(Color color)
{
	for (int i = 0; i < 2; i++)
	{
		int x = GetRandomValue(0, GetScreenWidth()) / 50 * 50;
		int y = GetRandomValue(0, GetScreenHeight()) / 50 * 50;
		DrawRectangle(x, y, 50, 50, color);
	}
}