local d = require "luci.dispatcher"
local appname = "passwall"

m = Map(appname)
-- [[ Rule Settings ]]--
s = m:section(TypedSection, "global_rules", translate("Rule status"))
s.anonymous = true
s:append(Template(appname .. "/rule/rule_version"))

--[[
o = s:option(Flag, "adblock", translate("Enable adblock"))
o.rmempty = false
]]--

---- Enable custom url
--[[
o = s:option(Flag, "enable_custom_url", translate("Enable custom url"))
o.default = 0
o.rmempty = false
]]--

---- gfwlist URL
o = s:option(Value, "gfwlist_url", translate("gfwlist Update url"))
o:value("https://cdn.jsdelivr.net/gh/Loukky/gfwlist-by-loukky/gfwlist.txt", translate("Loukky/gfwlist-by-loukky"))
o:value("https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt", translate("gfwlist/gfwlist"))
o.default = "https://cdn.jsdelivr.net/gh/Loukky/gfwlist-by-loukky/gfwlist.txt"
--o:depends("enable_custom_url", 1)

----chnroute  URL
o = s:option(Value, "chnroute_url", translate("Chnroute Update url"))
o:value("https://ispip.clang.cn/all_cn.txt", translate("Clang.CN"))
o:value("https://ispip.clang.cn/all_cn_cidr.txt", translate("Clang.CN.CIDR"))
o.default = "https://ispip.clang.cn/all_cn.txt"
--o:depends("enable_custom_url", 1)

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

s = m:section(TypedSection, "shunt_rules", "V2ray" .. translate("Shunt") .. translate("Rule"))
s.template = "cbi/tblsection"
s.anonymous = false
s.addremove = true
s.extedit = d.build_url("admin", "services", appname, "shunt_rules", "%s")
function s.create(e, t)
    TypedSection.create(e, t)
    luci.http.redirect(e.extedit:format(t))
end

o = s:option(DummyValue, "remarks", translate("Remarks"))

-- [[ App Settings ]]--
s = m:section(TypedSection, "global_app", translate("App Update"),
              "<font color='red'>" ..
                  translate("Please confirm that your firmware supports FPU.") ..
                  "</font>")
s.anonymous = true
s:append(Template(appname .. "/rule/v2ray_version"))
s:append(Template(appname .. "/rule/trojan_go_version"))
s:append(Template(appname .. "/rule/kcptun_version"))
s:append(Template(appname .. "/rule/brook_version"))

---- V2ray Path
o = s:option(Value, "v2ray_file", translate("V2ray Path"), translatef("if you want to run from memory, change the path, such as %s, Then save the application and update it manually.", "/tmp/v2ray/"))
o.default = "/usr/bin/v2ray/"
o.rmempty = false

---- Trojan-Go Path
o = s:option(Value, "trojan_go_file", translate("Trojan-Go Path"), translatef("if you want to run from memory, change the path, such as %s, Then save the application and update it manually.", "/tmp/trojan-go"))
o.default = "/usr/bin/trojan-go"
o.rmempty = false

o = s:option(Value, "trojan_go_latest", translate("Trojan-Go Version API"), translate("alternate API URL for version checking"))
o.default = "https://api.github.com/repos/peter-tank/trojan-go/releases/latest"

---- Kcptun client Path
o = s:option(Value, "kcptun_client_file", translate("Kcptun Client Path"), translatef("if you want to run from memory, change the path, such as %s, Then save the application and update it manually.", "/tmp/kcptun-client"))
o.default = "/usr/bin/kcptun-client"
o.rmempty = false

--[[
o = s:option(Button,  "_check_kcptun",  translate("Manually update"), translatef("Make sure there is enough space to install %s", "kcptun"))
o.template = appname .. "/kcptun"
o.inputstyle = "apply"
o.btnclick = "onBtnClick_kcptun(this);"
o.id = "_kcptun-check_btn"]] --

---- Brook Path
o = s:option(Value, "brook_file", translate("Brook Path"), translatef("if you want to run from memory, change the path, such as %s, Then save the application and update it manually.", "/tmp/brook"))
o.default = "/usr/bin/brook"
o.rmempty = false

return m
