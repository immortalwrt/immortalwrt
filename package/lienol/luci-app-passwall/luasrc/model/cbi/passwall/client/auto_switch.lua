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

o = s:option(ListValue, "tcp_main", "TCP " .. translate("Main node"))
for k, v in pairs(nodes_table) do
     o:value(v.id, v.remarks)
end
    
o = s:option(DynamicList, "tcp_node", "TCP " .. translate("List of backup nodes"))
for k, v in pairs(nodes_table) do
    o:value(v.id, v.remarks)
end

o = s:option(Flag, "restore_switch", "TCP " .. translate("Restore Switch"), translate("When detects main node is available, switch back to the main node."))

return m
