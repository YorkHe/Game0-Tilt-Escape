.PHONY : all

HOSTNAME := $(shell hostname)

ifeq ($(HOSTNAME), incepchow)
	BLENDER = ~/blender-2.79b-linux-glibc219-x86_64/blender
else
	BLENDER = blender
endif

DIST=../dist

all : \
	$(DIST)/board.blob \


$(DIST)/board.blob : board.blend export-meshes.py
	$(BLENDER) --background --enable-new-depsgraph --python export-meshes.py -- '$<' '$@'
