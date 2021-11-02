src = $(wildcard *.cc)

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	obj = $(src:.cc=.w32.o)
	alib = ../w32/libgmath.a
else
	obj = $(src:.cc=.o)
	alib = ../libgmath.a
endif

CXXFLAGS = -O3 -ffast-math

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

%.w32.o: %.cc
	$(CXX) -o $@ $(CXXFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
