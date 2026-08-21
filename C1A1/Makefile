include sources.mk

TARGET = main

CC = gcc
CPPFLAGS =
CFLAGS = -Wall -Werror -g -O0 -std=c99

PPFILES = $(SOURCES:.c=.i)
DEPS = $(SOURCES:.c=.d)
ASMFILES = $(SOURCES:.c=.asm)
OBJECTS = $(SOURCES:.c=.o)

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
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@
	size $@

.PHONY: $(TARGET).asm
$(TARGET).asm: $(TARGET).out
	objdump -S $< > $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(PPFILES) $(DEPS) $(ASMFILES) $(TARGET).out $(TARGET).asm $(TARGET).d $(TARGET).o
