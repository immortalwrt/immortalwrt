#!/bin/sh
# $Id: testupnppermissions.sh,v 1.2 2015/09/22 15:12:14 nanard Exp $

RULE_1="allow 1-20000 11.12.13.14/22 1234"
RULEA_1="allow 1-20000 0b0c0d0e/fffffc00 1234-1234"
RULEB_1="allow 1-20000 11.12.13.14/255.255.252.0 1234-1234"
RULE_2="deny 55 21.22.23.24/17 555-559"
RULEA_2="deny 55-55 15161718/ffff8000 555-559"
RULEB_2="deny 55-55 21.22.23.24/255.255.128.0 555-559"

i=1
s=1
./testupnppermissions "$RULE_1" "$RULE_2" | while read l;
do
	if [ -z "$l" ]; then i=$(($i+1)); s=1; else
		#echo "$i $s : checking '$l'"
		case $s in
			1)
			val=$(eval echo "\${RULE_$i}")
			if [ "$i '$val'" != "$l" ] ; then
				exit $s
			fi;;
			2)
			val=$(eval echo "\${RULEA_$i}")
			if [ "Permission read successfully" = "$l" ] ; then
				s=$(($s+1))
			elif [ "perm rule added : $val" != "$l" ] ; then
				exit $s
			fi;;
			3)
			if [ "Permission read successfully" != "$l" ] ; then
				exit $s
			fi;;
			4)
			val=$(eval echo "\${RULEB_$i}")
			if [ "$val" != "$l" ] ; then
				exit $s
			fi;;
			*)
			echo "$i $s : $l"
			exit $s
			;;
		esac
		s=$(($s+1))
	fi
done

# retrieve return status from subshell
r=$?

if [ $r -eq 0 ] ; then
	echo "testupnppermissions tests OK"
else
	echo "testupnppermissions tests FAILED"
fi
exit $r
