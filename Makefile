default: build

LDFLAGS=`pkg-config --libs --cflags gtk+-3.0` -lsndfile
CC=gcc

build:
	$(CC) $(CFLAGS) $(LDFLAGS) -o halo-resources src/main.c