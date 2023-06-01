.ONESHELL:

BIN_NAME 	= panel

FILES		= button.c \
			  coolmouse.c \
			  dialog.c \
			  main.c 

LIBS		= -lraylib -lm -ldl -lGL -lpthread

CC 			= gcc

build:
	mkdir -p ../bin
	@$(CC) $(FILES) -o ../bin/$(BIN_NAME) $(LIBS)