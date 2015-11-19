TOPDIR:=$(CURDIR)
export TOPDIR

include $(TOPDIR)/config.mk
include $(TOPDIR)/lib/library.mk

#SERVER_TARGET:=$(notdir $(shell pwd))
SERVER_TARGET:=transconn
CLIENT_TARGET:=connect_listen
TARGET:=$(SERVER_TARGET) $(CLIENT_TARGET)
export SERVER_TARGET CLIENT_TARGET

INCLUDE+=-I$(TOPDIR)/include -I$(TOPDIR)/lib

STD_LDFLAGS:=-lpthread
export INCLUDE STD_LDFLAGS

SERVER_DMACRO:=-DTARGET_NAME=\"$(SERVER_TARGET)\" -DCOMM_SERVER
CLIENT_DMACRO:=-DTARGET_NAME=\"$(CLIENT_TARGET)\" -DCOMM_CLIENT
include mconfig/server_config
include mconfig/client_config
export SERVER_DMACRO CLIENT_DMACRO

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
SUB_DIRS:=$(strip $(patsubst $(TOPDIR)/%/,%, \
		$(dir $(shell find -L $(TOPDIR) -name "transconn.mk"))))
SUB_MODULES:=debug $(SUB_DIRS)
export SUB_MODULES

$(foreach d, $(SUB_DIRS), \
    $(eval include $(d)/transconn.mk) \
    $(eval SERVER_SRCS:=$(patsubst %,$(d)/%,$(SERVER_SRCS))) \
    $(eval CLIENT_SRCS:=$(patsubst %,$(d)/%,$(CLIENT_SRCS))) \
    $(eval $(call dependsrcs,$(d),$(SERVER_SRCS),$(CLIENT_SRCS))) \
)

ALL_HEARDS:=$(shell find -L $(patsubst %,$(TOPDIR)/%,$(SUB_MODULES)) -name *.h)

SERVER_OBJS:=$(patsubst %.c,%-s.o,$(SERVER_SOURCES)) $(patsubst %,%/built-s.o,$(SUB_DIRS))
CLIENT_OBJS:=$(patsubst %.c,%-c.o,$(CLIENT_SOURCES)) $(patsubst %,%/built-c.o,$(SUB_DIRS))

.PHONY:all alls tests distclean clean cclean sclean help

all:$(SERVER_TARGET)

alls:$(TARGET) tests

include $(TOPDIR)/include/include.mk

$(SERVER_TARGET):$(inc_deps) $(inc_dirs_deps) server_comshow $(SERVER_OBJS) libs-s
	$(call echocmd,TAR,$(SERVER_TARGET), \
	  $(STARGET_CC) $(SERVER_DMACRO) $(INCLUDE) $(LDPATH) $(SERVER_LDPATH) -O2 -o $@ $(SERVER_OBJS) $(patsubst %,%-s,$(LDFLAGS)) $(SERVER_LDFLAG)) $(STD_LDFLAGS)
	@$(STARGET_STRIP) $@

$(CLIENT_TARGET):$(inc_deps) $(inc_dirs_deps) client_comshow $(CLIENT_OBJS) libs-c
	$(call echocmd,TAR,$(CLIENT_TARGET), \
	  $(CTARGET_CC) $(CLIENT_DMACRO) $(INCLUDE) $(LDPATH) $(CLIENT_LDPATH) -O2 -o $@ $(CLIENT_OBJS) $(patsubst %,%-c,$(LDFLAGS)) $(CLIENT_LDFLAG)) $(STD_LDFLAGS)
	@$(CTARGET_STRIP) $@

%-s.o:%.c $(ALL_HEARDS) mconfig/server_config
	$(call echocmd,CC, $@, \
	  $(STARGET_CC) $(SERVER_DMACRO) $(INCLUDE) -O2 -o $@ -c $<)

%-c.o:%.c $(ALL_HEARDS) mconfig/client_config
	$(call echocmd,CC, $@, \
	  $(CTARGET_CC) $(CLIENT_DMACRO) $(INCLUDE) -O2 -o $@ -c $<)

libs-s:
	@make -C $(TOPDIR)/lib $(patsubst %,%-s.a,$(patsubst %.a,%,$(LDLIBS)))

libs-c:
	@make -C $(TOPDIR)/lib $(patsubst %,%-c.a,$(patsubst %.a,%,$(LDLIBS)))

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
	(find -name "*-c.[oa]" | xargs $(RM)) && $(RM) $(CLIENT_TARGET)

sclean:
	(find -name "*-s.[oa]" | xargs $(RM)) && $(RM) $(SERVER_TARGET)

clean:cclean sclean
	$(RM) -r $(patsubst %/,include/%,debug/ $(dir $(shell ls */transconn.mk))) $(inc_dirs_deps)

distclean:clean
	@make -C tests clean

help:
	@echo "help:"
	@echo "  sprefix=[compiler]	\"Server Cross compiler prefix.\""
	@echo "  cprefix=[compiler]	\"Client Cross compiler prefix.\""
	@echo "  V=[1|99]		\"Default value is 1, and 99 show more details.\""
