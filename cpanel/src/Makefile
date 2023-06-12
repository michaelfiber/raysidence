.ONESHELL:

BIN_NAME=cpanel

build:
	mkdir -p ../bin
	gcc json/cmonson.c update.c hue.c rooms.c sprites.c main.c -o ../bin/$(BIN_NAME) -lraylib -lm -ldl -lpthread -lrt

run: build
	cd ..
	HUE_KEY=$$HUE_KEY HUE_SERVER_IP=$$HUE_SERVER_IP ./bin/$(BIN_NAME)

build.test:
	mkdir -p ../bin
	gcc test-sender.c -o ../bin/test-$(BIN_NAME) -lrt

run.test: build.test
	../bin/test-$(BIN_NAME)