local e = require "nixio.fs"
local e = require "luci.sys"
-- local t = luci.sys.exec("cat /usr/share/passwall/dnsmasq.d/gfwlist.conf|grep -c ipset")

m = Map("passwall")
-- [[ Rule Settings ]]--
s = m:section(TypedSection, "global_rules", translate("Rule status"))
s.anonymous = true
s:append(Template("passwall/rule/rule_version"))

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

-- [[ Subscribe Settings ]]--
s = m:section(TypedSection, "global_subscribe", translate("Node Subscribe"),
              translate(
                  "Please input the subscription url first, save and submit before updating. If you subscribe to update, it is recommended to delete all subscriptions and then re-subscribe."))
s.anonymous = true

---- Subscribe via proxy
o = s:option(Flag, "subscribe_proxy", translate("Subscribe via proxy"))
o.default = 0
o.rmempty = false

---- Enable auto update subscribe
o = s:option(Flag, "auto_update_subscribe",
             translate("Enable auto update subscribe"))
o.default = 0
o.rmempty = false

---- Week update rules
o = s:option(ListValue, "week_update_subscribe", translate("Week update rules"))
o:value(7, translate("Every day"))
for e = 1, 6 do o:value(e, translate("Week") .. e) end
o:value(0, translate("Week") .. translate("day"))
o.default = 0
o:depends("auto_update_subscribe", 1)

---- Day update rules
o = s:option(ListValue, "time_update_subscribe", translate("Day update rules"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end
o.default = 0
o:depends("auto_update_subscribe", 1)

---- Manual subscription
o = s:option(Button, "_update", translate("Manual subscription"))
o.inputstyle = "apply"
function o.write(e, e)
    luci.sys.call(
        "lua /usr/share/passwall/subscribe.lua start >> /var/log/passwall.log 2>&1 &")
    luci.http.redirect(luci.dispatcher.build_url("admin", "vpn", "passwall",
                                                 "log"))
end

---- Subscribe Delete All
o = s:option(Button, "_stop", translate("Delete All Subscribe Node"))
o.inputstyle = "remove"
function o.write(e, e)
    luci.sys.call(
        "lua /usr/share/passwall/subscribe.lua truncate >> /var/log/passwall.log 2>&1 &")
    luci.http.redirect(luci.dispatcher.build_url("admin", "vpn", "passwall",
                                                 "log"))
end

s = m:section(TypedSection, "subscribe_list")
s.addremove = true
s.anonymous = true
s.sortable = true
s.template = "cbi/tblsection"

o = s:option(Flag, "enabled", translate("Enabled"))
o.rmempty = false

o = s:option(Value, "remark", translate("Subscribe Remark"))
o.width = "auto"
o.rmempty = false

o = s:option(Value, "url", translate("Subscribe URL"))
o.width = "auto"
o.rmempty = false

-- [[ App Settings ]]--
s = m:section(TypedSection, "global_app", translate("App Update"),
              translate("Please confirm that your firmware supports FPU."))
s.anonymous = true
s:append(Template("passwall/rule/v2ray_version"))
s:append(Template("passwall/rule/kcptun_version"))
s:append(Template("passwall/rule/brook_version"))

---- V2ray Path
o = s:option(Value, "v2ray_file", translate("V2ray Path"), translate(
                 "if you want to run from memory, change the path, such as /tmp/v2ray/, Then save the application and update it manually."))
o.default = "/usr/bin/v2ray/"
o.rmempty = false

---- Kcptun client Path
o = s:option(Value, "kcptun_client_file", translate("Kcptun Client Path"),
             translate(
                 "if you want to run from memory, change the path, such as /tmp/kcptun-client, Then save the application and update it manually."))
o.default = "/usr/bin/kcptun-client"
o.rmempty = false

--[[
o = s:option(Button,  "_check_kcptun",  translate("Manually update"), translate("Make sure there is enough space to install Kcptun"))
o.template = "passwall/kcptun"
o.inputstyle = "apply"
o.btnclick = "onBtnClick_kcptun(this);"
o.id = "_kcptun-check_btn"]] --

---- Brook Path
o = s:option(Value, "brook_file", translate("Brook Path"), translate(
                 "if you want to run from memory, change the path, such as /tmp/brook, Then save the application and update it manually."))
o.default = "/usr/bin/brook"
o.rmempty = false

return m
