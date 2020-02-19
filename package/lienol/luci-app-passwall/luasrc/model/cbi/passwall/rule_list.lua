local fs = require "nixio.fs"

m = Map("passwall")
-- [[ Rule List Settings ]]--
s = m:section(TypedSection, "global", translate("Set Blacklist And Whitelist"))
s.anonymous = true

---- Whitelist Hosts
local w_host_file = "/etc/config/passwall_rule/whitelist_host"
o = s:option(TextValue, "whitelist_host", translate("Whitelist Hosts"))
o.description = translate(
                    "Join the white list of domain names will not go agent.")
o.rows = 5
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(w_host_file) or "" end
o.write = function(self, section, value) fs.writefile(w_host_file, value:gsub("\r\n", "\n")) end
o.remove = function(self, section, value) fs.writefile(w_host_file, "") end

---- Whitelist IP
local w_ip_file = "/etc/config/passwall_rule/whitelist_ip"
o = s:option(TextValue, "whitelist_ip", translate("Whitelist IP"))
o.description = translate(
                    "These had been joined ip addresses will not use proxy.Please input the ip address or ip address segment,every line can input only one ip address.For example,112.123.134.145/24 or 112.123.134.145.")
o.rows = 5
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(w_ip_file) or "" end
o.write = function(self, section, value) fs.writefile(w_ip_file, value:gsub("\r\n", "\n")) end
o.remove = function(self, section, value) fs.writefile(w_ip_file, "") end

---- Blacklist Hosts
local b_host_file = "/etc/config/passwall_rule/blacklist_host"
o = s:option(TextValue, "blacklist_host", translate("Blacklist Hosts"))
o.description = translate(
                    "These had been joined websites will use proxy.Please input the domain names of websites,every line can input only one website domain.For example,google.com.")
o.rows = 5
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(b_host_file) or "" end
o.write = function(self, section, value) fs.writefile(b_host_file, value:gsub("\r\n", "\n")) end
o.remove = function(self, section, value) fs.writefile(b_host_file, "") end

---- Blacklist IP
local b_ip_file = "/etc/config/passwall_rule/blacklist_ip"
o = s:option(TextValue, "blacklist_ip", translate("Blacklist IP"))
o.description = translate(
                    "These had been joined ip addresses will use proxy.Please input the ip address or ip address segment,every line can input only one ip address.For example,112.123.134.145/24 or 112.123.134.145.")
o.rows = 5
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(b_ip_file) or "" end
o.write = function(self, section, value) fs.writefile(b_ip_file, value:gsub("\r\n", "\n")) end
o.remove = function(self, section, value) fs.writefile(b_ip_file, "") end

---- Router Hosts
local router_file = "/etc/config/passwall_rule/router"
o = s:option(TextValue, "routerlist", translate("Router Hosts"))
o.description = translate(
                    "These had been joined websites will use proxy,but only Router model.Please input the domain names of websites,every line can input only one website domain.For example,google.com.")
o.rows = 5
o.wrap = "off"
o.cfgvalue = function(self, section) return fs.readfile(router_file) or "" end
o.write = function(self, section, value) fs.writefile(router_file, value:gsub("\r\n", "\n")) end
o.remove = function(self, section, value) fs.writefile(router_file, "") end

return m
