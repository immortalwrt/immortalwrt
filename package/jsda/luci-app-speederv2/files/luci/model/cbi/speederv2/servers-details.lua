local m, s, o
local sid = arg[1]

m = Map("speederv2", "%s - %s" %{translate("speederv2-tunnel"), translate("Edit Config")})
m.redirect = luci.dispatcher.build_url("admin/services/speederv2/servers")
m.sid = sid

if m.uci:get("speederv2", sid) ~= "servers" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, sid, "servers")
s.anonymous = true
s.addremove = false

--Basic Settings
s:tab("basic_settings", translate("Basic Settings"))
o = s:taboption("basic_settings", Value, "alias", translate("Alias"))
o.default = sid
o.rmempty = false

o = s:taboption("basic_settings", Value, "server_ip", translate("Server IP"))
o.datatype = "ipaddr"
o.placeholder = "127.0.0.1"

o = s:taboption("basic_settings", Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.rmempty = false

o = s:taboption("basic_settings", Value, "listen_ip", translate("Local Listening IP"))
o.datatype = "ipaddr"
o.placeholder = "0.0.0.0"

o = s:taboption("basic_settings", Value, "listen_port", translate("Local Listening Port"))
o.datatype = "port"
o.rmempty = false

--Common Settings
s:tab("common_settings", translate("Common Settings"))
o = s:taboption("common_settings", Value, "key", translate("Password"))
o.password = true

o = s:taboption("common_settings", Value, "fec_str", translate("FEC"), translate("1. forward error correction, send y redundant packets for every x packets. 2. fine-grained fec parameters,may help save bandwidth. example: \"-f 1:3,2:4,10:6,20:10\". check repo for details."))
o.datatype = "string"
o.placeholder = "20:10"

o = s:taboption("common_settings", Value, "fec_timeout", translate("Timeout"), translate("how long could a packet be held in queue before doing fec, unit: ms, default: 8ms."))
o.datatype = "uinteger"
o.placeholder = "8"

o = s:taboption("common_settings", Value, "report", translate("Report Interval"), translate("turn on send/recv report, and set a period for reporting, unit: s."))
o.datatype = "uinteger"

--Advanced Settings
s:tab("advanced_settings", translate("Advanced Settings"))
o = s:taboption("advanced_settings", ListValue, "fec_mode", translate("Mode"), translate("fec-mode,available values: 0,1; mode 0(default) costs less bandwidth,no mtu problem. mode 1 usually introduces less latency, but you have to care about mtu."))
o:value("0")
o:value("1")
o.default = "0"

o = s:taboption("advanced_settings", Value, "fec_mtu", translate("MTU"), translate("mtu. for mode 0, the program will split packet to segment smaller than mtu value. for mode 1, no packet will be split, the program just check if the mtu is exceed."))
o.datatype = "range(100,1400)"
o.placeholder = "1250"

o = s:taboption("advanced_settings", Value, "fec_quene_len", translate("Quene Len"), translate("fec queue len, only for mode 0, fec will be performed immediately after queue is full. default value: 200."))
o.datatype = "uinteger"
o:depends("fec_mode", "0")
o.placeholder = "200"

o = s:taboption("advanced_settings", Value, "jitter", translate("Jitter"), translate("1. simulated jitter. randomly delay first packet for 0~<number> ms, default value: 0. do not use if you dont know what it means. 2. create jitter randomly between jmin and jmax."))
o.datatype = "uinteger"
o.placeholder = "0"

o = s:taboption("advanced_settings", Value, "interval", translate("Interval"), translate("1. scatter each fec group to a interval of <number> ms, to protect burst packet loss. default value: 0. do not use if you dont know what it means. 2. scatter randomly between imin and imax."))
o.datatype = "uinteger"
o.placeholder = "0"

o = s:taboption("advanced_settings", Value, "random_drop", translate("Random Drop"), translate("simulate packet loss, unit: 0.01%. default value: 0."))
o.datatype = "uinteger"
o.placeholder = "0"

o = s:taboption("advanced_settings", Value, "disable_obscure", translate("Disable Obscure"), translate("disable obscure, to save a bit bandwidth and cpu."))
o.datatype = "uinteger"

--Developer Settings
s:tab("developer_settings", translate("Developer Settings"))
o = s:taboption("developer_settings", Value, "fifo", translate("FIFO Path"), translate("use a fifo(named pipe) for sending commands to the running program, so that you can change fec encode parameters dynamically, check readme.md in repository for ksupported commands."))
o.datatype = "string"

o = s:taboption("developer_settings", Value, "decode_buf", translate("Decode Buffer"), translate("size of buffer of fec decoder,u nit: packet, default: 2000."))
o.datatype = "uinteger"
o.placeholder = "2000"

o = s:taboption("developer_settings", Value, "fix_latency", translate("Fix Latency"), translate("try to stabilize latency, only for mode 0."))
o.datatype = "uinteger"
o:depends("fec_mode", "0")

o = s:taboption("developer_settings", Value, "delay_capacity", translate("Delay Capacity"), translate("max number of delayed packets."))
o.datatype = "uinteger"

o = s:taboption("developer_settings", Value, "disable_fec", translate("Disable FEC"), translate("completely disable fec, turn the program into a normal udp tunnel."))
o.datatype = "uinteger"

o = s:taboption("developer_settings", Value, "sock_buf", translate("Socket Buffer"), translate("buf size for socket, >=10 and <=10240, unit: kbyte, default: 1024."))
o.datatype = "uinteger"
o.placeholder = "1024"

--log Settings
s:tab("log_settings", translate("Log Settings"))
o = s:taboption("log_settings", Value, "log_level", translate("Log Level"), translate("0: never    1: fatal   2: error   3: warn  4: info (default)      5: debug   6: trace"))
o.datatype = "range(0,6)"
o.placeholder = "4"

o = s:taboption("log_settings", Value, "log_position", translate("Log Position"), translate("enable file name, function name, line number in log."))
o.datatype = "string"

o = s:taboption("log_settings", Flag, "disable_color", translate("Disable Color"), translate("disable log color."))

return m
