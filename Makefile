TOPDIR:=$(CURDIR)
export TOPDIR

$(shell find -name "*.[ch]" | xargs chmod -x)
$(shell find -name "*_config" | xargs chmod -x)

include $(TOPDIR)/config.mk
include $(TOPDIR)/lib/library.mk

#SERVER_TARGET:=$(notdir $(shell pwd))
SERVER_TARGET:=transconn
CLIENT_TARGET:=connect_listen
TARGET:=$(addprefix $(DIR),$(SERVER_TARGET) $(CLIENT_TARGET))
export SERVER_TARGET CLIENT_TARGET

INCLUDE +=-I$(TOPDIR)/include -I$(TOPDIR)/lib
ifneq ($(DIR),)
INCLUDE +=-I$(TOPDIR)/$(DIR)include
endif

STD_LDFLAGS:=-lpthread
export INCLUDE STD_LDFLAGS

SERVER_DMACRO:=-DTARGET_NAME=\"$(SERVER_TARGET)\" -DCOMM_SERVER
CLIENT_DMACRO:=-DTARGET_NAME=\"$(CLIENT_TARGET)\" -DCOMM_CLIENT
include mconfig/server_config
include mconfig/client_config
export SERVER_DMACRO CLIENT_DMACRO

SUB_MODULES:=debug module protocol services
export SUB_MODULES

define dependsrcs
$(DIR)$(1)/built-s.o:$(addprefix $(DIR),$(patsubst %.c,%-s.o,$(2)))
	$(call echocmd,LD, $$@, \
	  $(STARGET_LD) -r -o $$@ $$^)
$(DIR)$(1)/built-c.o:$(addprefix $(DIR),$(patsubst %.c,%-c.o,$(3)))
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

ALL_HEARDS:=$(shell find -L $(patsubst %,$(TOPDIR)/%,$(SUB_MODULES)) -name *.h)

SERVER_OBJS:=$(addprefix $(DIR),$(patsubst %.c,%-s.o,$(SERVER_SOURCES)) $(patsubst %,%/built-s.o,$(SUB_DIRS)))
CLIENT_OBJS:=$(addprefix $(DIR),$(patsubst %.c,%-c.o,$(CLIENT_SOURCES)) $(patsubst %,%/built-c.o,$(SUB_DIRS)))

.PHONY:all alls tests distclean clean cclean sclean help

all:$(TARGET)

alls:$(TARGET) tests

include $(TOPDIR)/include/include.mk

$(DIR)$(SERVER_TARGET):$(inc_deps) $(inc_dirs_deps) server_comshow $(SERVER_OBJS) libs-s
	$(call echocmd,TAR,$@, \
	  $(STARGET_CC) $(SERVER_DMACRO) $(INCLUDE) $(LDPATH) $(SERVER_LDPATH) -w -O2 -o $@ $(SERVER_OBJS) $(patsubst %,%-s,$(LDFLAGS)) $(SERVER_LDFLAG)) $(STD_LDFLAGS)
	@$(STARGET_STRIP) $@

$(DIR)$(CLIENT_TARGET):$(inc_deps) $(inc_dirs_deps) client_comshow $(CLIENT_OBJS) libs-c
	$(call echocmd,TAR,$@, \
	  $(CTARGET_CC) $(CLIENT_DMACRO) $(INCLUDE) $(LDPATH) $(CLIENT_LDPATH) -O2 -o $@ $(CLIENT_OBJS) $(patsubst %,%-c,$(LDFLAGS)) $(CLIENT_LDFLAG)) $(STD_LDFLAGS)
	@$(CTARGET_STRIP) $@

$(DIR)%-s.o:%.c $(ALL_HEARDS) mconfig/server_config
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CC, $@, \
	  $(STARGET_CC) $(SERVER_DMACRO) $(INCLUDE) -w -O2 -o $@ -c $<)

$(DIR)%-c.o:%.c $(ALL_HEARDS) mconfig/client_config
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CC, $@, \
	  $(CTARGET_CC) $(CLIENT_DMACRO) $(INCLUDE) -O2 -o $@ -c $<)

libs-s:
	@make -C $(TOPDIR)/lib $(patsubst %,$(TOPDIR)/$(DIR)lib/%-s.a,$(patsubst %.a,%,$(LDLIBS)))

libs-c:
	@make -C $(TOPDIR)/lib $(patsubst %,$(TOPDIR)/$(DIR)lib/%-c.a,$(patsubst %.a,%,$(LDLIBS)))

server_comshow:
	@echo ""
	@echo ===========================================================
	@echo **compile server:
	@echo ===========================================================

client_comshow:
	@echo ""
	@echo ===========================================================
	@echo **compile client:
	@echo ===========================================================

tests:
	@make -C tests

cclean:
	(find -name "*-c.[oa]" | xargs $(RM)) && $(RM) $(DIR)$(CLIENT_TARGET)

sclean:
	(find -name "*-s.[oa]" | xargs $(RM)) && $(RM) $(DIR)$(SERVER_TARGET)

clean:cclean sclean
	$(RM) -r $(patsubst %/,$(DIR)include/%,debug/ $(dir $(shell ls */transconn.mk))) $(inc_dirs_deps) $(DIR)

distclean:clean
	@make -C tests clean

help:
	@echo "help:"
	@echo "  sprefix=[compiler]	\"Server Cross compiler prefix.\""
	@echo "  cprefix=[compiler]	\"Client Cross compiler prefix.\""
	@echo "  V=[1|99]		\"Default value is 1, and 99 show more details.\""
	@echo "  dir=<path>		\"binaries path.\""
