
APP=lsudev

CFLAGS+=-Wall

PKG_CONFIG?=pkg-config

CFLAGS+=$(shell $(PKG_CONFIG) --cflags libudev)
LDLIBS+=$(shell $(PKG_CONFIG) --libs libudev)

all: $(APP)

$(APP): $(APP).o

clean:
	rm -rf $(APP) *.o
