ifeq ($(TOPDIR),)
TOPDIR ?= $(dir $(shell pwd))
endif

include $(TOPDIR)/config.mk
include $(TOPDIR)/lib/library.mk

SERVER_LIBS:=$(patsubst %,%-s.a,$(patsubst %.a,%,$(LDLIBS)))
CLIENT_LIBS:=$(patsubst %,%-c.a,$(patsubst %.a,%,$(LDLIBS)))

$(foreach libsrc,$(LDLIBS), $(eval LIBSRC_FILES += $($(patsubst lib%.a,%_libsrc,$(libsrc)))))
SLIBSRC_TARGETS := $(patsubst %.c,%-s.o,$(LIBSRC_FILES))
CLIBSRC_TARGETS := $(patsubst %.c,%-c.o,$(LIBSRC_FILES))

all:$(SERVER_LIBS) $(CLIENT_LIBS)
.PHONY: all clean preshow

#$(foreach libsrc,$(TARGET), $(eval $(libsrc):$($(patsubst lib%.a,%_libsrc,$(libsrc)))))

$(SERVER_LIBS):$(SLIBSRC_TARGETS)
	$(call echocmd,AR, $@, \
	  $(STARGET_AR) crs $@  $(patsubst %.c,%-s.o,$($(patsubst lib%-s.a,%_libsrc,$@))))

$(SLIBSRC_TARGETS):$(LIBSRC_FILES)
	$(call echocmd,CC, $@, \
	  $(STARGET_CC) $(SERVER_DMACRO) $(INCLUDE) -o $@ -c $(patsubst %-s.o,%.c,$@))

$(CLIENT_LIBS):$(CLIBSRC_TARGETS)
	$(call echocmd,AR, $@, \
	  $(CTARGET_AR) crs $@  $(patsubst %.c,%-c.o,$($(patsubst lib%-c.a,%_libsrc,$@))))

$(CLIBSRC_TARGETS):$(LIBSRC_FILES)
	$(call echocmd,CC, $@, \
	  $(CTARGET_CC) $(CLIENT_DMACRO) $(INCLUDE) -o $@ -c $(patsubst %-c.o,%.c,$@))

clean:
	(find -name "*.[oa]" | xargs $(RM))
