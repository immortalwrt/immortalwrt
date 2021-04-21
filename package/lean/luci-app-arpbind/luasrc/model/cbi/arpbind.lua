

local sys = require "luci.sys"
local ifaces = sys.net:devices()

m = Map("arpbind", translate("IP/MAC Binding"),
        translatef("ARP is used to convert a network address (e.g. an IPv4 address) to a physical address such as a MAC address.Here you can add some static ARP binding rules."))

s = m:section(TypedSection, "arpbind", translate("Rules"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

a = s:option(Value, "ipaddr", translate("IP Address"))
a.optional = false
a.datatype = "ipaddr"
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               a:value(entry.dest:string())
       end
end)

a = s:option(Value, "macaddr", translate("MAC Address"))
a.datatype = "macaddr"
a.optional = false
luci.ip.neighbors({family = 4}, function(neighbor)
	if neighbor.reachable then
		a:value(neighbor.mac, "%s (%s)" %{neighbor.mac, neighbor.dest:string()})
	end
end)

a = s:option(ListValue, "ifname", translate("Interface"))
for _, iface in ipairs(ifaces) do
	if iface ~= "lo" then
		a:value(iface)
	end
end
a.default = "br-lan"
a.rmempty = false

local e = luci.http.formvalue("cbi.apply")
if e then
    local IPAddr = {}
    local MACAddr = {}
    local IFName = {}
    local index = {}
    for key, val in pairs(luci.http.formvalue()) do
        local i
        if(string.find(key,"cbid.arpbind")) then
            i = string.sub((string.gsub(key, "cbid.arpbind.", "")), 1, string.find((string.gsub(key, "cbid.arpbind.", "")), ".", 1, true)-1)
            local flag = true
            for _, v in pairs(index) do
                if i == v then
                    flag = false
                end
            end
            if flag == true then table.insert(index,i) end
        end
        if(string.find(key,"ipaddr")) then
            IPAddr[i] = val
        elseif(string.find(key,"macaddr")) then
            MACAddr[i] = val
        elseif(string.find(key,"ifname")) then
            IFName[i] = val
        end
    end
    for k, v in pairs(index) do
        io.popen("ip neigh add "..IPAddr[v].." lladdr "..MACAddr[v].." nud permanent dev "..IFName[v].." || ip neigh change "..IPAddr[v].." lladdr "..MACAddr[v].." nud permanent dev "..IFName[v])
    end
end

return m


