EXEC = switch

SRC=switch_fun.c switch_753x.c switch_ioctl.c switch_netlink.c

all: $(EXEC)

switch: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS) $(LIBS)

romfs:
	$(ROMFSINST) /bin/switch

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
