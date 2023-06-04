#ifndef HUE_H
#define HUE_H

typedef struct {
	char * serverIp;
	int serverPort;
} Hue;

typedef struct {
	int code;
	char *msg;
} HueError;

extern Hue HUE;

HueError *HueLoadServers();

#endif