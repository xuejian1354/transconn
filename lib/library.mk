define dependlibs
$(strip \
  $(foreach lib, $(1), \
    $(patsubst lib%.a,lib%$(2).a,$(filter $(patsubst -l%,lib%.a,$(lib)), $(LDLIBS))) \
  ) \
)
endef

define dependlibs-relations
$(1):$(call dependlibs,$(2),$(3))

$(call dependlibs,$(2),$(3)):$(patsubst %,lib/%,$(datastruct_libsrc))
	@make -C $(TOPDIR)/lib $$@
endef


LDLIBS := libdatastruct.a

datastruct_libsrc := mincode.c tpool.c

LDPATH := -L$(TOPDIR)/lib
LDFLAGS += $(patsubst lib%.a,-l%,$(filter %.a, $(LDLIBS)))
