
module("luci.controller.familycloud", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/familycloud") then
		return
	end

	local page = entry({"admin", "services", "familycloud"},firstchild(), _("天翼家庭云/云盘提速"), 80)
	page.dependent = false
	page.acl_depends = { "luci-app-familycloud" }
	entry({"admin", "services", "familycloud", "general"},cbi("familycloud"), _("Base Setting"), 1)
  entry({"admin", "services", "familycloud", "log"},form("familycloudlog"), _("Log"), 2)
  
	entry({"admin","services","familycloud","status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("ps | grep speedup.sh | grep -v grep >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end

