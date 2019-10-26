-- This is a free software, use it under GNU General Public License v3.0.
-- Created By [CTCGFW]Project OpenWRT
-- https://github.com/project-openwrt

module("luci.controller.ssocks", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/ssocks") then
		return
	end

	entry({"admin", "services", "ssocks"},firstchild(), _("sSocks Server"), 50).dependent = false
	entry({"admin", "services", "ssocks", "general"},cbi("ssocks"), _("Base Setting"), 1)
	entry({"admin", "services", "ssocks", "status"},call("act_status")).leaf=true
end

function act_status()
	local e={}
	e.running=luci.sys.call("ps |grep ssocks |grep -v grep >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
