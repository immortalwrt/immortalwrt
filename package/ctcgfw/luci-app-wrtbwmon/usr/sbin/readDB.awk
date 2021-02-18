#!/usr/bin/awk

function inInterfaces(host) {
	return(interfaces ~ "(^| )" host "($| )")
}

function newRule(arp_ip, ipt_cmd) {
	# checking for existing rules shouldn't be necessary if newRule is
	# always called after db is read, arp table is read, and existing
	# iptables rules are read.
	ipt_cmd=iptKey " -t mangle -j RETURN -s " arp_ip
	system(ipt_cmd " -C RRDIPT_FORWARD 2>/dev/null || " ipt_cmd " -A RRDIPT_FORWARD")
	ipt_cmd=iptKey " -t mangle -j RETURN -d " arp_ip
	system(ipt_cmd " -C RRDIPT_FORWARD 2>/dev/null || " ipt_cmd " -A RRDIPT_FORWARD")
}

function delRule(arp_ip, ipt_cmd) {
	ipt_cmd=iptKey " -t mangle -D RRDIPT_FORWARD -j RETURN "
	system(ipt_cmd "-s " arp_ip " 2>/dev/null")
	system(ipt_cmd "-d " arp_ip " 2>/dev/null")
}

function total(i) {
	return(bw[i "/in"] + bw[i "/out"])
}

BEGIN {
	if (ipv6) {
		iptNF	= 8
		iptKey	= "ip6tables"
	} else {
		iptNF	= 9
		iptKey	= "iptables"
	}
}

/^#/ { # get DB filename
	FS	= ","
	dbFile	= FILENAME
	next
}

# data from database; first file
ARGIND==1 { #!@todo this doesn't help if the DB file is empty.
	lb=$1

	if (lb !~ "^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$") next

	if (!(lb in mac)) {
		mac[lb]		= $1
		ip[lb]		= $2
		inter[lb]	= $3
		speed[lb "/in"]	= 0
		speed[lb "/out"]= 0
		bw[lb "/in"]	= $6
		bw[lb "/out"]	= $7
		firstDate[lb]	= $9
		lastDate[lb]	= $10
		ignore[lb]	= 1
	} else {
		if ($9 < firstDate[lb])
			firstDate[lb]	= $9
		if ($10 > lastDate[lb]) {
			ip[lb]		= $2
			inter[lb]	= $3
			lastDate[lb]	= $10
		}
		bw[lb "/in"]	+= $6
		bw[lb "/out"]	+= $7
		ignore[lb]	= 0
	}
	next
}

# not triggered on the first file
FNR==1 {
	FS=" "
	if(ARGIND == 2) next
}

# arp: ip hw flags hw_addr mask device
ARGIND==2 {
	#!@todo regex match IPs and MACs for sanity
	if (ipv6) {
		statFlag= ($4 != "FAILED" && $4 != "INCOMPLETE")
		macAddr	= $5
		hwIF	= $3
	} else {
		statFlag= ($3 != "0x0")
		macAddr	= $4
		hwIF	= $6
	}

	lb=$1
	if (hwIF != wanIF && statFlag && macAddr ~ "^([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}$") {
		hosts[lb]		= 1
		arp_mac[lb]		= macAddr
		arp_ip[lb]		= $1
		arp_inter[lb]		= hwIF
		arp_bw[lb "/in"]	= 0
		arp_bw[lb "/out"]	= 0
		arp_firstDate[lb]	= systime()
		arp_lastDate[lb]	= arp_firstDate[lb]
		arp_ignore[lb]		= 1
	}
	next
}

#!@todo could use mangle chain totals or tailing "unnact" rules to
# account for data for new hosts from their first presence on the
# network to rule creation. The "unnact" rules would have to be
# maintained at the end of the list, and new rules would be inserted
# at the top.
ARGIND==3 && NF==iptNF && $1!="pkts" { # iptables input
	if (ipv6) {
		lfn = 5
		tag = "::/0"
	} else {
		lfn = 6
		tag = "0.0.0.0/0"
	}

	if ($(lfn) != "*") {
		m = $(lfn)
		n = m "/in"
	} else if ($(++lfn) != "*") {
		m = $(lfn)
		n = m "/out"
	} else if ($(++lfn) != tag) {
		m = $(lfn)
		n = m "/out"
	} else { # $(++lfn) != tag
		m = $(++lfn)
		n = m "/in"
	}

	if (mode == "diff" || mode == "noUpdate") print n, $2
	if (mode != "noUpdate") {
		if (inInterfaces(m)) { # if label is an interface
			if (!(m in arp_mac)) {
				cmd = "cat /sys/class/net/" m "/address"
				cmd | getline arp_mac[m]
				close(cmd)

				if (length(arp_mac[m]) == 0) arp_mac[m] = "00:00:00:00:00:00"

				arp_ip[m]		= "NA"
				arp_inter[m] 		= m
				arp_bw[m "/in"]		= 0
				arp_bw[m "/out"]	= 0
				arp_firstDate[m]	= systime()
				arp_lastDate[m]		= arp_firstDate[m]
				arp_ignore[lb]		= 1
			}
		} else {
			if (!(m in arp_mac)) hosts[m] = 0
			else delete hosts[m]
		}

		if ($2 > 0) {
			arp_bw[n]	= $2
			arp_lastDate[m]	= systime()
			arp_ignore[m]	= 0
		}
	}
}

END {
	if (mode == "noUpdate") exit

	for (i in arp_ip) {
		lb = arp_mac[i]
		if (!arp_ignore[i] || !(lb in mac)) {
			ignore[lb]	= 0

			if (lb in mac) {
				bw[lb "/in"]	+= arp_bw[i "/in"]
				bw[lb "/out"]	+= arp_bw[i "/out"]
				lastDate[lb]	= arp_lastDate[i]
			} else {
				bw[lb "/in"]	= arp_bw[i "/in"]
				bw[lb "/out"]	= arp_bw[i "/out"]
				firstDate[lb]	= arp_firstDate[i]
				lastDate[lb]	= arp_lastDate[i]
			}
			mac[lb]		= arp_mac[i]
			ip[lb]		= arp_ip[i]
			inter[lb]	= arp_inter[i]

			if (interval != 0) {
				speed[lb "/in"]	= int(arp_bw[i "/in"] / interval)
				speed[lb "/out"]= int(arp_bw[i "/out"] / interval)
			}
		}
	}

	close(dbFile)
	for (i in mac) {
		if (!ignore[i]) {
			print "#mac,ip,iface,speed_in,speed_out,in,out,total,first_date,last_date" > dbFile
			OFS=","
			for (i in mac)
				print mac[i], ip[i], inter[i], speed[i "/in"], speed[i "/out"], bw[i "/in"], bw[i "/out"], total(i), firstDate[i], lastDate[i] > dbFile
			close(dbFile)
			break
		}
	}

	# for hosts without rules
	for (i in hosts)
		if (hosts[i]) newRule(i)
		else delRule(i)
}
