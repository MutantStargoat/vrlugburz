src = src/main.c src/lview.c src/level.c
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = dunger

CFLAGS = -pedantic -Wall -g -DLEVEL_EDITOR -I../../src -fcommon
LDFLAGS = -lGL -lGLU -lglut -lutk -ldrawtext -ltreestore

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

src/level.c: ../../src/level.c
	cp $< $@

-include $(dep)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
