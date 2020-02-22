-- Created By [CTCGFW]Project-OpenWrt
-- https://github.com/project-openwrt

conf_token = luci.sys.exec("uci get 233ddns.@233ddns[0].token 2>/dev/null")
if (conf_token == nil or conf_token == '') then
	conf_token = luci.sys.exec("ifconfig -a |grep eth0 |sed -n 1p |awk '{print $5}' |md5sum |head -c 10")
end

mp = Map("233ddns", translate("233DDNS"))
mp.description = translate("A simple, fast, security DDNS client, powered by Oxygen233.") .. "<br/>" .. translate("Instructions for use:") .." https://233.ro/archives/233DDNS.html<br/>" .. translate("Please add u.233.ro to your whitelist if you use proxy programs.")

mp:section(SimpleSection).template = "233ddns/233ddns_status"

s = mp:section(TypedSection, "233ddns")
s.anonymous=true
s.addremove=false

enable = s:option(Flag, "enable", translate("Enable"))
enable.description = translate("No need to register, simply select \"Enable\" and apply the configure, and you will have your own DDNS domain.")
enable.default = 0
enable.rmempty = false

token = s:option(Value, "token", translate("Token"))
token.description = translate("Must be 10 in length. You can change it anytime, but notice the DDNS domain will be changed simultaneously.")
token.default = conf_token
token.placeholder = conf_token
token.datatype = "string"
token.rmempty = false

return mp
