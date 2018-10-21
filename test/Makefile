TARG = mp3fun-test-driver
CC = gcc
HFILES = $(wildcard ../include/*.h)
CSRC = $(wildcard src/*.c) ../src/mp3fun_util.c munit/munit.c
OBJ = $(CSRC:.c=.o)
INC = -Iinclude -I../include -Imunit
CFLAGS = $(INC)
LDFLAGS = 

all: $(TARG)

debug: CFLAGS += -DDEBUG -g
debug: $(TARG)

$(TARG): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm $(TARG) $(OBJ)
