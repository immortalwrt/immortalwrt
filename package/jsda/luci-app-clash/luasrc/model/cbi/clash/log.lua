
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()


m = Map("clash")
s = m:section(TypedSection, "clash")
m.pageaction = false
s.anonymous = true
s.addremove=false

local clog = "/tmp/clash.log"
log = s:option(TextValue, "clog")
log.readonly=true
log.description = translate("")
log.rows = 29
log.wrap = "off"
log.cfgvalue = function(self, section)
	return NXFS.readfile(clog) or ""
end
log.write = function(self, section, value)
end

o = s:option(Button,"log")
o.inputtitle = translate("Clear Logs")
o.write = function()
  SYS.call('echo "" > /tmp/clash.log 2>&1 &')
  HTTP.redirect(DISP.build_url("admin", "services", "clash", "log"))
end

return m
