include $(TOPDIR)/config.mk

ifeq ($(TOPDIR),)
$(error TOPDIR value no set)
endif

inc_dirs :=/usr/local/mariadb/include/mysql

inc_files := $(strip $(foreach n, $(SUB_MODULES), \
			  $(patsubst %, $(n)/%, \
			    $(filter %.h, \
				  $(notdir $(shell ls $(TOPDIR)/$(n)/)) \
				) \
			  ) \
			))

inc_deps := $(patsubst %, include/%, $(inc_files))
inc_dirs_deps := $(strip $(foreach d, $(inc_dirs), \
			      $(patsubst %, include/%, $(notdir $(d))) \
			  ))

ifeq ($(V),1)
$(inc_dirs_deps):$(inc_dirs)
	@if [ ! -d $@ ]; then echo "    [GEN]    $@" && ln -s $< $@; fi;
else
$(inc_dirs_deps):$(inc_dirs)
	@if [ ! -d $@ ]; then echo "ln -s $< $@" && ln -s $< $@; fi;
endif

$(inc_deps):$(inc_dirs_deps)
	@mkdir -p $(dir $@)
	$(call echocmd,GEN,$@, \
	  ln -s $(patsubst include/%,../../%,$@) $@)
