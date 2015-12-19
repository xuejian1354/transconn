ifneq ($(WHOLE_BUILD),1)
ifneq ("$(origin prefix)", "command line")
  prefix :=
endif

ifneq ("$(origin sprefix)", "command line")
  sprefix :=
endif

ifneq ("$(origin cprefix)", "command line")
  cprefix :=mipsel-openwrt-linux-
endif

ifneq ("$(origin dir)", "command line")
  dir :=
endif

ifneq ("$(dir)", "")
  DIR :=$(dir)/
else
  DIR :=
endif

ifneq ("$(origin V)", "command line")
  V := 1
endif

ifneq ("$(origin S)", "command line")
  S := 1
endif

export DIR V S

TARGET_CC := $(prefix)gcc
TARGET_CXX := $(prefix)g++
TARGET_LD := $(prefix)ld
TARGET_AR := $(prefix)ar
TARGET_STRIP := $(prefix)strip
export TARGET_CC TARGET_CXX TARGET_LD TARGET_AR TARGET_STRIP

STARGET_CC := $(sprefix)gcc
STARGET_CXX := $(sprefix)g++
STARGET_LD := $(sprefix)ld
STARGET_AR := $(sprefix)ar
STARGET_STRIP := $(sprefix)strip
export STARGET_CC STARGET_CXX STARGET_LD STARGET_AR STARGET_STRIP

CTARGET_CC := $(cprefix)gcc
CTARGET_CXX := $(cprefix)g++
CTARGET_LD := $(cprefix)ld
CTARGET_AR := $(cprefix)ar
CTARGET_STRIP := $(cprefix)strip
export CTARGET_CC CTARGET_CXX CTARGET_LD CTARGET_AR CTARGET_STRIP

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

ifneq ($S,1)
  ifneq ($S,99)
    $(error invalid value S=$(S), please set correct value of 1 or 99)
  endif
endif
