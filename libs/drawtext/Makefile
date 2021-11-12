sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	obj = font.w32.o draw.w32.o drawgl.w32.o drawrast.w32.o utf8.w32.o
	lib = ../w32/libdrawtext.a
else
	obj = font.o draw.o drawgl.o drawrast.o utf8.o
	lib = ../libdrawtext.a
endif

CFLAGS = -pedantic -Wall -O3 -DNO_FREETYPE -DNO_GLU -fcommon

$(lib): $(obj)
	$(AR) rcs $@ $(obj)

%.w32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(lib)
