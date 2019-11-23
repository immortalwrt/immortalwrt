local app_name = "brook_server"
local d = require "luci.dispatcher"

map = Map(app_name, "Brook " .. translate("Server Config"))
map.redirect = d.build_url("admin", "vpn", "brook_server")

t = map:section(NamedSection, arg[1], "user", "")
t.addremove = false
t.dynamic = false

enable = t:option(Flag, "enable", translate("Enable"))
enable.default = "1"
enable.rmempty = false

remarks = t:option(Value, "remarks", translate("Remarks"))
remarks.default = translate("Remarks")
remarks.rmempty = false

port = t:option(Value, "port", translate("Port"))
port.datatype = "port"
port.rmempty = false

protocol = t:option(ListValue, "protocol", translate("Protocol"), translate(
                        "if shadowsocks server mode, fixed method is aes-256-cfb"))
protocol:value("server", translate("Brook"))
protocol:value("ssserver", translate("Shadowsocks"))

password = t:option(Value, "password", translate("Password"))
password.password = true
password.rmempty = false

return map
