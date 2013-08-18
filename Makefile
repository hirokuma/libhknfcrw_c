#!/bin/sh

CC       = arm-none-eabi-gcc
AR       = arm-none-eabi-ar


USE_SNEP_TARGET = yes
USE_SNEP_INITIATOR = yes
USE_SNEP_SERVER = yes
USE_LASTERR = yes

TARGET = libhknfcrw.a

SRCDIR = ./src
OBJDIR = ./obj
INCDIR = ./inc

SRCS = \
	HkNfcRw.c \
	HkNfcA.c \
	HkNfcB.c \
	HkNfcF.c \
	HkNfcNdef.c \
	NfcPcd.c

#####################################################
#SNEP Initiator
ifeq ($(USE_SNEP_INITIATOR),yes)

ifneq ($(USE_SNEP_TARGET),yes)
SRCS += \
	HkNfcLlcp.c \
	HkNfcSnep.c
CFLAGS += -DHKNFCRW_USE_SNEP
endif

#SNEP Target
ifeq ($(USE_SNEP_TARGET),yes)
SRCS += \
	HkNfcLlcp.c \
	HkNfcSnep.c
CFLAGS += -DHKNFCRW_USE_SNEP -DHKNFCRW_USE_SNEP_TARGET
endif

CFLAGS += -DHKNFCRW_USE_SNEP_INITIATOR
endif

#SNEP Server
ifeq ($(USE_SNEP_SERVER),yes)
CFLAGS += -DHKNFCRW_USE_SNEP_SERVER
endif

#debug
ifeq ($(USE_LASTERR),yes)
CFLAGS += -DHKNFCRW_USE_LASTERR
endif
#####################################################



OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))
SRCP = $(addprefix $(SRCDIR)/, $(SRCS))
vpath %.c $(SRCDIR)

CINC = -I$(INCDIR)
#CFLAGS = $(CINC) -Wextra -O3 -mcpu='cortex-m3' -mthumb -mthumb-interwork
#CFLAGS = $(CINC) -Wextra -g -O0 -ffunction-sections -mcpu='cortex-m3' -mthumb -mthumb-interwork
CFLAGS += $(CINC) -Wextra -g -O0 -mcpu='cortex-m3' -mthumb -mthumb-interwork
LDOPT  = 

# rules

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) r $(TARGET) $(OBJS)

$(OBJDIR)/%.o : %.c
	$(CC) -o $@ -c $(CFLAGS) $<

clean:
	$(RM) $(OBJDIR)/*.o $(TARGET) .Depend

.Depend:
	-$(CC) $(CINC) -MM $(SRCP) > .tmp
	@if [ ! -d $(OBJDIR) ]; then \
		echo ";; mkdir $(OBJDIR)"; mkdir $(OBJDIR); \
	fi
	sed -e '/^[^ ]/s,^,$(OBJDIR)/,' .tmp> .Depend
	rm .tmp

include .Depend
