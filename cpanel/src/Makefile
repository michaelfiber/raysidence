.ONESHELL:

BIN_NAME=cpanel

build:
	mkdir -p ../../bin
	gcc update.c hue.c rooms.c sprites.c main.c -o ../../bin/$(BIN_NAME) -lraylib -lm -ldl -lpthread -lrt

run: build
	../../bin/$(BIN_NAME)

build.test-client:
	mkdir -p ../../bin
	gcc test-sender.c -o ../../bin/test-client -lrt

run.test-client: build.test-client
	../../bin/test-client