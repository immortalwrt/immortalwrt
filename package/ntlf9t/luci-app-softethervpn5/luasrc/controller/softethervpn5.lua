#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>

module("luci.controller.softethervpn5", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/softethervpn5") then
		return
	end
	
	entry({"admin", "vpn", "softethervpn5"},alias("admin", "vpn", "softethervpn5", "setting"),_("SoftEther VPN5"), 10).dependent = true
	entry({"admin", "vpn", "softethervpn5", "setting"},arcombine(cbi("softethervpn5/setting"), form("softethervpn5/setting-config")),_("VPN Setting"), 10).leaf = true
	
	entry({"admin", "vpn", "softethervpn5", "server"},form("softethervpn5/server"),_("VPN Server"), 20).leaf = true
	entry({"admin", "vpn", "softethervpn5", "bridge"},form("softethervpn5/bridge"),_("VPN Bridge"), 30).leaf = true
	entry({"admin", "vpn", "softethervpn5", "client"},form("softethervpn5/client"),_("VPN Client"), 40).leaf = true
	entry({"admin", "vpn", "softethervpn5", "status"},form("softethervpn5/status"),_("Status"), 50).leaf = true
end
