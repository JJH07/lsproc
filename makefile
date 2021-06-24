CC = gcc
CFLAGS = -Wall
ssu_lsproc: ssu_lsproc.o func.o main.o
	gcc -o ssu_lsproc ssu_lsproc.c func.c main.c
