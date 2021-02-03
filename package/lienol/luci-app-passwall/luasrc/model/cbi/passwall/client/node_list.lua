local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname

m = Map(appname)

-- [[ Other Settings ]]--
s = m:section(TypedSection, "global_other")
s.anonymous = true

o = s:option(MultiValue, "nodes_ping", "Ping")
o:value("auto_ping", translate("Auto Ping"), translate("This will automatically ping the node for latency"))
o:value("tcping", translate("Tcping"), translate("This will use tcping replace ping detection of node"))

-- [[ Add the node via the link ]]--
s:append(Template(appname .. "/node_list/link_add_node"))

local nodes_ping = m:get("@global_other[0]", "nodes_ping") or ""
local nodes_display = m:get("@global_other[0]", "nodes_display") or ""

-- [[ Node List ]]--
s = m:section(TypedSection, "nodes")
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"
s.extedit = api.url("node_config", "%s")
function s.create(e, t)
    local uuid = api.gen_uuid()
    t = uuid
    TypedSection.create(e, t)
    luci.http.redirect(e.extedit:format(t))
end

function s.remove(e, t)
    s.map.proceed = true
    s.map:del(t)
    luci.http.redirect(api.url("node_list"))
end

s.sortable = true
-- 简洁模式
if true then
    o = s:option(DummyValue, "add_mode", "")
    o.cfgvalue = function(t, n)
        local v = Value.cfgvalue(t, n)
        if v and v ~= '' then
            local group = m:get(n, "group") or ""
            if group ~= "" then
                v = v .. " " .. group
            end
            return v
        else
            return ''
        end
    end
    o = s:option(DummyValue, "remarks", translate("Remarks"))
    o.rawhtml = true
    o.cfgvalue = function(t, n)
        local str = ""
        local is_sub = m:get(n, "is_sub") or ""
        local group = m:get(n, "group") or ""
        local remarks = m:get(n, "remarks") or ""
        local type = m:get(n, "type") or ""
        str = str .. string.format("<input type='hidden' id='cbid.%s.%s.type' value='%s'/>", appname, n, type)
        if type == "Xray" then
            local protocol = m:get(n, "protocol")
            if protocol == "_balancing" then
                protocol = "负载均衡"
            elseif protocol == "_shunt" then
                protocol = "分流"
            elseif protocol == "vmess" then
                protocol = "VMess"
            elseif protocol == "vless" then
                protocol = "VLESS"
            else
                protocol = protocol:gsub("^%l",string.upper)
            end
            type = type .. " " .. protocol
        end
        local address = m:get(n, "address") or ""
        local port = m:get(n, "port") or ""
        str = str .. translate(type) .. "：" .. remarks
        if address ~= "" and port ~= "" then
            if datatypes.ip6addr(address) then
                str = str .. string.format("（[%s]:%s）", address, port)
            else
                str = str .. string.format("（%s:%s）", address, port)
            end
            str = str .. string.format("<input type='hidden' id='cbid.%s.%s.address' value='%s'/>", appname, n, address)
            str = str .. string.format("<input type='hidden' id='cbid.%s.%s.port' value='%s'/>", appname, n, port)
        end
        return str
    end
end

---- Ping
o = s:option(DummyValue, "ping", translate("Latency"))
o.width = "8%"
o.rawhtml = true
o.cfgvalue = function(t, n)
    local result = "---"
    if not nodes_ping:find("auto_ping") then
        result = string.format('<span class="ping"><a href="javascript:void(0)" onclick="javascript:ping_node(\'%s\',this)">Ping</a></span>', n)
    else
        result = string.format('<span class="ping_value" cbiid="%s">---</span>', n)
    end
    return result
end

m:append(Template(appname .. "/node_list/node_list"))

return m
