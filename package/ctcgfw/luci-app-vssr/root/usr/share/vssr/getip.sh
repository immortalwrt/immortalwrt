#!/bin/bash
# Copyright (C) 2019 Jerryk <jerrykuku@qq.com>
python=python3
host=$1
$python -c "import maxminddb;
import json;
reader = maxminddb.open_database('/usr/share/vssr/GeoLite2-Country.mmdb');
aa = reader.get('${host}');
reader.close();
flags = aa['country']['iso_code'].lower();
country = aa['country']['names']['zh-CN'];
data = {
    'flag' : flags,
    'country' : country
    
}
print(data)"
