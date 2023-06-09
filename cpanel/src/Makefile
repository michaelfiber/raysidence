.ONESHELL:

BIN_NAME=cpanel

build:
	mkdir -p ../bin
	gcc update.c hue.c rooms.c sprites.c main.c -o ../bin/$(BIN_NAME) -lraylib -lm -ldl -lpthread -lrt

run: build
	../bin/$(BIN_NAME)

build.test:
	mkdir -p ../bin
	gcc test-sender.c -o ../bin/test-$(BIN_NAME) -lrt

run.test: build.test
	../bin/test-$(BIN_NAME)