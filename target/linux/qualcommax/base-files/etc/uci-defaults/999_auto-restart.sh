#!/bin/sh

/etc/init.d/network restart
/etc/init.d/odhcpd restart
/etc/init.d/rpcd restart

exit 0
