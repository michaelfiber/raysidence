#include "cmonson.h"
#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

void Pathify(struct json_value_s *v)
{
	process_value(v, "");
}

struct json_object_element_s *cos_get_key(struct json_object_s *o, char *key)
{
	struct json_object_element_s *t = o->start;
	while (t != NULL)
	{
		if (strncmp(t->name->string, key, strlen(key)) == 0)
		{
			return t;
		}
		t = t->next;
	}
	return NULL;
}

struct json_object_element_s *cos_get_key_bool(struct json_object_s *o, char *key, bool *result)
{
	struct json_object_element_s *e = cos_get_key(o, key);
	if (e != NULL)
	{
		*result = e->value->type == json_type_true;
	}
	return e;
}

struct json_object_element_s *cos_get_key_object(struct json_object_s *o, char *key, struct json_object_s **result)
{
	struct json_object_element_s *e = cos_get_key(o, key);
	if (e != NULL)
	{
		*result = (struct json_object_s *)e->value->payload;
	}
	return e;
}

struct json_object_element_s *cos_get_key_number(struct json_object_s *o, char *key, char *result)
{
	struct json_object_element_s *e = cos_get_key(o, key);
	if (e != NULL)
	{
		strcpy(result, ((struct json_number_s *)e->value->payload)->number);
	}
	return e;
}

struct json_object_element_s *cos_get_key_int(struct json_object_s *o, char *key, int *result)
{
	char num_str[128] = {0};
	struct json_object_element *e = cos_get_key_number(o, key, num_str);
	if (e != NULL)
	{
		*result = atoi(num_str);
		return e;
	}
	return NULL;
}

struct json_object_element_s *cos_get_key_string(struct json_object_s *o, char *key, char *result)
{
	struct json_object_element_s *e = cos_get_key(o, key);
	if (e != NULL)
	{
		strcpy(result, ((struct json_string_s *)e->value->payload)->string);
	}
	return e;
}

void cos_fill_string_array(struct json_array_s *a, char *dest, int length, int count)
{
	struct json_array_element_s *e = a->start;
	int i = 0;
	while (e != NULL && i < count)
	{
		int index = i * length;
		strncpy(dest + index, ((struct json_string_s *)e->value->payload)->string, length - 1);
		i++;
		e = e->next;
	}
}