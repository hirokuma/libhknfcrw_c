#!/bin/sh

TARGET = getuid

SRCDIR = .
OBJDIR = ./obj
INCDIR = ../../inc

LIBNAME = ../../libhknfcrw.a


SRCS = \
	misc.c \
	devaccess_pasori.c \
	main.c

OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))
SRCP = $(addprefix $(SRCDIR)/, $(SRCS))
vpath %.c $(SRCDIR)


CFLAGS = -I$(INCDIR) -std=c99 -Wextra -O3
LDOPT  = -L.. $(LIBNAME) -lusb-1.0

# rules

all: $(TARGET)

$(TARGET): $(OBJS) $(LIBNAME)
	$(CC) $(OBJS) -o $(TARGET) $(LDOPT)

$(OBJDIR)/%.o : %.c
	$(CC) -o $@ -c $(CFLAGS) $<

clean:
	$(RM) $(OBJDIR)/*.o $(TARGET)

