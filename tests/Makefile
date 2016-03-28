ifeq ($(TESTTOP),)
TESTTOP ?= $(dir $(shell pwd))
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

INCLUDE+=-I$(TESTTOP)include -I$(TESTTOP)lib
LDFLAGS:=-lpthread

DMACRO:=-DTARGET_NAME=\"$(TARGET)\" -DCLIENT_TEST -DTHREAD_POOL_SUPPORT -DTIMER_SUPPORT -DTRANS_UDP_SERVICE -DSELECT_SUPPORT

.PHONY: all clean
all:perznet deulog clar

perznet:perznet.c $(TESTTOP)module/serial.c
	$(TARGET_CC) -o $@ $^ -DSERIAL_SUPPORT $(INCLUDE) -w $(LDFLAGS)
	@$(TARGET_STRIP) $@

deulog:deulog.c
	$(TARGET_CC) -o $@ $^ $(INCLUDE) -DDE_TRANS_UDP_STREAM_LOG $(LDFLAGS)
	@$(TARGET_STRIP) $@

clar:clar.c
	$(TARGET_CC) -o $@ $^ $(INCLUDE) -DDE_TRANS_UDP_CONTROL $(LDFLAGS)
	@$(TARGET_STRIP) $@

clean:
	$(RM) *.o $(TARGET) perznet deulog clar fork-echoserv echoserv testclient
