local m, s, o
local uci = luci.model.uci.cursor()
local servers = {}

local function has_bin(name)
	return luci.sys.call("command -v %s >/dev/null" %{name}) == 0
end

if has_bin("udpspeeder") then
    uci:foreach("speederv2", "servers", function(s)
        if s.server_port and s.listen_port then
            servers[#servers+1] = {name = s[".name"], alias = s.alias or "%s:%s" %{s.server_port, s.listen_port}}
        end
    end)

    m = Map("speederv2", "%s - %s" %{translate("speederv2-tunnel"), translate("Settings")})
    --Running Status
    m:append(Template("speederv2/status"))

    --General Settings
    s = m:section(NamedSection, "general", "general", translate("Running Management"))
    s.anonymous = true
    s.addremove = false

    o = s:option(DynamicList, "server", translate("Server"))
    o.template = "speederv2/dynamiclist"
    o:value("nil", translate("Disable"))
    for _, s in ipairs(servers) do o:value(s.name, s.alias) end
    o.default = "nil"
    o.rmempty = false

    o = s:option(ListValue, "daemon_user", translate("Run Daemon as User"))
    for u in luci.util.execi("cat /etc/passwd | cut -d ':' -f1") do o:value(u) end
    o.default = "root"
    o.rmempty = false
else
	return Map("speederv2", "%s - %s" %{translate("speederv2-tunnel"),
		translate("Settings")}, '<b style="color:red">speederv2-tunnel binary file(/usr/bin/speederv2) not found. </b>')
end

return m
