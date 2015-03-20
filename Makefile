default: build

LDFLAGS=`pkg-config --libs gtk+-3.0`
override CFLAGS += `pkg-config --cflags gtk+-3.0`
CC=gcc

build:
	$(CC) $(CFLAGS) $(LDFLAGS) -o halo-resources src/main.c