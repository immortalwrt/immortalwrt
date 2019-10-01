
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()
local fs = require "luci.clash"
local http = luci.http


m = Map("clash")
s = m:section(TypedSection, "clash")
m.pageaction = false
s.anonymous = true
s.addremove=false


local conf = "/usr/share/clash/config/upload/config.yaml"
sev = s:option(TextValue, "conf")
sev.readonly=true
sev.rows = 20
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or ""
end
sev.write = function(self, section, value)
end

o = s:option(Button,"configrm")
o.inputtitle = translate("Delete Config")
o.write = function()
  SYS.call("rm -rf /usr/share/clash/config/upload/config.yaml >/dev/null 2>&1 &")
end

return  m
