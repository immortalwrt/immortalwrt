--
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "nixio.fs"

m = Map("clash", translate("DNS Acceleration & Forwarder"))
s = m:section(TypedSection, "clash")
s.anonymous = true

dns = s:option(Flag, "dns", translate("Enable DNS"))
dns.default = 1
dns.rmempty = false
dns.description = translate("Enable DNS Cache Acceleration and anti ISP DNS pollution")

o = s:option(ListValue, "dnsmodel", translate("Choose DNS Resolve Model"))
o.description = translate("Choose DNS Resolve With White or Black List Model")
o:value("gfw", translate("GFW List Mode"))
o:value("oversea", translate("Oversea Mode"))
o:depends("dns", 1)

o = s:option(Value, "dnsserver", translate("Domestic DNS Server"))
o.description = translate("DNS Servers located in China")
o:value("119.29.29.29", translate("烟台帝思普网络BPG DNS (119.29.29.29)"))
o:value("101.132.183.99", translate("无污染PdoMo-DNS (101.132.183.99)"))
o:value("193.112.15.186", translate("无污染PdoMo-DNS (193.112.15.186)"))
o:value("114.114.115.115", translate("南京信风网络GreatbitDNS (114.114.115.115)"))
o:value("223.6.6.6", translate("阿里云AliDNS (223.6.6.6)"))
o:value("223.5.5.5", translate("阿里云AliDNS (223.5.5.5)"))
o:value("1.2.4.8", translate("中国互联网络SecureDNS (1.2.4.8)"))
o:depends("dns", 1)

o = s:option(Value, "dnsserver_d", translate("Global DNS Server"))
o.description = translate("DNS Servers located Overseas")
o:value("208.67.222.222", translate("OpenDNS (208.67.222.222)"))
o:value("208.67.220.220", translate("OpenDNS (208.67.220.220)"))
o:value("8.8.4.4", translate("Google Public DNS (8.8.4.4)"))
o:value("8.8.8.8", translate("Google Public DNS (8.8.8.8)"))
o:value("209.244.0.3", translate("Level 3 Public DNS (209.244.0.3)"))
o:value("209.244.0.4", translate("Level 3 Public DNS (209.244.0.4)"))
o:value("4.2.2.1", translate("Level 3 Public DNS (4.2.2.1)"))
o:value("4.2.2.2", translate("Level 3 Public DNS (4.2.2.2)"))
o:value("4.2.2.3", translate("Level 3 Public DNS (4.2.2.3)"))
o:value("4.2.2.4", translate("Level 3 Public DNS (4.2.2.4)"))
o:value("1.1.1.1", translate("Cloudflare DNS (1.1.1.1)"))
o:depends("dns", 1)


gfw_list = s:option(Value, "gfw_list", translate("Self Defined GFW_List"), translate("Please modify the file here."))
gfw_list.template = "cbi/tvalue"
gfw_list.rows = 13
gfw_list.wrap = "off"
gfw_list:depends("dnsmodel", "gfw")

function gfw_list.cfgvalue(self, section)
	return fs.readfile("/etc/config/clash_gfw.list") or ""
end
function gfw_list.write(self, section, value)
	if value then
		value = value:gsub("\r\n", "\n")
		fs.writefile("/etc/config/clash_gfw.list", value)
	end
end

o = s:option(Button,"update")
o.title = translate("Apply Modify List")
o.inputtitle = translate("Apply Modify List")
o.inputstyle = "reload"
o:depends("dnsmodel", "gfw")
o.write = function()
  SYS.call("bash /usr/share/clash/gfw2ipset.sh && /etc/init.d/dnsmasq restart >/dev/null 2>&1 &")
end

o = s:option(Button,"update_gfw")
o.title = translate("Update GFW List")
o.inputtitle = translate("Update GFW List")
o.inputstyle = "reload"
o:depends("dnsmodel", "gfw")
o.write = function()
  SYS.call("bash /usr/share/clash/update.sh && /etc/init.d/dnsmasq restart >/dev/null 2>&1")
end



local apply = luci.http.formvalue("cbi.apply")
if apply then
	os.execute("/etc/init.d/clash restart >/dev/null 2>&1 &")
end





return m
