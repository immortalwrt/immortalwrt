local fs = require "nixio.fs"

local f = SimpleForm("minieap",
	translate("minieap LOG"),
	translate("Log file:/var/log/minieap.log"))

local o = f:field(Value, "minieap_log")

o.template = "cbi/tvalue"
o.rows = 32

function o.cfgvalue(self, section)
	return fs.readfile("/var/log/minieap.log")
end

function o.write(self, section, value)
	require("luci.sys").call('cat /dev/null > /var/log/minieap.log 2>/dev/null')
end

f.submit = translate("Clear log")
f.reset = false

return f
