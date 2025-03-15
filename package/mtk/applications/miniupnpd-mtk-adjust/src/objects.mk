BASEOBJS = miniupnpd.o upnphttp.o upnpdescgen.o upnpsoap.o \
           upnpreplyparse.o minixml.o portinuse.o \
           upnpredirect.o getifaddr.o daemonize.o \
           options.o upnppermissions.o minissdp.o natpmp.o pcpserver.o \
           upnpglobalvars.o upnpevents.o upnputils.o getconnstatus.o \
           upnpstun.o upnppinhole.o pcplearndscp.o asyncsendto.o

# sources in linux/ directory
LNXOBJS = getifstats.o ifacewatcher.o getroute.o
