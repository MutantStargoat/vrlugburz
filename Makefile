# build options --------
vrbuild = true
# ----------------------

src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = game

#opt = -O3 -ffast-math -fno-strict-aliasing
dbg = -g
warn = -pedantic -Wall
def = -DMINIGLUT_USE_LIBC -DGLDEBUG
inc = -Ilibs -Ilibs/treestore -Ilibs/drawtext -Ilibs/imago/src -Ilibs/goatvr

CFLAGS = $(warn) $(opt) $(dbg) $(def) $(inc) -fcommon -MMD
LDFLAGS = $(libdir) $(libsys) $(libgl) $(libal) $(libvr) -ldrawtext -limago -ltreestore \
		  -lanim $(libc)

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	obj = $(src:.c=.w32.o)
	bin = game.exe
	libdir = -Llibs/w32
	libgl = -lopengl32
	libal = -lopenal
	libsys = -lmingw32 -lgdi32 -lwinmm -mconsole
	libc = -lm

	ifeq ($(vrbuild), true)
		libvr = -lgoatvr -lgmath -lovr -lopenvr_api
	endif
else
	libdir = -Llibs
	libgl = -lGL -lX11 -lXext
	libal = -lopenal
	libc = -lm -ldl

	ifeq ($(vrbuild), true)
		libvr = -lgoatvr -lgmath -lopenvr_api
	endif
endif

ifeq ($(vrbuild), true)
	def += -DBUILD_VR
	LD = $(CXX)
else
	LD = $(CC)
endif

$(bin): $(obj)
	$(LD) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.w32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: libs
libs:
	$(MAKE) -C libs

.PHONY: clean-libs
clean-libs:
	$(MAKE) -C libs clean


.PHONY: cross
cross:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw

.PHONY: cross-libs
cross-libs:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw -C libs

.PHONY: cross-clean
cross-clean:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw clean

.PHONY: cross-clean-libs
cross-clean-libs:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw -C libs clean
