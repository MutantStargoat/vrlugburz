src = $(wildcard src/*.cc)

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	obj = $(src:.cc=.w32.o)
	alib = ../w32/libgoatvr.a

	mod = -DUSE_MOD_OCULUS -DUSE_MOD_OPENVR
else
	obj = $(src:.cc=.o)
	alib = ../libgoatvr.a

	mod = -DUSE_MOD_OPENVR
endif

warn = -Wno-int-to-pointer-cast
CXXFLAGS = $(warn) -O3 -ffast-math -I. -I.. $(mod)

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

%.w32.o: %.cc
	$(CXX) -o $@ $(CXXFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
