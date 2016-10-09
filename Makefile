TOPDIR :=$(CURDIR)
export TOPDIR

$(shell find -name "*[.c][ch]" | xargs chmod -x)
$(shell find -name "*_config" | xargs chmod -x)
$(shell find -name "*.txt" | xargs chmod -x)

include $(TOPDIR)/config.mk
include $(TOPDIR)/lib/library.mk

TARGET ?=cullive
TARGET :=$(addprefix $(DIR),$(TARGET))
export TARGET

INCLUDE +=-I$(TOPDIR)/include $(patsubst %,-I%,$(shell find -L $(TOPDIR)/lib -type d))
ifneq ($(DIR),)
INCLUDE += -I$(TOPDIR)/$(DIR)include
endif

STD_LDFLAGS :=-lpthread
export INCLUDE STD_LDFLAGS

DMACRO :=-DTARGET_NAME=\"$(TARGET)\" -DCOMM_TARGET
include mconfig/cullive_config
export DMACRO

define target_dependsrcs
TARGET_OBJS +=$(DIR)$(1)/built.o
$(DIR)$(1)/built.o:$(addprefix $(DIR),$(patsubst %.c,%.o,$(2)))
	$(call echocmd,LD, $$@, \
          $(TARGET_LD) -r -o $$@ $$^)
endef

TARGET_SOURCES :=main.cc
SUB_DIRS :=$(strip $(patsubst $(TOPDIR)/%/,%, \
		$(dir $(shell find -L $(TOPDIR) -name "transconn.mk"))))
SUB_MODULES :=debug $(SUB_DIRS)
export SUB_MODULES

TARGET_OBJS :=$(filter %.o,$(addprefix $(DIR),$(patsubst %.c,%.o,$(TARGET_SOURCES))))
TARGET_OBJS +=$(filter %.o,$(addprefix $(DIR),$(patsubst %.cc,%.o,$(TARGET_SOURCES))))

$(foreach d, $(SUB_DIRS), \
    $(eval include $(d)/transconn.mk) \
    $(if $(SRCS), \
    	$(eval $(call target_dependsrcs,$(d),$(patsubst %,$(d)/%,$(SRCS)))) \
    )  \
)

ALL_HEARDS :=$(shell find -L $(patsubst %,$(TOPDIR)/%,$(SUB_MODULES)) -name *.h)

.PHONY:all alls tests distclean clean help

all:$(DIR)$(TARGET)

alls:$(DIR)$(TARGET) tests

include $(TOPDIR)/include/include.mk

$(DIR)$(TARGET):$(inc_deps) $(inc_dirs_deps) target_comshow $(TARGET_OBJS) libs
	$(call echocmd,TAR,$@, \
	  $(TARGET_CC) $(DMACRO) $(INCLUDE) $(LDPATH) $(TARGET_LDPATH) -Werror -w -O2 -o $@ $(TARGET_OBJS) $(patsubst %,%,$(LDFLAGS)) $(TARGET_LDFLAG)) $(STD_LDFLAGS)
	@$(TARGET_STRIP) $@

$(DIR)%.o:%.c $(ALL_HEARDS) mconfig/cullive_config
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CC, $@, \
	  $(TARGET_CC) $(DMACRO) $(INCLUDE) -O2 -o $@ -c $<)

$(DIR)%.o:%.cc $(ALL_HEARDS) mconfig/cullive_config
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CXX,$@, \
	  $(TARGET_CXX) $(DMACRO) $(INCLUDE) -O2 -o $@ -c $<)

libs:
	@make -C $(TOPDIR)/lib $(patsubst %,$(TOPDIR)/$(DIR)lib/%.a,$(patsubst %.a,%,$(LDLIBS)))

target_comshow:
	@echo ===========================================================
	@echo **compiler: $(TARGET_CC)
	@echo ===========================================================

tests:
	@make -C tests

clean:
	(find -name "*.[oa]" | xargs $(RM)) && $(RM) $(DIR)$(TARGET)
	$(RM) -r $(patsubst %/,$(DIR)include/%,debug/ $(dir $(shell ls */transconn.mk))) $(inc_dirs_deps) $(DIR)

distclean:clean
	@make -C tests clean

help:
	@echo "help:"
	@echo "  prefix=[compiler]	\"Cross compiler prefix.\""
	@echo "  V=[1|99]		\"Default value is 1, and 99 show more details.\""
	@echo "  dir=<path>		\"binaries path.\""
