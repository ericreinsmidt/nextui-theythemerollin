# TheyTheMeRollin — Cross-compile Makefile for tg5040
# Run inside Docker toolchain container

CROSS    = aarch64-nextui-linux-gnu-
CC       = $(CROSS)gcc
SYSROOT  = /opt/aarch64-nextui-linux-gnu/aarch64-nextui-linux-gnu/libc

INCLUDES = -I../../third_party/apostrophe/include \
           -I../../third_party/pakkit/include \
           -I../../third_party/cjson

CFLAGS   = -O2 -Wall -Wextra -DPLATFORM_TG5040 $(INCLUDES) --sysroot=$(SYSROOT)
LDFLAGS  = --sysroot=$(SYSROOT) -lSDL2 -lSDL2_ttf -lSDL2_image -lpthread -lm

OBJDIR   = obj
OUT      = pak/bin/theythemerollin

OBJS     = $(OBJDIR)/main.o $(OBJDIR)/cJSON.o

HEADERS  = ../../third_party/apostrophe/include/apostrophe.h \
           ../../third_party/pakkit/include/pakkit_ui.h \
           ../../third_party/cjson/cJSON.h

.PHONY: all clean

all: $(OUT)

$(OUT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/main.o: ../../src/main.c $(HEADERS)
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/cJSON.o: ../../third_party/cjson/cJSON.c ../../third_party/cjson/cJSON.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(OUT)
