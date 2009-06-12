###############################################################################
##
## Copyright (C) 2006, Waseda University
##
## File path:	Tools/Console/Makefile
##

SRCS=	vm-cons-client.c
TARGET=	$(SRCS:.c=)

CFLAGS=	-Wall -O2

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -DSOCK_NAME=\"/tmp/vmware-$(shell whoami)/ttyS0\" $^ -o $@

.PHONY: clean
clean:
	-rm -f $(TARGET) ~*
