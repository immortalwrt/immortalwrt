#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

require("nixio.fs")
require("luci.http")

m = Map("softethervpn5", translate("softethervpn5"))

-- Basic
s = m:section(TypedSection, "softethervpn5", translate("Settings"), translate("Save and restart the service after setup"))
s.anonymous = true

---- Eanble
vpn_server_enabled = s:option(Flag, "vpn_server_enabled", translate("VPN Server Enabled"))
vpn_server_enabled.default = 0
vpn_server_enabled.rmempty = false

---- Eanble
vpn_bridge_enabled = s:option(Flag, "vpn_bridge_enabled", translate("VPN Bridge Enabled"))
vpn_bridge_enabled.default = 0
vpn_bridge_enabled.rmempty = false

---- Eanble
vpn_client_enabled = s:option(Flag, "vpn_client_enabled", translate("VPN Client Enabled"))
vpn_client_enabled.default = 0
vpn_client_enabled.rmempty = false

o = s:option(ListValue, "lang", translate("Language"))
o:value("cn", translate("Simplified Chinese"))
o:value("tw", translate("Traditional Chinese"))
o:value("ja", translate("Japanese"))
o:value("en", translate("English"))
o:value("ko", translate("Korean"))
o:value("ru", translate("Russian"))


return m