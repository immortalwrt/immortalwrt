
module("luci.controller.unblockmusic", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/unblockmusic") then
		return
	end

	local page = entry({"admin", "services", "unblockmusic"},firstchild(), _("Unblock Netease Music"), 50)
	page.dependent = false
	page.acl_depends = { "luci-app-unblockmusic" }
	
	entry({"admin", "services", "unblockmusic", "general"},cbi("unblockmusic"), _("Base Setting"), 1)
	entry({"admin", "services", "unblockmusic", "log"},form("unblockmusiclog"), _("Log"), 2)
  
	entry({"admin", "services", "unblockmusic", "status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("busybox ps -w | grep UnblockNeteaseMusic | grep -v grep | grep -v logcheck.sh >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end
