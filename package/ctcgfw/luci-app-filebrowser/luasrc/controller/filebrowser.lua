module("luci.controller.filebrowser", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/filebrowser") then
		return
	end
	entry({"admin", "nas"}, firstchild(), _("NAS") , 45).dependent = false
	local page
	page = entry({"admin", "nas", "filebrowser"}, cbi("filebrowser"), _("文件管理器"), 100)
	page.dependent = true
	entry({"admin","nas","filebrowser","status"},call("act_status")).leaf=true
end

function act_status()
	local e={}
	e.running=luci.sys.call("pgrep filebrowser >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
