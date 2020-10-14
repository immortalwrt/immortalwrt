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

m = Map(appname)

-- [[ Auto Switch Settings ]]--
s = m:section(TypedSection, "auto_switch")
s.anonymous = true

---- Enable
o = s:option(Flag, "enable", translate("Enable"))
o.default = 0
o.rmempty = false

---- Testing Time
o = s:option(Value, "testing_time", translate("How often is a diagnosis made"), translate("Units:minutes"))
o.default = "3"

---- TCP Node
-- local tcp_node_num = tonumber(m:get("@global_other[0]", "tcp_node_num") or 1)
-- 暂时只支持TCP1
local tcp_node_num = 1
for i = 1, tcp_node_num, 1 do
    o = s:option(ListValue, "tcp_main" .. i, "TCP " .. i .. " " .. translate("Main node"))
    for k, v in pairs(nodes_table) do
        o:value(v.id, v.remarks)
    end
    
    o = s:option(DynamicList, "tcp_node" .. i, "TCP " .. i .. " " .. translate("List of backup nodes"))
    for k, v in pairs(nodes_table) do
        o:value(v.id, v.remarks)
    end
end

return m
