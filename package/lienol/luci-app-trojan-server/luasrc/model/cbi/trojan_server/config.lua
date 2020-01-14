local app_name = "trojan_server"
local d = require "luci.dispatcher"

map = Map(app_name, "Trojan " .. translate("Server Config"))
map.redirect = d.build_url("admin", "vpn", "trojan_server")

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

password = t:option(DynamicList, "password", translate("Password"))

tcp_fast_open = t:option(ListValue, "tcp_fast_open", translate("TCP Fast Open"),
                         translate(
                             "Enable TCP fast open (kernel support required)"))
tcp_fast_open:value("false")
tcp_fast_open:value("true")

remote_enable = t:option(Flag, "remote_enable", translate("Enable Remote"),
                         translate(
                             "You can forward to Nginx/Caddy/V2ray WebSocket and more."))
remote_enable.default = "1"
remote_enable.rmempty = false

remote_address = t:option(Value, "remote_address", translate("Remote Address"))
remote_address.default = "127.0.0.1"
remote_address:depends("remote_enable", 1)

remote_port = t:option(Value, "remote_port", translate("Remote Port"))
remote_port.datatype = "port"
remote_port.default = "80"
remote_port:depends("remote_enable", 1)

tls_certFile = t:option(Value, "ssl_certFile",
                        "HTTPS" .. translate("Public key absolute path"),
                        translate("as:") .. "/etc/ssl/fullchain.pem")

tls_keyFile = t:option(Value, "ssl_keyFile",
                       "HTTPS" .. translate("Private key absolute path"),
                       translate("as:") .. "/etc/ssl/private.key")

return map
