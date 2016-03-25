TOPDIR:=$(CURDIR)
export TOPDIR

$(shell find -name "*[.c][ch]" | xargs chmod -x)
$(shell find -name "*_config" | xargs chmod -x)
$(shell find -name "*.txt" | xargs chmod -x)

include $(TOPDIR)/config.mk
include $(TOPDIR)/lib/library.mk

#SERVER_TARGET:=$(notdir $(shell pwd))
SERVER_TARGET:=smartplat
CLIENT_TARGET:=smartlab
TARGET:=$(addprefix $(DIR),$(SERVER_TARGET) $(CLIENT_TARGET))
export SERVER_TARGET CLIENT_TARGET

INCLUDE +=-I$(TOPDIR)/include $(patsubst %,-I%,$(shell find -L $(TOPDIR)/lib -type d))
ifneq ($(DIR),)
INCLUDE += -I$(TOPDIR)/$(DIR)include
endif

STD_LDFLAGS:=-lpthread
export INCLUDE STD_LDFLAGS

SERVER_DMACRO:=-DTARGET_NAME=\"$(SERVER_TARGET)\" -DCOMM_SERVER
CLIENT_DMACRO:=-DTARGET_NAME=\"$(CLIENT_TARGET)\" -DCOMM_CLIENT
include mconfig/server_config
include mconfig/client_config
export SERVER_DMACRO CLIENT_DMACRO

define server_dependsrcs
SERVER_OBJS +=$(DIR)$(1)/built-s.o
$(DIR)$(1)/built-s.o:$(addprefix $(DIR),$(patsubst %.c,%-s.o,$(2)))
	$(call echocmd,LD, $$@, \
	  $(STARGET_LD) -r -o $$@ $$^)
endef

define client_dependsrcs
CLIENT_OBJS +=$(DIR)$(1)/built-c.o
$(DIR)$(1)/built-c.o:$(addprefix $(DIR),$(patsubst %.c,%-c.o,$(2)))
	$(call echocmd,LD, $$@, \
          $(CTARGET_LD) -r -o $$@ $$^)
endef

SERVER_SOURCES:=smain.cc
CLIENT_SOURCES:=cmain.cc
SUB_DIRS:=$(strip $(patsubst $(TOPDIR)/%/,%, \
		$(dir $(shell find -L $(TOPDIR) -name "transconn.mk"))))
SUB_MODULES:=debug $(SUB_DIRS)
export SUB_MODULES

SERVER_OBJS:=$(filter %-s.o,$(addprefix $(DIR),$(patsubst %.c,%-s.o,$(SERVER_SOURCES))))
SERVER_OBJS+=$(filter %-s.o,$(addprefix $(DIR),$(patsubst %.cc,%-s.o,$(SERVER_SOURCES))))
CLIENT_OBJS:=$(filter %-c.o,$(addprefix $(DIR),$(patsubst %.c,%-c.o,$(CLIENT_SOURCES))))
CLIENT_OBJS+=$(filter %-c.o,$(addprefix $(DIR),$(patsubst %.cc,%-c.o,$(CLIENT_SOURCES))))

$(foreach d, $(SUB_DIRS), \
    $(eval include $(d)/transconn.mk) \
    $(if $(SERVER_SRCS), \
	$(eval $(call server_dependsrcs,$(d),$(patsubst %,$(d)/%,$(SERVER_SRCS)))) \
    )  \
    $(if $(CLIENT_SRCS), \
    	$(eval $(call client_dependsrcs,$(d),$(patsubst %,$(d)/%,$(CLIENT_SRCS)))) \
    )  \
)

ALL_HEARDS:=$(shell find -L $(patsubst %,$(TOPDIR)/%,$(SUB_MODULES)) -name *.h)

.PHONY:all alls tests distclean clean cclean sclean help

all:$(DIR)$(SERVER_TARGET) $(DIR)$(CLIENT_TARGET)

alls:$(TARGET) tests

include $(TOPDIR)/include/include.mk

$(DIR)$(SERVER_TARGET):$(inc_deps) $(inc_dirs_deps) server_comshow $(SERVER_OBJS) libs-s
	$(call echocmd,TAR,$@, \
	  $(STARGET_CC) $(SERVER_DMACRO) $(INCLUDE) $(LDPATH) $(SERVER_LDPATH) -Werror -w -O2 -o $@ $(SERVER_OBJS) $(patsubst %,%-s,$(LDFLAGS)) $(SERVER_LDFLAG)) $(STD_LDFLAGS)
	@$(STARGET_STRIP) $@

$(DIR)$(CLIENT_TARGET):$(inc_deps) $(inc_dirs_deps) client_comshow $(CLIENT_OBJS) libs-c
	$(call echocmd,TAR,$@, \
	  $(CTARGET_CC) $(CLIENT_DMACRO) $(INCLUDE) $(LDPATH) $(CLIENT_LDPATH) -Werror -w -O2 -o $@ $(CLIENT_OBJS) $(patsubst %,%-c,$(LDFLAGS)) $(CLIENT_LDFLAG)) $(STD_LDFLAGS)
	@$(CTARGET_STRIP) $@

$(DIR)%-s.o:%.c $(ALL_HEARDS) mconfig/server_config
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CC, $@, \
	  $(STARGET_CC) $(SERVER_DMACRO) $(INCLUDE) -Werror -w -O2 -o $@ -c $<)

$(DIR)%-s.o:%.cc $(ALL_HEARDS) mconfig/server_config
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CXX,$@, \
	  $(STARGET_CXX) $(SERVER_DMACRO) $(INCLUDE) -Werror -w -O2 -o $@ -c $<)

$(DIR)%-c.o:%.c $(ALL_HEARDS) mconfig/client_config
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CC, $@, \
	  $(CTARGET_CC) $(CLIENT_DMACRO) $(INCLUDE) -O2 -o $@ -c $<)

$(DIR)%-c.o:%.cc $(ALL_HEARDS) mconfig/client_config
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CXX,$@, \
	  $(CTARGET_CXX) $(CLIENT_DMACRO) $(INCLUDE) -O2 -o $@ -c $<)

libs-s:
	@make -C $(TOPDIR)/lib $(patsubst %,$(TOPDIR)/$(DIR)lib/%-s.a,$(patsubst %.a,%,$(LDLIBS)))

libs-c:
	@make -C $(TOPDIR)/lib $(patsubst %,$(TOPDIR)/$(DIR)lib/%-c.a,$(patsubst %.a,%,$(LDLIBS)))

server_comshow:
	@echo ""
	@echo ===========================================================
	@echo **compile server: $(STARGET_CC)
	@echo ===========================================================

client_comshow:
	@echo ""
	@echo ===========================================================
	@echo **compile client: $(CTARGET_CC)
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
