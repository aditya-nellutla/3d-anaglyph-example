.PHONY: all install uninstall clean

TGTFS_PATH:=/path/to/NFS/Flesystem
GSDK_ROOT:=/home1/aditya/linux/Graphics_SDK_4_08_00_02
CROSS_COMPILE:=arm-arago-linux-gnueabi-

CC = $(CROSS_COMPILE)g++
CX = $(CROSS_COMPILE)gcc
INCS     ?= -I../gst_plugin/src \
            -I$(GSDK_ROOT)/include/OGLES \
            -I$(GSDK_ROOT)/include/EGL \
	    -I$(GSDK_ROOT)/targetfs/XSGX/include

LIB_PATH ?= -L$(GSDK_ROOT)/gfx_rel_es8.x \
            -Wl,--rpath=$(GSDK_ROOT)/gfx_rel_es8.x \

INST_DEST := $(TGTFS_PATH)/opt/gstbc

CFLAGS   := -W -Wall -O2 -DLINUX $(INCS)
LIBS     := -lEGL
LDFLAGS  := $(LIB_PATH)

LIBS    += -lGLES_CM

ifeq ($(XORG_BUILD),1)
XLIB = -L$(GSDK_ROOT)/targetfs/XSGX/lib/ -Wl,--rpath=$(GSDK_ROOT)/targetfs/XSGX/lib/ -lX11
LIBS    += $(XLIB)
CFLAGS += -DXORG_BUILD
endif

TARGET = sterio3d

SOURCES = gl_sterio3d.cpp 
HEADRERS = 
OBJFILES += $(SOURCES:%.c=%.o)

all:	$(TARGET)

$(TARGET):	$(OBJFILES) 
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS) $(CFLAGS)

$(OBJFILES):	%.o: %.c $(HEADRERS)
	$(CX) -c $< -o $@ $(CFLAGS)

install:	$(TARGET)
	@mkdir -p $(INST_DEST)
	@cp ../init.sh $(INST_DEST)
	install -m 0755 $^ $(INST_DEST)

uninstall:
	cd $(INST_DEST) && rm -f $(TARGET)

.PHONY: clean
clean:
	-rm -f $(TARGET)
