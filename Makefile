MAKEFLAGS := --jobs=$(shell nproc --ignore 1)

CCFLAGS := -std=gnu17 -Wall -Wextra -Wpedantic -Wconversion
CPPFLAGS := -MMD -MP -Iinclude
LDFLAGS :=

SRC := $(wildcard day_*/main.c)
BIN := $(SRC:%.c=%)
OBJ := $(SRC:%.c=%.o)
DEP := $(SRC:%.c=%.d)

.PHONY: all clean
all: $(BIN)

debug: CCFLAGS += -g
debug: $(BIN)

%.o: %.c
	$(CC) $(CCFLAGS) $(CPPFLAGS) -c $< -o $@

day_14/main: LDFLAGS += -lncurses
%/main: %/main.o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	$(RM) $(OBJ) $(DEP) $(BIN)

-include $(DEP)
