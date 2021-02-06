module("luci.controller.turboacc", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/turboacc") then
		return
	end
	local page
	page = entry({"admin", "network", "turboacc"}, cbi("turboacc"), _("Turbo ACC Center"), 101)
	page.i18n = "turboacc"
	page.dependent = true
	page.acl_depends = "luci-app-turboacc"
	
	entry({"admin", "network", "turboacc", "status"}, call("action_status"))
end

local function fastpath_status()
	return luci.sys.call("{ [ -e /sys/module/xt_FLOWOFFLOAD/refcnt ] && [ x$(cat /sys/module/xt_FLOWOFFLOAD/refcnt 2>/dev/null) != x0 ]; } || lsmod | grep -q fast_classifier") == 0
end

local function bbr_status()
	return luci.sys.call("[ x$(cat /proc/sys/net/ipv4/tcp_congestion_control 2>/dev/null) = xbbr ]") == 0
end

local function fullconebat_status()
	return luci.sys.call("[ -e /sys/module/xt_FULLCONENAT/refcnt ] && [ x$(cat /sys/module/xt_FULLCONENAT/refcnt 2>/dev/null) != x0 ]") == 0
end

local function dnscaching_status()
	return luci.sys.call("pgrep dnscache >/dev/null") == 0
end

function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		fastpath_state = fastpath_status(),
		bbr_state = bbr_status(),
		fullconenat_state = fullconebat_status(),
		dnscaching_state = dnscaching_status()
	})
end
