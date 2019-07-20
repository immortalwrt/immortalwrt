-- Copyright (C) 2016 OpenWrt-dist
-- Copyright (C) 2016 Jian Chang <aa65535@live.com>
-- Licensed to the public under the GNU General Public License v3.

module("luci.controller.chinadns", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/chinadns") then
		return
	end

	entry({"admin", "services", "chinadns"}, cbi("chinadns"), _("ChinaDNS"), 70).dependent = true
	entry({"admin", "services", "chinadns", "status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("ps | grep /usr/sbin/chinadns | grep -v grep >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
