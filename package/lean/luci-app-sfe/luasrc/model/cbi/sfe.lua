local trport = 3001
local button = ""
if luci.sys.call("pidof AdGuardHome >/dev/null") == 0 then
	button = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type=\"button\" value=\" " .. translate("Open Web Interface") .. " \" onclick=\"window.open('http://'+window.location.hostname+':" .. trport .. "')\"/>"
end

m = Map("sfe")
m.title	= translate("Turbo ACC Acceleration Settings")
m.description = translate("Opensource Qualcomm Shortcut FE driver (Fast Path)")

m:append(Template("sfe/status"))

s = m:section(TypedSection, "sfe", "")
s.addremove = false
s.anonymous = true


enable = s:option(Flag, "enabled", translate("Enable SFE Fast Path"))
enable.default = 0
enable.rmempty = false
enable.description = translate("Enable Fast Path offloading for connections. (decrease cpu load / increase routing throughput)")

wifi = s:option(Flag, "wifi", translate("Bridge Acceleration"))
wifi.default = 0
wifi.rmempty = false
wifi.description = translate("Enable Bridge Acceleration (may be functional conflict with bridge-mode VPN Server)")
wifi:depends("enabled", 1)

ipv6 = s:option(Flag, "ipv6", translate("IPv6 Acceleration"))
ipv6.default = 0
ipv6.rmempty = false
ipv6.description = translate("Enable IPv6 Acceleration")
ipv6:depends("enabled", 1)

bbr = s:option(Flag, "bbr", translate("Enable BBR"))
bbr.default = 0
bbr.rmempty = false
bbr.description = translate("Bottleneck Bandwidth and Round-trip propagation time (BBR)")

dns = s:option(Flag, "dns", translate("DNS Acceleration"))
dns.default = 0
dns.rmempty = false
dns.description = translate("Enable DNS Cache Acceleration and anti ISP DNS pollution")

o = s:option(ListValue, "dnscache_enable", translate("Resolve Dns Mode"), translate("AdGuardHome's login username/passwd: AdGuardHome, web console addr: IP:3001") .. button)
o:value("1", translate("Use Pdnsd query and cache"))
if nixio.fs.access("/usr/bin/dnsforwarder") then
o:value("2", translate("Use dnsforwarder query and cache"))
end
if nixio.fs.access("/usr/bin/AdGuardHome") then
o:value("3", translate("Use AdGuardHome query and cache"))
end
o.default = 1
o:depends("dns", 1)

o = s:option(Value, "dns_server", translate("Upsteam DNS Server"))
o.default = "114.114.114.114,114.114.115.115,223.5.5.5,223.6.6.6,180.76.76.76,119.29.29.29,119.28.28.28,1.2.4.8,210.2.4.8"
o.description = translate("Muitiple DNS server can saperate with ','")
o:depends("dnscache_enable", 1)
o:depends("dnscache_enable", 2)

o = s:option(Value, "ipv6dns_server", translate("Upsteam IPV6 DNS Server"))
o.default = "2001:4860:4860::8888,2001:4860:4860::8844,2001:2001::1111,2001:2001::1001,2400:da00::6666,240C::6666,240C::6644"
o.description = translate("Muitiple IPV6 DNS server can saperate with ','")
o:depends("dnscache_enable", 2)

return m
