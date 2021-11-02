ifeq ($(sys), mingw)
	obj = treestore.w32.o text.w32.o dynarr.w32.o
	lib = ../w32/libtreestore.a
else
	obj = treestore.o text.o dynarr.o
	lib = ../libtreestore.a
endif

CFLAGS = -O3

$(lib): $(obj)
	$(AR) rcs $@ $(obj)

%.w32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(lib)
