
local m, s, o
local clash = "clash"
local uci = luci.model.uci.cursor()
local ipkg = require("luci.model.ipkg")
local fs = require "nixio.fs"
local sys = require "luci.sys"
local sid = arg[1]


m = Map(clash, translate("Edit Proxy Provider"))
m.redirect = luci.dispatcher.build_url("admin/services/clash/create")
if m.uci:get(clash, sid) ~= "provider" then
	luci.http.redirect(m.redirect)
	return
end

s = m:section(NamedSection, sid, "provider")
s.anonymous = true
s.addremove   = false

o = s:option(ListValue, "type", translate("Provider Type"))
o.rmempty = true
o.description = translate("Provider Type")
o:value("http")
o:value("file")

o = s:option(Value, "name", translate("Provider Name"))
o.rmempty = false

o = s:option(Value, "path", translate("Provider Path"))
o.description = translate("【HTTP】./hk.yaml or 【File】/etc/clash/hk.yaml")
o.rmempty = false

o = s:option(Value, "provider_url", translate("Provider URL"))
o.rmempty = false
o:depends("type", "http")

o = s:option(Value, "provider_interval", translate("Provider Interval"))
o.default = "3600"
o.rmempty = false
o:depends("type", "http")

o = s:option(ListValue, "health_check", translate("Provider Health Check"))
o:value("false", translate("Disable"))
o:value("true", translate("Enable"))
o.default=true

o = s:option(Value, "health_check_url", translate("Health Check URL"))
o.default = "http://www.gstatic.com/generate_204"
o.rmempty = false

o = s:option(Value, "health_check_interval", translate("Health Check Interval"))
o.default = "300"
o.rmempty = false

o = s:option(DynamicList, "groups", translate("Policy Group"))
o.rmempty = true
m.uci:foreach("clash", "groups",
		function(s)
			o:value(s.name)
		end)


return m