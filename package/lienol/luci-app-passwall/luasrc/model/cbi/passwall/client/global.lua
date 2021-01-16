local d = require "luci.dispatcher"
local uci = require"luci.model.uci".cursor()
local api = require "luci.model.cbi.passwall.api.api"
local appname = "passwall"

local nodes_table = {}
for k, e in ipairs(api.get_valid_nodes()) do
    nodes_table[#nodes_table + 1] = {
        id = e[".name"],
        remarks = e.remarks_name
    }
end

local socks_table = {}
uci:foreach(appname, "socks", function(s)
    if s.enabled == "1" and s.node then
        local id, remarks
        local same, i = s.node:match("^(tcp)([1-9])$")
        if same then
            remarks = translatef("Same as the tcp %s node", i)
        else
            for k, n in pairs(nodes_table) do
                if (s.node == n.id) then
                    remarks = n.remarks; break
                end
            end
        end
        id = "0.0.0.0" .. ":" .. s.port
        socks_table[#socks_table + 1] = {
            id = id,
            remarks = id .. " - " .. (remarks or translate("Misconfigured"))
        }
    end
end)

local doh_validate = function(self, value, t)
    if value ~= "" then
        local flag = 0
        local util = require "luci.util"
        local val = util.split(value, ",")
        local url = val[1]
        val[1] = nil
        for i = 1, #val do
            local v = val[i]
            if v then
                if not datatypes.ipmask4(v) then
                    flag = 1
                end
            end
        end
        if flag == 0 then
            return value
        end
    end
    return nil, translate("DoH request address") .. " " .. translate("Format must be:") .. " URL,IP"
end

m = Map(appname)
local status = m:get("@global_other[0]", "status") or ""
if status:find("big_icon") then
    m:append(Template(appname .. "/global/status"))
else
    m:append(Template(appname .. "/global/status2"))
end

s = m:section(TypedSection, "global")
s.anonymous = true
s.addremove = false

s:tab("Main", translate("Main"))

-- [[ Global Settings ]]--
o = s:taboption("Main", Flag, "enabled", translate("Main switch"))
o.rmempty = false

---- TCP Node
o = s:taboption("Main", ListValue, "tcp_node", translate("TCP Node"))
o.title = translate("TCP Node")
o.description = ""
--o.description = translate("For proxy specific list.")
--o.description = o.description .. "<br />"
local current_node = luci.sys.exec(string.format("[ -f '/var/etc/%s/id/TCP' ] && echo -n $(cat /var/etc/%s/id/TCP)", appname, appname))
if current_node and current_node ~= "" and current_node ~= "nil" then
    local n = uci:get_all(appname, current_node)
    if n then
        if tonumber(m:get("@auto_switch[0]", "enable") or 0) == 1 then
            local remarks = api.get_full_node_remarks(n)
            local url = d.build_url("admin", "services", appname, "node_config", current_node)
            o.description = o.description .. translatef("Current node: %s", string.format('<a href="%s">%s</a>', url, remarks)) .. "<br />"
        end
        if n.protocol and n.protocol == "_shunt" then
            uci:foreach(appname, "shunt_rules", function(e)
                local id = e[".name"]
                local remarks = translate(e.remarks)
                if n[id] and n[id] ~= "nil" then
                    local url = d.build_url("admin", "services", appname, "node_config", n[id])
                    local r = api.get_full_node_remarks(uci:get_all(appname, n[id]))
                    o.description = o.description .. remarks .. "：" .. string.format('<a href="%s">%s</a>', url, r) .. "<br />"
                end
            end)
            local id = "default_node"
            local remarks = translate("Default")
            if n[id] and n[id] ~= "nil" then
                local url = d.build_url("admin", "services", appname, "node_config", n[id])
                local r = api.get_full_node_remarks(uci:get_all(appname, n[id]))
                o.description = o.description .. remarks .. "：" .. string.format('<a href="%s">%s</a>', url, r) .. "<br />"
            end
        end
    end
end
o:value("nil", translate("Close"))
for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end

o = s:taboption("Main", ListValue, "udp_node", translate("UDP Node"))
o:value("nil", translate("Close"))
o.title = translate("UDP Node")
--o.description = translate("For proxy game network, DNS hijack etc.") .. "<br />" .. translate("The selected server will not use Kcptun.")
o:value("tcp_", translate("Same as the tcp node"))
--o:value("tcp", translate("Same as the tcp node"))
--o:value("tcp_", translate("Same as the tcp node") .. "（" .. translate("New process") .. "）")
for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end

s:tab("DNS", translate("DNS"))

if api.is_finded("chinadns-ng") then
    o = s:taboption("DNS", Flag, "chinadns_ng", translate("Use ChinaDNS-NG"), translate("When checked, forced to be set to dnsmasq upstream DNS."))
    o.default = "0"

    o = s:taboption("DNS", Flag, "fair_mode", translate("ChinaDNS-NG Fair Mode"))
    o.default = "1"
    o:depends("chinadns_ng", "1")
end

o = s:taboption("DNS", Value, "up_china_dns", translate("Local DNS") .. "(UDP)")
o.description = translate("IP:Port mode acceptable, multi value split with english comma.") .. "<br />" .. translate("When the selection is not the default, this DNS is forced to be set to dnsmasq upstream DNS.")
o.default = "default"
o:value("default", translate("Default"))
if api.is_finded("xray") then
    o:value("xray_doh", "Xray DNS(DoH)")
end
o:value("223.5.5.5", "223.5.5.5 (" .. translate("Ali") .. "DNS)")
o:value("223.6.6.6", "223.6.6.6 (" .. translate("Ali") .. "DNS)")
o:value("114.114.114.114", "114.114.114.114 (114DNS)")
o:value("114.114.115.115", "114.114.115.115 (114DNS)")
o:value("119.29.29.29", "119.29.29.29 (DNSPOD DNS)")
o:value("182.254.116.116", "182.254.116.116 (DNSPOD DNS)")
o:value("1.2.4.8", "1.2.4.8 (CNNIC DNS)")
o:value("210.2.4.8", "210.2.4.8 (CNNIC DNS)")
o:value("180.76.76.76", "180.76.76.76 (" .. translate("Baidu") .. "DNS)")

---- DoH
o = s:taboption("DNS", Value, "up_china_dns_doh", translate("DoH request address"))
o:value("https://dns.alidns.com/dns-query,223.5.5.5", "AliDNS")
o:value("https://doh.pub/dns-query,119.29.29.29", "DNSPod")
o.default = "https://dns.alidns.com/dns-query,223.5.5.5"
o.validate = doh_validate
o:depends("up_china_dns", "xray_doh")

---- DNS Forward Mode
o = s:taboption("DNS", ListValue, "dns_mode", translate("Filter Mode"))
o.rmempty = false
o:reset_values()
if api.is_finded("pdnsd") then
    o:value("pdnsd", "pdnsd " .. translatef("Requery DNS By %s", translate("TCP Node")))
end
if api.is_finded("dns2socks") then
    o:value("dns2socks", "dns2socks")
end
if api.is_finded("xray") then
    o:value("xray_doh", "Xray DNS(DoH)")
end
o:value("udp", translatef("Requery DNS By %s", translate("UDP Node")))
o:value("nonuse", translate("No Filter"))
o:value("custom", translate("Custom DNS"))

---- Custom DNS
o = s:taboption("DNS", Value, "custom_dns", translate("Custom DNS"))
o.default = "127.0.0.1#5353"
o.validate = function(self, value, t)
    local v = string.gsub(value, "#", ":")
    if not datatypes.ipaddrport(v) then
        return nil, translate("Custom DNS") .. " " .. translate("Not valid IP format, please re-enter!")
    end
    return value
end
o:depends({dns_mode = "custom"})

o = s:taboption("DNS", ListValue, "up_trust_doh_dns", translate("Resolver For The List Proxied"))
o:value("tcp", translatef("Requery DNS By %s", translate("TCP Node")))
o:value("socks", translatef("Requery DNS By %s", translate("Socks Node")))
o:depends("dns_mode", "xray_doh")

o = s:taboption("DNS", Value, "socks_server", translate("Socks Server"), translate("Make sure socks service is available on this address."))
for k, v in pairs(socks_table) do o:value(v.id, v.remarks) end
o.validate = function(self, value, t)
    if not datatypes.ipaddrport(value) then
        return nil, translate("Socks Server") .. " " .. translate("Not valid IP format, please re-enter!")
    end
    return value
end
o:depends({dns_mode = "dns2socks"})
o:depends({dns_mode = "xray_doh", up_trust_doh_dns = "socks"})

---- DoH
o = s:taboption("DNS", Value, "up_trust_doh", translate("DoH request address"))
o:value("https://dns.adguard.com/dns-query,176.103.130.130", "AdGuard")
o:value("https://cloudflare-dns.com/dns-query,1.1.1.1", "Cloudflare")
o:value("https://security.cloudflare-dns.com/dns-query,1.1.1.2", "Cloudflare-Security")
o:value("https://doh.opendns.com/dns-query,208.67.222.222", "OpenDNS")
o:value("https://dns.google/dns-query,8.8.8.8", "Google")
o:value("https://doh.libredns.gr/dns-query,116.202.176.26", "LibreDNS")
o:value("https://doh.libredns.gr/ads,116.202.176.26", "LibreDNS (No Ads)")
o:value("https://dns.quad9.net/dns-query,9.9.9.9", "Quad9-Recommended")
o.default = "https://dns.google/dns-query,8.8.8.8"
o.validate = doh_validate
o:depends({dns_mode = "xray_doh"})

---- DNS Forward
o = s:taboption("DNS", Value, "dns_forward", translate("Remote DNS"))
--o.description = translate("IP:Port mode acceptable, multi value split with english comma.") .. " " .. translate("If you use dns2socks, only the first one is valid.")
o.default = "8.8.8.8"
o:value("8.8.8.8", "8.8.8.8 (Google DNS)")
o:value("8.8.4.4", "8.8.4.4 (Google DNS)")
o:value("208.67.222.222", "208.67.222.222 (Open DNS)")
o:value("208.67.220.220", "208.67.220.220 (Open DNS)")
o:depends({dns_mode = "dns2socks"})
o:depends({dns_mode = "pdnsd"})
o:depends({dns_mode = "udp"})

--[[
o = s:taboption("DNS", Flag, "dns_cache", translate("Cache Resolved"))
o.default = "1"
o:depends({dns_mode = "dns2socks"})
o:depends({dns_mode = "pdnsd"})
]]--

o = s:taboption("DNS", Button, "clear_ipset", translate("Clear IPSET"), translate("Try this feature if the rule modification does not take effect."))
o.inputstyle = "remove"
function o.write(e, e)
    luci.sys.call("/etc/init.d/" .. appname .. " stop && /usr/share/" .. appname .. "/iptables.sh flush_ipset && /etc/init.d/" .. appname .. " restart")
end

s:tab("Proxy", translate("Mode"))

---- TCP Default Proxy Mode
o = s:taboption("Proxy", ListValue, "tcp_proxy_mode", "TCP" .. translate("Default") .. translate("Proxy Mode"))
-- o.description = translate("If not available, try clearing the cache.")
o.default = "chnroute"
o.rmempty = false
o:value("disable", translate("No Proxy"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("Not China List"))
o:value("returnhome", translate("China List"))

---- UDP Default Proxy Mode
o = s:taboption("Proxy", ListValue, "udp_proxy_mode", "UDP" .. translate("Default") .. translate("Proxy Mode"))
o.default = "chnroute"
o.rmempty = false
o:value("disable", translate("No Proxy"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("Game Mode"))
o:value("returnhome", translate("China List"))

---- Localhost TCP Proxy Mode
o = s:taboption("Proxy", ListValue, "localhost_tcp_proxy_mode", translate("Router Localhost") .. "TCP" .. translate("Proxy Mode"))
-- o.description = translate("The server client can also use this rule to scientifically surf the Internet.")
o:value("default", translate("Default"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("Not China List"))
o:value("returnhome", translate("China List"))
o.default = "default"
o.rmempty = false

---- Localhost UDP Proxy Mode
o = s:taboption("Proxy", ListValue, "localhost_udp_proxy_mode", translate("Router Localhost") .. "UDP" .. translate("Proxy Mode"))
o:value("default", translate("Default"))
o:value("global", translate("Global Proxy"))
o:value("gfwlist", translate("GFW List"))
o:value("chnroute", translate("Game Mode"))
o:value("returnhome", translate("China List"))
o:value("disable", translate("No Proxy"))
o.default = "default"
o.rmempty = false

s:tab("log", translate("Log"))
o = s:taboption("log", Flag, "close_log_tcp", translate("Close") .. translate("Log") .. " " .. translate("TCP Node"))
o.rmempty = false

o = s:taboption("log", Flag, "close_log_udp", translate("Close") .. translate("Log") .. " " .. translate("UDP Node"))
o.rmempty = false

loglevel = s:taboption("log", ListValue, "loglevel", "X/V2ray" .. translate("Log Level"))
loglevel.default = "warning"
loglevel:value("debug")
loglevel:value("info")
loglevel:value("warning")
loglevel:value("error")

trojan_loglevel = s:taboption("log", ListValue, "trojan_loglevel", "Trojan" ..  translate("Log Level"))
trojan_loglevel.default = "2"
trojan_loglevel:value("0", "all")
trojan_loglevel:value("1", "info")
trojan_loglevel:value("2", "warn")
trojan_loglevel:value("3", "error")
trojan_loglevel:value("4", "fatal")

s:tab("tips", translate("Tips"))

o = s:taboption("tips", DummyValue, "")
o.template = appname .. "/global/tips"

-- [[ Socks Server ]]--
o = s:taboption("Main", Flag, "socks_enabled", "Socks" .. translate("Main switch"))
o.rmempty = false

s = m:section(TypedSection, "socks", translate("Socks Config"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"
function s.create(e, t)
    TypedSection.create(e, api.gen_uuid())
end

o = s:option(DummyValue, "status", translate("Status"))
o.rawhtml = true
o.cfgvalue = function(t, n)
    return string.format('<div class="_status" socks_id="%s"></div>', n)
end

---- Enable
o = s:option(Flag, "enabled", translate("Enable"))
o.default = 1
o.rmempty = false

o = s:option(ListValue, "node", translate("Socks Node"))
o:value("tcp", translate("Same as the tcp node"))
for k, v in pairs(nodes_table) do o:value(v.id, v.remarks) end

o = s:option(Value, "port", "Socks" .. translate("Listen Port"))
o.default = 9050
o.datatype = "port"
o.rmempty = false

if api.is_finded("xray") or api.is_finded("v2ray") then
    o = s:option(Value, "http_port", "HTTP" .. translate("Listen Port") .. " " .. translate("0 is not use"))
    o.default = 0
    o.datatype = "port"
end

m:append(Template(appname .. "/global/footer"))

--[[
local apply = luci.http.formvalue("cbi.apply")
if apply then
os.execute("/etc/init.d/" .. appname .." restart")
end
--]]

return m
