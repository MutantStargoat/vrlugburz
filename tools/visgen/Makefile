obj = main.o
bin = visgen

inc = -I../../libs -I../../libs/treestore
libts = ../../libs/libtreestore.a

CFLAGS = -pedantic -Wall -g $(inc)
LDFLAGS = $(libts)

$(bin): $(obj) $(libts)
	$(CC) -o $@ $(obj) $(LDFLAGS)

$(libts):
	$(MAKE) -C ../../libs/treestore

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
