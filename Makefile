CC = gcc
CFLAGS = -Werror -Wall -Wextra -Wno-unused -O2 -Iinclude -MMD -MP
LDFLAGS = -lm

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
DEP = $(OBJ:.o=.d)

TARGET = build/ascii

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

build/%.o: src/%.c | build
	$(CC) -c $< $(CFLAGS) -o $@

build:
	mkdir -p build

run: all
	./run.sh

clean:
	rm -rf build

.PHONY: all clean

-include $(DEP)
