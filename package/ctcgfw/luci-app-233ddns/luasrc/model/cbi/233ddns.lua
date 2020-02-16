-- Created By [CTCGFW]Project-OpenWrt
-- https://github.com/project-openwrt

conf_token = luci.sys.exec("uci get 233ddns.@233ddns[0].token 2>/dev/null")
if (conf_token == nil or conf_token == '') then
	conf_token = luci.sys.exec("cat /dev/urandom | head -n 10 | md5sum | head -c 10")
end
ddns_domain = luci.sys.exec("/etc/init.d/233ddns gen_subdomain " .. conf_token)

mp = Map("233ddns", translate("233DDNS"))
mp.description = translate("A simple, fast, security DDNS client, powered by Oxygen233.") .. "<br/>" .. translate("Instructions for use:") .." https://233.ro/archives/233DDNS.html<br/>" .. translate("Please add u.233.ro to your whitelist if you use proxies program.")

s = mp:section(TypedSection, "233ddns")
s.anonymous=true
s.addremove=false

enable = s:option(Flag, "enable", translate("Enable"))
enable.description = translate("Your DDNS domain is") .. " " .. ddns_domain .. translate(".233ddns.me")
enable.default = 0
enable.rmempty = false

push_ipv6 = s:option(Flag, "push_ipv6", translate("Push IPv6 address"))
push_ipv6.description = translate("Push IPv6 record to remote instead of IPv4.")
push_ipv6.default = 0
push_ipv6.rmempty = false

token = s:option(Value, "token", translate("Token"))
token.description = translate("Must be 10 in length.")
token.default = conf_token
token.placeholder = conf_token
token.datatype = "string"
token.rmempty = false

return mp
