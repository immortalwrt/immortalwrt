local sys = require "luci.sys"
local webadmin = require "luci.tools.webadmin"
local uci = require"luci.model.uci".cursor()
local api = require "luci.model.cbi.passwall.api.api"
local appname = "passwall"

local n = {}
uci:foreach(appname, "nodes", function(e)
    if e.type and e.address and e.remarks then
        if e.use_kcp and e.use_kcp == "1" then
            n[e[".name"]] = "%s+%s：[%s]" % {e.type, "Kcptun", e.remarks}
        else
            n[e[".name"]] = "%s：[%s]" % {e.type, e.remarks}
        end
    end
end)

local key_table = {}
for key, _ in pairs(n) do table.insert(key_table, key) end
table.sort(key_table)

m = Map("passwall")

-- [[ ACLs Settings ]]--
s = m:section(TypedSection, "acl_rule", translate("ACLs"), translate(
                  "ACLs is a tools which used to designate specific IP proxy mode, IP or MAC address can be entered."))
s.template = "cbi/tblsection"
s.sortable = true
s.anonymous = true
s.addremove = true

---- Enable
o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty = false

---- Remarks
o = s:option(Value, "remarks", translate("Remarks"))
o.rmempty = true

---- IP Address
o = s:option(Value, "ip", translate("IP"))
o.datatype = "ip4addr"
o.rmempty = true

local temp = {}
for index, n in ipairs(luci.ip.neighbors({family = 4})) do
    if n.dest then temp[index] = n.dest:string() end
end
local ips = {}
for _, key in pairs(temp) do table.insert(ips, key) end
table.sort(ips)

for index, key in pairs(ips) do o:value(key, temp[key]) end
-- webadmin.cbi_add_knownips(o)

---- MAC Address
o = s:option(Value, "mac", translate("MAC"))
o.rmempty = true
sys.net.mac_hints(function(e, t) o:value(e, "%s " % {e}) end)

---- TCP Node
local tcp_node_num = api.uci_get_type("global_other", "tcp_node_num", 1)
if tonumber(tcp_node_num) > 1 then
    o = s:option(ListValue, "tcp_node", translate("TCP Node"))
    for i = 1, tcp_node_num, 1 do o:value(i, "TCP_" .. i) end
end

---- UDP Node
local udp_node_num = api.uci_get_type("global_other", "udp_node_num", 1)
if tonumber(udp_node_num) > 1 then
    o = s:option(ListValue, "udp_node", translate("UDP Node"))
    for i = 1, udp_node_num, 1 do o:value(i, "UDP_" .. i) end
end

---- Proxy Mode
o = s:option(ListValue, "proxy_mode", translate("Proxy Mode"))
o.default = "default"
o.rmempty = false
o:value("default", translate("Default"))
o:value("disable", translate("No Proxy"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("China WhiteList"))
-- o:value("gamemode", translate("Game Mode"))
o:value("returnhome", translate("Return Home"))

---- TCP No Redir Ports
o = s:option(Value, "tcp_no_redir_ports", translate("TCP No Redir Ports"))
o.default = "default"
o:value("disable", translate("No patterns are used"))
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))

---- UDP No Redir Ports
o = s:option(Value, "udp_no_redir_ports", translate("UDP No Redir Ports"))
o.default = "default"
o:value("disable", translate("No patterns are used"))
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))

---- TCP Redir Ports
o = s:option(Value, "tcp_redir_ports", translate("TCP Redir Ports"))
o.default = "default"
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))
o:value("80,443", "80,443")
o:value("80:", "80 " .. translate("or more"))
o:value(":443", "443 " .. translate("or less"))

---- UDP Redir Ports
o = s:option(Value, "udp_redir_ports", translate("UDP Redir Ports"))
o.default = "default"
o:value("default", translate("Default"))
o:value("1:65535", translate("All"))
o:value("53", "53")

return m
