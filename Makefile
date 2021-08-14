src = $(wildcard src/*.c)
obj = $(src:.c=.o)
bin = lugburz

#opt = -O3
dbg = -g
warn = -pedantic -Wall
def = -DMINIGLUT_USE_LIBC

CFLAGS = $(warn) $(opt) $(dbg) $(def) $(inc) -fcommon -MMD
LDFLAGS = -lGL -lX11 -lm

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
