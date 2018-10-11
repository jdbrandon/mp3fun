TARG = mp3fun
CC = gcc
HFILES = $(wildcard include/*.h)
CSRC = $(wildcard src/*.c)
OBJ = $(CSRC:.c=.o)
INC = include
CFLAGS = -I$(INC)
LDFLAGS = 

all: $(TARG)

debug: CFLAGS += -DDEBUG -g
debug: $(TARG)

$(TARG): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm $(TARG) $(OBJ)
