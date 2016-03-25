LDLIBS := libdatastruct.a libcjson.a libmd.a libnopoll.a

datastruct_libsrc := strings_t.c mincode.c tpool.c
cjson_libsrc := cjson.c
md_libsrc := md/md5c.c md/md5hl.c md/sha.c md/shahl.c
nopoll_libsrc := nopoll/nopoll.c \
		  nopoll/nopoll_decl.c \
		  nopoll/nopoll_ctx.c \
		  nopoll/nopoll_conn.c \
		  nopoll/nopoll_log.c \
		  nopoll/nopoll_listener.c \
		  nopoll/nopoll_loop.c \
		  nopoll/nopoll_io.c \
		  nopoll/nopoll_msg.c \
		  nopoll/nopoll_win32.c \
		  nopoll/nopoll_conn_opts.c

LDPATH := -L$(TOPDIR)/$(DIR)lib
LDFLAGS += $(patsubst lib%.a,-l%,$(filter %.a, $(LDLIBS)))
LIBS_INC := $(patsubst %,-I%,$(shell find -type d))
