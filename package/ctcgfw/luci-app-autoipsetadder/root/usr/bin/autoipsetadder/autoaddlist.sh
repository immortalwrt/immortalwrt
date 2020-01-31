#!/bin/sh
PATH="/usr/sbin:/usr/bin:/sbin:/bin"
logfile=$(uci get autoipsetadder.autoipsetadder.logfile)
[ -z "$logfile" ] && logfile="/tmp/addlist.log"
dnslogfile=$(uci get autoipsetadder.autoipsetadder.dnslogfile)
[ -z "$logfile" ] && dnslogfile="/tmp/dnsmasq.log"
config=$(uci get autoipsetadder.autoipsetadder.config 2>/dev/null)
[ "${config//nochina/}" == "$config" ] && nochina="0" || nochina="1"
[ "${config//packetpass/}" == "$config" ] && packetpass="0" || packetpass="1"
(tail -F $dnslogfile & echo $! >/var/run/autoipsetadder.pid ) | awk -v nochina="$nochina" -v packetpass="$packetpass" -F "[, ]+" '/reply/{
ip=$8;
if (ip=="" || ip=="127.0.0.1"|| ip=="0.0.0.0")
{
next;
}
if (index(ip,"<CNAME>")!=0)
{
if (cname==1)
{
    next;
}
cname=1;
domain=$6;
#第一次cname时锁定域名，防止解析cname对其改动
next;
}
#以上获得上行是否为cname，本行不是cname执行以下内容
#lastdomain记录上次非cname的域名，与本次域名对比
if(lastdomain!=$6){
    for (ipindex in ipcache)
    {
        delete ipcache[ipindex];
    }
    ipcount=0;
    createpid=1;
#上行非cname，并且不是同cname解析域名的多个ip，更新域名，清理同域名免试flag
if (cname!=1)
{
    domain=$6;
    testall=0;
}}
ipcount++;
cname=0;
lastdomain=$6
#去除非ipv4
if (index(ip,".")==0)
{
    next;
}
#不重复探测ip
if (!(ip in a))
{   
#包数>12的同域名放过
if (passdomain==domain)
{
    print(ip" "domain" pass by same domain ok");
    a[ip]=domain;
    next;
}
if (nochina==1){
"ipset test china "ip" 2>&1"| getline ipset;
close("ipset test china "ip" 2>&1");
if (index(ipset,"Warning")!=0){
	print("china "ip" pass");
	a[ip]=domain;
	next;
}}
"ipset test gfwlist "ip" 2>&1"| getline ipset;
close("ipset test gfwlist "ip" 2>&1");
if (index(ipset,"Warning")!=0){
	print("gfwlist "ip" pass");
	a[ip]=domain;
	next;
    }

#ip压入缓存，用于未检测到443/80的缓存
ipcache[ipcount]=ip;
if (testall==0){
    tryhttps=0;
    tryhttp=0;
    #探测 nf_conntrack 的443/80
    while ("grep "ip" /proc/net/nf_conntrack"| getline ret > 0)
    {
        split(ret, b," +");
        split(b[11], pagnum,"=");
        #包数>12的放过
        if (packetpass==1 && pagnum[2]>12)
        {
            print("pass by packets="pagnum[2]" "ip" "domain);
            for (ipindex in ipcache)
            {
                a[ipcache[ipindex]]=domain;
                delete ipcache[ipindex];
            }
            passdomain=domain;
            close("grep "ip" /proc/net/nf_conntrack");
            ipcount--;
            next;
        }
        if (b[8]=="dst="ip)
        {
            if (b[10]=="dport=443"){
                tryhttps=1;
                break;
            }
            else if (b[10]=="dport=80"){
                tryhttp=1;
            }
        }
    }
    close("grep "ip" /proc/net/nf_conntrack");
}else{
    if (testall==443)
    {
        tryhttps=1
    }else{
        tryhttp=1
    }
}
if (tryhttps==1)
{   if (createpid==1)
    {
        print "">"/tmp/run/autoipsetadder/"domain
        close("/tmp/run/autoipsetadder/"domain);
        print("create"domain);
        print(ip" "domain" 443"ipcount-1);
        a[ip]=domain;
        #正在使用的ip用最大延迟，最后探测，减少打断tcp的可能
        system("/usr/bin/autoipsetadder/testip.sh "ip" "domain" 443 "ipcount-1" &");
        delete ipcache[ipcount];
        createpid=0;
    }
    #未检测到443/80同域名缓存的ip进行测试，ipindex-1为测试延迟时间
    for (ipindex in ipcache){
        print(ipcache[ipindex]" "domain" 443 "ipindex-1);
        a[ipcache[ipindex]]=domain;
        system("/usr/bin/autoipsetadder/testip.sh "ipcache[ipindex]" "domain" 443 "ipindex-1" &");
        delete ipcache[ipindex];
    }
    #后续同域名ip免nf_conntrack测试
    testall=443;
}
else if (tryhttp==1)
{   
    if (createpid==1)
    {
        print "">"/tmp/run/autoipsetadder/"domain
        close("/tmp/run/autoipsetadder/"domain);
        print("create"domain);
        print(ip" "domain" 80 "ipcount-1);
        a[ip]=domain;
        system("/usr/bin/autoipsetadder/testip.sh "ip" "domain" 80 "ipcount-1" &");
        delete ipcache[ipcount];
        createpid=0;
    }
    for (ipindex in ipcache){
        print(ipcache[ipindex]" "domain" 80 "ipindex-1);
        a[ipcache[ipindex]]=domain;
        system("/usr/bin/autoipsetadder/testip.sh "ipcache[ipindex]" "domain" 80 "ipindex-1" &");
        delete ipcache[ipindex];
    }
    testall=80;
}}
}'  >> $logfile