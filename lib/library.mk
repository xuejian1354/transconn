LDLIBS := libdatastruct.a libcjson.a

datastruct_libsrc := strings_t.c mincode.c tpool.c
cjson_libsrc := cjson.c

LDPATH := -L$(TOPDIR)/lib
LDFLAGS += $(patsubst lib%.a,-l%,$(filter %.a, $(LDLIBS)))
