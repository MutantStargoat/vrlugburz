src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = game

#opt = -O3 -fno-strict-aliasing
dbg = -g
warn = -pedantic -Wall
def = -DMINIGLUT_USE_LIBC
inc = -Ilibs -Ilibs/treestore -Ilibs/drawtext -Ilibs/imago/src
libdir = -Llibs/treestore -Llibs/imago -Llibs/drawtext -Llibs/anim

CFLAGS = $(warn) $(opt) $(dbg) $(def) $(inc) -fcommon -MMD
LDFLAGS = $(libdir) -ldrawtext -limago -ltreestore -lanim -lgoatvr $(libgl) -lm

libgl = -lGL -lX11 -lXext

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

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
