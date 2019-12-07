local d = require "luci.dispatcher"
local fs = require "nixio.fs"
local sys = require "luci.sys"
local uci = require"luci.model.uci".cursor()
local api = require "luci.model.cbi.passwall.api.api"
local appname = "passwall"

m = Map(appname)

-- [[ Other Settings ]]--
s = m:section(TypedSection, "global_other")
s.anonymous = true

---- Auto Ping
o = s:option(Flag, "auto_ping", translate("Auto Ping"),
             translate("This will automatically ping the node for latency"))
o.default = 0

-- [[ Add the node via the link ]]--
s:append(Template("passwall/node_list/link_add_node"))

-- [[ Node List ]]--
s = m:section(TypedSection, "nodes")
s.anonymous = true
s.sortable = true
s.addremove = true
s.template = "cbi/tblsection"
s.extedit = d.build_url("admin", "vpn", "passwall", "node_config", "%s")
function s.create(e, t)
    local e = TypedSection.create(e, t)
    luci.http
        .redirect(d.build_url("admin", "vpn", "passwall", "node_config", e))
end

function s.remove(t, a)
    s.map.proceed = true
    s.map:del(a)
    luci.http.redirect(d.build_url("admin", "vpn", "passwall", "node_list"))
end

---- Remarks
o = s:option(DummyValue, "remarks", translate("Remarks"))

---- Add Mode
o = s:option(DummyValue, "add_mode", translate("Add Mode"))
o.cfgvalue = function(t, n)
    local v = Value.cfgvalue(t, n)
    if v and v ~= '' then
        return v
    else
        return '手动'
    end
    return str
end

---- Type
o = s:option(DummyValue, "type", translate("Type"))

---- Address
o = s:option(DummyValue, "address", translate("Address"))

---- Port
o = s:option(DummyValue, "port", translate("Port"))

---- Encrypt Method
--[[ o = s:option(DummyValue, "encrypt_method", translate("Encrypt Method"))
o.width = "15%"
o.cfgvalue = function(t, n)
    local str = "无"
    local type = api.uci_get_type_id(n, "type") or ""
    if type == "SSR" then
        return api.uci_get_type_id(n, "ssr_encrypt_method")
    elseif type == "SS" then
        return api.uci_get_type_id(n, "ss_encrypt_method")
    elseif type == "V2ray" then
        return api.uci_get_type_id(n, "v2ray_security")
    end
    return str
end--]]

---- Ping
o = s:option(DummyValue, "ping", translate("Ping"))
if api.uci_get_type("global_other", "auto_ping", "0") == "0" then
    o.template = "passwall/node_list/ping"
else
    o.template = "passwall/node_list/auto_ping"
end

---- Apply
o = s:option(DummyValue, "apply", translate("Apply"))
o.template = "passwall/node_list/apply"

m:append(Template("passwall/node_list/node_list"))

if luci.http.formvalue("cbi.apply") then
    luci.http.redirect(d.build_url("admin", "vpn", "passwall", "node_list"))
end

return m
