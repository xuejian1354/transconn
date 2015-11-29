LDLIBS := libdatastruct.a libcjson.a libmd.a

datastruct_libsrc := strings_t.c mincode.c tpool.c
cjson_libsrc := cjson.c
md_libsrc := md5c.c md5hl.c sha.c shahl.c

LDPATH := -L$(TOPDIR)/$(DIR)lib
LDFLAGS += $(patsubst lib%.a,-l%,$(filter %.a, $(LDLIBS)))
