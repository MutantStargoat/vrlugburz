src = $(wildcard src/*.c) ../../src/level.c
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = dunger

CFLAGS = -pedantic -Wall -g
LDFLAGS = -lGL -lGLU -lglut -lutk -ldrawtext -ltreestore

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
