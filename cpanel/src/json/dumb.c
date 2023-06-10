#include "dumb.h"
#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process_value(struct json_value_s *v, char *prefix);
void process_object(struct json_object_s *o, char *prefix);
void process_array(struct json_array_s *a, char *prefix);

char *concat(char *prefix, char *a)
{
	char *result = malloc(strlen(prefix) + strlen(a) + 2);
	strcpy(result, prefix);
	strcat(result, "/");
	strcat(result, a);
	return result;
}

void process_array(struct json_array_s *a, char *prefix)
{
	struct json_array_element_s *e = a->start;

	for (int i = 0; i < a->length && e != NULL; i++)
	{
		char index[10] = {0};
		snprintf(index, 9, "%d", i);
		process_value(e->value, concat(prefix, index));
		e = e->next;
	}
}

void process_object(struct json_object_s *o, char *prefix)
{
	struct json_object_element_s *e = o->start;

	while (e != NULL)
	{
		process_value(e->value, concat(prefix, e->name->string));
		e = e->next;
	}
}

void process_value(struct json_value_s *v, char *prefix)
{
	switch (v->type)
	{
	case json_type_string:
		printf("%s = %s\n", prefix, ((struct json_string_s *)v->payload)->string);
		break;
	case json_type_number:
		printf("%s = %s\n", prefix, ((struct json_number_s *)v->payload)->number);
		break;
	case json_type_false:
		printf("%s = false\n", prefix);
		break;
	case json_type_true:
		printf("%s = true\n", prefix);
		break;
	case json_type_null:
		printf("%s = null\n", prefix);
		break;
	case json_type_object:
		process_object((struct json_object_s *)v->payload, prefix);
		break;
	case json_type_array:
		process_array((struct json_array_s *)v->payload, prefix);
		break;
	}
}

void Pathify(struct json_value_s *v, char *prefix)
{
	process_value(v, prefix);
}
