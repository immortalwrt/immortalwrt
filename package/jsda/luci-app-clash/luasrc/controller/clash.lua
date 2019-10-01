module("luci.controller.clash", package.seeall)

function index()

	if not nixio.fs.access("/etc/config/clash") then
		return
	end

	entry({"admin", "services", "clash"},alias("admin", "services", "clash", "overview"), _("Clash"), 10).dependent = false
	entry({"admin", "services", "clash", "overview"},cbi("clash/overview"),_("Overview"), 10).leaf = true
	entry({"admin", "services", "clash", "client"},cbi("clash/client"),_("Client"), 20).leaf = true
	entry({"admin", "services", "clash", "servers"},cbi("clash/servers"),_("Servers"), 30).leaf = true
        entry({"admin", "services", "clash", "servers-config"},cbi("clash/servers-config"), nil).leaf = true
        entry({"admin", "services", "clash", "groups"},cbi("clash/groups"), nil).leaf = true

	entry({"admin", "services", "clash", "settings"}, firstchild(),_("Settings"), 40)
	entry({"admin", "services", "clash", "settings", "port"},cbi("clash/port"),_("Proxy Ports"), 40).leaf = true
	entry({"admin", "services", "clash", "settings", "dns"},cbi("clash/dns"),_("DNS Settings"), 50).leaf = true
	entry({"admin", "services", "clash", "settings", "list"},cbi("clash/list"),_("Custom List"), 60).leaf = true
	entry({"admin", "services", "clash", "settings", "access"},cbi("clash/access"),_("Access Control"), 70).leaf = true
			
	entry({"admin", "services", "clash", "config"},firstchild(),_("Config"), 80)
	entry({"admin", "services", "clash", "config", "actconfig"},cbi("clash/actconfig"),_("Config In Use"), 90).leaf = true
	entry({"admin", "services", "clash", "config", "subconfig"},cbi("clash/subconfig"),_("Subscribe Config"), 100).leaf = true
	entry({"admin", "services", "clash", "config", "upconfig"},cbi("clash/upconfig"),_("Uploaded Config"), 110).leaf = true
	entry({"admin", "services", "clash", "config", "cusconfig"},cbi("clash/cusconfig"),_("Custom Config"), 120).leaf = true
	
	entry({"admin","services","clash","status"},call("action_status")).leaf=true
	entry({"admin", "services", "clash", "log"},cbi("clash/log"),_("Log"), 130).leaf = true
	entry({"admin", "services", "clash", "update"},cbi("clash/update"),_("Update"), 140).leaf = true
	entry({"admin","services","clash","check_status"},call("check_status")).leaf=true
	entry({"admin", "services", "clash", "ping"}, call("act_ping")).leaf=true

	
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
	return luci.sys.exec("sh /usr/share/clash/check_luci_version.sh")
end

local function check_core()
	return luci.sys.exec("sh /usr/share/clash/check_core_version.sh")
end

local function current_version()
	return luci.sys.exec("sed -n 1p /usr/share/clash/luci_version")
end

local function new_version()
	return luci.sys.exec("sed -n 1p /usr/share/clash/new_luci_version")
end

local function new_core_version()
	return luci.sys.exec("sed -n 1p /usr/share/clash/new_core_version")
end

local function e_mode()
	return luci.sys.exec("egrep '^ {0,}enhanced-mode' /etc/clash/config.yaml |grep enhanced-mode: |awk -F ': ' '{print $2}'")
end


local function clash_core()
	if nixio.fs.access("/usr/share/clash/core_version") then
		return luci.sys.exec("sed -n 1p /usr/share/clash/core_version")
	else
		return "0"
	end
end

function check_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		check_version = check_version(),
		check_core = check_core(),
		current_version = current_version(),
		new_version = new_version(),
		clash_core = clash_core(),
		new_core_version = new_core_version()
		

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
		clash_core = clash_core(),
		dash_pass = dash_pass(),
		e_mode = e_mode()

	})
end

function act_ping()
	local e={}
	e.index=luci.http.formvalue("index")
	e.ping=luci.sys.exec("ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'"%luci.http.formvalue("domain"))
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end


