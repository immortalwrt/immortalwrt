all: libipt_FULLCONENAT.so libip6t_FULLCONENAT.so

libipt_FULLCONENAT.so: libipt_FULLCONENAT.o
	$(CC) -shared -lxtables -o $@ $^;
libipt_FULLCONENAT.o: libipt_FULLCONENAT.c
	$(CC) ${CFLAGS} -fPIC -D_INIT=$*_init -c -o $@ $<;
libip6t_FULLCONENAT.so: libip6t_FULLCONENAT.o
	$(CC) -shared -lxtables -o $@ $^;
libip6t_FULLCONENAT.o: libip6t_FULLCONENAT.c
	$(CC) ${CFLAGS} -fPIC -D_INIT=$*_init -c -o $@ $<;

obj-m += xt_FULLCONENAT.o
