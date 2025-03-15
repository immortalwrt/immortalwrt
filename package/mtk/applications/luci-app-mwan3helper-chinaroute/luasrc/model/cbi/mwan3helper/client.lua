

m = Map("mwan3helper")
m.title	= translate("MWAN3 Helper")

s = m:section(TypedSection, "mwan3helper")
s.addremove = false
s.anonymous = true

o = s:option(Flag, "enabled", translate("启用转发"))
o.rmempty = false


o = s:option(ListValue, "mode", translate("转发流量模式"))
o:value("0", translate("国外流量"))
o:value("1", translate("GFWLIST流量"))

o = s:option(Value, "dns", translate("转发流量至IP"))
o = s:option(Value, "dev", translate("设备"))
o = s:option(Flag, "dnsen", translate("启用 DNS 防污染"))
o.rmempty = false
o = s:option(Value, "dnsserver", translate("Anti-pollution DNS Server"))
return m
