#include "raylib.h"
#include "sprites.h"
#include "rooms.h"
#include "hue.h"
#include "update.h"

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
	StartMQThread();
	
	InitWindow(800, 480, "raysidence");

	bgFadedBlack = Fade(BLACK, 0.07f);

	center.x = GetScreenWidth() / 2;
	center.y = GetScreenHeight() / 2;

	SetTargetFPS(30);

	// TODO: Start update thread

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

	BeginDrawing();
	{
		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), bgFadedBlack);
		DrawBackgroundGrid(Fade(darkCol, 0.1f));
		DrawSprites();

		for (int i = 0; i < ROOM_COUNT; i++)
		{
			if (!rooms[i].IsActive)
				continue;
			if (rooms[i].GroupIndex >= HUE_GROUP_COUNT)
				continue;

			Room room = rooms[i];
			HueGroup group = groups[room.GroupIndex];
			if (group.AnyOn)
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
			for (int j = 0; j < group.LightCount; j++) {
				
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