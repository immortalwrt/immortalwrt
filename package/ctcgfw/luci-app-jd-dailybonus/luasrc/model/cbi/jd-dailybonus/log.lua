local fs = require "nixio.fs"
local jd = "jd-dailybonus"
local conffile = "/www/JD_DailyBonus.htm"

s = SimpleForm("logview")
s.submit = false
s.reset = false

-- [[ 日志显示 ]]--
t = s:field(TextValue, "1", nil)
t.rmempty = true
t.rows = 30
function t.cfgvalue()
	return fs.readfile(conffile) or ""
end
t.readonly="readonly"

return s