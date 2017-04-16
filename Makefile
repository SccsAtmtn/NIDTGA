all: nid_management 

CC = gcc

INCLUDE = ./include/

CFLAGS = -g -Wall -ansi

MAKE = make

nid_management: 
	(cd src; $(MAKE); mv nid_management ..)
