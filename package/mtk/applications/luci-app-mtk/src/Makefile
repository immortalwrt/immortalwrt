OBJ_IOCTL = ioctl_helper.o rate_calc.o security.o
CFLAGS += -I.
CFLAGS += -I$(ROOTDIR)/user/luci/lua-5.1.5/src
CFLAGS += -Wall -shared -fPIC
LUCI_APP_MTK_TARGET = ioctl_helper

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(LUCI_APP_MTK_TARGET)

ioctl_helper: $(OBJ_IOCTL)
	$(CC) -o $@.so $^ $(CFLAGS)

compile: $(LUCI_APP_MTK_TARGET)

install: compile
	mkdir -p $(DESTDIR)/usr/lib/lua
	cp -pR ioctl_helper.so $(DESTDIR)/usr/lib/lua/

clean:
	rm -f *.o *.so

romfs:
