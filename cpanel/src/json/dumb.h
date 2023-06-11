#ifndef DUMB_H
#define DUMB_H

#include "json.h"
#include <stdbool.h>

#define PATHIFY_MAX_NAME_LENGTH 4096

typedef struct pathify_node {
	char name[PATHIFY_MAX_NAME_LENGTH];
	int type;
	void *value;
	struct pathify_node *next;
};

typedef struct pathify {
	struct pathify_node *root;
};

void Pathify(struct json_value_s *v);
struct json_object_element_s *get_key(struct json_object_s *o, char *key);
struct json_object_element_s *get_key_bool(struct json_object_s *o, char *key, bool *result);
struct json_object_element_s *get_key_number(struct json_object_s *o, char *key, char *result);
struct json_object_element_s *get_key_object(struct json_object_s *o, char *key, struct json_object_s *result)
void fill_string_array(struct json_array_s *a, char *dest, int length, int count);

#endif