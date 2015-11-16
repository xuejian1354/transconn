include $(TOPDIR)/config.mk

ifeq ($(TOPDIR),)
$(error TOPDIR value no set)
endif

inc_dirs :=/usr/local/mariadb/include/mysql /home/sam/openwrt/barrier_breaker/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include/curl /home/sam/openwrt/barrier_breaker/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include/libxml2/libxml

inc_files := $(strip $(foreach n, $(SUB_MODULES), \
			  $(patsubst %, $(n)/%, \
			    $(filter %.h, \
				  $(notdir $(shell ls $(TOPDIR)/$(n)/)) \
				) \
			  ) \
			))

inc_deps :=$(patsubst %, include/%, $(inc_files))
inc_dirs_deps :=$(strip $(foreach d, $(inc_dirs), \
			      $(patsubst %, include/%, $(notdir $(d))) \
			  ))

ifeq ($(V),1)
$(inc_dirs_deps):$(inc_dirs)
	@if [ ! -d $@ ]; then echo "    [GEN]    $@" && ln -s $(filter %/$(notdir $@),$(inc_dirs)) $@; fi;
else
$(inc_dirs_deps):$(inc_dirs)
	@if [ ! -d $@ ]; then echo "ln -s $(filter %/$(notdir $@),$(inc_dirs)) $@" && ln -s $(filter %/$(notdir $@),$(inc_dirs)) $@; fi;
endif

$(foreach s,$(inc_files),$(eval include/$(s):$(TOPDIR)/$(s)))

include/%:$(TOPDIR)/%
	@mkdir -p $(dir $@)
	$(call echocmd,GEN,$@,ln -s $< $@)
