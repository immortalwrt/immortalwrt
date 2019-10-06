#!/bin/sh
# Copyright (C) 2016 NVACG , 穿越蓝天/animefans_xj
# af_xj@hotmail.com
#
#Licensed under the GNU GPL License, Version 3 (the "license");
#you may not use this file except in compliance with the License.
#you may obtain a copy of the License at
#
#	http://www.gnu.org/licenses/gpl.txt

. /lib/functions.sh
. /usr/lib/smartinfo/smart_functions.sh

config_load "smartinfo"

# 执行S.M.A.R.T检测
config_list_foreach main device do_simple_smart_check
