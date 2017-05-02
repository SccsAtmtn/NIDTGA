all: generate 

CC = gcc

INCLUDE = ./include/

CFLAGS = -g -Wall

MAKE = make

generate: 
	(mkdir ./bin; cd src; $(MAKE); mv nid_management ../bin; mv ip_generation ../bin;)

clean:
	-rm -rf ./bin
