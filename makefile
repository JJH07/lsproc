CC = gcc
CFLAGS = -Wall
ssu_lsproc: ssu_lsproc.o func.o main.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ssu_lsproc.o func.o main.o: myheader.h
