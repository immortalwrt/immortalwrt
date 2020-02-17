local fs = require "nixio.fs"
local net = require"luci.model.network".init()
local ifaces = require"luci.sys".net:devices()

m = Map("passwall")

-- [[ Delay Settings ]]--
s = m:section(TypedSection, "global_delay", translate("Delay Settings"))
s.anonymous = true
s.addremove = false

---- Delay Start
o = s:option(Value, "start_delay", translate("Delay Start"),
             translate("Units:seconds"))
o.default = "1"
o.rmempty = true

---- Open and close Daemon
o = s:option(Flag, "start_daemon", translate("Open and close Daemon"))
o.default = 1
o.rmempty = false

--[[
---- Open and close automatically
o = s:option(Flag, "auto_on", translate("Open and close automatically"))
o.default = 0
o.rmempty = false

---- Automatically turn off time
o = s:option(ListValue, "time_off", translate("Automatically turn off time"))
o.default = nil
o:depends("auto_on", "1")
o:value(nil, translate("Disable"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end

---- Automatically turn on time
o = s:option(ListValue, "time_on", translate("Automatically turn on time"))
o.default = nil
o:depends("auto_on", "1")
o:value(nil, translate("Disable"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end

---- Automatically restart time
o = s:option(ListValue, "time_restart", translate("Automatically restart time"))
o.default = nil
o:depends("auto_on", "1")
o:value(nil, translate("Disable"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end
--]]

-- [[ Forwarding Settings ]]--
s = m:section(TypedSection, "global_forwarding",
              translate("Forwarding Settings"))
s.anonymous = true
s.addremove = false

---- TCP No Redir Ports
o = s:option(Value, "tcp_no_redir_ports", translate("TCP No Redir Ports"),
             translate(
                 "Fill in the ports you don't want to be forwarded by the agent, with the highest priority."))
o.default = "disable"
o:value("disable", translate("No patterns are used"))
o:value("1:65535", translate("All"))

---- UDP No Redir Ports
o = s:option(Value, "udp_no_redir_ports", translate("UDP No Redir Ports"),
             translate(
                 "Fill in the ports you don't want to be forwarded by the agent, with the highest priority."))
o.default = "disable"
o:value("disable", translate("No patterns are used"))
o:value("1:65535", translate("All"))

---- TCP Redir Ports
o = s:option(Value, "tcp_redir_ports", translate("TCP Redir Ports"))
o.default = "22,25,53,143,465,587,993,995,80,443"
o:value("1:65535", translate("All"))
o:value("22,25,53,143,465,587,993,995,80,443", translate("Common Use"))
o:value("80,443", translate("Only Web"))
o:value("80:", "80 " .. translate("or more"))
o:value(":443", "443 " .. translate("or less"))

---- UDP Redir Ports
o = s:option(Value, "udp_redir_ports", translate("UDP Redir Ports"))
o.default = "1:65535"
o:value("1:65535", translate("All"))
o:value("53", "DNS")

---- Multi SS/SSR Process Option
o = s:option(Value, "process", translate("Multi Process Option"),
             translate("you can start SS/SSR with multiple process"))
o.default = "0"
o.rmempty = false
o:value("0", translate("Auto"))
o:value("1", translate("1 Process"))
o:value("2", "2 " .. translate("Process"))
o:value("3", "3 " .. translate("Process"))
o:value("4", "4 " .. translate("Process"))

---- Socks5 Proxy Port
o = s:option(Value, "socks5_proxy_port", translate("Socks5 Proxy Port"))
o.datatype = "port"
o.default = 1081
o.rmempty = true

---- Proxy IPv6
o = s:option(Flag, "proxy_ipv6", translate("Proxy IPv6"),
             translate("The IPv6 traffic can be proxyed when selected"))
o.default = 0

--[[
---- TCP Redir Port
o = s:option(Value, "tcp_redir_port", translate("TCP Redir Port"))
o.datatype = "port"
o.default = 1041
o.rmempty = true

---- UDP Redir Port
o = s:option(Value, "udp_redir_port", translate("UDP Redir Port"))
o.datatype = "port"
o.default = 1051
o.rmempty = true

---- Kcptun Port
o = s:option(Value, "kcptun_port", translate("Kcptun Port"))
o.datatype = "port"
o.default = 12948
o.rmempty = true
--]]

-- [[ Other Settings ]]--
s = m:section(TypedSection, "global_other", translate("Other Settings"),
              translatef(
                  "You can only set up a maximum of %s nodes for the time being, Used for access control.",
                  "3"))
s.anonymous = true
s.addremove = false

---- TCP Node Number Option
o = s:option(ListValue, "tcp_node_num", "TCP" .. translate("Node Number"))
o.default = "1"
o.rmempty = false
o:value("1")
o:value("2")
o:value("3")

---- UDP Node Number Option
o = s:option(ListValue, "udp_node_num", "UDP" .. translate("Node Number"))
o.default = "1"
o.rmempty = false
o:value("1")
o:value("2")
o:value("3")

---- Socks5 Node Number Option
o = s:option(ListValue, "socks5_node_num", "Socks5" .. translate("Node Number"))
o.default = "1"
o.rmempty = false
o:value("1")
o:value("2")
o:value("3")

---- 状态使用大图标
o = s:option(Flag, "status_use_big_icon", translate("Status Use Big Icon"))
o.default = "1"
o.rmempty = false

---- 显示节点检测
o =
    s:option(Flag, "status_show_check_port", translate("Status Show Check Port"))
o.default = "0"
o.rmempty = false

---- 显示IP111
o = s:option(Flag, "status_show_ip111", translate("Status Show IP111"))
o.default = "0"
o.rmempty = false

---- Hide Menu
o = s:option(Button, "hide", translate("Hide Menu"),
             translate(
                 "After the hidden to the display, input example in the address bar: ") ..
                 "http://192.168.1.1/cgi-bin/luci/admin/vpn/passwall/show")
o.inputstyle = "remove"
function o.write(e, e)
    luci.http.redirect(luci.dispatcher.build_url("admin", "vpn", "passwall",
                                                 "hide"))
end

return m
