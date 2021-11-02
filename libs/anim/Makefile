sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	obj = anim.w32.o track.w32.o dynarr.w32.o
	lib = ../w32/libanim.a
else
	obj = anim.o track.o dynarr.o
	lib = ../libanim.a
endif

CFLAGS = -O3

$(lib): $(obj)

$(lib): $(obj)
	$(AR) rcs $@ $(obj)

%.w32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(lib)
