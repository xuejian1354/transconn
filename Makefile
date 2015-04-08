TOPDIR:=$(CURDIR)
export TOPDIR

include $(TOPDIR)/config.mk
include $(TOPDIR)/lib/library.mk

SERVER_TARGET:=$(notdir $(shell pwd))
CLIENT_TARGET:=connect_listen
TARGET:=$(SERVER_TARGET) $(CLIENT_TARGET)
export SERVER_TARGET CLIENT_TARGET
#TARGET  := $(notdir $(shell pwd))

INCLUDE+=-I$(TOPDIR)/include -I$(TOPDIR)/lib

STD_LDFLAGS:=-lpthread
export INCLUDE STD_LDFLAGS

SERVER_DMACRO:=-DTARGET_NAME=\"$(SERVER_TARGET)\" -DCOMM_SERVER
CLIENT_DMACRO:=-DTARGET_NAME=\"$(CLIENT_TARGET)\" -DCOMM_CLIENT
include mconfig/server_config
include mconfig/client_config
export SERVER_DMACRO CLIENT_DMACRO

SUB_MODULES:=debug module protocol control services
export SUB_MODULES

define dependsrcs
$(1)/built-s.o:$(patsubst %.c,%-s.o,$(2))
	$(call echocmd,LD, $$@, \
	  $(STARGET_LD) -r -o $$@ $$^)
$(1)/built-c.o:$(patsubst %.c,%-c.o,$(3))
	$(call echocmd,LD, $$@, \
          $(CTARGET_LD) -r -o $$@ $$^)
endef

SERVER_SOURCES:=smain.c
CLIENT_SOURCES:=cmain.c
SUB_DIRS:=$(filter-out debug, $(SUB_MODULES))
$(foreach d, $(SUB_DIRS), \
    $(eval include $(d)/transconn.mk) \
    $(eval SERVER_SRCS:=$(patsubst %,$(d)/%,$(SERVER_SRCS))) \
    $(eval CLIENT_SRCS:=$(patsubst %,$(d)/%,$(CLIENT_SRCS))) \
    $(eval $(call dependsrcs,$(d),$(SERVER_SRCS),$(CLIENT_SRCS))) \
)

SERVER_OBJS:=$(patsubst %.c,%-s.o,$(SERVER_SOURCES)) $(patsubst %,%/built-s.o,$(SUB_DIRS))
CLIENT_OBJS:=$(patsubst %.c,%-c.o,$(CLIENT_SOURCES)) $(patsubst %,%/built-c.o,$(SUB_DIRS))

.PHONY: all clean cclean sclean help

all:$(TARGET)

include $(TOPDIR)/include/include.mk

$(SERVER_TARGET):$(inc_deps) server_comshow $(SERVER_OBJS)
	$(call echocmd,TAR,$(SERVER_TARGET), \
	  $(STARGET_CC) $(SERVER_DMACRO) $(INCLUDE) $(LDPATH) -o $@ $(SERVER_OBJS) $(STD_LDFLAGS) $(patsubst %,%-s,$(LDFLAGS)))
	@$(STARGET_STRIP) $@

$(CLIENT_TARGET):$(inc_deps) client_comshow $(CLIENT_OBJS)
	$(call echocmd,TAR,$(CLIENT_TARGET), \
	  $(CTARGET_CC) $(CLIENT_DMACRO) $(INCLUDE) -o $@ $(CLIENT_OBJS) $(LDPATH) $(STD_LDFLAGS) $(patsubst %,%-c,$(LDFLAGS)))
	@$(CTARGET_STRIP) $@

%-s.o:%.c mconfig/server_config
	$(call echocmd,CC, $@, \
	  $(STARGET_CC) $(SERVER_DMACRO) $(INCLUDE) -o $@ -c $<)

%-c.o:%.c mconfig/client_config
	$(call echocmd,CC, $@, \
	  $(CTARGET_CC) $(CLIENT_DMACRO) $(INCLUDE) -o $@ -c $<)

$(eval $(call dependlibs-relations,$(SERVER_TARGET),$(LDFLAGS),-s))
$(eval $(call dependlibs-relations,$(CLIENT_TARGET),$(LDFLAGS),-c))

server_comshow:
	@echo ===========================================================
	@echo compile server:

client_comshow:
	@echo ===========================================================
	@echo compile client:

cclean:
	(find -name "*-c.[oa]" | xargs $(RM)) && $(RM) $(CLIENT_TARGET)

sclean:
	(find -name "*-s.[oa]" | xargs $(RM)) && $(RM) $(SERVER_TARGET)

clean:cclean sclean
	$(RM) -r $(dir $(inc_deps))

help:
	@echo "help:"
	@echo "  sprefix=[compiler]	\"Server Cross compiler prefix.\""
	@echo "  cprefix=[compiler]	\"Client Cross compiler prefix.\""
	@echo "  V=[1|99]		\"Default value is 1, and 99 show more details.\""
