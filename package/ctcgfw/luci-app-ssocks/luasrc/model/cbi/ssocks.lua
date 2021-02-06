-- This is a free software, use it under GNU General Public License v3.0.
-- Created By ImmortalWrt
-- https://github.com/immortalwrt

mp = Map("ssocks", translate("sSocks Server"))
mp.description = translate("sSocks Server is a simple, small, and easy-to-use Socks5 server program, but supports TCP on IPv4 only.")

mp:section(SimpleSection).template = "ssocks/ssocks_status"

s = mp:section(TypedSection, "ssocks")
s.anonymous=true
s.addremove=false

enable = s:option(Flag, "enable", translate("Enable"))
enable.default = 0
enable.rmempty = false

bind_addr = s:option(ListValue, "bind_addr", translate("Bind Address"))
bind_addr:value("lan", translate("LAN"))
bind_addr:value("wan", translate("WAN"))
bind_addr.description = translate("The address that sSocks Server binded.")
bind_addr.default = "wan"
bind_addr.rmempty = false

listen_port = s:option(Value, "listen_port", translate("Listen Port"))
listen_port.description = translate("The port that sSocks Server listened at, don't reuse the port with other program.")
listen_port.placeholder = "10800"
listen_port.default = "10800"
listen_port.datatype = "port"
listen_port.rmempty = false

username = s:option(Value, "username", translate("Username"))
username.description = translate("The authorization username, leave blank to deauthorize.")
username.placeholder = "Username"
username.default = "ctcgfw"
username.datatype = "string"

password = s:option(Value, "password", translate("Password"))
password.description = translate("The authorization password, leave blank to deauthorize.")
password.placeholder = "Password"
password.default = "password"
password.datatype = "string"
password.password = true

return mp
