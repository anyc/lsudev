
PKG_CONFIG?=pkg-config

CFLAGS+=$(shell $(PKG_CONFIG) --cflags libudev)
LDLIBS+=$(shell $(PKG_CONFIG) --libs libudev)

all: lsdev

lsdev: lsdev.o
