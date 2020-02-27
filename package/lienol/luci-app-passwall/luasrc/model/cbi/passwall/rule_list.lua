local fs = require "nixio.fs"

m = Map("passwall")
-- [[ Rule List Settings ]]--
s = m:section(TypedSection, "global_rules")
s.anonymous = true

---- Whitelist Hosts
s:tab("w_hosts", translate("Whitelist Hosts"), "<font color='red'>" ..
          translate("Join the white list of domain names will not go agent.") ..
          "</font>")
local w_host_file = "/usr/share/passwall/rules/whitelist_host"
o = s:taboption("w_hosts", TextValue, "whitelist_host")
o.rows = 20
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(w_host_file) or "" end
o.write = function(self, section, value)
    fs.writefile(w_host_file, value:gsub("\r\n", "\n"):gsub("http://", ""):gsub(
                     "https://", ""))
end
o.remove = function(self, section, value) fs.writefile(w_host_file, "") end

---- Whitelist IP
s:tab("w_ip", translate("Whitelist IP"), "<font color='red'>" .. translate(
          "These had been joined ip addresses will not use proxy.Please input the ip address or ip address segment,every line can input only one ip address.For example,192.168.0.0/24 or 223.5.5.5.") ..
          "</font>")
local w_ip_file = "/usr/share/passwall/rules/whitelist_ip"
o = s:taboption("w_ip", TextValue, "whitelist_ip")
o.rows = 20
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(w_ip_file) or "" end
o.write = function(self, section, value)
    fs.writefile(w_ip_file, value:gsub("\r\n", "\n"):gsub("http://", ""):gsub(
                     "https://", ""))
end
o.remove = function(self, section, value) fs.writefile(w_ip_file, "") end

---- Blacklist Hosts
s:tab("b_hosts", translate("Blacklist Hosts"),
      "<font color='red'>" .. translate(
          "These had been joined websites will use proxy.Please input the domain names of websites,every line can input only one website domain.For example,google.com.") ..
          "</font>")
local b_host_file = "/usr/share/passwall/rules/blacklist_host"
o = s:taboption("b_hosts", TextValue, "blacklist_host")
o.rows = 20
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(b_host_file) or "" end
o.write = function(self, section, value)
    fs.writefile(b_host_file, value:gsub("\r\n", "\n"):gsub("http://", ""):gsub(
                     "https://", ""))
end
o.remove = function(self, section, value) fs.writefile(b_host_file, "") end

---- Blacklist IP
s:tab("b_ip", translate("Blacklist IP"), "<font color='red'>" .. translate(
          "These had been joined ip addresses will use proxy.Please input the ip address or ip address segment,every line can input only one ip address.For example,35.24.0.0/24 or 8.8.4.4.") ..
          "</font>")
local b_ip_file = "/usr/share/passwall/rules/blacklist_ip"
o = s:taboption("b_ip", TextValue, "blacklist_ip")
o.rows = 20
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(b_ip_file) or "" end
o.write = function(self, section, value)
    fs.writefile(b_ip_file, value:gsub("\r\n", "\n"):gsub("http://", ""):gsub(
                     "https://", ""))
end
o.remove = function(self, section, value) fs.writefile(b_ip_file, "") end

return m
