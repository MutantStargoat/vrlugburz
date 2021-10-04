.PHONY: all
all: treestore imago drawtext anim

.PHONY: clean
clean: clean-treestore clean-imago clean-drawtext clean-anim

.PHONY: treestore
treestore:
	$(MAKE) -C treestore

.PHONY: clean-treestore
clean-treestore:
	$(MAKE) -C treestore clean

.PHONY: imago
imago:
	$(MAKE) -C imago

.PHONY: clean-imago
clean-imago:
	$(MAKE) -C imago clean

.PHONY: drawtext
drawtext:
	$(MAKE) -C drawtext

.PHONY: clean-drawtext
clean-drawtext:
	$(MAKE) -C drawtext clean

.PHONY: anim
anim:
	$(MAKE) -C anim

.PHONY: clean-anim
clean-anim:
	$(MAKE) -C anim clean
