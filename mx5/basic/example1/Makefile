# Makefile for ipu_test1

SRCS = ipu_test1.c
CFLAGS += -I /home/rogerio/ltib/sept_2011/ltib/rootfs/usr/include \
	  -I /home/rogerio/ltib/sept_2011/ltib/rpm/BUILD/linux/include

LFLAGS += -L /home/rogerio/ltib/sept_2011/ltib/rootfs/usr/lib -lipu -lIpuScreenLayer -lm -lrt


OBJS = ipu_test1
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



