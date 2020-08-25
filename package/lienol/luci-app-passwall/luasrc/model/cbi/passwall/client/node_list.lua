local d = require "luci.dispatcher"
local _api = require "luci.model.cbi.passwall.api.api"
local appname = "passwall"

m = Map(appname)

-- [[ Other Settings ]]--
s = m:section(TypedSection, "global_other")
s.anonymous = true

o = s:option(MultiValue, "nodes_ping", "Ping")
o:value("auto_ping", translate("Auto Ping"), translate("This will automatically ping the node for latency"))
o:value("tcping", translate("Tcping"), translate("This will use tcping replace ping detection of node"))

o = s:option(MultiValue, "nodes_display", translate("Operation"))
o:value("compact_display_nodes", translate("Concise display nodes"))
o:value("show_add_mode", translate("Show Add Mode"))
o:value("show_group", translate("Show Group"))

-- [[ Add the node via the link ]]--
s:append(Template(appname .. "/node_list/link_add_node"))

local nodes_ping = m:get("@global_other[0]", "nodes_ping") or ""
local nodes_display = m:get("@global_other[0]", "nodes_display") or ""

-- [[ Node List ]]--
s = m:section(TypedSection, "nodes")
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"
s.extedit = d.build_url("admin", "services", appname, "node_config", "%s")
function s.create(e, t)
    local uuid = _api.gen_uuid()
    t = uuid
    TypedSection.create(e, t)
    luci.http.redirect(e.extedit:format(t))
end

function s.remove(e, t)
    s.map.proceed = true
    s.map:del(t)
    luci.http.redirect(d.build_url("admin", "services", appname, "node_list"))
end

if nodes_display:find("show_group") then
    show_group = s:option(DummyValue, "group", translate("Group"))
    show_group.cfgvalue = function(t, n)
        local group = m:get(n, "group") or "无"
        return group ~= "" and group or "无"
    end
end

s.sortable = true
-- 简洁模式
if nodes_display:find("compact_display_nodes") then
    if show_group then show_group.width = "25%" end
    o = s:option(DummyValue, "remarks", translate("Remarks"))
    o.cfgvalue = function(t, n)
        local str = ""
        local is_sub = m:get(n, "is_sub") or ""
        local group = m:get(n, "group") or ""
        local remarks = m:get(n, "remarks") or ""
        local type = m:get(n, "type") or ""
        if type == "V2ray" then
            local protocol = m:get(n, "protocol")
            if protocol == "_balancing" then
                type = type .. " 负载均衡"
            elseif protocol == "_shunt" then
                type = type .. " 分流"
            end
        end
        local address = m:get(n, "address") or ""
        local port = m:get(n, "port") or ""
        str = str .. translate(type) .. "：" .. remarks
        if address ~= "" and port ~= "" then
            local s = " （" .. address .. ":" .. port .. "）"
            str = str .. s
        end
        return str
    end
else
    ---- Add Mode
    if nodes_display:find("show_add_mode") then
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
    end

    ---- Type
    o = s:option(DummyValue, "type", translate("Type"))
    o.cfgvalue = function(t, n)
        local result = ""
        local v = Value.cfgvalue(t, n)
        if v then
            result = translate(v)
            if v == "V2ray" then
                local protocol = m:get(n, "protocol")
                if protocol == "_balancing" then
                    result = result .. " 负载均衡"
                elseif protocol == "_shunt" then
                    result = result .. " 分流"
                end
            end
        end
        return result
    end

    ---- Remarks
    o = s:option(DummyValue, "remarks", translate("Remarks"))

    ---- Address
    o = s:option(DummyValue, "address", translate("Address"))
    o.cfgvalue = function(t, n)
        return Value.cfgvalue(t, n) or "---"
    end

    ---- Port
    o = s:option(DummyValue, "port", translate("Port"))
    o.cfgvalue = function(t, n)
        return Value.cfgvalue(t, n) or "---"
    end
end

---- Ping
o = s:option(DummyValue, "ping", translate("Latency"))
o.width = "6%"
if not nodes_ping:find("auto_ping") then
    o.template = appname .. "/node_list/ping"
else
    o.template = appname .. "/node_list/auto_ping"
end
--[[
o.cfgvalue = function(t, n)
    local type = m:get(n, "type") or ""
    if type == "V2ray" then
        local protocol = m:get(n, "protocol","")
        if protocol == "_balancing" or protocol == "_shunt" then
            return "---"
        end
    end
end
--]]

m:append(Template(appname .. "/node_list/node_list"))

return m
