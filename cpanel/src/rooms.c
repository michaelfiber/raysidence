#include "raylib.h"
#include "rooms.h"
#include "json/json.h"
#include "json/cmonson.h"
#include "hue.h"

Room rooms[ROOM_COUNT];

void LoadRoomsJson()
{
	char *rooms_str = LoadFileText("./rooms.json");
	TraceLog(LOG_INFO, "rooms_str loaded from ./rooms.json");
	struct json_parse_result_s *parse_result = NULL;
	struct json_value_s *rooms_root = json_parse_ex(rooms_str, strlen(rooms_str), 0, NULL, NULL, parse_result);
	printf("rooms_root type = %d\n", rooms_root->type);
	if (rooms_root == NULL)
	{
		printf("rooms root is null.\n");
		exit(1);
	}
	struct json_object_s *rooms_o = (struct json *)rooms_root->payload;
	struct json_object_element_s *rooms_e = cos_get_key(rooms_o, "rooms");
	if (rooms_e != NULL)
	{
		printf("Got a rooms object element\n");
	}
	struct json_array_s *rooms_a = ((struct json_array_s *)rooms_e->value->payload);
	printf("Rooms array has %d items\n", rooms_a->length);
	struct json_array_element_s *target = rooms_a->start;
	if (target == NULL)
	{
		printf("target is null\n");
	}
	int current_room = 0;
	while (target != NULL && current_room < ROOM_COUNT)
	{
		struct json_object_s *target_o = (struct json_object_s *)target->value->payload;
		struct json_object_s **rec = NULL;
		if (cos_get_key_object(target_o, "rec", &rec) != NULL)
		{
			int new_int = 0;
			if (cos_get_key_int(rec, "X", &new_int) != NULL)
			{
				rooms[current_room].Rec.x = (float)new_int;
			}
			if (cos_get_key_int(rec, "Y", &new_int) != NULL)
			{
				rooms[current_room].Rec.y = (float)new_int;
			}
			if (cos_get_key_int(rec, "Width", &new_int) != NULL)
			{
				rooms[current_room].Rec.width = (float)new_int;
			}
			if (cos_get_key_int(rec, "Height", &new_int) != NULL)
			{
				rooms[current_room].Rec.height = (float)new_int;
			}
		}

		cos_get_key_string(target_o, "name", rooms[current_room].Name);

		rooms[current_room].IsActive = true;

		current_room++;
		target = target->next;
	}
}