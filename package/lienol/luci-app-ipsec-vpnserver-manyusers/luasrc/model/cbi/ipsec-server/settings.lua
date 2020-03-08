local s = require "luci.sys"
local net = require"luci.model.network".init()
local ifaces = s.net:devices()
local m, s, o
mp = Map("ipsec", translate("IPSec VPN Server"))
mp.description = translate(
                     "IPSec VPN connectivity using the native built-in VPN Client on iOS or Andriod (IKEv1 with PSK and Xauth)")
mp.template = "ipsec-server/index"

s = mp:section(TypedSection, "service")
s.anonymous = true
o = s:option(DummyValue, "ipsec-server_status", translate("Current Condition"))
o.template = "ipsec-server/status"
enabled = s:option(Flag, "enabled", translate("Enable"))
enabled.default = 0
enabled.rmempty = false

clientip = s:option(Value, "clientip", translate("VPN Client IP"))
clientip.datatype = "ip4addr"
clientip.description = translate(
                           "VPN Client reserved started IP addresses with the same subnet mask")
clientip.optional = false
clientip.rmempty = false

--[[
clientdns = s:option(Value, "clientdns", translate("VPN Client DNS"))
clientdns.datatype = "ip4addr"
clientdns.description = translate("DNS using in VPN tunnel.")
clientdns.optional = false
clientdns.rmempty = false
]]--

secret = s:option(Value, "secret", translate("Secret Pre-Shared Key"))
secret.password = true

function mp.on_save(self)
    require "luci.model.uci"
    require "luci.sys"

    local have_ike_rule = false
    local have_ipsec_rule = false
    local have_ah_rule = false
    local have_esp_rule = false

    luci.model.uci.cursor():foreach('firewall', 'rule', function(section)
        if section.name == 'ike' then have_ike_rule = true end
        if section.name == 'ipsec' then have_ipsec_rule = true end
        if section.name == 'ah' then have_ah_rule = true end
        if section.name == 'esp' then have_esp_rule = true end
    end)

    if not have_ike_rule then
        local cursor = luci.model.uci.cursor()
        local ike_rulename = cursor:add('firewall', 'rule')
        cursor:tset('firewall', ike_rulename, {
            ['name'] = 'ike',
            ['target'] = 'ACCEPT',
            ['src'] = 'wan',
            ['proto'] = 'udp',
            ['dest_port'] = 500
        })
        cursor:save('firewall')
        cursor:commit('firewall')
    end
    if not have_ipsec_rule then
        local cursor = luci.model.uci.cursor()
        local ipsec_rulename = cursor:add('firewall', 'rule')
        cursor:tset('firewall', ipsec_rulename, {
            ['name'] = 'ipsec',
            ['target'] = 'ACCEPT',
            ['src'] = 'wan',
            ['proto'] = 'udp',
            ['dest_port'] = 4500
        })
        cursor:save('firewall')
        cursor:commit('firewall')
    end
    if not have_ah_rule then
        local cursor = luci.model.uci.cursor()
        local ah_rulename = cursor:add('firewall', 'rule')
        cursor:tset('firewall', ah_rulename, {
            ['name'] = 'ah',
            ['target'] = 'ACCEPT',
            ['src'] = 'wan',
            ['proto'] = 'ah'
        })
        cursor:save('firewall')
        cursor:commit('firewall')
    end
    if not have_esp_rule then
        local cursor = luci.model.uci.cursor()
        local esp_rulename = cursor:add('firewall', 'rule')
        cursor:tset('firewall', esp_rulename, {
            ['name'] = 'esp',
            ['target'] = 'ACCEPT',
            ['src'] = 'wan',
            ['proto'] = 'esp'
        })
        cursor:save('firewall')
        cursor:commit('firewall')
    end

end

return mp
