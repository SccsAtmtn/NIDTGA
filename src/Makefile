all: nid_management ip_generation

CC = gcc

CFLAGS = -g -Wall -ansi

INCLUDE = ../include

nid_management: nid_management.c
	$(CC) nid_management.c -I$(INCLUDE) $(CFLAGS) -o nid_management

ip_generation: ip_generation.c
	$(CC) ip_generation.c -I$(INCLUDE) $(CFLAGS) -o ip_generation
