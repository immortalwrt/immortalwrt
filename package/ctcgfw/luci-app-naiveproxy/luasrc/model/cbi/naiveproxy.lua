-- Created By ImmortalWrt
-- https://github.com/immortalwrt

mp = Map("naiveproxy", translate("NaiveProxy"))
mp.description = translate("NaïveProxy uses Chrome's network stack to camouflage traffic with strong censorship resistance and low detectability. Reusing Chrome's stack also ensures best practices in performance and security.")

mp:section(SimpleSection).template = "naiveproxy/naiveproxy_status"

s = mp:section(TypedSection, "naiveproxy")
s.anonymous=true
s.addremove=false

enable = s:option(Flag, "enable", translate("Enable"))
enable.default = 0
enable.rmempty = false

listen_addr = s:option(Value, "listen_addr", translate("Listen Address"))
listen_addr.description = translate("proto://[addr][:port]")
listen_addr.rmempty = false

server_addr = s:option(Value, "server_addr", translate("Server Address"))
server_addr.description = translate("proto://user:pass@hostname[:port]")
server_addr.rmempty = false

extra_argument = s:option(Value, "extra_argument", translate("Extra Argument"))
extra_argument.description = translate("Appends extra argument to NaiveProxy")

return mp
