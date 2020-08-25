-- This is a free software, use it under GNU General Public License v3.0.
-- Created By [CTCGFW]Project OpenWRT
-- https://github.com/project-openwrt

module("luci.controller.naiveproxy", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/naiveproxy") then
		return
	end

	local page
	page = entry({"admin", "services", "naiveproxy"}, cbi("naiveproxy"), _("NaiveProxy"), 100)
	page.dependent = true
	entry({"admin", "services", "naiveproxy", "status"},call("act_status")).leaf=true
end

function act_status()
	local e={}
	e.running=luci.sys.call("pgrep naive >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
