#ifndef DUMB_H
#define DUMB_H

#include "json.h"
#include <stdbool.h>
#include <stdio.h>

struct json_object_element_s *cos_get_key(struct json_object_s *o, char *key);
struct json_object_element_s *cos_get_key_bool(struct json_object_s *o, char *key, bool *result);
struct json_object_element_s *cos_get_key_number(struct json_object_s *o, char *key, char *result);
struct json_object_element_s *cos_get_key_int(struct json_object_s *o, char *key, int *result);
struct json_object_element_s *cos_get_key_object(struct json_object_s *o, char *key, struct json_object_s **result);
struct json_object_element_s *cos_get_key_string(struct json_object_s *o, char *key, char *result);
void cos_fill_string_array(struct json_array_s *a, char *dest, int length, int count);

#endif