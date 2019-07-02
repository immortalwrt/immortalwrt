module("luci.controller.clash", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/clash") then
		return
	end


	entry({"admin", "services", "clash"},alias("admin", "services", "clash", "client"), _("Clash"), 60).dependent = true
	entry({"admin", "services", "clash", "client"},cbi("clash/client"),_("Clash Client"), 10).leaf = true
	entry({"admin", "services", "clash", "dns"},cbi("clash/dns"),_("DNS Settings"), 20).leaf = true
	entry({"admin", "services", "clash", "config"},cbi("clash/config"),_("Server Config"), 30).leaf = true
	entry({"admin","services","clash","status"},call("action_status")).leaf=true
	entry({"admin", "services", "clash", "log"},cbi("clash/log"),_("Logs"), 40).leaf = true

	
end


local function is_running()
	return luci.sys.call("pidof clash >/dev/null") == 0
end

local function is_web()
	return luci.sys.call("pidof clash >/dev/null") == 0
end

local function is_bbr()
	return luci.sys.call("[ `cat /proc/sys/net/ipv4/tcp_congestion_control 2>/dev/null` = bbr ] 2>/dev/null") == 0
end

local function is_pdn()
	return luci.sys.call("pgrep dnscache >/dev/null") == 0
end

local function is_dns()
	return luci.sys.call("pgrep pdnsd >/dev/null") == 0
end

function action_status()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		web = is_web(),
		clash = is_running(),
		bbr = is_bbr(),
		pdnsd = is_pdn(),
		dnscache = is_dns()
	})
end
