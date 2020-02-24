local o = require "luci.dispatcher"
local fs = require "nixio.fs"
local sys = require "luci.sys"
local cursor = luci.model.uci.cursor()
local appname = "ssr_python_pro_server"
local a, t, e

a = Map(appname, translate("ShadowsocksR Python Server"))

t = a:section(TypedSection, "global", translate("Global Settings"))
t.anonymous = true
t.addremove = false

e = t:option(DummyValue, "status", translate("Current Condition"))
e.template = appname .. "/status"
e.value = translate("Collecting data...")

e = t:option(Flag, "enable", translate("Enable"))
e.rmempty = false

e =
    t:option(Flag, "auto_clear_transfer", translate("Enable Auto Clear Traffic"))
e.default = 0
e.rmempty = false

e = t:option(Value, "auto_clear_transfer_time",
             translate("Clear Traffic Time Interval"),
             translate("*,*,*,*,* is Min Hour Day Mon Week"))
e.default = "0,2,1,*,*"
e:depends("auto_clear_transfer", 1)

e = t:option(Button, "clear_transfer", translate("Clear All Users Traffic"))
e.inputstyle = "remove"
function e.write(t, section)
    local url = luci.dispatcher.build_url("admin", "vpn",
                                          "ssr_python_pro_server",
                                          "clear_traffic_all_users")
    e.description = "<script>if (confirm('确认吗？')==true){XHR.get('" ..
                        url ..
                        "',null,function(x,result){window.location.replace(window.location.href)})}</script>"
end

t = a:section(TypedSection, "user", translate("Users Manager"))
t.anonymous = true
t.addremove = true
t.template = "cbi/tblsection"
t.extedit = o.build_url("admin", "vpn", appname, "config", "%s")
function t.create(e, t)
    local e = TypedSection.create(e, t)
    luci.http.redirect(o.build_url("admin", "vpn", appname, "config", e))
end

function t.remove(t, a)
    t.map.proceed = true
    t.map:del(a)
    luci.http.redirect(o.build_url("admin", "vpn", appname))
end

e = t:option(Flag, "enable", translate("Enable"))
e.width = "5%"
e.rmempty = false

e = t:option(DummyValue, "status", translate("Status"))
e.template = "ssr_python_pro_server/users_status"
e.width = "5%"

e = t:option(DummyValue, "remarks", translate("Remarks"))
e.width = "10%"

e = t:option(DummyValue, "port", translate("Port"))
e.width = "10%"

e = t:option(DummyValue, "forbidden_port", translate("Forbidden Port"))
e.width = "10%"
e.cfgvalue = function(t, n)
    local str = translate("Null")
    local forbidden_port = a.uci:get(appname, n, "forbidden_port")
    if forbidden_port then str = forbidden_port end
    return str
end

e = t:option(DummyValue, "device_limit", translate("Device Limit"))
e.width = "10%"

e =
    t:option(DummyValue, "speed_limit_per_con", translate("Speed Limit Per Con"))
e.width = "10%"
e.cfgvalue = function(t, section)
    local str = translate("No Speed Limit")
    local speed_limit_per_con = a.uci:get(appname, section,
                                          "speed_limit_per_con")
    if speed_limit_per_con and tonumber(speed_limit_per_con) > 0 then
        speed_limit_per_con = tonumber(speed_limit_per_con)
        if speed_limit_per_con < 1024 then
            str = speed_limit_per_con .. "Kb/s"
        elseif speed_limit_per_con < 1024 * 1024 then
            str = string.format("%0.2f", speed_limit_per_con / 1024) .. "Mb/s"
        end
    end
    return str
end

e = t:option(DummyValue, "speed_limit_per_user",
             translate("Speed Limit Per User"))
e.width = "10%"
e.cfgvalue = function(t, section)
    local str = translate("No Speed Limit")
    local speed_limit_per_user = a.uci:get(appname, section,
                                           "speed_limit_per_user")
    if speed_limit_per_user and tonumber(speed_limit_per_user) > 0 then
        speed_limit_per_user = tonumber(speed_limit_per_user)
        if speed_limit_per_user < 1024 then
            str = speed_limit_per_user .. "Kb/s"
        elseif speed_limit_per_user < 1024 * 1024 then
            str = string.format("%0.2f", speed_limit_per_user / 1024) .. "Mb/s"
        end
    end
    return str
end

e = t:option(DummyValue, "transfer_enable", translate("Available Total Flow"))
e.width = "10%"
e.cfgvalue = function(t, section)
    local str = translate("Infinite")
    local transfer_enable = a.uci:get(appname, section, "transfer_enable")
    if transfer_enable and tonumber(transfer_enable) > 0 then
        str = transfer_enable .. "G"
    end
    return str
end

--[[e=t:option(DummyValue,"u",translate("Used Upload Traffic"))
e.width="10%"
e.cfgvalue=function(t,section)
	local result = translate("Null")
	local u_str = luci.sys.exec("cd /usr/share/ssr_python_pro_server && ./mujson_mgr.py -l -I "..section.." | sed -n 10p"):gsub("^%s*(.-)%s*$", "%1")
	local u = luci.sys.exec("echo "..u_str.." | awk '{print $3}'"):gsub("^%s*(.-)%s*$", "%1")
	if u == "" then u = 0 end
	local unit = luci.sys.exec("echo "..u_str.." | awk '{print $4}'"):gsub("^%s*(.-)%s*$", "%1")
	result = string.format("%0.2f",u)..unit
	return result
end

e=t:option(DummyValue,"d",translate("Used Download Traffic"))
e.width="10%"
e.cfgvalue=function(t,section)
	local result = translate("Null")
	local d_str = luci.sys.exec("cd /usr/share/ssr_python_pro_server && ./mujson_mgr.py -l -I "..section.." | sed -n 11p"):gsub("^%s*(.-)%s*$", "%1")
	local d = luci.sys.exec("echo "..d_str.." | awk '{print $3}'"):gsub("^%s*(.-)%s*$", "%1")
	if d == "" then d = 0 end
	local unit = luci.sys.exec("echo "..d_str.." | awk '{print $4}'"):gsub("^%s*(.-)%s*$", "%1")
	result = string.format("%0.2f",d)..unit
	return result
end]] --

e = t:option(DummyValue, "used_total_traffic", translate("Used Total Traffic"))
e.width = "10%"
e.template = appname .. "/users_total_traffic"
--[[e.cfgvalue=function(t,section)
	local result = translate("Null")
	local total_traffic_str = luci.sys.exec("cd /usr/share/ssr_python_pro_server && ./mujson_mgr.py -l -I "..section.." | sed -n 19p"):gsub("^%s*(.-)%s*$", "%1")
	local total_traffic = luci.sys.exec("echo "..total_traffic_str.." | awk '{print $3}'"):gsub("^%s*(.-)%s*$", "%1")
	if total_traffic == "" then total_traffic = 0 end
	local unit = luci.sys.exec("echo "..total_traffic_str.." | awk '{print $4}'"):gsub("^%s*(.-)%s*$", "%1")
	result = string.format("%0.2f",total_traffic)..unit
	return result
end]] --

e = t:option(DummyValue, "ssr_link", translate("SSR Link"))
e.width = "10%"
e.template = appname .. "/users_link"

e = t:option(Button, "clear_transfer", translate("Clear Traffic"))
e.inputstyle = "remove"
function e.write(t, section)
    local url = luci.dispatcher.build_url("admin", "vpn",
                                          "ssr_python_pro_server",
                                          "clear_traffic")
    e.description = "<script>if (confirm('确认吗？')==true){XHR.get('" ..
                        url .. "',{id:'" .. section ..
                        "'},function(x,result){window.location.replace(window.location.href)})}</script>"
end

a:append(Template(appname .. "/ssr_python"))

return a
