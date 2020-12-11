local function is_running(name)
    if luci.sys.call("pidof %s >/dev/null" %{name}) == 0 then
        return translate("RUNNING")
    else
        return translate("NOT RUNNING")
    end
end

local function is_online(ipaddr)
    if ipaddr == "0.0.0.0" or ipaddr == "nil" then 
        return translate("Pinghost not set")
    end
    if luci.sys.call("ping -c1 -w1 %s >/dev/null 2>&1" %{ipaddr}) == 0 then
        return translate("ONLINE")
    else
        return translate("NOT ONLINE")
    end
end

require("luci.sys")

m = Map("minieap", translate("minieap"), translate("Configure minieap 802.11x."))

s = m:section(TypedSection, "minieap", translate("Status"))
s.anonymous = true
status = s:option(DummyValue,"_minieap_status", "minieap")
status.value = "<span id=\"_minieap_status\">%s</span>" %{is_running("minieap")}
status.rawhtml = true
t = io.popen('uci get minieap.@minieap[0].pinghost')
netstat=is_online(tostring(t:read("*line")))
t:close()
if netstat ~= "" then
netstatus = s:option(DummyValue,"_network_status", translate("Network Status"))
netstatus.value = "<span id=\"_network_status\">%s</span>" %{netstat}
netstatus.rawhtml = true
end

o = m:section(TypedSection, "minieap", translate("Settings"))
o.addremove = false
o.anonymous = true

o:tab("base", translate("Normal Settings"))
o:tab("advanced", translate("Advanced Settings"))
o:tab("plugins", translate("Plugins List"))
o:tab("ruijie", translate("Ruijie EAP Settings"))

enable = o:taboption("base", Flag, "enable", translate("Enable"))
name = o:taboption("base", Value, "username", translate("Username"),translate("The username given to you by your network administrator"))
pass = o:taboption("base", Value, "password", translate("Password"), translate("The password you set or given to you by your network administrator"))
pass.password = true

ifname = o:taboption("base", ListValue, "nic", translate("Interfaces"), translate("Physical interface of WAN"))
for k, v in ipairs(luci.sys.net.devices()) do
    if v ~= "lo" then
        ifname:value(v)
    end
end

pinghost = o:taboption("base", Value, "pinghost", translate("PingHost"), translate("Ping host for drop detection, 0.0.0.0 to turn off this feature."))
pinghost.default = "0.0.0.0"

pingintval = o:taboption("base", Value, "pingintval", translate("Ping intval"), translate("Interval of each ping. (in second) [default: 30]"))
pingintval.default = "30"

pingcommand = o:taboption("base", Value, "pingcommand", translate("Offline command"), translate("Run Command when ping failed. [default: minieap -k 1]"))
pingcommand:value("minieap -k 1")
pingcommand.default = "minieap -k 1"

stage_timeout = o:taboption("advanced", Value, "stage_timeout", translate("Auth Timeout"), translate("802.11X auth timeout (in second). [default: 5]"))
stage_timeout.default = "5"

wait_after_fail = o:taboption("advanced", Value, "wait_after_fail", translate("Wait after failed"), translate("Wait time between failed and next time (in second). [default: 30]"))
wait_after_fail.default = "30"

max_fail = o:taboption("advanced", Value, "max_fail", translate("Max fail"), translate("Maximum allowed number of failures. [default: 3]"))
max_fail.default = "3"

no_auto_reauth = o:taboption("advanced", ListValue, "no_auto_reauth", translate("Disable auto reauth"), translate("Disable auto reauth after offline. [default: True]"))
no_auto_reauth:value(0, translate("False"))
no_auto_reauth:value(1, translate("True"))
no_auto_reauth.default = 1

proxy_lan_iface = o:taboption("advanced", Value, "proxy_lan_iface", translate("Proxy LAN's name"), translate("Name of LAN interface when use proxy auth. [default: None]"))
proxy_lan_iface.default = ""

auth_round = o:taboption("advanced", Value, "auth_round", translate("Auth times"), translate("Number of times required for auth. [default: 1]"))
auth_round.default = "1"

max_retries = o:taboption("advanced", Value, "max_retries", translate("Max timeout"), translate("Maximum retry time after timeout. [default: 3]"))
max_retries.default = "3"

pid_file = o:taboption("advanced", Value, "pid_file", translate("PID file"), translate("Path of PID file. (Set 'None' to disable) [default: /var/run/minieap.pid]"))
pid_file:value("/var/run/minieap.pid")
pid_file.default = "/var/run/minieap.pid"

if_impl = o:taboption("advanced", ListValue, "if_impl", translate("Network Module"), translate("Network module for send and recv packages (openwrt support sockraw only)"))
if_impl:value("sockraw")
if_impl.default = "sockraw"

plugins = o:taboption("plugins", DynamicList, "module", translate("Plugins list"), translate("Packets flow through these plug-ins in sequence. Pay attention to the order in the environment where the package plug-in is modified"))
plugins:value("printer", translate("printer: Print length of packets"))
plugins:value("rjv3", translate('rjv3: Ruijie 802.11X. Support V3 verification algorithm'))

heartbeat = o:taboption("ruijie", Value, "heartbeat", translate("Heartbeat interval"), translate("Interval for sending Heartbeat packets (seconds) [Default: 60]"))
heartbeat.default = "60"

eap_bcast_addr = o:taboption("ruijie", ListValue, "eap_bcast_addr", translate("Broadcast address"), translate("Broadcast address type when searching for servers [Default: Standard]"))
eap_bcast_addr:value(0, translate("Standard"))
eap_bcast_addr:value(1, translate("Ruijie private"))
eap_bcast_addr.default = 0

dhcp_type = o:taboption("ruijie", ListValue, "dhcp_type", translate("DhcpMode"), translate("DHCP method [Default: After certification]"))
dhcp_type:value(0, translate("None"))
dhcp_type:value(1, translate("Secondary authentication"))
dhcp_type:value(2, translate("Before certification"))
dhcp_type:value(3, translate("After certification"))
dhcp_type.default = 3

dhcp_script = o:taboption("ruijie", Value, "dhcp_script", translate("DhcpScript"), translate("DHCP script [Default: None]"))
dhcp_script.default = ""

service = o:taboption("ruijie", Value, "service", translate("Service"), translate("Service From Ruijie Server [Default: internet]"))
service.default = "internet"

version_str = o:taboption("ruijie", Value, "version_str", translate("Version String"), translate("Custom version [Default: RG-SU For Linux V1.0]"))
version_str:value("RG-SU For Linux V1.0")
version_str.default = "RG-SU For Linux V1.0"

fake_dns1 = o:taboption("ruijie", Value, "fake_dns1", translate("Main DNS server"), translate("Custom main DNS server [Default: FromSystem]"))

fake_dns2 = o:taboption("ruijie", Value, "fake_dns2", translate("Second DNS server"), translate("Custom second DNS server [Default: FromSystem]"))

fake_serial = o:taboption("ruijie", Value, "fake_serial", translate("Disk serial"), translate("Custom disk serial [Default: From /etc/mtab]"))

max_dhcp_count = o:taboption("ruijie", Value, "max_dhcp_count", translate("DHCP try times"), translate("DHCP try times [Default: 3]"))
max_dhcp_count.default = "3"

rj_option = o:taboption("ruijie", DynamicList, "rj_option", translate("Custom EAP Options"), translate("Format &lt;type&gt;:&lt;value&gt;[:r]. Add a option type: &lt;type&gt;, value: &lt;value&gt;. :r for replace"))

if nixio.fs.mkdir("/etc/minieap.conf.d") == true then
    nixio.fs.symlink("/etc/minieap.conf.d/minieap.conf.utf8", "/etc/minieap.conf")
end

local apply = luci.http.formvalue("cbi.apply")
if apply then
    luci.sys.call("minieap-conver | sort > /etc/minieap.conf.d/minieap.conf.utf8")
    io.popen("/etc/init.d/minieap restart")
end

return m
