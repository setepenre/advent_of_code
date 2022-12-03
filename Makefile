MAKEFLAGS := --jobs=$(shell nproc --ignore 1)

CCFLAGS := -std=gnu17 -Wall -Wextra -Wpedantic -Wconversion
CPPFLAGS := -MMD -MP -Iinclude

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

%/main: %/main.o
	$(CC) $^ -o $@

clean:
	$(RM) $(OBJ) $(DEP) $(BIN)

-include $(DEP)
