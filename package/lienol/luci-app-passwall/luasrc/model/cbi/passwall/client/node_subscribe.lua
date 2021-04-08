local api = require "luci.model.cbi.passwall.api.api"
local appname = api.appname

m = Map(appname)

-- [[ Subscribe Settings ]]--
s = m:section(TypedSection, "global_subscribe", "")
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
o:depends("auto_update_subscribe", true)

---- Day update rules
o = s:option(ListValue, "time_update_subscribe", translate("Day update rules"))
for e = 0, 23 do o:value(e, e .. translate("oclock")) end
o.default = 0
o:depends("auto_update_subscribe", true)

o = s:option(ListValue, "filter_keyword_mode", translate("Filter keyword Mode"))
o:value("0", translate("Close"))
o:value("1", translate("Discard List"))
o:value("2", translate("Keep List"))

o = s:option(DynamicList, "filter_discard_list", translate("Discard List"))

o = s:option(DynamicList, "filter_keep_list", translate("Keep List"))

o = s:option(Flag, "allowInsecure", translate("allowInsecure"), translate("Whether unsafe connections are allowed. When checked, Certificate validation will be skipped."))
o.default = "1"
o.rmempty = false

---- Manual subscription
o = s:option(Button, "_update", translate("Manual subscription"))
o.inputstyle = "apply"
function o.write(e, e)
    luci.sys.call("lua /usr/share/" .. appname .. "/subscribe.lua start log > /dev/null 2>&1 &")
    luci.http.redirect(api.url("log"))
end

---- Subscribe Delete All
o = s:option(Button, "_stop", translate("Delete All Subscribe Node"))
o.inputstyle = "remove"
function o.write(e, e)
    luci.sys.call("lua /usr/share/" .. appname .. "/subscribe.lua truncate log > /dev/null 2>&1 &")
    luci.http.redirect(api.url("log"))
end

s = m:section(TypedSection, "subscribe_list", "",
              "<font color='red'>" .. translate(
                  "Please input the subscription url first, save and submit before updating. If you subscribe to update, it is recommended to delete all subscriptions and then re-subscribe.") ..
                  "</font>")
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

return m
