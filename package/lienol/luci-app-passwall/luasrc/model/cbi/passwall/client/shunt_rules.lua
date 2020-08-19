local d = require "luci.dispatcher"
local appname = "passwall"

m = Map(appname, "V2ray" .. translate("Shunt") .. translate("Rule"))
m.redirect = d.build_url("admin", "services", appname)

s = m:section(NamedSection, arg[1], "shunt_rules", "")
s.addremove = false
s.dynamic = false

remarks = s:option(Value, "remarks", translate("Remarks"))
remarks.default = arg[1]
remarks.rmempty = false

domain_list = s:option(TextValue, "domain_list", translate("Domain"))
domain_list.rows = 15
domain_list.wrap = "off"
domain_list.validate = function(self, value)
    local hosts= {}
    string.gsub(value, '[^' .. "\r\n" .. ']+', function(w) table.insert(hosts, w) end)
    for index, host in ipairs(hosts) do
        if not datatypes.hostname(host) then
            return nil, host .. " " .. translate("Not valid domain name, please re-enter!")
        end
    end
    return value
end

ip_list = s:option(TextValue, "ip_list", "IP")
ip_list.rows = 15
ip_list.wrap = "off"
ip_list.validate = function(self, value)
    local ipmasks= {}
    string.gsub(value, '[^' .. "\r\n" .. ']+', function(w) table.insert(ipmasks, w) end)
    for index, ipmask in ipairs(ipmasks) do
        if not datatypes.ipmask4(ipmask) then
            return nil, ipmask .. " " .. translate("Not valid IP format, please re-enter!")
        end
    end
    return value
end

return m
