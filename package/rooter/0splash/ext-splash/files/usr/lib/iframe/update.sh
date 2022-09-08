#!/bin/sh 
. /lib/functions.sh

bwdata() {
	while IFS= read -r line; do
		if [ $line = '0' ]; then
			nodata="1"
			break
		else
			nodata="0"
			days=$line
			read -r line
			read -r line
			tused=$line
			read -r line
			read -r line
			tdwn=$line
			read -r line
			read -r line
			tup=$line
			read -r line
			read -r line
			project=$line
			break
		fi
	done < /tmp/bwdata
}

logtype=$(uci -q get iframe.login.logtype)
if [ $logtype = "1" ]; then
	STEMP="/tmp/www/itemp.html"
	STATUS="/usr/lib/iframe/iframe.html"
	IFSTATUS="/tmp/www/display.html"
	
	rm -f $STEMP
	cp $STATUS $STEMP
	bwdata
	
	sed -i -e "s!#TITLE#!Bandwidth Usage!g" $STEMP
	sed -i -e "s!#DAYS#!$days!g" $STEMP
	sed -i -e "s!#TOTAL#!$tused!g" $STEMP
	sed -i -e "s!#DOWN#!$tdwn!g" $STEMP
	sed -i -e "s!#UP#!$tup!g" $STEMP
	sed -i -e "s!#PROJ#!$project!g" $STEMP
	
	mv $STEMP $IFSTATUS
fi

if [ $logtype = "2" ]; then
	STEMP="/tmp/www/itemp.html"
	STATUS="/usr/lib/iframe/image.html"
	IFSTATUS="/tmp/www/display.html"
	
	rm -f $STEMP
	cp $STATUS $STEMP
	logimage=$(uci -q get iframe.login.logimage)
	sed -i -e "s!#IMAGE#!$logimage!g" $STEMP
	logimagewidth=$(uci -q get iframe.login.logimagewidth)
	sed -i -e "s!#WIDTH#!$logimagewidth!g" $STEMP
	
	mv $STEMP $IFSTATUS
fi

if [ $logtype = "4" ]; then
	STEMP="/tmp/www/itemp.html"
	STATUS="/usr/lib/iframe/speed.html"
	IFSTATUS="/tmp/www/display.html"
	
	rm -f $STEMP
	cp $STATUS $STEMP
	
	mv $STEMP $IFSTATUS
fi

if [ $logtype = "3" ]; then
	STEMP="/tmp/www/itemp.html"
	STATUS="/usr/lib/iframe/zerotier.html"
	IFSTATUS="/tmp/www/display.html"
	
	rm -f $STEMP
	cp $STATUS $STEMP
	
	ID=$(uci -q get zerotier.zerotier.secret)
	if [ -z $ID ]; then
		ID="xxxxxxxxxx"
	else
		ID=${ID:0:10}
	fi
	
	sed -i -e "s!#ID#!$ID!g" $STEMP
	source /etc/codename
	sed -i -e "s!#VER#!$CODENAME!g" $STEMP
		
	mv $STEMP $IFSTATUS
fi