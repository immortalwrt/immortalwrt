#!/bin/bash

#python3_ver=$(ls /usr/bin|grep -e "^python3[23]\.[1-9]\+$"|tail -1)
eval $(ps -ef | grep "[0-9] python3 server\\.py a" | awk '{print "kill "$2}')
