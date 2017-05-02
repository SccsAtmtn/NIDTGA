all: nid_management ip_generation

CC = gcc

CFLAGS = -g -Wall

LINK = -L/usr/lib/mysql/ -lmysqlclient

INCLUDE = ../include

nid_management: nid_management.c
	$(CC) nid_management.c -I$(INCLUDE) $(CFLAGS) $(LINK) -o nid_management

ip_generation: ip_generation.c
	$(CC) ip_generation.c -I$(INCLUDE) $(CFLAGS) $(LINK) -o ip_generation
