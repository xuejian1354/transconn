ifeq ($(TOPDIR),)
TOPDIR ?= $(patsubst %/,%,$(dir $(shell pwd)))
endif

BIN_DIR :=$(addsuffix lib/,$(TOPDIR)/$(DIR))

include $(TOPDIR)/config.mk
include $(TOPDIR)/lib/library.mk

SERVER_LIBS:=$(addprefix $(BIN_DIR),$(patsubst %,%-s.a,$(patsubst %.a,%,$(LDLIBS))))
CLIENT_LIBS:=$(addprefix $(BIN_DIR),$(patsubst %,%-c.a,$(patsubst %.a,%,$(LDLIBS))))

$(foreach libsrc,$(LDLIBS), $(eval LIBSRC_FILES += $($(patsubst lib%.a,%_libsrc,$(libsrc)))))
SLIBSRC_TARGETS := $(addprefix $(BIN_DIR),$(patsubst %.c,%-s.o,$(LIBSRC_FILES)))
CLIBSRC_TARGETS := $(addprefix $(BIN_DIR),$(patsubst %.c,%-c.o,$(LIBSRC_FILES)))

all:$(SERVER_LIBS) $(CLIENT_LIBS)
.PHONY: all clean

#$(foreach libsrc,$(TARGET), $(eval $(libsrc):$($(patsubst lib%.a,%_libsrc,$(libsrc)))))
LIB_HEARDS:=$(shell ls $(patsubst %,$(TOPDIR)/%,lib/*.h))

$(SERVER_LIBS):$(SLIBSRC_TARGETS)
	$(call echocmd,AR, $(patsubst $(TOPDIR)/%,%,$@), \
	  $(STARGET_AR) crs $@ $(patsubst %.c,$(BIN_DIR)%-s.o,$($(patsubst lib%-s.a,%_libsrc,$(notdir $@)))))

$(SLIBSRC_TARGETS):$(LIBSRC_FILES) $(LIB_HEARDS)
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CC, $(patsubst $(TOPDIR)/%,%,$@), \
	  $(STARGET_CC) $(SERVER_DMACRO) $(INCLUDE) -O2 -o $@ -c $(patsubst $(BIN_DIR)%-s.o,%.c,$@))

$(CLIENT_LIBS):$(CLIBSRC_TARGETS)
	$(call echocmd,AR, $(patsubst $(TOPDIR)/%,%,$@), \
	  $(CTARGET_AR) crs $@ $(patsubst %.c,$(BIN_DIR)%-c.o,$($(patsubst lib%-c.a,%_libsrc,$(notdir $@)))))

$(CLIBSRC_TARGETS):$(LIBSRC_FILES) $(LIB_HEARDS)
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi;
	$(call echocmd,CC, $(patsubst $(TOPDIR)/%,%,$@), \
	  $(CTARGET_CC) $(CLIENT_DMACRO) $(INCLUDE) -O2 -o $@ -c $(patsubst $(BIN_DIR)%-c.o,%.c,$@))

clean:
	if [ -d $(BIN_DIR) ]; then find -L $(BIN_DIR) -name "*.[oa]" | xargs $(RM); fi; 
	-if [ -d $(BIN_DIR) ]; then rmdir $(BIN_DIR); fi;
	-if [ -d $(TOPDIR)/$(DIR) ]; then rmdir $(TOPDIR)/$(DIR); fi;
