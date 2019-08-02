
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()

ful = Form("upload", nil)
ful.reset = false
ful.submit = false


m = Map("clash")
s = m:section(TypedSection, "clash")
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
	NXFS.writefile(clog, value:gsub("\r\n", "\n"))
end

o = s:option(Button,"log")
o.inputtitle = translate("Clear Logs")
o.write = function()
  SYS.call('echo "" > /tmp/clash.log')
  HTTP.redirect(DISP.build_url("admin", "services", "clash", "log"))
end

return m, ful
