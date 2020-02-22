-- This is a free software, use it under GNU General Public License v3.0.
-- Created By [CTCGFW]Project OpenWRT
-- https://github.com/project-openwrt

module("luci.controller.233ddns", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/233ddns") then
		return
	end
	local page
	page = entry({"admin", "services", "233ddns"}, cbi("233ddns"), _("233DDNS"), 100)
	page.dependent = true
	entry({"admin", "services", "233ddns", "status"},call("act_status")).leaf=true
end

function act_status()
	local e={}
	e.running=luci.sys.call("grep -q 'u.233.ro' '/etc/crontabs/root'")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
