-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Licensed to the public under the GNU General Public License v3.

local IPK_Version="20200328.1.31"
local m, s, o
local redir_run=0
local reudp_run=0
local sock5_run=0
local ssock5_run=0
local v2sock5_run=0
local server_run=0
local sserver_run=0
local v2server_run=0
local kcptun_run=0
local tunnel_run=0
local udp2raw_run=0
local udpspeeder_run=0
local gfw_count=0
local ad_count=0
local ip_count=0
local gfwmode=0
local pdnsd_run=0
local dnsforwarder_run=0
local dnscrypt_proxy_run=0
local chinadns_run=0
local dns2socks_run=0
local haproxy_run=0
local privoxy_run=0

if nixio.fs.access("/etc/dnsmasq.vssr/gfw_list.conf") then
gfwmode=1		
end

local vssr= "vssr"


-- html constants
font_blue = [[<font color="green">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

local fs = require "nixio.fs"
local sys = require "luci.sys"
local kcptun_version=translate("Unknown")
local kcp_file="/usr/bin/kcptun-client"
if not fs.access(kcp_file)  then
 kcptun_version=translate("Not exist")
else
 if not fs.access(kcp_file, "rwx", "rx", "rx") then
   fs.chmod(kcp_file, 755)
 end
 kcptun_version=sys.exec(kcp_file .. " -v | awk '{printf $3}'")
 if not kcptun_version or kcptun_version == "" then
     kcptun_version = translate("Unknown")
 end
        
end


local udp2raw_version=translate("Unknown")
local udp2raw_file="/usr/bin/udp2raw"
if not fs.access(udp2raw_file) then
	udp2raw_version=translate("Not exist")
else
	if not fs.access(udp2raw_file, "rwx", "rx", "rx") then
		fs.chmod(udp2raw_file, 755)
	end
	udp2raw_version=sys.exec(udp2raw_file .. " -h |grep 'git version' |awk -F ':' '{print $2}'|awk '{print $1}'")
	if not udp2raw_version or udp2raw_version == "" then
		udp2raw_version = translate("Unknown")
	end
end

local udpspeeder_version=translate("Unknown")
local udpspeeder_file="/usr/bin/udpspeeder"
if not fs.access(udpspeeder_file) then
	udpspeeder_version=translate("Not exist")
else
	if not fs.access(udpspeeder_file, "rwx", "rx", "rx") then
		fs.chmod(udpspeeder_file, 755)
	end
	udpspeeder_version=sys.exec(udpspeeder_file .. " -h |grep 'git version' |awk -F ':' '{print $2}'|awk '{print $1}'")
	if not udpspeeder_version or udpspeeder_version == "" then
		udpspeeder_version = translate("Unknown")
	end
end

if gfwmode==1 then 
 gfw_count = tonumber(sys.exec("cat /etc/dnsmasq.vssr/gfw_list.conf | wc -l"))/2
 if nixio.fs.access("/etc/dnsmasq.vssr/ad.conf") then
  ad_count=tonumber(sys.exec("cat /etc/dnsmasq.vssr/ad.conf | wc -l"))
 end
end
 
if nixio.fs.access("/etc/china_ssr.txt") then 
 ip_count = sys.exec("cat /etc/china_ssr.txt | wc -l")
end

function processlist()
	local data = {}
	local netf = {}
	local k
	local ps = luci.util.execi("/bin/busybox top -bn1 | egrep -v dnsmasq")
	local nets = luci.util.execi("netstat -netupl | egrep -v dnsmasq | awk '{print $1,$4,_,$6,$7}'")

	if not ps or not nets then
		return
	end

	for line in nets do
-- tcp        0      0 127.0.0.1:1234          0.0.0.0:*               LISTEN      5103/v2ray
-- udp        0      0 127.0.0.1:1234          0.0.0.0:*                           5147/v2ray
--		local proto, ip, port, nid = line:match("([^%s]+) +.* +([^ ]*):(%d+) +.* +(%d+)\/.*")
		local proto, ip, port, nid = line:match("([^%s]+) (.*):(%d+)[^%d]+(%d+)\/.*")
		local idx = tonumber(nid)
		if idx and ip then
			local newstr = "://" .. ip .. ":" .. port
			local isnew = (netf[idx] and netf[idx]['listen']) and netf[idx]['listen']:match(proto .. newstr)  or false
			netf[idx] = {
				['listen'] = ((netf[idx] and netf[idx]['listen']) and (not isnew) and (netf[idx]['listen'] .. "\n" .. proto) or proto) .. newstr,
			}
		end
	end
-- 5103     1 root     S     661m 543%   0% /usr/bin/v2ray/v2ray -config /var/etc/shadowsocksr.json
	for line in ps do
		local pid, ppid, user, stat, vsz, mem, cpu, cmd = line:match(
			"^ *(%d+) +(%d+) +(%S.-%S) +([RSDZTW][W ][<N ]) +(%d+.?) +(%d+%%) +(%d+%%) +(.+)"
		)
		if cmd then
		local idx = tonumber(pid)
		local bin, param, cfg = cmd:match("^.*\/([^ ]*) *([^ ]*) *\/var\/etc\/([^ ]*).*")
		if idx and cfg then
			local listen = "NONE"
			if netf[idx] and netf[idx]['listen'] then
				listen = netf[idx]['listen']
			end
			data[idx] = {
				['PID']     = pid,
				['COMMAND'] = bin,
				['LISTEN'] = listen,
				['CONFIG']   = cfg,
				['%MEM']    = mem,
				['%CPU']    = cpu,
			}
		end
		end
	end

	return data
end

function printstat(status, form, name)
	local tabs = {
		["Global Client"] = "shadowsocksr.json",
		["Game Mode UDP Relay"] = "shadowsocksr_u.json",
		["PDNSD"] = "pdnsd.conf",
		["DNS Forward"] = "shadowsocksr_d.json",
		["SOCKS5 Proxy"] = "shadowsocksr_s.json",
		["Global SSR Server"] = "shadowsocksr_0.json",
		
	}
	local stat = translate("Unknown")
	local sname = stat
	if tabs[name] and status then
	stat = translate("Not Running")
	for idx, cfg in pairs(status) do
		if status[idx]['CONFIG'] and status[idx]['CONFIG'] == tabs[name] then
			stat = font_blue .. bold_on .. translate("Running") .. bold_off .. " > " .. status[idx]['COMMAND'] .. " -c " .. status[idx]['CONFIG'] .. font_off
			sname = translate(status[idx]['COMMAND'])
			break
		end
	end
	end
	local section = form:field(DummyValue,name,translate(name) .. ": " .. sname)
	section.rawhtml  = true
	section.value = stat
	return section
end

procs=processlist()


local icount=sys.exec("ps -w | grep ssr-reudp |grep -v grep| wc -l")
if tonumber(icount)>0 then
reudp_run=1
else
icount=sys.exec("ps -w | grep ssr-retcp |grep \"\\-u\"|grep -v grep| wc -l")
if tonumber(icount)>0 then
reudp_run=1
end
end


if luci.sys.call("ps -w | grep ssr-retcp | grep -v grep >/dev/null") == 0 then
redir_run=1
end	

if luci.sys.call("pidof ssr-local >/dev/null") == 0 then
sock5_run=1
end

if luci.sys.call("pidof ss-local >/dev/null") == 0 then
ssock5_run=1
end

if luci.sys.call("ps -w | grep v2-ssr-local | grep -v grep >/dev/null") == 0 then
v2sock5_run=1
end

if luci.sys.call("pidof privoxy >/dev/null") == 0 then
privoxy_run=1
end

if luci.sys.call("pidof kcptun-client >/dev/null") == 0 then
kcptun_run=1
end	

if luci.sys.call("pidof ssr-server >/dev/null") == 0 then
server_run=1
end

if luci.sys.call("busybox ps -w | grep ssr-tunnel |grep -v grep >/dev/null") == 0 then
tunnel_run=1
end

if luci.sys.call("pidof ss-server >/dev/null") == 0 then
sserver_run=1
end

if luci.sys.call("ps -w | grep v2ray-server | grep -v grep >/dev/null") == 0 then
v2server_run=1
end	

if luci.sys.call("ps -w | grep ssr-tunnel |grep -v grep >/dev/null") == 0 then
tunnel_run=1
end

if luci.sys.call("pidof udp2raw >/dev/null") == 0 then
udp2raw_run=1
end

if luci.sys.call("pidof udpspeeder >/dev/null") == 0 then
udpspeeder_run=1
end
	
if luci.sys.call("pidof chinadns >/dev/null") == 0 then                 
chinadns_run=1     
end

if luci.sys.call("pidof pdnsd >/dev/null") == 0 then                 
pdnsd_run=1     
end	

if luci.sys.call("pidof dnsparsing >/dev/null") == 0 then                 
dnsforwarder_run=1     
end

if luci.sys.call("pidof dnscrypt-proxy >/dev/null") == 0 then                 
dnscrypt_proxy_run=1     
end

if luci.sys.call("pidof dns2socks >/dev/null") == 0 then                 
dns2socks_run=1     
end

if luci.sys.call("pidof haproxy >/dev/null") == 0 then                 
haproxy_run=1     
end	

m = SimpleForm("Version")
m.reset = false
m.submit = false

t = m:section(Table, procs, translate("Running Details: ") .. "(/var/etc)")
t:option(DummyValue, "PID", translate("PID"))
t:option(DummyValue, "COMMAND", translate("CMD"))
t:option(DummyValue, "LISTEN", translate("LISTEN"))
t:option(DummyValue, "%CPU", translate("CPU usage (%)"))
t:option(DummyValue, "%MEM", translate("Memory usage (%)"))

s=m:field(DummyValue,"redir_run",translate("Global Client")) 
s.rawhtml  = true
if redir_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end

s=m:field(DummyValue,"reudp_run",translate("Game Mode UDP Relay")) 
s.rawhtml  = true
if reudp_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end

s=m:field(DummyValue,"haproxy_run",translate("Load Balancing")) 
s.rawhtml  = true
if haproxy_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end

if nixio.fs.access("/usr/bin/chinadns") then
s=m:field(DummyValue,"chinadns_run",translate("ChinaDNS")) 
s.rawhtml  = true
if chinadns_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

s=m:field(DummyValue,"pdnsd_run",translate("PDNSD"))
s.rawhtml  = true                                              
if pdnsd_run == 1 then                             
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else             
s.value = translate("Not Running")
end 

if nixio.fs.access("/usr/bin/dns2socks") then
s=m:field(DummyValue,"dns2socks_run",translate("DNS2SOCKS"))
s.rawhtml  = true                                              
if dns2socks_run == 1 then                             
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else             
s.value = translate("Not Running")
end 
end 

s=m:field(DummyValue,"dnsforwarder_run",translate("dnsforwarder"))
s.rawhtml  = true                                              
if dnsforwarder_run == 1 then                             
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else             
s.value = translate("Not Running")
end

s=m:field(DummyValue,"dnscrypt_proxy_run",translate("dnscrypt_proxy"))
s.rawhtml  = true                                              
if dnscrypt_proxy_run == 1 then                             
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else             
s.value = translate("Not Running")
end 

if nixio.fs.access("/usr/bin/ss-local") then
s=m:field(DummyValue,"ssock5_run",translate("SSOCKS5 Proxy")) 
s.rawhtml  = true
if ssock5_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/ssr-local") then
s=m:field(DummyValue,"sock5_run",translate("SSR SOCKS5 Proxy")) 
s.rawhtml  = true
if sock5_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end


if nixio.fs.access("/usr/bin/v2ray/v2ray") then
s=m:field(DummyValue,"ssock5_run",translate("V2SOCKS5 Proxy")) 
s.rawhtml  = true
if v2sock5_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/sbin/privoxy") then
s=m:field(DummyValue,"privoxy_run",translate("HTTP Proxy")) 
s.rawhtml  = true
if privoxy_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/ssr-server") then
s=m:field(DummyValue,"server_run",translate("Global SSR Server")) 
s.rawhtml  = true
if server_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/ss-server") then
s=m:field(DummyValue,"sserver_run",translate("Global SS Server")) 
s.rawhtml  = true
if sserver_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/v2ray") then
s=m:field(DummyValue,"v2server_run",translate("Global V2RAY Server")) 
s.rawhtml  = true
if v2server_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

if nixio.fs.access("/usr/bin/kcptun-client") then
s=m:field(DummyValue,"kcp_version",translate("KcpTun Version")) 
s.rawhtml  = true
s.value =kcptun_version

s=m:field(DummyValue,"kcptun_run",translate("KcpTun")) 
s.rawhtml  = true
if kcptun_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
end

s=m:field(DummyValue,"version",translate("IPK Version")) 
s.rawhtml  = true
s.value =IPK_Version

s=m:field(DummyValue,"udp2raw_version",translate("udp2raw Version")) 
s.rawhtml  = true
s.value =udp2raw_version
s=m:field(DummyValue,"udp2raw_run",translate("udp2raw")) 
s.rawhtml  = true
if udp2raw_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end


s=m:field(DummyValue,"udpspeeder_version",translate("UDPspeeder Version")) 
s.rawhtml  = true
s.value =udpspeeder_version
s=m:field(DummyValue,"udpspeeder_run",translate("UDPspeeder")) 
s.rawhtml  = true
if udpspeeder_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end
s=m:field(DummyValue,"feedback",translate("Feedback"))
s.template = "vssr/feedback"
s.value =translate("No feedback")


return m
