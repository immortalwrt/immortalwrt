local map, section, net = ...

local l = section.tab_names
for i, v in ipairs(l) do
	if v == "firewall" then
		table.remove(l, i)
		break
	end
end
section.tabs["firewall"] = nil

l = section.tabs["advanced"].childs
for i, v in ipairs(l) do
	if v.option == "delegate" then
		table.remove(l, i)
		break
	end
end

section:tab("rjv3", translate("RJv3 Plugin Settings"), nil)

local i = #section.tab_names
local t = section.tab_names
t[i], t[i-1] = t[i-1], t[i]

local o = section:taboption("general", Value, "username", translate("Username"))

o = section:taboption("general", Value, "password", translate("Password"))
o.password = true

o = section:taboption("general", DynamicList, "module", translate("Packet plugins"))
o.rmempty = true
o.datatype = "list(string)"
o:value("rjv3", "rjv3")
o:value("printer", "printer")

o = section:taboption("advanced", Value, "max_fail", translate("Max fail"))
o.datatype = "uinteger"
o.placeholder = "3"

o = section:taboption("advanced", Value, "max_retries", translate("Max retries"))
o.datatype = "uinteger"
o.placeholder = "3"

o = section:taboption("advanced", Flag, "no_auto_reauth", translate("No auto reauth"))

o = section:taboption("advanced", Value, "wait_after_fail", translate("Wait after fail"))
o.datatype = "uinteger"
o.placeholder = "30"

o = section:taboption("advanced", Value, "stage_timeout", translate("Stage timeout"))
o.datatype = "uinteger"
o.placeholder = "5"

o = section:taboption("advanced", Value, "auth_round", translate("Auth Round"))
o.datatype = "uinteger"
o.placeholder = "1"

o = section:taboption("advanced", Value, "log_file", translate("Log file"))
o.placeholder = "/var/log/minieap.log"

-- rjv3 plugin
o = section:taboption("rjv3", Value, "heartbeat", translate("Heartbeat interval"))
o.datatype = "uinteger"
o.placeholder = "60"

o = section:taboption("rjv3", ListValue, "eap_bcast_addr", translate("EAP broadcast address"))
o:value(0, translate("Standard")) -- BROADCAST_STANDARD
o:value(1, translate("RJ private")) -- BROADCAST_RJ
-- o:value(2, translate("BROADCAST_CER")) -- BROADCAST_CER

o = section:taboption("rjv3", ListValue, "dhcp_type", translate("DHCP type"))
o:value(0, translate("Disabled")) -- DHCP_NONE
o:value(1, translate("Double Auth")) -- DHCP_DOUBLE_AUTH
o:value(2, translate("DHCP After Auth")) -- DHCP_AFTER_AUTH
o:value(3, translate("DHCP Before Auth")) -- DHCP_BEFORE_AUTH

o = section:taboption("rjv3", DynamicList, "rj_option", translate("RJ option"))
o.rmempty = true
o.placeholder = "<type>:<value>[:r]"

o = section:taboption("rjv3", Value, "service", translate("Service name"))
o.placeholder = "internet"

o = section:taboption("rjv3", Value, "version_str", translate("Version string"))
o.placeholder = "RG-SU For Linux V1.0"

o = section:taboption("rjv3", Value, "dhcp_script", translate("DHCP script"))

o = section:taboption("rjv3", Value, "fake_dns1", translate("Fake DNS 1"))
o.datatype = "ip4addr"

o = section:taboption("rjv3", Value, "fake_dns2", translate("Fake DNS 2"))
o.datatype = "ip4addr"

o = section:taboption("rjv3", Value, "fake_serial", translate("Fake HDD serial"))

o = section:taboption("rjv3", Value, "max_dhcp_count", translate("Max DHCP count"))
o.datatype = "uinteger"
o.placeholder = "3"
