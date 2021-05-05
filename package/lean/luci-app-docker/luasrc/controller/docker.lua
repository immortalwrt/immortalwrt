module("luci.controller.docker", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/dockerd") then
		return
	end
	
	local page = entry({"admin", "services", "docker"}, cbi("docker"), _("Docker CE Container"), 199)
	page.dependent = true
	page.acl_depends = { "luci-app-docker" }
	entry({"admin","services","docker","status"},call("act_status")).leaf=true
end

function act_status()
  local e={}
  e.running=luci.sys.call("pgrep /usr/bin/dockerd >/dev/null")==0
  luci.http.prepare_content("application/json")
  luci.http.write_json(e)
end