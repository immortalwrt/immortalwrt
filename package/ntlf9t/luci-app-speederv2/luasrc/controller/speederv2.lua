module("luci.controller.speederv2", package.seeall)

function index()
	if nixio.fs.access("/etc/config/speederv2") then
        entry({"admin", "services", "speederv2"}, firstchild(), _("speederv2-tunnel")).dependent = false
        entry({"admin", "services", "speederv2", "general"}, cbi("speederv2/general"), _("Settings"), 1)
        entry({"admin", "services", "speederv2", "servers"}, arcombine(cbi("speederv2/servers"), cbi("speederv2/servers-details")), _("Configure Management"), 2).leaf = true
        entry({"admin", "services", "speederv2", "status"}, call("action_status"))
    else
		return
	end
end

local function is_running(name)
	return luci.sys.call("pidof %s >/dev/null" %{name}) == 0
end

function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		running = is_running("udpspeeder")
	})
end
