#!/bin/bash
del_list="$(cat test | grep -Eo "target[.\/0-9a-zA-Z_-]+.patch" | uniq)"
for i in $del_list
do
	echo $i
	rm $i
done
