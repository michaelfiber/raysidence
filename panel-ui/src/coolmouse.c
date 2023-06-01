#include "raylib.h"

float cooldowns[3] = {0.0f, 0.0f, 0.0f};

void UpdateCooledMice()
{
	for (int i = 0; i < 3; i++)
	{
		cooldowns[i] += GetFrameTime();
	}
}

bool IsCooledMouseButtonPressed(int b)
{
	if (cooldowns[b] > 0.75f && IsMouseButtonPressed(b))
	{
		TraceLog(LOG_INFO, "cooled down mouse was pressed.");
		cooldowns[b] = 0.0f;
		return true;
	}
	return false;
}