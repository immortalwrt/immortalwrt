
local m
local sys = require "luci.sys"
local uci = luci.model.uci.cursor()

ful = Form("upload", nil)
ful.reset = false
ful.submit = false


m = Map("clash")
m:section(SimpleSection).template  = "clash/status"


return m, ful

