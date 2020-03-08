-- This is a free software, use it under GNU General Public License v3.0.
-- Created By [CTCGFW]Project OpenWRT
-- https://github.com/project-openwrt

module("luci.controller.ssocks", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ssocks") then
		return
	end
	local page
	page = entry({"admin", "vpn", "ssocks"}, cbi("ssocks"), _("sSocks Server"), 100)
	page.i18n = "ssocks"
	page.dependent = true
	entry({"admin", "vpn", "ssocks", "status"},call("act_status")).leaf=true
end

function act_status()
	local e={}
	e.running=luci.sys.call("ps |grep ssocks |grep -v grep >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
