# Makefile for alpha_ex1 - i.MX6

SRCS = alpha_ex1.c
INC += -I $(ROOTFS)/usr/include \
	-I $(ROOTFS)/usr/src/linux/include \

LFLAGS += -L $(ROOTFS)/usr/lib -lipu -lm -lrt

OBJS = alpha_ex1
$(OBJS): $(SRCS)
	$(VERBOSE) $(CC) -g $(INC) $(CFLAGS) $(LFLAGS) $^ $(LIBS) -o $@

all: $(OBJS)

.PHONY: clean
clean:
	rm -f $(OBJS)

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
RM		= rm -f
MAKEDIR		= mkdir -p



