
APP=lsudev
VERSION=0.1

CFLAGS+=-Wall

PKG_CONFIG?=pkg-config

CFLAGS+=$(shell $(PKG_CONFIG) --cflags libudev)
LDLIBS+=$(shell $(PKG_CONFIG) --libs libudev)

CFLAGS+=-DVERSION=\"$(VERSION)\"

all: $(APP)

$(APP): $(APP).o

clean:
	rm -rf $(APP) *.o
