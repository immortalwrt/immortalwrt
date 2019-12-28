local o = require "luci.dispatcher"
local fs = require "nixio.fs"
local sys = require "luci.sys"
local ipkg = require("luci.model.ipkg")
local uci = require"luci.model.uci".cursor()
local api = require "luci.model.cbi.passwall.api.api"
local appname = "passwall"

local function is_installed(e) return ipkg.installed(e) end

local function is_finded(e)
    return
        sys.exec("find /usr/*bin -iname " .. e .. " -type f") ~= "" and true or
            false
end

local n = {}
uci:foreach(appname, "nodes", function(e)
    if e.type and e.address and e.remarks then
        if e.use_kcp and e.use_kcp == "1" then
            n[e[".name"]] = "%s+%s：[%s] %s" %
                                {e.type, "Kcptun", e.remarks, e.address}
        else
            n[e[".name"]] = "%s：[%s] %s" % {e.type, e.remarks, e.address}
        end
    end
end)

local key_table = {}
for key, _ in pairs(n) do table.insert(key_table, key) end
table.sort(key_table)

m = Map(appname)
local status_use_big_icon = api.uci_get_type("global_other",
                                             "status_use_big_icon", 1)
if status_use_big_icon and status_use_big_icon == "1" then
    m:append(Template("passwall/global/status"))
else
    m:append(Template("passwall/global/status2"))
end

-- [[ Global Settings ]]--
s = m:section(TypedSection, "global", translate("Global Settings"),
              translate("If you can use it, very stable. If not, GG !!!"))
s.anonymous = true
s.addremove = false

---- TCP Node
local tcp_node_num = api.uci_get_type("global_other", "tcp_node_num", 1)
for i = 1, tcp_node_num, 1 do
    if i == 1 then
        o = s:option(ListValue, "tcp_node" .. i, translate("TCP Node"),
                     translate("For used to surf the Internet."))
    else
        o = s:option(ListValue, "tcp_node" .. i,
                     translate("TCP Node") .. " " .. i)
    end
    o:value("nil", translate("Close"))
    for _, key in pairs(key_table) do o:value(key, n[key]) end
end

---- UDP Node
local udp_node_num = api.uci_get_type("global_other", "udp_node_num", 1)
for i = 1, udp_node_num, 1 do
    if i == 1 then
        o = s:option(ListValue, "udp_node" .. i, translate("UDP Node"),
                     translate("For Game Mode or DNS resolution and more.") ..
                         translate("The selected server will not use Kcptun."))
        o:value("nil", translate("Close"))
        o:value("default", translate("Same as the tcp node"))
    else
        o = s:option(ListValue, "udp_node" .. i,
                     translate("UDP Node") .. " " .. i)
        o:value("nil", translate("Close"))
    end
    for _, key in pairs(key_table) do o:value(key, n[key]) end
end

---- Socks5 Node
local socks5_node_num = api.uci_get_type("global_other", "socks5_node_num", 1)
for i = 1, socks5_node_num, 1 do
    if i == 1 then
        o = s:option(ListValue, "socks5_node" .. i, translate("Socks5 Node"),
                     translate("The client can use the router's Socks5 proxy"))
    else
        o = s:option(ListValue, "socks5_node" .. i,
                     translate("Socks5 Node") .. " " .. i)
    end
    o:value("nil", translate("Close"))
    for _, key in pairs(key_table) do o:value(key, n[key]) end
end

---- DNS Forward Mode
o = s:option(ListValue, "dns_mode", translate("DNS Forward Mode"), translate(
                 "if you use no patterns are used, DNS of wan will be used by default as upstream of dnsmasq"))
o.rmempty = false
o:reset_values()
if is_finded("chinadns-ng") then o:value("chinadns-ng", "ChinaDNS-NG") end
if is_finded("dns2socks") then
    o:value("dns2socks", "dns2socks " .. translate("Need Socks5 server"))
end
if is_installed("pdnsd") or is_installed("pdnsd-alt") or is_finded("pdnsd") then
    o:value("pdnsd", "pdnsd")
end
o:value("local_7913", translate("Use local port 7913 as DNS"))
o:value("nonuse", translate("No patterns are used"))

---- DNS Forward
o = s:option(Value, "dns_forward", translate("DNS Forward Address"))
o.default = "8.8.4.4"
o:value("8.8.4.4", "8.8.4.4 (Google DNS)")
o:value("8.8.8.8", "8.8.8.8 (Google DNS)")
o:value("208.67.222.222", "208.67.222.222 (OpenDNS DNS)")
o:value("208.67.220.220", "208.67.220.220 (OpenDNS DNS)")
o:depends("dns_mode", "dns2socks")
o:depends("dns_mode", "pdnsd")

---- Use TCP Node Resolve DNS
o = s:option(Flag, "use_tcp_node_resolve_dns",
             translate("Use TCP Node Resolve DNS"),
             translate("If checked, DNS is resolved using the TCP node."))
o.default = 1
o:depends("dns_mode", "pdnsd")

---- upstreamm DNS Server for ChinaDNS-NG
o = s:option(ListValue, "up_chinadns_ng_mode",
             translate("upstreamm DNS Server for ChinaDNS-NG"), translate(
                 "Domestic DNS server in advanced Settings is used as domestic DNS by default"))
o.default = "208.67.222.222"
o:value("208.67.222.222", "208.67.222.222 (OpenDNS DNS)")
o:value("208.67.220.220", "208.67.220.220 (OpenDNS DNS)")
if is_finded("dns2socks") then
    o:value("dns2socks", "dns2socks " .. translate("Need Socks5 server"))
end
o:value("custom", translate("custom"))
o:depends("dns_mode", "chinadns-ng")

o = s:option(Value, "up_chinadns_ng_custom", translate("DNS Server"), translate(
                 "example: 127.0.0.1#5335<br>Need at least one,Other DNS services can be used as upstream, such as dns2socks."))
o.default = "208.67.222.222#443"
o:depends("up_chinadns_ng_mode", "custom")

---- Default Proxy Mode
o = s:option(ListValue, "proxy_mode",
             translate("Default") .. translate("Proxy Mode"))
o.default = "gfwlist"
o.rmempty = false
o:value("disable", translate("No Proxy"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("China WhiteList"))
o:value("gamemode", translate("Game Mode"))
o:value("returnhome", translate("Return Home"))

---- Localhost Proxy Mode
o = s:option(ListValue, "localhost_proxy_mode",
             translate("Localhost") .. translate("Proxy Mode"), translate(
                 "The server client can also use this rule to scientifically surf the Internet"))
o:value("default", translate("Default"))
-- o:value("global", translate("Global Proxy").."（"..translate("Danger").."）")
o:value("gfwlist", translate("GFW List"))
-- o:value("chnroute", translate("China WhiteList"))
o.default = "default"
o.rmempty = false

--[[
local apply = luci.http.formvalue("cbi.apply")
if apply then
os.execute("/etc/init.d/passwall restart")
end
--]]

return m
