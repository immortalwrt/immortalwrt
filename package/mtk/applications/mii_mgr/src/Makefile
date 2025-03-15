EXEC = mii_mgr

CFLAGS += -Wall -Werror

all: $(EXEC)

mii_mgr: mii_mgr.o

	$(CC) $(LDFLAGS) -o $@ $^

romfs:
	$(ROMFSINST) /bin/mii_mgr

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

