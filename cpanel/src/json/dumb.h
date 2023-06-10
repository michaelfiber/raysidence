#ifndef DUMB_H
#define DUMB_H

#include "json.h"

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

#endif