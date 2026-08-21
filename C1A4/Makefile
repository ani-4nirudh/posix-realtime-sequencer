# Makefile

include sources.mk

TARGET = main

CC = gcc
CPPFLAGS = -D_GNU_SOURCE
CFLAGS = -Wall -Werror -Wextra -Wpedantic -g -O0 -std=c99


OBJECTS = $(SOURCES:.c=.o)
DEPS = $(SOURCES:.c=.d)
ASMFILES = $(SOURCES:.c=.asm)
PPFILES = $(SOURCES:.c=.i)

%.i: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -E $< -o $@

%.asm: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -S $< -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -MMD -MP -MF $*.d -c $< -o $@

.PHONY: compile-all
compile-all: $(OBJECTS)

.PHONY: build
build: $(TARGET).out

$(TARGET).out: $(OBJECTS)
		$(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) $^ -o $@
	size $@

.PHONY: $(TARGET).asm
$(TARGET).asm: $(TARGET).out
	objdump -S $< > $@
	size $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(DEPS) $(ASMFILES) $(PPFILES) $(TARGET).out $(TARGET).asm $(TARGET).d $(TARGET).o
