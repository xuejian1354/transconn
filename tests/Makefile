ifeq ($(TOPDIR),)
TOPDIR ?= $(dir $(shell pwd))
endif

ifneq ($(WHOLE_BUILD),1)
ifneq ("$(origin prefix)", "command line")
  prefix :=
endif

TARGET_CC := $(prefix)gcc
TARGET_LD := $(prefix)ld
TARGET_AR := $(prefix)ar
TARGET_STRIP := $(prefix)strip

$(shell find -name "*.[ch]" | xargs chmod -x)

all:preshow

preshow:
	@echo cross compiler is $(TARGET_CC)

WHOLE_BUILD := 1
export WHOLE_BUILD
endif

TARGET:=client

INCLUDE+=-I$(TOPDIR)include -I$(TOPDIR)lib
LDFLAGS:=-lpthread

DMACRO:=-DTARGET_NAME=\"$(TARGET)\" -DCLIENT_TEST -DTHREAD_POOL_SUPPORT -DTIMER_SUPPORT -DTRANS_UDP_SERVICE -DSELECT_SUPPORT

SOURCES:=tests/client.c lib/mincode.c lib/tpool.c module/etimer.c module/netapi.c module/netlist.c protocol/framelysis.c protocol/protocol.c protocol/trframelysis.c protocol/trrequest.c services/corecomm.c services/globals.c services/mevent.c

OBJS:=$(patsubst %.c,%.o, $(notdir $(SOURCES)))
$(foreach s,$(SOURCES), \
    $(eval $(patsubst %.c,%.o,$(notdir $(s)))-s:=$(s)) \
)

.PHONY: all clean
all:$(TARGET)

$(TARGET):$(OBJS)
	$(TARGET_CC) -o $@ $(OBJS) $(LDFLAGS)
	$(TARGET_STRIP) $@

$(OBJS):
	$(TARGET_CC) -o $@ $(DMACRO) $(INCLUDE) -c $(TOPDIR)$($@-s)

clean:
	$(RM) *.o $(TARGET)
