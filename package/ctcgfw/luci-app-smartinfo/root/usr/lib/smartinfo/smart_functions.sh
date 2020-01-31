#!/bin/sh
# Copyright (C) 2016 NVACG , 穿越蓝天/animefans_xj
# af_xj@hotmail.com
#
#Licensed under the GNU GPL License, Version 3 (the "license");
#you may not use this file except in compliance with the License.
#you may obtain a copy of the License at
#
#	http://www.gnu.org/licenses/gpl.txt

do_smart_check() {
  # 看看指定的设备是否在线
  if [ -b $1 ]; then
    # 如果在线，看看是否支持S.M.A.R.T。支持，则对其执行SMART检测，判断输出是否包含PASSED
    result=`/usr/sbin/smartctl -d sat -H $1`
    if [ $? -eq "2" ]; then
        # 不支持S.M.A.R.T
        return 3
    fi
    
    result=`echo $result | grep -c PASSED`
    if [ $result -ne 0 ]; then
      #无故障
      return 0
    else
      #故障
      return 1
    fi
  else
    # 设备离线
    return "2"
  fi
}

do_simple_smart_check() {
  # 看看指定的设备是否在线
  if [ -b $1 ]; then
    # 如果在线，则对其执行SMART检测，判断输出是否包含PASSED
    result=`/usr/sbin/smartctl -d sat -H $1`
    if [ $? -eq "2" ]; then
        # 不支持S.M.A.R.T
        echo "$1:Unsupported"
        exit 0
    fi
    
    result=`echo $result | grep -c PASSED`
    if [ $result -ne 0 ]; then
      echo "$1:OK"
    else
      echo "$1:Failed"
    fi
  else
    # 如果不在线则输出设备离线
    echo "$1:Offline"
  fi
}
