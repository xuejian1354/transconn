include $(TOPDIR)/config.mk

ifeq ($(TOPDIR),)
$(error TOPDIR value no set)
endif

inc_files := $(strip $(foreach n, $(SUB_MODULES), \
			  $(patsubst %, $(n)/%, \
			    $(filter %.h, \
				  $(notdir $(shell ls $(TOPDIR)/$(n)/)) \
				) \
			  ) \
			))

inc_deps := $(patsubst %, include/%, $(inc_files))

$(inc_deps):
	@mkdir -p $(dir $@)
	$(call echocmd,GEN,$@, \
	  ln -s $(patsubst include/%,../../%,$@) $@)
