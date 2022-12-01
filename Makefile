MAKEFLAGS := --jobs=$(shell nproc --ignore 1)

CFLAGS := -std=gnu17 -Wall -Wextra -Wpedantic -Wconversion
CPPFLAGS := -MMD -MP

SRC := day_1/main.c
BIN := $(SRC:%.c=%)
OBJ := $(SRC:%.c=%.o)
DEP := $(SRC:%.c=%.d)

.PHONY: all clean
all: $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

day_1/main: day_1/main.o
$(BIN):
	$(CC) $^ -o $@

clean:
	$(RM) $(OBJ) $(DEP) $(BIN)

-include $(DEP)
