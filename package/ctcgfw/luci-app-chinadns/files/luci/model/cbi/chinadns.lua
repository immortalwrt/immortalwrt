-- Copyright (C) 2016 OpenWrt-dist
-- Copyright (C) 2016 Jian Chang <aa65535@live.com>
-- Copyright (C) 2017 Ian Li <OpenSource@ianli.xyz>
-- Licensed to the public under the GNU General Public License v3.

m = Map("chinadns", translate("ChinaDNS"))
m:section(SimpleSection).template  = "chinadns/chinadns_status"

s = m:section(TypedSection, "chinadns", translate("General Setting"))
s.anonymous   = true

o = s:option(Flag, "enable", translate("Enable"))
o.rmempty     = false

o = s:option(Flag, "yn_use_proxy", 
	translate("Used a proxy or not"),
	translate("If you used a proxy like shadowsocks, please tick this option."))
o.rmenpty     = false

o = s:option(Value, "port", translate("Local Port"))
o.placeholder = 5353
o.default     = 5353
o.datatype    = "port"
o.rmempty     = false

o = s:option(Value, "chnroute", translate("CHNRoute File"))
o.placeholder = "/etc/chinadns_chnroute.txt"
o.default     = "/etc/chinadns_chnroute.txt"
o.datatype    = "file"
o.rmempty     = false

o = s:option(Value, "foreign_subnet",
	translate("Foreign Subnet"),
	translate("Just foreign subnet. The local subnet will be automatically obtained."))
o.placeholder = "45.76.96.0"
o.default     = "45.76.96.0"
o.datatype    = "ipaddr"
o:depends("yn_use_proxy", 0)

o = s:option(Value, "server",
	translate("Upstream Server"),
	translate("A DNS server with edns-client-subnet support required, fill one only!"))
o.placeholder = "8.8.8.8"
o.default     = "8.8.8.8"
o.datatype    = "ipaddr"
o.rmempty     = false

return m
