ifneq ($(WHOLE_BUILD),1)
ifneq ("$(origin sprefix)", "command line")
  sprefix :=
endif

ifneq ("$(origin cprefix)", "command line")
  cprefix :=mipsel-openwrt-linux-
endif

ifneq ("$(origin V)", "command line")
  V := 1
endif

export V

$(shell find -name "*.[ch]" | xargs chmod -x)

STARGET_CC := $(sprefix)gcc
STARGET_LD := $(sprefix)ld
STARGET_AR := $(sprefix)ar
STARGET_STRIP := $(sprefix)strip
export STARGET_CC STARGET_LD STARGET_AR STARGET_STRIP

CTARGET_CC := $(cprefix)gcc
CTARGET_LD := $(cprefix)ld
CTARGET_AR := $(cprefix)ar
CTARGET_STRIP := $(cprefix)strip
export CTARGET_CC CTARGET_LD CTARGET_AR CTARGET_STRIP

all:preshow

preshow:
	@echo cross compiler is server:$(STARGET_CC) client:$(CTARGET_CC)

WHOLE_BUILD := 1
export WHOLE_BUILD
endif

ifeq ($(V),1)
  define echocmd
    @echo '    [$1]' '   $2' && $3
  endef
else
  ifeq ($(V),99)
    define echocmd
      $3
    endef
  else
    $(error invalid value V=$(V), please set correct value of 1 or 99)
  endif
endif
