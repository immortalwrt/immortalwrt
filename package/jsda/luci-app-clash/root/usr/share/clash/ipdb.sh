#!/bin/sh
   wget-ssl --no-check-certificate --user-agent="User-Agent: Mozilla" https://geolite.maxmind.com/download/geoip/database/GeoLite2-Country.tar.gz -O /tmp/ipdb.tar.gz
   if [ "$?" -eq "0" ]; then
      tar zxvf /tmp/ipdb.tar.gz -C /tmp\
      && rm -rf /tmp/ipdb.tar.gz >/dev/null 2>&1\
      && mv /tmp/GeoLite2-Country_*/GeoLite2-Country.mmdb /etc/clash/Country.mmdb\
      && rm -rf /tmp/GeoLite2-Country_* >/dev/null 2>&1
   fi
 