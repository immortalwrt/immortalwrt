local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname

m = Map(appname)
-- [[ Rule Settings ]]--
s = m:section(TypedSection, "global_rules", translate("Rule status"))
s.anonymous = true

--[[
o = s:option(Flag, "adblock", translate("Enable adblock"))
o.rmempty = false
]]--

---- gfwlist URL
o = s:option(Value, "gfwlist_url", translate("GFW domains(gfwlist) Update URL"))
o:value("https://cdn.jsdelivr.net/gh/Loukky/gfwlist-by-loukky/gfwlist.txt", translate("Loukky/gfwlist-by-loukky"))
o:value("https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt", translate("gfwlist/gfwlist"))
o.default = "https://cdn.jsdelivr.net/gh/Loukky/gfwlist-by-loukky/gfwlist.txt"

----chnroute  URL
o = s:option(Value, "chnroute_url", translate("China IPs(chnroute) Update URL"))
o:value("https://ispip.clang.cn/all_cn.txt", translate("Clang.CN"))
o:value("https://ispip.clang.cn/all_cn_cidr.txt", translate("Clang.CN.CIDR"))
o.default = "https://ispip.clang.cn/all_cn.txt"

----chnroute6 URL
o = s:option(Value, "chnroute6_url", translate("China IPv6s(chnroute6) Update URL"))
o:value("https://ispip.clang.cn/all_cn_ipv6.txt", translate("Clang.CN.IPv6"))
o.default = "https://ispip.clang.cn/all_cn_ipv6.txt"

----chnlist URL
o = s:option(DynamicList, "chnlist_url", translate("China List(Chnlist) Update URL"))

s:append(Template(appname .. "/rule/rule_version"))

---- Auto Update
o = s:option(Flag, "auto_update", translate("Enable auto update rules"))
o.default = 0
o.rmempty = false

---- Week Update
o = s:option(ListValue, "week_update", translate("Week update rules"))
o:value(7, translate("Every day"))
for e = 1, 6 do o:value(e, translate("Week") .. e) end
o:value(0, translate("Week") .. translate("day"))
o.default = 0
o:depends("auto_update", 1)

---- Time Update
o = s:option(ListValue, "time_update", translate("Day update rules"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end
o.default = 0
o:depends("auto_update", 1)

o = s:option(Value, "xray_location_asset", translate("Location of Xray asset"), translate("This variable specifies a directory where geoip.dat and geosite.dat files are."))
o.default = "/usr/share/xray/"
o.rmempty = false

s = m:section(TypedSection, "shunt_rules", "Xray" .. translate("Shunt") .. translate("Rule"))
s.template = "cbi/tblsection"
s.anonymous = false
s.addremove = true
s.sortable = true
s.extedit = api.url("shunt_rules", "%s")
function s.create(e, t)
    TypedSection.create(e, t)
    luci.http.redirect(e.extedit:format(t))
end

o = s:option(DummyValue, "remarks", translate("Remarks"))

return m
