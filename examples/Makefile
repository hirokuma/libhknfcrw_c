#!/bin/sh

TARGET = tst

SRCDIR = .
OBJDIR = ./obj
INCDIR = ../inc

LIBNAME = ../libhknfcrw_cyg.a

DEMO = nfc
#DEMO = dep
#DEMO = llcp
#DEMO = snep
#DEMO = reset

SRCS = \
	$(DEMO)test.c \
	cyg_misc.c \
	cyg_devaccess_uart.c \
	main.c

OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))
SRCP = $(addprefix $(SRCDIR)/, $(SRCS))
vpath %.c $(SRCDIR)


CFLAGS = -I$(INCDIR) -Wextra -O3
LDOPT  = -L.. $(LIBNAME)

# rules

all: $(TARGET)

$(TARGET): $(OBJS) $(LIBNAME)
	$(CC) $(OBJS) -o $(TARGET) $(LDOPT)

$(OBJDIR)/%.o : %.c
	$(CC) -o $@ -c $(CFLAGS) $<

clean:
	$(RM) $(OBJDIR)/*.o $(TARGET) $(TARGET).exe

