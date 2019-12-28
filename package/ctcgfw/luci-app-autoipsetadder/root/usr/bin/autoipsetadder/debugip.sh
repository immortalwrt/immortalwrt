#!/bin/sh
PATH="/usr/sbin:/usr/bin:/sbin:/bin"
dlchina=$1;
logfile=$(uci get autoipsetadder.autoipsetadder.logfile)
[ -z "$logfile" ] && logfile="/tmp/addlist.log"

dnslogfile=$(uci get autoipsetadder.autoipsetadder.dnslogfile)
[ -z "$logfile" ] && dnslogfile="/tmp/dnsmasq.log"


ipset list gfwlist | awk -v dlchina="$dlchina" -v dnslogfile="$dnslogfile" -v logfile="$logfile" '{
if (index($0,".")==0)
{
    next;
}
if ($0=="127.0.0.1") {system("ipset d gfwlist 127.0.0.1");next;}
"ipset test whitelist "$0" 2>&1"| getline ipset;
close("ipset test whitelist "$0" 2>&1");
    if (index(ipset,"Warning")==0){
        white=0;
    }else{
        white=1;
    }
    "ipset test china "$0" 2>&1"| getline ipset;
    close("ipset test china "$0" 2>&1");
    if (index(ipset,"Warning")!=0){
        china=1;
    }
    else{
        china=0;
    }
    if (white==1)
    {
        if (china==0)
        {
        print("warning white ip not china"$0);
        ret=system("grep "$0" "logfile);
        if (ret!=0)
        {
            ret=system("grep "$0" "dnslogfile);
        }
        }
    }else if (china==1)
	{
		print("warning china ip not white"$0);
		ret=system("grep "$0" "logfile)
		if (ret!=0)
		{
			ret=system("grep "$0" "dnslogfile);
		}
		if (dlchina)
		{
			system("ipset del gfwlist "$0);
		}
	}  
}'