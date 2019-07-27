module("luci.controller.clash", package.seeall)

function index()

	if not nixio.fs.access("/etc/config/clash") then
		return
	end

	entry({"admin", "services", "clash"},alias("admin", "services", "clash", "overview"), _("Clash"), 60).dependent = false
	entry({"admin", "services", "clash", "overview"},cbi("clash/overview"),_("Overview"), 10).leaf = true
	entry({"admin", "services", "clash", "client"},cbi("clash/client"),_("Client"), 20).leaf = true
	entry({"admin", "services", "clash", "settings"}, firstchild(),_("Settings"), 100)
	entry({"admin", "services", "clash", "settings", "port"},cbi("clash/port"),_("Proxy Ports"), 100).leaf = true
	entry({"admin", "services", "clash", "settings", "dns"},cbi("clash/dns"),_("DNS Settings"), 120).leaf = true
	entry({"admin", "services", "clash", "settings", "access"},cbi("clash/access"),_("Access Control"), 130).leaf = true
	entry({"admin", "services", "clash", "config"},cbi("clash/config"),_("Config"), 140).leaf = true
	entry({"admin","services","clash","status"},call("action_status")).leaf=true
	entry({"admin", "services", "clash", "log"},cbi("clash/log"),_("Logs"), 150).leaf = true
	entry({"admin","services","clash","check_status"},call("check_status")).leaf=true

	
end

local function dash_port()
	return luci.sys.exec("uci get clash.config.dash_port 2>/dev/null")
end
local function dash_pass()
	return luci.sys.exec("uci get clash.config.dash_pass 2>/dev/null")
end

local function is_running()
	return luci.sys.call("pidof clash >/dev/null") == 0
end

local function is_web()
	return luci.sys.call("pidof clash >/dev/null") == 0
end

local function localip()
	return luci.sys.exec("uci get network.lan.ipaddr")
end

local function check_version()
	return luci.sys.exec("sh /usr/share/clash/check_version.sh")
end

local function current_version()
	return luci.sys.exec("sed -n 1p /usr/share/clash/clash_version")
end

local function new_version()
	return luci.sys.exec("sed -n 1p /usr/share/clash/new_version")
end

function check_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		check_version = check_version(),
		current_version = current_version(),
		new_version = new_version()

	})
end
function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		web = is_web(),
		clash = is_running(),
		localip = localip(),
		dash_port = dash_port(),
		current_version = current_version(),
		new_version = new_version(),
		dash_pass = dash_pass()

	})
end

