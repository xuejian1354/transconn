LDLIBS := libdatastruct.a libcjson.a libmd.a libwslay.a

datastruct_libsrc := strings_t.c mincode.c tpool.c
cjson_libsrc := cjson.c
md_libsrc := md5c.c md5hl.c sha.c shahl.c
wslay_libsrc := wslay_frame.c wslay_net.c wslay_queue.c wslay_event.c

LDPATH := -L$(TOPDIR)/$(DIR)lib
LDFLAGS += $(patsubst lib%.a,-l%,$(filter %.a, $(LDLIBS)))
