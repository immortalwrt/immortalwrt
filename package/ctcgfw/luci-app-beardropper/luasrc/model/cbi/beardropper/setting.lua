
m = Map("beardropper", translate("BearDropper"), 
translate("luci-app-beardropper, the LuCI app built with the elegant firewall rule generation on-the-fly script bearDropper. <br /> <br /> Should you have any questions, please refer to the repo: ")..[[<a href="https://github.com/NateLol/luci-app-bearDropper" target="_blank">luci-app-beardropper</a>]]
)
m:chain("luci")

m:section(SimpleSection).template="beardropper/status"

s = m:section(TypedSection, "beardropper", translate(""))
s.anonymous = true
s.addremove = false

-- TABS 
s:tab("options", translate("Options"))
s:tab("blocked", translate("Blocked IP"))

o = s:taboption("options", Flag, "enabled",translate("Enabled"))
o.default = 0

-- OPTIONS
o = s:taboption("options", ListValue, "defaultMode", translate("Running Mode"))
o.default = "follow"
o:value("follow", translate("Follow"))
o:value("entire", translate("Entire"))
o:value("today", translate("Today"))
o:value("wipe", translate("Wipe"))


o = s:taboption("options", Value, "attemptCount", translate("Attempt Tolerance"), translate("failure attempts from a given IP required to trigger a ban"))

o = s:taboption("options", Value, "attemptPeriod", translate("Attempt Cycle"), translate("time period during which attemptCount must be exceeded in order to trigger a ban <br> Format: 1w2d3h4m5s represents 1week 2days 3hours 4minutes 5 seconds"))

o = s:taboption("options", Value, "banLength", translate("Ban Period"), translate("how long a ban exist once the attempt threshold is exceeded"))

o = s:taboption("options", ListValue, "logLevel", translate("Log Level"))
o.default = "1"
o:value("0", translate("Silent"))
o:value("1", translate("Default"))
o:value("2", translate("Verbose"))
o:value("3", translate("Debug"))


o = s:taboption("blocked", Value, "blocked", translate("Blocked IP List"))
o.template="cbi/tvalue"
o.rows=40
o.wrap="off"
o.readonly="true"
function o.cfgvalue(e, e)
	return luci.sys.exec("cat /tmp/beardropper.bddb | awk /'=1/'| awk -F '=' '{print $1}' | awk '{print substr($0,6)}' | awk 'gsub(/_/,\":\",$0)'")
end



return m
